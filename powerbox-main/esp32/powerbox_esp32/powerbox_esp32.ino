/*
 * PowerBox ESP32 firmware
 * -----------------------
 * Serves the PowerBox web UI (powerbox-control.html + powerbox-control.js)
 * directly from flash and provides the HTTP API the UI expects:
 *
 *   GET  /                        -> web UI (PROGMEM)
 *   GET  /powerbox-control.js     -> client logic (PROGMEM)
 *   POST /api/chat                -> proxy to the OpenAI-compatible AI endpoint
 *   POST /api/settings            -> store Wi-Fi + AI settings in NVS
 *   GET  /api/wifi/scan           -> scan Wi-Fi networks as JSON
 *
 * Board : ESP32 Dev Module (classic ESP32, tested on ESP32-D0WD-V3)
 * Port  : COM5 (CP210x USB-UART) on this machine
 * FQBN  : esp32:esp32:esp32:PartitionScheme=huge_app
 *
 * First boot has no Wi-Fi credentials, so the board starts a setup hotspot:
 *   SSID     : PowerBox_AP
 *   Password : 12345678
 *   URL      : http://192.168.4.1
 *
 * Open the UI there, go to Settings -> Wi-Fi, enter your home network and save.
 * The board joins it (the setup AP stays available) and keeps the credentials
 * in NVS across reboots. mDNS: http://powerbox.local
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>

#include "powerbox_assets.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

namespace {

// Fixed AP name, matching the upstream repo so saved profiles work.
// The AP password is configurable (NVS) and defaults to the old value.
const char *AP_SSID_NAME = "PowerBox_AP";
const char *AP_DEFAULT_PASSWORD = "12345678";

const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_MASK(255, 255, 255, 0);
const IPAddress AP_LEASE(192, 168, 4, 2);

const uint8_t AP_MAX_CLIENTS = 8;
const unsigned long AP_RESTART_DELAY_MS = 2500;

// Relays (see PINOUT.txt). JQC3F-05VDC modules are ACTIVE-LOW and driven
// open-drain: LOW sinks to GND (relay energised), HIGH floats (released).
const int OUTPUT1_PIN = 27; // Output 1 (normal load / DC circuit)
const int OUTPUT2_PIN = 33; // Output 2 (master shut-off)
const bool OUTPUT_ACTIVE_LOW = true;
const bool OUTPUT1_ON_ENERGISES = true;
const bool OUTPUT2_ON_ENERGISES = false;  // master ON cuts power

const uint32_t SERIAL_BAUD = 115200;
const unsigned long STA_RETRY_MIN_MS = 5000;
const unsigned long STA_RETRY_MAX_MS = 60000;
const unsigned long AI_TIMEOUT_MS = 45000;

/* ── Analog sensors (see PINOUT.txt) ─────────────────────────────────── */
// All three inputs are on ADC1 (GPIO 32-39): ADC2 does not work while Wi-Fi
// is active, and this firmware is always on Wi-Fi.

const int AC_PIN = 34;   // ZMPT101B            (ADC1_CH6)
const int DC_PIN = 35;   // battery divider     (ADC1_CH7)
const int AMP_PIN = 32;  // ACS712              (ADC1_CH4)

// ---- CALIBRATION: tune these against a multimeter, then re-flash ----------
const float ZMPT_CALIBRATION = 696.0f; // AC scale: ADC RMS -> mains RMS
                                       // (tuned 220.9 V real / 0.3174 V raw)
const float DC_DIVIDER_RATIO = 5.00f;  // (R1 + R2) / R2
const float DC_OFFSET = 0.00f;         // small correction, in volts
const float ACS_SENSITIVITY = 0.185f;  // 5A=0.185, 20A=0.100, 30A=0.066 V/A
// Battery % for a 3S Li-ion pack (12.6 V full, 9.0 V BMS cutoff), the pack
// fitted to this box (12 V, 25 A BMS, 2500 mAh). Li-ion sits on a nearly flat
// 3.7-3.9 V/cell plateau, so a straight empty/full line reads far too high in
// the middle; the table below follows the resting-voltage curve instead.
// Under a heavy load the pack sags and the % drops early (no DC current
// sensor exists to compensate). Readings under 2 V are treated as a wiring
// fault (bat = -1), not as a 0 % battery.
const bool BATT_PERCENT_ENABLED = true;
const float BATT_EMPTY_V = 9.00f;
const float BATT_FULL_V = 12.60f;

struct BattPoint {
  float volts;
  uint8_t pct;
};
const BattPoint BATT_CURVE[] = {
    {BATT_FULL_V, 100}, {12.30f, 90}, {12.00f, 80}, {11.76f, 70},
    {11.55f, 60},       {11.40f, 50}, {11.25f, 40}, {11.10f, 30},
    {10.95f, 20},       {10.50f, 10}, {BATT_EMPTY_V, 0},
};
// -------------------------------------------------------------------------

const unsigned long SENSOR_INTERVAL_MS = 1000;
const unsigned long SENSOR_LOG_INTERVAL_MS = 2000;

WebServer server(80);
Preferences prefs;

struct Config {
  String ssid;
  String pass;
  String apiKey;
  String apiUrl;
  String apiModel;
  String apPass;
} cfg;

bool staAnnounced = false;
unsigned long lastWifiRetry = 0;
unsigned long staRetryDelay = STA_RETRY_MIN_MS;
bool everConnected = false;

