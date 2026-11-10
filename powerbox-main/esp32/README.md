# PowerBox — ESP32 firmware

Firmware that serves the PowerBox web UI straight from the ESP32 and exposes the
HTTP API the UI expects. No Node.js server, no LittleFS upload step.

## Detected hardware (this machine)

| Item | Value |
|---|---|
| Chip | ESP32-D0WD-V3 (rev v3.1), 240 MHz, dual core |
| USB bridge | Silicon Labs CP210x (`VID_10C4&PID_EA60`) |
| Port | **COM5** |
| MAC | `1c:c3:ab:b3:da:e8` |
| FQBN | `esp32:esp32:esp32:PartitionScheme=huge_app` |
| Arduino core | esp32 **3.3.10** |

## First-time use (no Wi-Fi configured)

The board starts its own setup hotspot:

- **SSID:** `PowerBox_AP`
- **Password:** `12345678`
- **URL:** <http://192.168.4.1>

1. Connect your phone/laptop to `PowerBox_AP`.
2. Open <http://192.168.4.1>.
3. Tap **ตั้งค่า (Settings) → ตั้งค่า Wi-Fi**, enter your home Wi-Fi SSID and password, then **บันทึกการตั้งค่า**.

The board joins your network and keeps the credentials in NVS. The setup AP
stays up, so you can always reach it at `192.168.4.1`. Once joined, the UI is
also available at the board's LAN IP and at <http://powerbox.local>.

> If a serial log says `connecting to "<old SSID>"`, that network was left in
> NVS by a previous firmware. Either save new Wi-Fi credentials via the setup
> AP, or wipe it: `esptool.exe -p COM5 -b 115200 erase_flash`, then re-upload.

> Snapshots of the old `PowerBox-C31C` AP are gone; the name is now fixed as
> `PowerBox_AP` (same as the upstream git repo). If your device remembers an
> old password for that name, choose **Forget network** and reconnect with
> `12345678`.

## Files

| File | Purpose |
|---|---|
| `powerbox_esp32.ino` | Firmware: Wi-Fi, HTTP server, AI proxy, settings, scan |
| `powerbox_assets.h` | **Auto-generated** — HTML + JS embedded as PROGMEM literals |
| `secrets.h` | Real default API settings (git-ignored) |
| `secrets.example.h` | Template committed to the repo |
| `../tools/embed_assets.py` | Regenerates `powerbox_assets.h` from the web files |

## Endpoints

| Method | Path | Description |
|---|---|---|
| GET | `/` | Web UI |
| GET | `/powerbox-control.js` | Client logic (handles `?v=` query) |
| POST | `/api/chat` | Proxies the request body to `AI_BASE_URL` with `Authorization: Bearer <key>` |
| POST | `/api/settings` | Stores Wi-Fi + AI settings in NVS and joins the Wi-Fi. Rejects an empty SSID with HTTP 400 so saved credentials are never wiped. Also accepts `apPassword` (8–63 chars; blank = keep). Returns `{"ok":true,"wifi_changed":<bool>,"ap_restart":<bool>}` |
| GET | `/api/status` | Real state: `sta_up`, `sta_ssid` (only when connected), `sta_ip`, `rssi`, `configured`, `target_ssid`, `ap_ip`, free heap, relay state `master` / `dc`, plus sharing state `napt`, `clients`, `upstream_dns` |
| GET | `/api/power` | Current relay state: `{"ok":true,"master":<bool>,"dc":<bool>,"out1":<bool>,"out2":<bool>}` |
| GET | `/api/sensors` | Real ADC readings (`ac_v`, `dc_v`, `amp`, `bat`) plus raw mV (min/max/avg/pp per pin) and the active calibration factors — open this from a phone while tuning |
| GET/POST | `/api/sensors/recal` | Re-measures the ACS712 zero point and returns `{"ok":true,"acs_zero_v":<float>}`. GET is accepted so it can be triggered from a phone browser. The zero is the mean of ~400 ms of samples, so an *AC* load running during calibration averages out and does not skew it |
| POST | `/api/power` | Body `{"master":<bool>,"dc":<bool>}` (either or both) → drives the relays, returns the new state. HTTP 400 if neither field is present |
| GET | `/api/wifi/scan` | Returns `[{ssid,rssi,sec}]`; HTTP 500 + `{"error":...}` if the scan fails (no silent empty list) |