// Logical output state: true = relay commanded on. Output 2 is inverted in
// hardware, so output2State == false means the master relay is energised and
// power flows. Boot defaults follow the upstream sketch: master ON, DC off.
bool output1State = false;
bool output2State = false;

// Internet sharing (AP clients -> STA uplink via lwIP NAPT)
bool naptActive = false;
IPAddress shareDns;
unsigned long apRestartAt = 0;
unsigned long lastShareTry = 0;

// Measured values (updated by sampleSensors())
float acVolts = 0.0f;
float dcVolts = 0.0f;
float amps = 0.0f;
int battPct = -1;              // -1 = not configured (unknown chemistry)
float acsZeroVolts = 1.65f;    // ACS712 zero point, calibrated at boot
bool sensorReadOk = false;

// Last raw ADC capture, exposed by /api/sensors for calibration.
struct AcCapture {
  uint16_t mvMin;
  uint16_t mvMax;
  uint16_t mvAvg;
  uint16_t samples;
};
AcCapture acDebug = {0, 0, 0, 0};

unsigned long lastSensorRead = 0;
unsigned long lastSensorLog = 0;

/* ── NVS ─────────────────────────────────────────────────────────────── */

void loadConfig() {
  prefs.begin("powerbox", true);
  cfg.ssid     = prefs.getString("ssid", "");
  cfg.pass     = prefs.getString("pass", "");
  cfg.apiKey   = prefs.getString("apiKey", DEFAULT_API_KEY);
  cfg.apiUrl   = prefs.getString("apiUrl", DEFAULT_API_URL);
  cfg.apiModel = prefs.getString("apiModel", DEFAULT_API_MODEL);
  cfg.apPass   = prefs.getString("apPass", AP_DEFAULT_PASSWORD);
  prefs.end();

  if (cfg.apiUrl.isEmpty())   cfg.apiUrl = DEFAULT_API_URL;
  if (cfg.apiModel.isEmpty()) cfg.apiModel = DEFAULT_API_MODEL;
  if (cfg.apPass.length() < 8) cfg.apPass = AP_DEFAULT_PASSWORD;

  Serial.println("[cfg] loaded from NVS");
}

void saveConfig() {
  prefs.begin("powerbox", false);
  prefs.putString("ssid", cfg.ssid);
  prefs.putString("pass", cfg.pass);
  prefs.putString("apiKey", cfg.apiKey);
  prefs.putString("apiUrl", cfg.apiUrl);
  prefs.putString("apiModel", cfg.apiModel);
  prefs.putString("apPass", cfg.apPass);
  prefs.end();
  Serial.println("[cfg] saved to NVS");
}

/* ── Minimal JSON helpers (no external library needed) ───────────────── */

// Reads a flat string value: {"key": "value"}. Good enough for the settings
// payload the UI sends; escapes are decoded, \uXXXX is not needed for SSIDs.
bool jsonReadString(const String &src, const char *key, String &out) {
  String needle = String("\"") + key + "\"";
  int k = src.indexOf(needle);
  if (k < 0) return false;

  int colon = src.indexOf(':', k + needle.length());
  if (colon < 0) return false;

  int i = colon + 1;
  while (i < (int)src.length() && isspace((unsigned char)src[i])) i++;
  if (i >= (int)src.length() || src[i] != '"') return false;
  i++;

  String val;
  while (i < (int)src.length()) {
    char c = src[i];
    if (c == '\\') {
      i++;
      if (i >= (int)src.length()) break;
      switch (src[i]) {
        case 'n':  val += '\n'; break;
        case 'r':  val += '\r'; break;
        case 't':  val += '\t'; break;
        case '"':  val += '"';  break;
        case '\\': val += '\\'; break;
        case '/':  val += '/';  break;
        default:   val += src[i]; break;
      }
      i++;
    } else if (c == '"') {
      break;
    } else {
      val += c;
      i++;
    }
  }

  out = val;
  return true;
}

// Reads a flat boolean value: {"key": true}. Used for UI toggles.
bool jsonReadBool(const String &src, const char *key, bool &out) {
  String needle = String("\"") + key + "\"";
  int k = src.indexOf(needle);
  if (k < 0) return false;

  int colon = src.indexOf(':', k + needle.length());
  if (colon < 0) return false;

  int i = colon + 1;
  while (i < (int)src.length() && isspace((unsigned char)src[i])) i++;
  if (src.startsWith("true", i))  { out = true;  return true; }
  if (src.startsWith("false", i)) { out = false; return true; }
  return false;
}

String jsonEscape(const String &in) {
  String out;
  out.reserve(in.length() + 8);
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    switch (c) {
      case '"':  out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n";  break;
      case '\r': out += "\\r";  break;
      case '\t': out += "\\t";  break;
      default:
        if ((unsigned char)c < 0x20) {
          char buf[7];
          snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
          out += buf;
        } else {
          out += c;  // UTF-8 bytes pass through untouched
        }
    }
  }
  return out;
}

void sendJson(int code, const String &body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json; charset=utf-8", body);
}

// Send a large PROGMEM buffer in small pieces. A single 40-60 KB write can be
// truncated on some clients / weak links, which leaves the browser with a
// half-received page (dark background, no content).
void sendProgmemPage(int code, const char *contentType,
                     const char *data, size_t len) {
  server.client().setNoDelay(true);
  server.setContentLength(len);
  server.send(code, contentType, "");

  const size_t CHUNK = 1024;
  for (size_t offset = 0; offset < len; offset += CHUNK) {
    size_t n = (len - offset > CHUNK) ? CHUNK : (len - offset);
    server.sendContent_P(data + offset, n);
  }
}

/* ── Relay outputs ───────────────────────────────────────────────────── */

// Drive one relay pin given whether its coil should be energised. With the
// active-low module the pin is open-drain, so HIGH means "release" (high-Z).
void driveRelay(int pin, bool energised) {
  if (OUTPUT_ACTIVE_LOW) {
    digitalWrite(pin, energised ? LOW : HIGH);
  } else {
    digitalWrite(pin, energised ? HIGH : LOW);
  }
}

// num 1 = Output 1 (DC circuit), num 2 = Output 2 (master shut-off).
// `on` is the LOGICAL state used by the UI: for output 2, on = power flows.
void setOutput(int num, bool on) {
  if (num == 1) {
    output1State = on;
    bool energised = OUTPUT1_ON_ENERGISES ? on : !on;
    driveRelay(OUTPUT1_PIN, energised);
    Serial.printf("[power] out1 %s -> GPIO%d %s\n", on ? "ON" : "OFF",
                  OUTPUT1_PIN, energised ? "LOW" : "HIGH");
  } else if (num == 2) {
    output2State = on;
    // Master is inverted in hardware: logical ON releases the coil and the
    // shunt CUTS power; the coil stays energised while power FLOWS.
    bool energised = OUTPUT2_ON_ENERGISES ? on : !on;
    driveRelay(OUTPUT2_PIN, energised);
    Serial.printf("[power] out2 %s (power %s) -> GPIO%d %s\n",
                  on ? "ON" : "OFF", on ? "CUT" : "LIVE",
                  OUTPUT2_PIN, energised ? "LOW" : "HIGH");
  }
}

// Apply both relays from the UI's point of view (master ON = power flows).
void applyPower(bool masterOn, bool dcOn) {
  setOutput(2, !masterOn);  // master ON  -> Output 2 logical off -> power live
  setOutput(1, dcOn);
}

/* ── Analog sensors ──────────────────────────────────────────────────── */

// Raw pin readings kept for /api/sensors (calibration aid).
float lastDcMv = 0.0f;
float lastAmpMv = 0.0f;

// AC RMS from the ZMPT101B. The signal is a sine riding on a DC bias
// (~VCC/2), so like the current channel the RMS is taken around the batch's
// own mean: immune to the 5 V rail sagging during Wi-Fi bursts, and far
// less jittery than the old min/max peak method, where single noise spikes
// stretch the envelope. acDebug still reports min/max for the pot-tuning
// workflow (watch ac_pp_mv while turning the pot).
// One batch of raw ADC RMS on the AC pin (~42 ms window).
float readAcBatch(uint16_t &mn, uint16_t &mx, float &meanOut) {
  float sumSq = 0.0f, sumV = 0.0f;
  // analogReadMilliVolts costs ~42 us, so 1000 samples fill ~42 ms, about
  // 2.1 mains cycles at 50 Hz. A shorter window truncates the sine and the
  // RMS jumps around with the sampling phase (seen as +-15% jitter at 400).
  const int samples = 1000;

  mn = 5000; mx = 0;
  for (int i = 0; i < samples; i++) {
    uint16_t v = (uint16_t)analogReadMilliVolts(AC_PIN);
    float fv = v / 1000.0f;
    sumSq += fv * fv;
    sumV += fv;
    if (v < mn) mn = v;
    if (v > mx) mx = v;
  }

  meanOut = sumV / samples;
  float variance = (sumSq / samples) - (meanOut * meanOut);
  if (variance < 0.0f) variance = 0.0f;
  return sqrtf(variance);
}

float readAcVolts() {
  // Median of 3 batches: a Wi-Fi burst can inflate one batch's variance,
  // and the median throws that batch away instead of averaging it in.
  float rms[3];
  uint16_t mn, mx, mnAll = 5000, mxAll = 0;
  float mean, meanSum = 0.0f;
  for (int b = 0; b < 3; b++) {
    rms[b] = readAcBatch(mn, mx, mean);
    meanSum += mean;
    if (mn < mnAll) mnAll = mn;
    if (mx > mxAll) mxAll = mx;
  }

  acDebug.mvMin = mnAll;
  acDebug.mvMax = mxAll;
  acDebug.mvAvg = (uint16_t)((meanSum / 3.0f) * 1000.0f);
  acDebug.samples = 3000;

  if (rms[0] > rms[1]) { float t = rms[0]; rms[0] = rms[1]; rms[1] = t; }
  if (rms[1] > rms[2]) { float t = rms[1]; rms[1] = rms[2]; rms[2] = t; }
  if (rms[0] > rms[1]) { float t = rms[0]; rms[0] = rms[1]; rms[1] = t; }
  float mid = rms[1];

  if (mid < 0.05f) return 0.0f;  // noise floor when the primary sees no mains
  return mid * ZMPT_CALIBRATION;
}