Android/iOS/Windows captive-portal probes (`/generate_204`, `/hotspot-detect.html`,
`/connecttest.txt`, `/ncsi.txt`) redirect to `http://192.168.4.1/`.

The firmware keeps the model calls server-side, so the API key never has to be
used directly by the browser. (The current UI still contains a default key of
its own — see the note below.)

## Rebuild & re-upload

```powershell
$cli = "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
$fqbn = "esp32:esp32:esp32:PartitionScheme=huge_app"

# after editing powerbox-control.html / powerbox-control.js:
py tools/embed_assets.py

& $cli compile --fqbn $fqbn --output-dir esp32\build esp32\powerbox_esp32
& $cli upload  -p COM5 --fqbn $fqbn --input-dir esp32\build esp32\powerbox_esp32
```

`huge_app` is used because the 3 MB app partition leaves plenty of room for the
embedded UI. The sketch currently uses ~1.24 MB.

## Notes / limitations

- The UI makes **no external requests**. The Google Fonts `<link>` tags were
  removed because the setup AP has no internet, and a blocking font request
  made the page render blank until it timed out.
- HTML/JS are served in 1 KB chunks (`sendProgmemPage`) instead of one large
  write, which avoids truncated pages on slow clients.
- TLS uses `setInsecure()` (no certificate pinning) — fine for a prototype,
  not for production.
- `/api/chat` forwards the body as-is; the model is whatever the UI sent.
- The UI's own `DEFAULT_AI_CONFIG` in `powerbox-control.js` still contains a
  plaintext API key. Because it is in the repo, treat it as leaked and rotate it.
- The setup AP and home Wi-Fi run in `WIFI_AP_STA` mode at the same time.
- **Internet sharing (AP → STA NAT):** always on whenever the uplink is up. The
  board runs lwIP NAPT (`WiFi.AP.enableNAPT`) so devices on `PowerBox_AP` get
  internet through the home Wi-Fi. The DHCP server hands out the uplink's DNS
  (`WiFi.STA.dnsIP(0)`, falling back to the gateway / 8.8.8.8) instead of
  `192.168.4.1`. There is no toggle in the UI and no NVS setting — sharing
  cannot be switched off by accident.
  - Single 2.4 GHz radio: AP and STA share one channel, so expect roughly
    2–8 Mbps and higher latency. Fine for chat/basic browsing, not for 4K video.
  - Max 8 AP clients; the DHCP pool has 10 addresses.
  - AP restarts briefly when the uplink changes channel.
  - Clients that were already connected before the uplink came up may need
    to toggle their Wi-Fi once to pick up the new DNS.
- **Power control (real relays):** Output 1 = GPIO 5 (DC circuit), Output 2 =
  GPIO 4 (master shut-off). Both are `OUTPUT_OPEN_DRAIN` and active-LOW. Output
  2 is inverted: logical ON energises the coil and **cuts** power, so the boot
  default (`out1` off, `out2` off) leaves the master **live** — same as the
  upstream sketch. Switch state is not persisted, so every reboot/power cut
  starts with master ON and DC OFF. The UI drives it through `/api/power` and
  syncs back from `/api/status` every 5 s.
- The AP password is configurable from the UI (8–63 chars). Changing it restarts
  the AP ~2.5 s after the save response, so connected devices must rejoin.
- The Wi-Fi settings UI shows only real data: the status bar and the top-bar
  connection pill poll `/api/status` every 5 s, the scan list only shows results
  from `/api/wifi/scan` (no built-in fake networks), and "saved" is reported
  only after `/api/settings` returns `ok` and the board actually joins the
  network (up to 20 s wait).
- No authentication on the HTTP API: anyone on the same network can control the
  box or read the settings endpoint.