// Battery voltage through the resistor divider.
float readDcVolts() {
  uint32_t sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogReadMilliVolts(DC_PIN);
  }
  lastDcMv = sum / 20.0f;
  return (lastDcMv / 1000.0f) * DC_DIVIDER_RATIO + DC_OFFSET;
}

// AC current from the ACS712. RMS is taken around the batch's own mean, not
// the stored zero: mains current is pure AC, so the mean of the output IS
// the zero point. This makes the reading immune to the 5 V rail sagging
// during Wi-Fi bursts (the ratiometric zero moves with VCC).
float readAmps() {
  float sumSq = 0.0f;
  float sumV = 0.0f;
  const int samples = 1000;  // ~42 ms, about 2.1 mains cycles (see readAcVolts)

  for (int i = 0; i < samples; i++) {
    float v = analogReadMilliVolts(AMP_PIN) / 1000.0f;
    sumSq += v * v;
    sumV += v;
  }
  float mean = sumV / samples;
  lastAmpMv = mean * 1000.0f;

  // variance = E[x^2] - E[x]^2  (i.e. RMS^2 around the batch mean)
  float variance = (sumSq / samples) - (mean * mean);
  if (variance < 0.0f) variance = 0.0f;   // float rounding guard
  float irms = sqrtf(variance) / ACS_SENSITIVITY;
  if (irms < 0.05f) irms = 0.0f;   // dead zone near zero
  return irms;
}

// 0-100 % from the pack voltage, or -1 while the chemistry is unknown /
// the divider is disconnected (a real 12 V pack never reads under 2 V).
// BATT_CURVE is sorted high -> low; interpolate the segment containing the
// reading so the flat middle of the Li-ion discharge curve is respected.
int batteryPercent(float volts) {
  if (!BATT_PERCENT_ENABLED) return -1;
  if (volts < 2.0f) return -1;   // wiring fault -> UI shows the raw voltage

  const int n = (int)(sizeof(BATT_CURVE) / sizeof(BATT_CURVE[0]));
  if (volts >= BATT_CURVE[0].volts) return 100;
  if (volts <= BATT_CURVE[n - 1].volts) return 0;

  for (int i = 0; i < n - 1; i++) {
    float hi = BATT_CURVE[i].volts;
    float lo = BATT_CURVE[i + 1].volts;
    if (volts <= hi && volts >= lo) {
      float t = (volts - lo) / (hi - lo);
      float pct = BATT_CURVE[i + 1].pct +
                  t * (BATT_CURVE[i].pct - BATT_CURVE[i + 1].pct);
      return (int)(pct + 0.5f);
    }
  }
  return 0;
}

// ACS712 zero point. Must run with no load current: called before the master
// relay is energised, so the box is still unpowered for these ~400 ms.
void calibrateCurrentZero() {
  uint32_t sum = 0;
  const int samples = 200;
  for (int i = 0; i < samples; i++) {
    sum += analogReadMilliVolts(AMP_PIN);
    delay(2);
  }
  acsZeroVolts = (sum / (float)samples) / 1000.0f;
  Serial.printf("[adc] ACS712 zero = %.3f V\n", acsZeroVolts);
}

void sampleSensors() {
  float ac = readAcVolts();
  // Light smoothing on the AC channel: the UI polls every 5 s, so a raw
  // +-2% ADC/mains jitter would look jumpy. Half-and-half converges within
  // ~3 s while staying responsive; jump-start from zero after boot.
  acVolts = (acVolts < 1.0f && ac > 0.0f) ? ac : (acVolts + ac) * 0.5f;
  dcVolts = readDcVolts();
  amps = readAmps();
  battPct = batteryPercent(dcVolts);
  sensorReadOk = true;
}

/* ── Internet sharing (AP clients get the STA uplink) ────────────────── */

// DNS that AP clients should use: the uplink's DNS, else its gateway, else a
// public resolver. Without this the DHCP server would advertise the ESP32
// itself (192.168.4.1), which has no DNS service.
IPAddress upstreamDns() {
  IPAddress dns = WiFi.STA.dnsIP(0);
  if ((uint32_t)dns != 0) return dns;
  IPAddress gw = WiFi.gatewayIP();
  if ((uint32_t)gw != 0) return gw;
  return IPAddress(8, 8, 8, 8);
}

// Turn lwIP NAPT on/off to follow the uplink state. Sharing is always on —
// it is no longer a user setting, so there is no way to disable it by mistake.
// Called from loop(); only does work when something actually changed.
void applyShareState() {
  bool want = (WiFi.status() == WL_CONNECTED);

  if (want) {
    IPAddress dns = upstreamDns();
    if (!naptActive || dns != shareDns) {
      // Retry at most every 3 s so a persistent failure cannot hammer the AP.
      if (lastShareTry != 0 && (millis() - lastShareTry) < 3000) return;
      lastShareTry = millis();
      // Re-apply the DHCP DNS option, then enable NAPT on the AP interface.
      WiFi.softAPConfig(AP_IP, AP_IP, AP_MASK, AP_LEASE, dns);
      bool ok = WiFi.AP.enableNAPT(true);
      naptActive = ok;
      shareDns = dns;
      Serial.printf("[share] %s (dns %s, clients %u)\n",
                    ok ? "ON" : "FAILED", dns.toString().c_str(),
                    (unsigned)WiFi.softAPgetStationNum());
    }
  } else if (naptActive) {
    WiFi.AP.enableNAPT(false);
    naptActive = false;
    Serial.println("[share] OFF");
  }
}

/* ── HTTP handlers ───────────────────────────────────────────────────── */

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  sendProgmemPage(200, "text/html; charset=utf-8",
                  POWERBOX_HTML, sizeof(POWERBOX_HTML) - 1);
}

void handleJs() {
  server.sendHeader("Cache-Control", "no-store");
  sendProgmemPage(200, "application/javascript; charset=utf-8",
                  POWERBOX_JS, sizeof(POWERBOX_JS) - 1);
}

// Minimal page used to tell "server unreachable" apart from "big page cut off".
void handleTest() {
  server.send(200, "text/html; charset=utf-8",
              "<!doctype html><meta name=viewport content=\"width=device-width\">"
              "<body style=\"background:#151515;color:#eee;font:16px sans-serif;padding:24px\">"
              "<h1 style=\"color:#ff5a4a\">PowerBox OK</h1>"
              "<p>The ESP32 web server is reachable.</p>"
              "<p><a style=\"color:#ff8a7a\" href=\"/\">Open the app</a></p>"
              "</body>");
}

// Small diagnostic endpoint: confirms the web server is reachable and shows
// whether the upstream link is up. Open http://192.168.4.1/api/status.
void handleStatus() {
  bool staUp = (WiFi.status() == WL_CONNECTED);
  char buf[640];
  snprintf(buf, sizeof(buf),
           "{\"ap\":\"%s\",\"ap_ip\":\"%s\",\"sta_up\":%s,"
           "\"sta_ssid\":\"%s\",\"sta_ip\":\"%s\",\"rssi\":%d,"
           "\"configured\":%s,\"target_ssid\":\"%s\",\"heap\":%u,"
           "\"master\":%s,\"dc\":%s,"
           "\"ac_v\":%.1f,\"dc_v\":%.2f,\"amp\":%.2f,\"bat\":%d,"
           "\"napt\":%s,\"clients\":%u,\"upstream_dns\":\"%s\"}",
           AP_SSID_NAME,
           WiFi.softAPIP().toString().c_str(),
           staUp ? "true" : "false",
           staUp ? jsonEscape(WiFi.SSID()).c_str() : "",
           staUp ? WiFi.localIP().toString().c_str() : "",
           staUp ? (int)WiFi.RSSI() : 0,
           cfg.ssid.isEmpty() ? "false" : "true",
           jsonEscape(cfg.ssid).c_str(),
           (unsigned)ESP.getFreeHeap(),
           output2State ? "false" : "true",   // master ON = Output 2 released
           output1State ? "true" : "false",
           acVolts, dcVolts, amps, battPct,
           naptActive ? "true" : "false",
           (unsigned)WiFi.softAPgetStationNum(),
           naptActive ? shareDns.toString().c_str() : "");
  sendJson(200, buf);
}

// GET /api/sensors -> raw ADC values + calibration factors. Open this from a
// phone while tuning ZMPT_CALIBRATION / DC_DIVIDER_RATIO / ACS_SENSITIVITY.
void handleSensors() {
  char buf[640];
  snprintf(buf, sizeof(buf),
           "{\"ok\":true,\"ac_v\":%.2f,\"dc_v\":%.2f,\"amp\":%.2f,\"bat\":%d,"
           "\"raw\":{\"ac_min_mv\":%u,\"ac_max_mv\":%u,\"ac_avg_mv\":%u,"
           "\"ac_pp_mv\":%u,\"ac_samples\":%u,\"dc_mv\":%.0f,\"amp_mv\":%.0f},"
           "\"cal\":{\"ac_scale\":%.3f,\"dc_ratio\":%.3f,\"dc_offset\":%.3f,"
           "\"acs_sensitivity\":%.3f,\"acs_zero_v\":%.3f}}",
           acVolts, dcVolts, amps, battPct,
           (unsigned)acDebug.mvMin, (unsigned)acDebug.mvMax,
           (unsigned)acDebug.mvAvg,
           (unsigned)(acDebug.mvMax >= acDebug.mvMin ? acDebug.mvMax - acDebug.mvMin : 0),
           (unsigned)acDebug.samples, lastDcMv, lastAmpMv,
           ZMPT_CALIBRATION, DC_DIVIDER_RATIO, DC_OFFSET,
           ACS_SENSITIVITY, acsZeroVolts);
  sendJson(200, buf);
}

// POST /api/sensors/recal -> re-measure the ACS712 zero point. Only valid
// while no load current flows (same rule as the boot calibration): switch
// the loads off first, or use it right after wiring the sensor.
void handleSensorsRecal() {
  calibrateCurrentZero();
  char buf[96];
  snprintf(buf, sizeof(buf), "{\"ok\":true,\"acs_zero_v\":%.3f}", acsZeroVolts);
  sendJson(200, buf);
}

// GET /api/power -> current relay state; POST -> set master and/or dc.
void handlePowerGet() {
  char buf[160];
  snprintf(buf, sizeof(buf),
           "{\"ok\":true,\"master\":%s,\"dc\":%s,\"out1\":%s,\"out2\":%s}",
           output2State ? "false" : "true",
           output1State ? "true" : "false",
           output1State ? "true" : "false",
           output2State ? "true" : "false");
  sendJson(200, buf);
}

void handlePowerSet() {
  if (!server.hasArg("plain")) {
    sendJson(400, "{\"ok\":false,\"error\":\"empty request body\"}");
    return;
  }
  String body = server.arg("plain");

  bool master = (output2State == false);  // current logical master state
  bool dc = output1State;
  bool wantMaster, wantDc;
  bool hasMaster = jsonReadBool(body, "master", wantMaster);
  bool hasDc = jsonReadBool(body, "dc", wantDc);

  if (!hasMaster && !hasDc) {
    sendJson(400, "{\"ok\":false,\"error\":\"expected master and/or dc\"}");
    return;
  }
  if (hasMaster) master = wantMaster;
  if (hasDc) dc = wantDc;

  applyPower(master, dc);

  char buf[160];
  snprintf(buf, sizeof(buf),
           "{\"ok\":true,\"master\":%s,\"dc\":%s,\"out1\":%s,\"out2\":%s}",
           master ? "true" : "false",
           dc ? "true" : "false",
           output1State ? "true" : "false",
           output2State ? "true" : "false");
  sendJson(200, buf);
}

// Captive-portal probes: send phones/laptops straight to the web app.
void redirectToPortal() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain; charset=utf-8", "");
}

const char *authName(wifi_auth_mode_t mode) {
  switch (mode) {
    case WIFI_AUTH_OPEN:            return "OPEN";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENT";
    case WIFI_AUTH_WPA3_PSK:        return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/WPA3";
    default:                        return "UNKNOWN";
  }
}

void handleWifiScan() {
  WiFi.scanDelete();
  int found = WiFi.scanNetworks();
  if (found < 0) {
    // บอกความล้มเหลวให้ UI รู้ ไม่ใช่ส่ง [] แล้วให้ UI เดา
    sendJson(500, "{\"error\":\"wifi scan failed\"}");
    return;
  }

  String json = "[";
  bool first = true;
  for (int i = 0; i < found; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.isEmpty()) continue;  // hidden network
    if (!first) json += ",";
    first = false;

    json += "{\"ssid\":\"" + jsonEscape(ssid) + "\"";
    json += ",\"rssi\":" + String(WiFi.RSSI(i));
    json += ",\"sec\":\"" + String(authName(WiFi.encryptionType(i))) + "\"}";
  }
  json += "]";

  WiFi.scanDelete();
  sendJson(200, json);
}

void handleChat() {
  if (!server.hasArg("plain") || server.arg("plain").isEmpty()) {
    sendJson(400, "{\"error\":{\"message\":\"empty request body\"}}");
    return;
  }
  if (cfg.apiKey.isEmpty()) {
    sendJson(500, "{\"error\":{\"message\":\"NO_API_KEY: open Settings and save an API key\"}}");
    return;
  }
  if (!cfg.apiUrl.startsWith("http")) {
    sendJson(500, "{\"error\":{\"message\":\"API base URL must start with http/https\"}}");
    return;
  }

  HTTPClient http;
  WiFiClientSecure tls;
  tls.setInsecure();  // prototype: no certificate pinning

  http.setConnectTimeout(15000);
  http.setTimeout(AI_TIMEOUT_MS);
  http.setReuse(false);

  if (!http.begin(tls, cfg.apiUrl)) {
    sendJson(502, "{\"error\":{\"message\":\"cannot open AI endpoint\"}}");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.addHeader("Authorization", "Bearer " + cfg.apiKey);

  Serial.printf("[ai] POST %s\n", cfg.apiUrl.c_str());
  int code = http.POST(server.arg("plain"));
  String payload = http.getString();
  http.end();

  Serial.printf("[ai] response %d (%u bytes)\n", code, (unsigned)payload.length());

  if (code <= 0) {
    sendJson(502, "{\"error\":{\"message\":\"AI request failed (network/timeout)\"}}");
    return;
  }
  if (payload.isEmpty()) {
    sendJson(502, "{\"error\":{\"message\":\"empty AI response\"}}");
    return;
  }

  sendJson(code, payload);
}

void handleSettings() {
  if (!server.hasArg("plain")) {
    sendJson(400, "{\"ok\":false,\"error\":\"empty request body\"}");
    return;
  }
  String body = server.arg("plain");

  String newSsid, newPass, newKey, newUrl, newModel, newApPass;
  bool hasSsid = false, hasApPass = false;
  bool wifiChanged = false, apChanged = false;

  hasSsid = jsonReadString(body, "ssid", newSsid) || jsonReadString(body, "wifiSsid", newSsid);
  if (hasSsid) newSsid.trim();
  if (hasSsid && newSsid.isEmpty()) {
    // ห้ามเขียนทับ credentials เดิมด้วย SSID ว่าง
    sendJson(400, "{\"ok\":false,\"error\":\"SSID must not be empty\"}");
    return;
  }

  hasApPass = jsonReadString(body, "apPassword", newApPass);
  if (hasApPass && !newApPass.isEmpty() &&
      (newApPass.length() < 8 || newApPass.length() > 63)) {
    sendJson(400, "{\"ok\":false,\"error\":\"AP password must be 8-63 characters\"}");
    return;
  }

  if (hasSsid) {
    if (newSsid != cfg.ssid) wifiChanged = true;
    cfg.ssid = newSsid;
  }
  if (jsonReadString(body, "password", newPass) || jsonReadString(body, "wifiPass", newPass)) {
    if (newPass != cfg.pass) wifiChanged = true;
    cfg.pass = newPass;
  }
  if (hasApPass && !newApPass.isEmpty() && newApPass != cfg.apPass) {
    cfg.apPass = newApPass;
    apChanged = true;
  }

  if (jsonReadString(body, "API_KEY", newKey) || jsonReadString(body, "apiKey", newKey)) {
    newKey.trim();
    cfg.apiKey = newKey;
  }
  if (jsonReadString(body, "AI_BASE_URL", newUrl) || jsonReadString(body, "apiUrl", newUrl)) {
    newUrl.trim();
    if (!newUrl.isEmpty()) cfg.apiUrl = newUrl;
  }
  if (jsonReadString(body, "AI_MODEL", newModel) || jsonReadString(body, "apiModel", newModel)) {
    newModel.trim();
    if (!newModel.isEmpty()) cfg.apiModel = newModel;
  }

  saveConfig();

  // ต่อใหม่เมื่อค่าเปลี่ยน หรือเมื่อยังไม่ต่อติด (เช่น กดบันทึกซ้ำหลังเปิด router)
  bool shouldJoin = wifiChanged || (WiFi.status() != WL_CONNECTED);
  if (shouldJoin && !cfg.ssid.isEmpty()) {
    Serial.printf("[wifi] joining \"%s\"\n", cfg.ssid.c_str());
    WiFi.disconnect();  // ตัดการเชื่อมต่อเดิมก่อนสลับเครือข่าย
    WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
    staAnnounced = false;
    lastWifiRetry = millis();
    staRetryDelay = STA_RETRY_MIN_MS;
  }

  // Changing the AP password drops every AP client, so restart the AP a
  // little later — after this HTTP response has been sent.
  if (apChanged) {
    apRestartAt = millis() + AP_RESTART_DELAY_MS;
  }

  char resp[96];
  snprintf(resp, sizeof(resp),
           "{\"ok\":true,\"wifi_changed\":%s,\"ap_restart\":%s}",
           wifiChanged ? "true" : "false",
           apChanged ? "true" : "false");
  sendJson(200, resp);
}

void handleNotFound() {
  // The UI requests the JS with "?v=5"; be tolerant of the query string.
  if (server.uri().startsWith("/powerbox-control.js")) {
    handleJs();
    return;
  }
  server.sendHeader("Cache-Control", "no-store");
  server.send(404, "text/plain; charset=utf-8", "Not found");
}

/* ── Wi-Fi status helpers ────────────────────────────────────────────── */

void printNetworkInfo() {
  Serial.println();
  Serial.println("--------- PowerBox network ---------");
  Serial.printf("AP SSID   : %s\n", WiFi.softAPSSID().c_str());
  Serial.printf("AP pass   : %s\n", cfg.apPass.isEmpty() ? "(none)" : "(set, 8+ chars)");
  Serial.printf("AP URL    : http://%s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("Share net : %s\n", naptActive ? "ON (NAPT)" : "auto - starts when the uplink is up");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Home Wi-Fi: %s\n", WiFi.SSID().c_str());
    Serial.printf("STA IP    : http://%s\n", WiFi.localIP().toString().c_str());
    Serial.println("mDNS      : http://powerbox.local");
  } else {
    Serial.println("Home Wi-Fi: not connected (setup AP only)");
  }
  Serial.println("------------------------------------");
  Serial.println();
}

// Wi-Fi events: makes AP/client problems visible on the serial log.
void onWifiEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.printf("[wifi] AP started: \"%s\"\n", WiFi.softAPSSID().c_str());
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.printf("[wifi] AP client joined, IP %s\n",
                    IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("[wifi] AP client left");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      staRetryDelay = STA_RETRY_MIN_MS;
      Serial.printf("[wifi] STA got IP %s\n", WiFi.localIP().toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.printf("[wifi] STA disconnected (reason %d)\n",
                    (int)info.wifi_sta_disconnected.reason);
      break;
    default:
      break;
  }
}

void beginWifi() {
  WiFi.persistent(false);
  WiFi.setSleep(false);
  WiFi.mode(WIFI_AP_STA);

  // Explicit AP addressing + DHCP pool (same as the upstream repo), so clients
  // always get an address instead of falling back to 169.254.x.x.
  if (!WiFi.softAPConfig(AP_IP, AP_IP, AP_MASK, AP_LEASE)) {
    Serial.println("[wifi] softAPConfig failed");
  }

  bool apOk = WiFi.softAP(AP_SSID_NAME, cfg.apPass.c_str(), 1, 0, AP_MAX_CLIENTS);
  Serial.printf("[wifi] softAP \"%s\" %s, IP %s\n",
                AP_SSID_NAME, apOk ? "OK" : "FAILED",
                WiFi.softAPIP().toString().c_str());

  if (!cfg.ssid.isEmpty()) {
    Serial.printf("[wifi] connecting to \"%s\"\n", cfg.ssid.c_str());
    WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
  } else {
    Serial.println("[wifi] no saved credentials - setup AP only");
  }
}

}  // namespace

/* ── Arduino lifecycle ───────────────────────────────────────────────── */

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);
  Serial.println();
  Serial.println("PowerBox ESP32 booting...");

  // Relays first: pre-load HIGH before switching to open-drain so the pin
  // never glitches LOW (which would energise an active-low relay). In this
  // init window both coils are released, so the master is CUT and no load
  // current flows while the ACS712 zero is measured below. applyPower(true,
  // false) right after energises Output 2 -> master LIVE, DC off (upstream
  // boot state).
  digitalWrite(OUTPUT1_PIN, HIGH);
  digitalWrite(OUTPUT2_PIN, HIGH);
  if (OUTPUT_ACTIVE_LOW) {
    pinMode(OUTPUT1_PIN, OUTPUT_OPEN_DRAIN);
    pinMode(OUTPUT2_PIN, OUTPUT_OPEN_DRAIN);
  } else {
    pinMode(OUTPUT1_PIN, OUTPUT);
    pinMode(OUTPUT2_PIN, OUTPUT);
  }

  // ADC setup before anything is read. All three pins are ADC1, which keeps
  // working while Wi-Fi is on.
  analogReadResolution(12);
  analogSetPinAttenuation(AC_PIN, ADC_11db);
  analogSetPinAttenuation(DC_PIN, ADC_11db);
  analogSetPinAttenuation(AMP_PIN, ADC_11db);

  // ACS712 zero point. The master relay is still released here (power cut),
  // so no load current flows while we measure the offset.
  calibrateCurrentZero();

  applyPower(true, false);
  Serial.printf("[power] boot master=ON dc=OFF (out1=%d out2=%d)\n",
                output1State ? 1 : 0, output2State ? 1 : 0);

  sampleSensors();
  Serial.printf("[adc] first: ac=%.1fV dc=%.2fV amp=%.2fA\n",
                acVolts, dcVolts, amps);

  loadConfig();
  beginWifi();
  WiFi.onEvent(onWifiEvent);

  if (MDNS.begin("powerbox")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("[mdns] http://powerbox.local");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/test", HTTP_GET, handleTest);
  server.on("/powerbox-control.js", HTTP_GET, handleJs);
  server.on("/api/chat", HTTP_POST, handleChat);
  server.on("/api/settings", HTTP_POST, handleSettings);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/sensors", HTTP_GET, handleSensors);
  server.on("/api/sensors/recal", HTTP_POST, handleSensorsRecal);
  // GET too, so the calibration can be triggered from a phone browser.
  server.on("/api/sensors/recal", HTTP_GET, handleSensorsRecal);
  server.on("/api/power", HTTP_GET, handlePowerGet);
  server.on("/api/power", HTTP_POST, handlePowerSet);
  server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  server.on("/favicon.ico", HTTP_GET, []() { server.send(204); });

  // Captive-portal probes used by Android / iOS / Windows.
  server.on("/generate_204", HTTP_GET, redirectToPortal);
  server.on("/gen_204", HTTP_GET, redirectToPortal);
  server.on("/hotspot-detect.html", HTTP_GET, redirectToPortal);
  server.on("/connecttest.txt", HTTP_GET, redirectToPortal);
  server.on("/ncsi.txt", HTTP_GET, redirectToPortal);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[http] listening on port 80");
  printNetworkInfo();
}

void loop() {
  server.handleClient();

  unsigned long now = millis();
  bool connected = (WiFi.status() == WL_CONNECTED);

  // Apply a pending AP password change after the HTTP response was flushed.
  if (apRestartAt != 0 && (long)(now - apRestartAt) >= 0) {
    apRestartAt = 0;
    Serial.println("[wifi] applying new AP password");
    WiFi.softAP(AP_SSID_NAME, cfg.apPass.c_str(), 1, 0, AP_MAX_CLIENTS);
    naptActive = false;  // re-apply sharing on the next tick
  }

  if (connected && !staAnnounced) {
    staAnnounced = true;
    everConnected = true;
    Serial.printf("[wifi] connected to \"%s\" - http://%s\n",
                  WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    printNetworkInfo();
  }

  if (!connected && staAnnounced) {
    staAnnounced = false;
    Serial.println("[wifi] connection lost");
  }

  // Enable/disable internet sharing (NAPT) to match config + uplink state.
  applyShareState();

  // Sample the analog inputs once per second (~40-80 ms per pass).
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;
    sampleSensors();
  }

  // Periodic serial line so the readings can be compared with a multimeter.
  if (now - lastSensorLog >= SENSOR_LOG_INTERVAL_MS) {
    lastSensorLog = now;
    Serial.printf("[adc] ac=%.3fV (pp %umV) dc=%.2fV (%.0fmV) amp=%.2fA bat=%d%%\n",
                  acVolts, (unsigned)(acDebug.mvMax >= acDebug.mvMin ? acDebug.mvMax - acDebug.mvMin : 0),
                  dcVolts, lastDcMv, amps, battPct);
  }

  // Keep trying saved credentials with exponential backoff. Each attempt scans
  // all channels on the single radio, which briefly starves the AP, so we space
  // them out instead of retrying every few seconds.
  if (!connected && !cfg.ssid.isEmpty() && now - lastWifiRetry > staRetryDelay) {
    lastWifiRetry = now;
    Serial.printf("[wifi] retrying \"%s\" (backoff %lus)\n",
                  cfg.ssid.c_str(), staRetryDelay / 1000UL);
    WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
    staRetryDelay = min(staRetryDelay * 2UL, STA_RETRY_MAX_MS);
  }
}
