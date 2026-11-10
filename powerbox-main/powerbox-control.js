(function(){
  "use strict";

  /* ── สถานะกล่อง PowerBox ─────────────────── */
  var state = {
    master: false,   // สวิตช์ไฟทั้งกล่อง (เปิด = AC ติด + จ่ายไฟไปเช็กสวิตช์ DC, ปิด = AC และ DC ไม่ติด)
    dc: false,       // สวิตช์วงจร DC (จะติดได้ต้องเปิดไฟทั้งกล่องด้วย)
    acV: 0,          // แรงดัน AC ที่วัดได้จริงจาก ADC (V)
    dcV: 0,          // แรงดันแบต/DC ที่วัดได้จริงจาก ADC (V)
    totalA: 0,       // กระแส AC ที่วัดได้จริง (A) — ไม่มีเซนเซอร์กระแส DC
    bat: -1          // % แบต; -1 = ยังไม่รู้ชนิดแบต (โชว์แรงดันจริงแทน)
  };

  /* ── ตารางงาน (Schedules) และเงื่อนไข (Conditions) ที่ตั้งโดย AI ── */
  var schedules = []; // [{ id, kind: 'timer'|'condition', label, instruction, remainingSec, totalSec, metric, operator, value, autoSwitches }]
  var scheduleSeq = 1;
  var chatHistory = []; // ประวัติการสนทนากับ AI

  /* ── ตั้งค่า AI และ Wi-Fi สำหรับรันบน ESP32 / Browser โดยตรง ── */
  var DEFAULT_AI_CONFIG = {
    API_KEY: 'tm_test_edjkKi9ig0sI_wQykQWYX-N4nev_WeLO7_iUz1XFXOkjKocLQofmzkWo',
    AI_BASE_URL: 'https://api.tokenme.limited/api/v1/chat/completions',
    AI_MODEL: 'google/gemma-4-31b-it'
  };

  var AI_CONFIG = {
    API_KEY: DEFAULT_AI_CONFIG.API_KEY,
    AI_BASE_URL: DEFAULT_AI_CONFIG.AI_BASE_URL,
    AI_MODEL: DEFAULT_AI_CONFIG.AI_MODEL
  };

  // ไม่มีค่า Wi-Fi ปลอมเป็น default: ค่าจริงมาจาก ESP32 (/api/status)
  // หรือจากสิ่งที่ผู้ใช้บันทึกไว้ใน localStorage ครั้งก่อน
  var WIFI_CONFIG = {
    ssid: '',
    password: ''
  };

  var STORAGE_KEY_SETTINGS = 'powerbox_settings_v1';

  // ค่า SSID ปลอมที่เวอร์ชันเก่าเคยใส่เป็น default — ถอยออกจาก localStorage
  // เดิม เพื่อไม่ให้ฟอร์ม pre-fill ชื่อเครือข่ายที่ไม่มีอยู่จริง
  var LEGACY_FAKE_SSIDS = ['PowerBox_Home_2.4G'];

  function loadSavedSettings(){
    try {
      var raw = localStorage.getItem(STORAGE_KEY_SETTINGS);
      if (!raw) return;
      var parsed = JSON.parse(raw);
      if (parsed && typeof parsed === 'object'){
        if (typeof parsed.apiKey === 'string' && parsed.apiKey.trim()){
          AI_CONFIG.API_KEY = parsed.apiKey.trim();
        }
        if (typeof parsed.apiUrl === 'string' && parsed.apiUrl.trim()){
          AI_CONFIG.AI_BASE_URL = parsed.apiUrl.trim();
        }
        if (typeof parsed.apiModel === 'string' && parsed.apiModel.trim()){
          AI_CONFIG.AI_MODEL = parsed.apiModel.trim();
        }
        if (typeof parsed.wifiSsid === 'string' && parsed.wifiSsid.trim() &&
            LEGACY_FAKE_SSIDS.indexOf(parsed.wifiSsid.trim()) === -1){
          WIFI_CONFIG.ssid = parsed.wifiSsid.trim();
        }
        if (typeof parsed.wifiPass === 'string'){
          WIFI_CONFIG.password = parsed.wifiPass;
        }
      }
    } catch (_e){}
  }
  loadSavedSettings();

  function isAcActive(){
    return state.master;
  }

  function isDcActive(){
    return state.master && state.dc;
  }

  var $ = function(id){ return document.getElementById(id); };
  var fmt = function(n){ return Number(n || 0).toFixed(2); };
  var fmtV = function(n){ return Number(n || 0).toFixed(1); };

  function badge(state_, onText, offText){
    return '<b class="mono">' + (state_ ? onText : offText) + '</b>';
  }

  function metricEl(id){
    return document.querySelector('[data-od-id="'+id+'"]');
  }

  function eventLog(text){
    var el = $('event-log');
    var t = new Date();
    var hh = ('0'+t.getHours()).slice(-2)+':'+('0'+t.getMinutes()).slice(-2)+':'+('0'+t.getSeconds()).slice(-2);
    el.textContent = hh + ' · ' + text;
  }

  function formatDuration(sec){
    var s = Math.max(0, Math.ceil(sec));
    var mm = Math.floor(s / 60);
    var ss = s % 60;
    return ('0' + mm).slice(-2) + ':' + ('0' + ss).slice(-2);
  }

  /* ── อัปเดตค่าแรงดันและกระแสทันทีเมื่อสวิตช์เปลี่ยน ── */
  function jitter(base, amp){ return base + (Math.random()*2-1)*amp; }

  function refreshInstantTelemetry(){
    // โหมดอุปกรณ์: ค่าที่แสดงต้องมาจาก ADC ของบอร์ดเท่านั้น ห้ามสร้างค่าเอง
    if (isDeviceUi()) return;
    var acOn = isAcActive();
    var dcOn = isDcActive();
    state.acV = acOn ? jitter(220.0, 0.8) : 0;
    state.dcV = dcOn ? jitter(12.4, 0.1) : 0;
    var acA = acOn ? jitter(2.15, 0.1) : 0;
    var dcA = dcOn ? jitter(6.2, 0.2) : 0;
    state.totalA = Math.max(0, acA + dcA);
  }

  /* ── อัปเดตหน้าจอตามสถานะ ──────────────── */
  function render(){
    var acOn = isAcActive();
    var dcOn = isDcActive();

    // ปุ่มทั้งกล่อง: เปิด -> AC ติด และดูว่า DC เปิดหรือปิด / ปิด -> AC และ DC ไม่ติด
    var mc = $('master-card'), mb = $('master-btn');
    mc.classList.toggle('live', state.master);
    $('master-chip').textContent = state.master ? 'ON' : 'OFF';
    mb.classList.toggle('on', state.master);
    mb.textContent = state.master ? 'ปิดไฟทั้งกล่อง' : 'เปิดไฟทั้งกล่อง';

    // ปุ่มและการ์ด วงจร DC
    var dcard = $('dc-card'), db = $('dc-btn');
    dcard.classList.toggle('live', dcOn);
    $('dc-chip').textContent = dcOn ? 'ON' : (state.dc ? 'STANDBY' : 'OFF');
    db.classList.toggle('on', state.dc);
    db.textContent = state.dc ? 'ปิดไฟวงจร DC' : 'เปิดไฟวงจร DC';

    // ค่าสถานะ: แรงดัน AC, แรงดันแบต/DC, กระแส AC ที่วัดได้จริง
    $('ac-volt').textContent = fmtV(state.acV);
    $('dc-volt').textContent = fmtV(state.dcV);
    $('total-val').textContent = fmt(state.totalA);

    $('ac-foot').textContent = !acOn
      ? 'วงจร AC ไม่ติด'
      : (state.acV >= 30 ? 'จ่ายไฟฝั่ง AC อยู่' : 'เปิดอยู่แต่วัดแรงดัน AC ไม่ได้');
    $('dc-foot').textContent = dcOn
      ? (state.dcV > 1 ? 'แรงดันแบตที่วัดได้ ' + fmtV(state.dcV) + ' V' : 'เปิดอยู่แต่วัดแรงดันไม่ได้')
      : (state.dc ? 'สวิตช์ DC เปิดอยู่ (รอเปิดไฟทั้งกล่อง)' : 'วงจร DC ไม่ติด');
    $('total-foot').textContent = state.totalA > 0
      ? ('กระแส AC ที่วัดได้ (' + usageNote() + ')')
      : 'ไม่มีโหลดในขณะนี้';

    // การ์ดเมตริก: ชี้แถบที่กำลังทำงาน
    metricEl('metric-ac-voltage').classList.toggle('idle', !acOn);
    metricEl('metric-dc-voltage').classList.toggle('idle', !dcOn);
    metricEl('metric-total-current').classList.toggle('idle', state.totalA === 0);
    metricEl('metric-total-current').classList.toggle('alert', state.totalA >= 8.5);

    // แบตเตอรี่: โชว์ % เมื่อรู้ช่วงแบตแล้ว ไม่งั้นโชว์แรงดันจริงที่วัดได้
    var batKnown = state.bat >= 0;
    var bp = batKnown ? Math.round(state.bat) : 0;
    $('bat-val').textContent = batKnown ? bp : '—';
    var fill = $('bat-fill');
    fill.style.width = bp + '%';
    fill.classList.toggle('warn', batKnown && bp < 40 && bp >= 20);
    fill.classList.toggle('low', batKnown && bp < 20);
    metricEl('metric-battery').classList.toggle('alert', batKnown && bp < 20);
    $('bat-foot').textContent = !batKnown
      ? (state.dcV > 1
          ? 'แรงดันแบต ' + fmtV(state.dcV) + ' V (ยังไม่ตั้งช่วง % แบต)'
          : 'ยังวัดแรงดันแบตไม่ได้')
      : ((!acOn && !dcOn)
          ? 'ไม่มีโหลดในขณะนี้'
          : (bp < 20 ? 'แบตใกล้หมด — ควรลดโหลด' : 'กำลังจ่ายไฟให้วงจร ' + usageNote()));

    // สรุปวงจร
    var n = (acOn?1:0) + (dcOn?1:0);
    $('circuit-summary').textContent = n === 0
      ? (state.dc ? 'ไฟไม่ติด (ปิดไฟทั้งกล่องอยู่ · สวิตช์ DC เปิดไว้)' : 'ไฟปิดอยู่ทั้งหมด')
      : ('วงจรที่ติด: ' + circText());
    // สถานะการเชื่อม — ใช้ข้อมูลจริงจาก ESP32 เท่านั้น
    var wifiLabel;
    if (!isDeviceUi()){
      wifiLabel = 'เปิดผ่านไฟล์ในเครื่อง (ไม่ได้ต่อกับกล่อง)';
    } else if (!DEVICE_STATUS.known){
      wifiLabel = 'กำลังตรวจสอบสถานะกล่อง…';
    } else if (!DEVICE_STATUS.reachable){
      wifiLabel = 'ติดต่อกล่องไม่ได้';
    } else if (DEVICE_STATUS.staUp && DEVICE_STATUS.ssid){
      wifiLabel = 'Wi-Fi: ' + DEVICE_STATUS.ssid +
        (DEVICE_STATUS.rssi ? ' (' + DEVICE_STATUS.rssi + ' dBm)' : '');
      if (DEVICE_STATUS.napt){
        wifiLabel += ' · แชร์เน็ต ' + DEVICE_STATUS.clients + ' เครื่อง';
      }
    } else {
      wifiLabel = 'โหมดตั้งค่า (AP) — ยังไม่ต่อ Wi-Fi บ้าน';
    }
    var batText = batKnown
      ? 'แบต ' + bp + '%'
      : (state.dcV > 1 ? 'แบต ' + fmtV(state.dcV) + ' V' : 'แบต —');
    $('conn-text').textContent = wifiLabel + ' · ' + batText;
    var connDot = $('conn-dot');
    if (connDot){
      var deviceOnline = !isDeviceUi() || (DEVICE_STATUS.known && DEVICE_STATUS.reachable);
      connDot.classList.toggle('off', !deviceOnline);
    }
    applyWifiPanelStatus();

    renderSchedules();
  }

  function renderSchedules(){
    var box = $('schedule-box');
    var list = $('schedule-list');
    var countEl = $('schedule-count');
    if (!box || !list) return;

    if (schedules.length === 0){
      box.hidden = true;
      list.innerHTML = '';
      return;
    }

    box.hidden = false;
    countEl.textContent = schedules.length + ' รายการ';
    list.innerHTML = schedules.map(function(item, idx){
      var badgeText = item.kind === 'timer'
        ? ('⏱ [' + item.id + '] อีก ' + formatDuration(item.remainingSec))
        : ('⚡ [' + item.id + '] เงื่อนไข: ' + item.metric + ' ' + item.operator + ' ' + item.value);
      return (
        '<div class="schedule-item" data-sched-id="' + item.id + '">' +
          '<div class="s-info">' +
            '<span class="s-timer">' + badgeText + '</span>' +
            '<span>' + escapeHtml((idx + 1) + '. ' + item.label) + '</span>' +
          '</div>' +
          '<div class="s-actions">' +
            '<button type="button" data-action="run-now" data-id="' + item.id + '" title="ทดสอบทริกเกอร์นี้ทันทีโดยไม่ต้องรอเวลา">รันทันที</button>' +
            '<button type="button" data-action="cancel" data-id="' + item.id + '" title="ยกเลิกรายการนี้">ยกเลิก</button>' +
          '</div>' +
        '</div>'
      );
    }).join('');
  }

  function escapeHtml(str){
    return String(str || '')
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }

  /* ── สถานะจริงของอุปกรณ์ (จาก ESP32 /api/status) ───────────────────
     หน้าจอทั้งหมดอ่านค่าจากที่นี่ ห้ามเดาสถานะเอง ถ้าคุยกับอุปกรณ์ไม่ได้
     ให้ขึ้นว่า "ติดต่อกล่องไม่ได้" ไม่ใช่แสดงค่า Wi-Fi ปลอมเหมือนเดิม */
  var DEVICE_STATUS = {
    known: false,      // เคยได้คำตอบจาก /api/status หรือยัง
    reachable: false,  // คำขอล่าสุดสำเร็จหรือไม่
    staUp: false,
    ssid: '',
    ip: '',
    rssi: 0,
    napt: false,       // NAPT ทำงานจริงอยู่หรือไม่
    clients: 0,        // จำนวนเครื่องที่ต่อ PowerBox_AP
    upstreamDns: '',
    powerPending: false, // กำลังส่งคำสั่งเปิด/ปิดไฟอยู่ (กัน poll ทับ)
    pending: false
  };

  function isDeviceUi(){
    return window.location.protocol !== 'file:' && window.location.port !== '5500';
  }

  function fetchDeviceStatus(){
    if (!isDeviceUi() || DEVICE_STATUS.pending){
      return Promise.resolve(DEVICE_STATUS);
    }
    DEVICE_STATUS.pending = true;
    return fetch('/api/status', { cache: 'no-store' })
      .then(function(res){
        if (!res.ok) throw new Error('HTTP ' + res.status);
        return res.json();
      })
      .then(function(d){
        DEVICE_STATUS.known = true;
        DEVICE_STATUS.reachable = true;
        DEVICE_STATUS.staUp = !!d.sta_up;
        DEVICE_STATUS.ssid = d.sta_ssid || '';
        DEVICE_STATUS.ip = d.sta_ip || '';
        DEVICE_STATUS.rssi = (typeof d.rssi === 'number') ? d.rssi : 0;
        DEVICE_STATUS.napt = !!d.napt;
        DEVICE_STATUS.clients = (typeof d.clients === 'number') ? d.clients : 0;
        DEVICE_STATUS.upstreamDns = d.upstream_dns || '';
        // สวิตช์จริงบนกล่อง — ใช้ sync ทุกเครื่องที่เปิดหน้านี้อยู่
        // (ข้ามระหว่างที่เรากำลังส่งคำสั่ง เพื่อไม่ให้ค่าที่เพิ่งกดถูกทับ)
        if (!DEVICE_STATUS.powerPending){
          if (typeof d.master === 'boolean' && state.master !== d.master){
            state.master = d.master;
            refreshInstantTelemetry();
            render();
          }
          if (typeof d.dc === 'boolean' && state.dc !== d.dc){
            state.dc = d.dc;
            refreshInstantTelemetry();
            render();
          }
        }
        // ค่าวัดจริงจาก ADC — ใช้แทนค่าจำลองทั้งหมดในโหมดอุปกรณ์
        if (typeof d.ac_v === 'number') state.acV = d.ac_v;
        if (typeof d.dc_v === 'number') state.dcV = d.dc_v;
        if (typeof d.amp === 'number') state.totalA = d.amp;
        if (typeof d.bat === 'number') state.bat = d.bat;   // -1 = ยังไม่รู้ %
        // ถ้าต่ออยู่จริง ให้ใช้ SSID จริงของอุปกรณ์เป็นค่าตั้งต้นในฟอร์ม
        if (DEVICE_STATUS.staUp && DEVICE_STATUS.ssid){
          WIFI_CONFIG.ssid = DEVICE_STATUS.ssid;
        } else if (!DEVICE_STATUS.staUp && d.target_ssid){
          // ยังต่อไม่ติด แต่ ESP32 มี SSID ที่บันทึกไว้ — ใช้เป็นค่าเริ่มต้น
          WIFI_CONFIG.ssid = d.target_ssid;
        }
        return DEVICE_STATUS;
      })
      .catch(function(){
        DEVICE_STATUS.known = true;
        DEVICE_STATUS.reachable = false;
        return DEVICE_STATUS;
      })
      .then(function(st){
        DEVICE_STATUS.pending = false;
        return st;
      });
  }

  function waitForWifiJoin(ssid, timeoutMs){
    var deadline = Date.now() + (timeoutMs || 20000);
    function attempt(){
      if (Date.now() > deadline) return Promise.resolve(false);
      return fetchDeviceStatus().then(function(st){
        if (st.reachable && st.staUp && st.ssid === ssid) return true;
        return new Promise(function(resolve){
          setTimeout(function(){ resolve(attempt()); }, 1000);
        });
      });
    }
    return attempt();
  }

  function applyWifiPanelStatus(){
    var dot = $('wifi-status-dot');
    var label = $('wifi-cur-ssid');
    if (!dot && !label) return;

    var text, online;
    if (!isDeviceUi()){
      text = 'เปิดผ่านไฟล์ในเครื่อง — ไม่มีข้อมูลจริง';
      online = false;
    } else if (!DEVICE_STATUS.known){
      text = 'กำลังตรวจสอบ…';
      online = false;
    } else if (!DEVICE_STATUS.reachable){
      text = 'ติดต่อกล่องไม่ได้';
      online = false;
    } else if (DEVICE_STATUS.staUp && DEVICE_STATUS.ssid){
      text = DEVICE_STATUS.ssid + (DEVICE_STATUS.ip ? ' · ' + DEVICE_STATUS.ip : '');
      online = true;
    } else {
      text = 'ยังไม่เชื่อมต่อ Wi-Fi บ้าน (โหมด AP)';
      online = false;
    }

    if (dot) dot.classList.toggle('off', !online);
    if (label) label.textContent = text;
  }

  function circText(){
    var parts = [];
    if (isAcActive()) parts.push('AC');
    if (isDcActive()) parts.push('DC');
    return parts.join(' + ');
  }
  function usageNote(){
    var acOn = isAcActive(), dcOn = isDcActive();
    return acOn && dcOn ? 'AC + DC' : (dcOn ? 'DC' : (acOn ? 'AC' : ''));
  }

  /* ── ลูปอัปเดตค่าหน้าจอ + ตรวจเช็ก Schedule / เงื่อนไข ทุก 1 วิ ── */
  setInterval(function(){
    // โหมดอุปกรณ์: ห้ามจำลองค่าหรือกร่อนแบตเอง — รอค่าจริงจาก /api/status
    if (!isDeviceUi()){
      var acOn = isAcActive();
      var dcOn = isDcActive();

      state.acV = acOn ? jitter(220.0, 1.2) : 0;
      state.dcV = dcOn ? jitter(12.4, 0.15) : 0;

      var acA = acOn ? jitter(2.15, 0.15) : 0;
      var dcA = dcOn ? jitter(6.2, 0.35) : 0;
      state.totalA = Math.max(0, acA + dcA);

      if (acOn || dcOn){
        state.bat = Math.max(0, state.bat - state.totalA * 0.055);
      }
    }

    checkSchedulesAndConditions();
    render();
  }, 1000);

  // โหมดพรีวิว: เริ่มจากค่าแบตจำลอง เพื่อให้หน้าจอเดโมดูสมจริง
  if (!isDeviceUi()) state.bat = 78;
  render();
  eventLog('ระบบพร้อมใช้งาน');

  /* ── สั่งงานรีเลย์จริงบนกล่อง ────────────── */
  var POWER_BUSY = false;

  // ส่งคำสั่งเปิด/ปิดไฟไปยัง ESP32 (/api/power) แล้วใช้สถานะที่อุปกรณ์ตอบกลับ
  // เป็นจริง; ถ้าสั่งไม่สำเร็จจะย้อนสถานะเดิมและแจ้งผู้ใช้ ไม่เดาเอาเอง
  function pushPower(masterOn, dcOn){
    if (!isDeviceUi()){
      // โหมดพรีวิว (เปิดไฟล์ในเครื่อง) — จำลองเฉพาะในเบราว์เซอร์
      state.master = masterOn;
      state.dc = dcOn;
      refreshInstantTelemetry();
      render();
      return Promise.resolve(true);
    }
    if (POWER_BUSY) return Promise.resolve(false);
    POWER_BUSY = true;
    DEVICE_STATUS.powerPending = true;   // กัน poll /api/status ทับระหว่างสั่ง

    var prevMaster = state.master, prevDc = state.dc;
    state.master = masterOn;
    state.dc = dcOn;
    refreshInstantTelemetry();
    render();

    return fetch('/api/power', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ master: masterOn, dc: dcOn })
    }).then(function(res){
      return res.json().then(function(p){
        if (!res.ok || !p || p.ok !== true) throw new Error('bad response');
        state.master = !!p.master;
        state.dc = !!p.dc;
        return true;
      });
    }).catch(function(){
      state.master = prevMaster;
      state.dc = prevDc;
      eventLog('สั่งงานไม่สำเร็จ — ติดต่อกล่องไม่ได้ (ย้อนสถานะเดิมแล้ว)');
      return false;
    }).then(function(ok){
      POWER_BUSY = false;
      DEVICE_STATUS.powerPending = false;
      refreshInstantTelemetry();
      render();
      return ok;
    });
  }

  /* ── ปุ่มควบคุม ────────────────────────── */
  $('master-btn').addEventListener('click', function(){
    var next = !state.master;
    pushPower(next, state.dc).then(function(ok){
      if (!ok) return;
      if (state.master){
        eventLog(state.dc
          ? 'เปิดไฟทั้งกล่อง — AC ติด และ DC ติด (สวิตช์ DC เปิดอยู่)'
          : 'เปิดไฟทั้งกล่อง — AC ติด (ส่วน DC ไม่ติดเพราะปิดสวิตช์ DC อยู่)');
      } else {
        eventLog('ปิดไฟทั้งกล่อง — AC และ DC ไม่ติด');
      }
    });
  });

  $('dc-btn').addEventListener('click', function(){
    var next = !state.dc;
    pushPower(state.master, next).then(function(ok){
      if (!ok) return;
      if (state.master){
        eventLog((state.dc ? 'เปิด' : 'ปิด') + 'ไฟวงจร DC (ขณะเปิดไฟทั้งกล่อง)');
      } else {
        eventLog((state.dc ? 'เปิด' : 'ปิด') + 'สวิตช์ DC ไว้ — ไฟยังไม่ติดจนกว่าจะเปิดไฟทั้งกล่อง');
      }
    });
  });

  if ($('schedule-list')){
    $('schedule-list').addEventListener('click', function(e){
      var btn = e.target.closest('button[data-action]');
      if (!btn) return;
      var id = btn.getAttribute('data-id');
      var action = btn.getAttribute('data-action');
      var idx = schedules.findIndex(function(s){ return s.id === id; });
      if (idx === -1) return;
      var item = schedules[idx];
      schedules.splice(idx, 1);
      render();
      if (action === 'run-now'){
        executeTriggeredSchedule(item);
      } else {
        eventLog('ยกเลิกตารางงาน: ' + item.label);
      }
    });
  }

  /* ── หน้าต่างแชทแบบลอย (เปิด/ปิด & ลากย้ายซ้อนทับส่วนควบคุม) ── */
  var chatPanel = $('chat-panel');
  var chatFab = $('chat-fab');
  var chatBadge = $('chat-badge');
  var chatCloseBtn = $('chat-close-btn');
  var chatDragHandle = $('chat-drag-handle');
  var chatBody = $('chat-body');
  var chatOpen = false;
  var unreadCount = 1;

  function updateChatWindowUI(){
    chatPanel.classList.toggle('open', chatOpen);
    chatPanel.setAttribute('aria-hidden', chatOpen ? 'false' : 'true');
    chatFab.classList.toggle('active', chatOpen);
    chatFab.setAttribute('aria-expanded', chatOpen ? 'true' : 'false');

    if (chatOpen){
      unreadCount = 0;
      chatBadge.hidden = true;
      setTimeout(function(){
        $('chat-input').focus();
      }, 120);
    } else if (unreadCount > 0){
      chatBadge.textContent = String(unreadCount);
      chatBadge.hidden = false;
    } else {
      chatBadge.hidden = true;
    }
  }

  function toggleChat(forceState){
    chatOpen = typeof forceState === 'boolean' ? forceState : !chatOpen;
    updateChatWindowUI();
  }

  chatFab.addEventListener('click', function(){ toggleChat(); });
  if (chatCloseBtn){
    chatCloseBtn.addEventListener('click', function(){ toggleChat(false); });
  }
  document.addEventListener('keydown', function(e){
    if (e.key === 'Escape' && chatOpen){
      toggleChat(false);
    }
  });

  // รองรับการลากย้ายหน้าต่างแชทไปวางซ้อนทับจุดไหนก็ได้
  (function initDraggableChat(){
    if (!chatDragHandle || !chatPanel) return;
    var dragging = false;
    var startX = 0, startY = 0, origLeft = 0, origTop = 0;

    function onPointerDown(e){
      if (e.target.closest('button')) return;
      var rect = chatPanel.getBoundingClientRect();
      dragging = true;
      startX = e.clientX;
      startY = e.clientY;
      origLeft = rect.left;
      origTop = rect.top;
      chatPanel.style.left = origLeft + 'px';
      chatPanel.style.top = origTop + 'px';
      chatPanel.style.right = 'auto';
      chatPanel.style.bottom = 'auto';
      chatPanel.classList.add('dragging');
      if (chatDragHandle.setPointerCapture && e.pointerId !== undefined){
        chatDragHandle.setPointerCapture(e.pointerId);
      }
    }

    function onPointerMove(e){
      if (!dragging) return;
      var dx = e.clientX - startX;
      var dy = e.clientY - startY;
      var maxLeft = Math.max(8, window.innerWidth - chatPanel.offsetWidth - 8);
      var maxTop = Math.max(8, window.innerHeight - chatPanel.offsetHeight - 8);
      var nextLeft = Math.min(maxLeft, Math.max(8, origLeft + dx));
      var nextTop = Math.min(maxTop, Math.max(8, origTop + dy));
      chatPanel.style.left = nextLeft + 'px';
      chatPanel.style.top = nextTop + 'px';
    }

    function onPointerUp(){
      if (!dragging) return;
      dragging = false;
      chatPanel.classList.remove('dragging');
    }

    chatDragHandle.addEventListener('pointerdown', onPointerDown);
    window.addEventListener('pointermove', onPointerMove);
    window.addEventListener('pointerup', onPointerUp);
  })();

  /* ── ระบบ Schedule & Condition Trigger Engine ────────────── */
  function evaluateOperator(actual, op, target){
    var a = Number(actual);
    var b = Number(target);
    if (op === '<') return a < b;
    if (op === '<=') return a <= b;
    if (op === '>') return a > b;
    if (op === '>=') return a >= b;
    if (op === '==' || op === '=') return Math.abs(a - b) < 0.01;
    return false;
  }

  function getMetricValue(metric){
    if (metric === 'bat' || metric === 'battery') return state.bat;
    if (metric === 'totalA' || metric === 'current') return state.totalA;
    if (metric === 'acV' || metric === 'ac_voltage') return state.acV;
    if (metric === 'dcV' || metric === 'dc_voltage') return state.dcV;
    return 0;
  }

  function checkSchedulesAndConditions(){
    var triggered = [];
    for (var i = schedules.length - 1; i >= 0; i--){
      var item = schedules[i];
      if (item.kind === 'timer'){
        item.remainingSec -= 1;
        if (item.remainingSec <= 0){
          triggered.unshift(item);
          schedules.splice(i, 1);
        }
      } else if (item.kind === 'condition'){
        var actualVal = getMetricValue(item.metric);
        if (evaluateOperator(actualVal, item.operator, item.value)){
          triggered.unshift(item);
          schedules.splice(i, 1);
        }
      }
    }

    triggered.forEach(function(item){
      executeTriggeredSchedule(item);
    });
  }

  function applySwitchChange(sw, sourceLabel){
    if (!sw || typeof sw !== 'object') return false;
    var master = (typeof sw.master === 'boolean') ? sw.master : state.master;
    var dc = (typeof sw.dc === 'boolean') ? sw.dc : state.dc;
    if (master === state.master && dc === state.dc) return false;

    // สั่งรีเลย์จริง (หรือจำลองในโหมดพรีวิว) แล้วบันทึกผลเมื่อสำเร็จ
    pushPower(master, dc).then(function(ok){
      if (!ok) return;
      eventLog((sourceLabel || 'AI สั่งงาน') + ' → ไฟทั้งกล่อง: ' + (state.master ? 'ON' : 'OFF') + ', สวิตช์ DC: ' + (state.dc ? 'ON' : 'OFF'));
    });
    return true;
  }

  function executeTriggeredSchedule(item){
    // ถ้ามีการตั้ง autoSwitches ไว้ ให้สับสวิตช์ทันทีเมื่อถึงเวลา/เข้าเงื่อนไข
    if (item.autoSwitches){
      applySwitchChange(item.autoSwitches, 'ทริกเกอร์ (' + item.label + ')');
    }
    pushMsg('sys', '⚡ <b>ทริกเกอร์ทำงาน:</b> ' + escapeHtml(item.label));

    // เริ่ม Turn ของ AI อัตโนมัติเพื่อให้ AI ประมวลผลและแจ้งผลลัพธ์กับผู้ใช้
    var triggerPrompt =
      '[SYSTEM_TRIGGER_FIRED]\n' +
      'ทริกเกอร์ที่ตั้งไว้ทำงานแล้ว: "' + item.label + '"\n' +
      'คำสั่งที่ต้องทำใน Turn นี้: ' + (item.instruction || item.label) + '\n' +
      'กรุณาตรวจสอบสถานะกล่องล่าสุด สั่งเปิด/ปิดสวิตช์ผ่าน <powerbox_cmd> (หากยังไม่ได้เปลี่ยน) และแจ้งผลให้ผู้ใช้ทราบอย่างเป็นธรรมชาติ';

    aiRespond(triggerPrompt, true);
  }

  /* ── เชื่อมต่อ AI สำหรับ ESP32 / Browser (ไม่ต้องพึ่ง Node.js Server) ── */
  function resolveAiConfig(){
    return {
      API_KEY: AI_CONFIG.API_KEY || '',
      AI_BASE_URL: AI_CONFIG.AI_BASE_URL,
      AI_MODEL: AI_CONFIG.AI_MODEL
    };
  }

  function buildSystemPrompt(){
    var acOn = isAcActive();
    var dcOn = isDcActive();
    var nowStr = new Date().toLocaleTimeString('th-TH', { hour12: false });
    var activeSchedStr = schedules.length === 0
      ? 'ไม่มี'
      : schedules.map(function(s){
          return s.kind === 'timer'
            ? ('- [ID:' + s.id + '] อีก ' + s.remainingSec + ' วินาที: ' + s.label)
            : ('- [ID:' + s.id + '] เมื่อ ' + s.metric + ' ' + s.operator + ' ' + s.value + ': ' + s.label);
        }).join('\n');

    return (
      'คุณคือสมองกลฝังตัวของกล่องควบคุมไฟอัจฉริยะ "PowerBox" ของจริง (ไม่ใช่ระบบจำลองในบทสนทนา พูดคุยในฐานะที่คุณคือตัวกล่อง PowerBox จริงที่เชื่อมต่อกับรีเลย์และเซนเซอร์โดยตรง)\n\n' +
      '=== กฎทางฮาร์ดแวร์ของกล่อง PowerBox ===\n' +
      '1. สวิตช์ "ไฟทั้งกล่อง" (master):\n' +
      '   - เมื่อเปิด (master = true): วงจร AC จะติดทันที (~220V) และจ่ายไฟต่อไปยังสวิตช์วงจร DC (ถ้า dc = true ไฟ DC ~12.4V จะติด / ถ้า dc = false ไฟ DC จะไม่ติด)\n' +
      '   - เมื่อปิด (master = false): ตัดไฟทั้งกล่อง ทั้ง AC และ DC จะไม่ติดทั้งหมด (0V, 0A)\n' +
      '2. สวิตช์ "ไฟวงจร DC" (dc):\n' +
      '   - เปิด/ปิดวงจร 12V DC โดยไฟ DC จะติดจริงเมื่อ master = true และ dc = true เท่านั้น\n' +
      '3. ค่าที่วัดได้มี: แรงดัน AC แยก (V), แรงดัน DC แยก (V), กระแสรวม AC+DC (A), และแบตเตอรี่ (%)\n\n' +
      '=== สถานะเซนเซอร์จริง ณ เวลา ' + nowStr + ' ===\n' +
      '- สวิตช์ไฟทั้งกล่อง (master): ' + (state.master ? 'ON (เปิด)' : 'OFF (ปิด)') + '\n' +
      '- สวิตช์วงจร DC (dc): ' + (state.dc ? 'ON (เปิด)' : 'OFF (ปิด)') + '\n' +
      '- สถานะไฟ AC จริง: ' + (acOn ? 'ติด (' + fmtV(state.acV) + ' V)' : 'ไม่ติด (0.0 V)') + '\n' +
      '- สถานะไฟ DC จริง: ' + (dcOn ? 'ติด (' + fmtV(state.dcV) + ' V)' : 'ไม่ติด (0.0 V)') + '\n' +
      '- กระแสรวม (AC + DC): ' + fmt(state.totalA) + ' A\n' +
      '- แบตเตอรี่คงเหลือ: ' + Math.round(state.bat) + '%\n' +
      '- ตารางงาน/เงื่อนไขที่ตั้งไว้ตอนนี้:\n' + activeSchedStr + '\n\n' +
      '=== วิธีสั่งเปิด/ปิดไฟ และตั้ง Schedule / เงื่อนไขเริ่ม Turn อัตโนมัติ ===\n' +
      'ตอบกลับผู้ใช้เป็นภาษาไทยที่กระชับ เป็นธรรมชาติ และหากต้องสั่งสวิตช์ ตั้งเวลา (Schedule) หรือตั้งเงื่อนไขเพื่อเริ่ม Turn อัตโนมัติ ให้แนบแท็ก <powerbox_cmd>JSON</powerbox_cmd> ไว้ท้ายข้อความเสมอ ตามรูปแบบนี้:\n' +
      '<powerbox_cmd>\n' +
      '{\n' +
      '  "set_switches": { "master": true, "dc": true },\n' +
      '  "add_schedules": [\n' +
      '    {\n' +
      '      "delay_seconds": 600,\n' +
      '      "label": "ตัดไฟทั้งกล่อง (ครบ 10 นาที)",\n' +
      '      "instruction": "ครบ 10 นาทีแล้ว สั่งปิดไฟทั้งกล่องและแจ้งผู้ใช้",\n' +
      '      "auto_switches": { "master": false }\n' +
      '    }\n' +
      '  ],\n' +
      '  "add_conditions": [\n' +
      '    {\n' +
      '      "metric": "bat",\n' +
      '      "operator": "<=",\n' +
      '      "value": 50,\n' +
      '      "label": "เมื่อแบต <= 50% ให้ตัดไฟทั้งกล่อง",\n' +
      '      "instruction": "แบตลดลงถึง 50% แล้ว สั่งตัดไฟทั้งกล่องและเตือนผู้ใช้",\n' +
      '      "auto_switches": { "master": false }\n' +
      '    }\n' +
      '  ],\n' +
      '  "cancel_schedules": ["all"]\n' +
      '}\n' +
      '</powerbox_cmd>\n' +
      'หมายเหตุ:\n' +
      '- ใส่เฉพาะคีย์ที่ต้องการใช้งานใน <powerbox_cmd> เท่านั้น (ถ้าแค่ตอบคำถามทั่วไปไม่ต้องใส่ <powerbox_cmd>)\n' +
      '- หากผู้ใช้สั่งยกเลิกตารางงาน/เวลา/เงื่อนไขทั้งหมด ให้ใส่ "cancel_schedules": ["all"] หรือถ้ายกเลิกเฉพาะบางรายการให้ใส่ ID ของรายการนั้น เช่น "cancel_schedules": ["S1"] หรือ ["C1"]\n' +
      '- แปลงหน่วยเวลาเป็นวินาทีใน delay_seconds เสมอ (เช่น 10 นาที = 600, 1 นาที = 60, 30 วินาที = 30)\n' +
      '- metric ที่รองรับใน add_conditions คือ: "bat" (แบตเตอรี่ %), "totalA" (กระแสรวม A), "acV" (แรงดัน AC), "dcV" (แรงดัน DC)'
    );
  }

  function parseAndExecuteAiCommands(rawText){
    var cmdRegex = /<powerbox_cmd>([\s\S]*?)<\/powerbox_cmd>/gi;
    var match;
    while ((match = cmdRegex.exec(rawText)) !== null){
      try {
        var cmd = JSON.parse(match[1].trim());
        if (cmd.set_switches){
          applySwitchChange(cmd.set_switches, 'AI สั่งสวิตช์');
        }
        var cancelTarget = cmd.cancel_schedules !== undefined ? cmd.cancel_schedules : cmd.cancel_schedule;
        if (cancelTarget !== undefined && cancelTarget !== null){
          var targets = Array.isArray(cancelTarget) ? cancelTarget : [cancelTarget];
          var normTargets = targets.map(function(t){
            return String(t).trim().replace(/^ID:/i, '').toUpperCase();
          });
          if (normTargets.indexOf('ALL') !== -1 || normTargets.indexOf('*') !== -1 || cancelTarget === true){
            var countAll = schedules.length;
            schedules = [];
            eventLog('AI ยกเลิกตารางงานทั้งหมด (' + countAll + ' รายการ)');
          } else {
            var removedLabels = [];
            schedules = schedules.filter(function(s, idx){
              var sid = String(s.id).toUpperCase();
              var idx1 = String(idx + 1);
              var matchId = normTargets.indexOf(sid) !== -1 ||
                            normTargets.indexOf(idx1) !== -1 ||
                            normTargets.indexOf('#' + idx1) !== -1 ||
                            (normTargets.indexOf('TIMER') !== -1 && s.kind === 'timer') ||
                            (normTargets.indexOf('CONDITION') !== -1 && s.kind === 'condition');
              if (matchId){
                removedLabels.push(s.label);
                return false;
              }
              return true;
            });
            if (removedLabels.length > 0){
              eventLog('AI ยกเลิกตารางงาน: ' + removedLabels.join(', '));
            }
          }
        }
        if (Array.isArray(cmd.add_schedules)){
          cmd.add_schedules.forEach(function(s){
            var sec = Math.max(1, Number(s.delay_seconds) || 60);
            var item = {
              id: 'S' + (scheduleSeq++),
              kind: 'timer',
              remainingSec: sec,
              totalSec: sec,
              label: s.label || ('ตั้งเวลา ' + sec + ' วินาที'),
              instruction: s.instruction || s.label || 'ดำเนินการตามที่ตั้งเวลาไว้',
              autoSwitches: s.auto_switches || null
            };
            schedules.push(item);
            eventLog('AI ตั้งเวลา: ' + item.label + ' (' + formatDuration(sec) + ')');
          });
        }
        if (Array.isArray(cmd.add_conditions)){
          cmd.add_conditions.forEach(function(c){
            var item = {
              id: 'C' + (scheduleSeq++),
              kind: 'condition',
              metric: c.metric || 'bat',
              operator: c.operator || '<=',
              value: Number(c.value),
              label: c.label || ('เมื่อ ' + (c.metric || 'bat') + ' ' + (c.operator || '<=') + ' ' + c.value),
              instruction: c.instruction || c.label || 'ดำเนินการตามเงื่อนไขที่ตั้งไว้',
              autoSwitches: c.auto_switches || null
            };
            schedules.push(item);
            eventLog('AI ตั้งเงื่อนไข: ' + item.label);
          });
        }
        render();
      } catch (e){
        console.warn('Failed to parse <powerbox_cmd>:', e);
      }
    }
    var cleanText = rawText.replace(/<powerbox_cmd>[\s\S]*?<\/powerbox_cmd>/gi, '').trim();
    return cleanText;
  }

  async function callAiApi(messages){
    var env = resolveAiConfig();

    // 1. ถ้ารันอยู่บน ESP32 WebServer ที่มี endpoint /api/chat (ยิงผ่าน HTTPClient ของ ESP32 ไม่ติด CORS)
    if (window.location.protocol !== 'file:' && window.location.port !== '5500'){
      try {
        var espRes = await fetch('/api/chat', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            model: env.AI_MODEL,
            messages: messages,
            temperature: 0.3
          })
        });
        if (espRes.ok){
          var espData = await espRes.json();
          var espMsg = espData.choices && espData.choices[0] && espData.choices[0].message;
          if (espMsg && (espMsg.content || espMsg.reasoning)){
            return espMsg.content || espMsg.reasoning;
          }
        }
      } catch (_e){
        // ถ้าไม่มี /api/chat บนเซิร์ฟเวอร์ ให้ไปขั้นตอนถัดไป
      }
    }

    // 2. ลองยิงตรงจาก Browser ไปยัง AI_BASE_URL
    if (!env || !env.API_KEY){
      throw new Error('NO_API_KEY');
    }

    var directRes = await fetch(env.AI_BASE_URL, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': 'Bearer ' + env.API_KEY
      },
      body: JSON.stringify({
        model: env.AI_MODEL,
        messages: messages,
        temperature: 0.3,
        chat_template_kwargs: { enable_thinking: false }
      })
    });

    var directData = await directRes.json();
    if (!directRes.ok){
      throw new Error((directData && directData.error && directData.error.message) || ('HTTP ' + directRes.status));
    }
    var msgObj = directData.choices && directData.choices[0] && directData.choices[0].message;
    return (msgObj && (msgObj.content || msgObj.reasoning)) || '';
  }

  /* ── ระบบประมวลผลคำสั่ง AI ในตัว (ทำงานได้ทันทีบน Browser / ESP32 แม้ติด CORS หรือออฟไลน์) ── */
  function localSmartFallback(userText){
    var q = userText.trim();
    var lower = q.toLowerCase();

    if (q.indexOf('[SYSTEM_TRIGGER_FIRED]') !== -1){
      return 'ดำเนินการตามตารางเวลา/เงื่อนไขที่ตั้งไว้เรียบร้อยแล้วครับ ตอนนี้ไฟทั้งกล่อง ' +
        badge(state.master, 'เปิด', 'ปิด') + ' (AC ' + badge(isAcActive(), 'ติด', 'ไม่ติด') +
        ', DC ' + badge(isDcActive(), 'ติด', 'ไม่ติด') + ') แบตเตอรี่อยู่ที่ <span class="mono">' + Math.round(state.bat) + '%</span> ครับ';
    }

    // ยกเลิกตารางงาน / เวลา / เงื่อนไข (รองรับทั้งการยกเลิกทั้งหมด และยกเลิกรายการย่อย)
    var isCancelIntent =
      /(ยกเลิก|ลบ|ล้าง|เคลียร์|เอาออก|หยุดนับ|หยุดตั้ง|cancel|clear)/.test(lower) ||
      /(ไม่ต้อง|ไม่เอา).*(ตัด|ปิด|เปิด|ตั้งเวลา|ตาราง|เงื่อนไข|แล้ว)/.test(lower);
    if (isCancelIntent){
      if (schedules.length === 0){
        return 'ตอนนี้ไม่มีตารางงานหรือเงื่อนไขอัตโนมัติที่ตั้งไว้ครับ';
      }

      // 1) ถ้าระบุ ID โดยตรง เช่น S1, C1, S2
      var idMatches = q.match(/\b([SC]\d+)\b/gi);
      if (idMatches && !/(ทั้งหมด|ทุกอัน|ทุกรายการ|all)/.test(lower)){
        var targetIds = idMatches.map(function(x){ return x.toUpperCase(); });
        var matchedById = schedules.filter(function(s){ return targetIds.indexOf(s.id.toUpperCase()) !== -1; });
        if (matchedById.length > 0){
          var namesById = matchedById.map(function(s){ return '[' + s.id + '] ' + s.label; }).join(', ');
          return 'ยกเลิกตารางงาน <b>' + escapeHtml(namesById) + '</b> ให้เรียบร้อยแล้วครับ <powerbox_cmd>' +
            JSON.stringify({ cancel_schedules: targetIds }) + '</powerbox_cmd>';
        }
      }

      // 2) ถ้าระบุลำดับ เช่น อันแรก, รายการที่ 1, อันที่ 2, อันล่าสุด, อันเมื่อกี้
      var numMatch = lower.match(/(?:รายการที่|อันที่|ข้อที่|ข้อ|เบอร์)\s*(\d+)/);
      var targetIndex = -1;
      if (numMatch){
        targetIndex = parseInt(numMatch[1], 10) - 1;
      } else if (/(อันแรก|รายการแรก|ตัวแรก)/.test(lower)){
        targetIndex = 0;
      } else if (/(อันล่าสุด|อันสุดท้าย|รายการล่าสุด|อันเมื่อกี้|เพิ่งตั้ง)/.test(lower)){
        targetIndex = schedules.length - 1;
      }
      if (targetIndex >= 0 && targetIndex < schedules.length && !/(ทั้งหมด|ทุกอัน|ทุกรายการ|all)/.test(lower)){
        var targetItem = schedules[targetIndex];
        return 'ยกเลิกรายการที่ ' + (targetIndex + 1) + ' (<b>[' + targetItem.id + '] ' + escapeHtml(targetItem.label) + '</b>) ให้เรียบร้อยแล้วครับ <powerbox_cmd>' +
          JSON.stringify({ cancel_schedules: [targetItem.id] }) + '</powerbox_cmd>';
      }

      // 3) ถ้าระบุประเภทเฉพาะเมื่อมีหลายรายการ เช่น "ยกเลิกเงื่อนไขแบต" หรือ "ยกเลิกตั้งเวลา"
      if (schedules.length > 1 && !/(ทั้งหมด|ทุกอัน|ทุกรายการ|ตารางงานทั้งหมด|all)/.test(lower)){
        var wantCancelCond = /(เงื่อนไข|แบต|battery|กระแส)/.test(lower) && !/(เวลา|นาที|วินาที)/.test(lower);
        var wantCancelTimer = /(ตั้งเวลา|นับถอยหลัง|นาที|วินาที|ชั่วโมง)/.test(lower) && !/(เงื่อนไข|แบต)/.test(lower);
        if (wantCancelCond || wantCancelTimer){
          var filtered = schedules.filter(function(s){
            return wantCancelCond ? s.kind === 'condition' : s.kind === 'timer';
          });
          if (filtered.length > 0 && filtered.length < schedules.length){
            var fIds = filtered.map(function(s){ return s.id; });
            var fNames = filtered.map(function(s){ return '[' + s.id + '] ' + s.label; }).join(', ');
            return 'ยกเลิก<b>' + escapeHtml(fNames) + '</b> ให้เรียบร้อยแล้วครับ <powerbox_cmd>' +
              JSON.stringify({ cancel_schedules: fIds }) + '</powerbox_cmd>';
          }
        }
      }

      // 4) ยกเลิกตารางงานทั้งหมด
      var allNames = schedules.map(function(s){ return '[' + s.id + '] ' + s.label; }).join(', ');
      return 'ยกเลิกตารางงานและเงื่อนไขอัตโนมัติทั้งหมด (' + schedules.length + ' รายการ: <b>' + escapeHtml(allNames) + '</b>) ให้เรียบร้อยแล้วครับ <powerbox_cmd>{"cancel_schedules":["all"]}</powerbox_cmd>';
    }

    // ตรวจจับการตั้งเวลา เช่น "อีก 10 นาทีตัดไฟ", "อีก 30 วินาทีเปิดไฟ"
    var timeMatch = q.match(/(?:อีก|ในอีก)?\s*(\d+)\s*(วินาที|วิ|นาที|ชั่วโมง|ชม\.?)/);
    if (timeMatch && /(เปิด|ปิด|ตัด|สับ|ไฟ|dc|ac|ทั้งกล่อง)/.test(lower)){
      var amount = parseInt(timeMatch[1], 10);
      var unit = timeMatch[2];
      var sec = unit.indexOf('นาที') !== -1 ? amount * 60 : (unit.indexOf('ชั่วโมง') !== -1 || unit.indexOf('ชม') !== -1 ? amount * 3600 : amount);
      var wantOff = /(ปิด|ตัด|ดับ|off)/.test(lower);
      var wantDcOnly = /dc/.test(lower) && !/ทั้งกล่อง|ทั้งหมด|ac/.test(lower);
      var wantBothOn = !wantOff && (/ทั้งกล่อง|ทั้งหมด/.test(lower) && /dc/.test(lower));
      var autoSw = wantOff
        ? (wantDcOnly ? { dc: false } : { master: false })
        : (wantBothOn ? { master: true, dc: true } : (wantDcOnly ? { master: true, dc: true } : { master: true }));
      var label = (wantOff ? 'ตัด' : 'เปิด') + (wantDcOnly ? 'ไฟวงจร DC' : (wantBothOn ? 'ไฟทั้งกล่อง + DC' : 'ไฟทั้งกล่อง')) + ' (ครบ ' + amount + ' ' + unit + ')';
      var cmdJson = JSON.stringify({
        add_schedules: [{
          delay_seconds: sec,
          label: label,
          instruction: 'ครบกำหนดเวลา ' + amount + ' ' + unit + ' แล้ว ให้' + label + 'และรายงานผู้ใช้',
          auto_switches: autoSw
        }]
      });
      return 'ตั้งเวลาให้เรียบร้อยแล้วครับ อีก <b>' + amount + ' ' + unit + '</b> ระบบจะ<b>' + label + '</b>ให้อัตโนมัติครับ <powerbox_cmd>' + cmdJson + '</powerbox_cmd>';
    }

    // ตรวจจับการตั้งเงื่อนไข (แบตเตอรี่ หรือ กระแส)
    var condMatch = q.match(/(?:ถ้า|เมื่อ|หาก).*?(แบต|battery|กระแส|แอมป์|current|ไฟ).*?(ต่ำกว่า|น้อยกว่า|ไม่เกิน|เกิน|มากกว่า|<|<=|>|>=|=)\s*(\d+(?:\.\d+)?)/i);
    if (condMatch){
      var metricWord = condMatch[1].toLowerCase();
      var opWord = condMatch[2];
      var targetVal = parseFloat(condMatch[3]);
      var metricKey = /(กระแส|แอมป์|current)/.test(metricWord) ? 'totalA' : 'bat';
      var opSymbol = /(เกิน|มากกว่า|>)/.test(opWord) ? (opWord === '>' ? '>' : '>=') : (opWord === '<' ? '<' : '<=');
      var wantTurnOn = /(เปิด|ติด|on)/.test(lower) && !/(ปิด|ตัด|ดับ|off)/.test(lower);
      var wantDc = /dc/.test(lower) && !/ทั้งกล่อง|ทั้งหมด/.test(lower);
      var condSw = wantTurnOn
        ? (wantDc ? { master: true, dc: true } : { master: true })
        : (wantDc ? { dc: false } : { master: false });
      var metricLabel = metricKey === 'totalA' ? ('กระแสรวม ' + opSymbol + ' ' + targetVal + 'A') : ('แบต ' + opSymbol + ' ' + targetVal + '%');
      var actionLabel = (wantTurnOn ? 'เปิด' : 'ตัด') + (wantDc ? 'ไฟ DC' : 'ไฟทั้งกล่อง');
      var condLabel = 'เมื่อ' + metricLabel + ' ให้' + actionLabel;
      var condJson = JSON.stringify({
        add_conditions: [{
          metric: metricKey,
          operator: opSymbol,
          value: targetVal,
          label: condLabel,
          instruction: metricLabel + ' ตามเงื่อนไขแล้ว ให้' + actionLabel + 'และแจ้งผู้ใช้',
          auto_switches: condSw
        }]
      });
      return 'ตั้งเงื่อนไขอัตโนมัติเรียบร้อยครับ: <b>' + condLabel + '</b> ระบบจะตรวจสอบค่าทุกวินาทีและทำงานทันทีเมื่อถึงเกณฑ์ครับ <powerbox_cmd>' + condJson + '</powerbox_cmd>';
    }

    // สั่งเปิดไฟ
    if (/(เปิด|สับขึ้น|จ่ายไฟ|turn\s*on)/.test(lower) && /(ไฟ|dc|ac|ทั้งกล่อง|ทั้งหมด|ระบบ)/.test(lower)){
      var hasDc = /dc|12v/.test(lower);
      var hasMaster = /ทั้งกล่อง|ทั้งหมด|ac|220v|หลัก/.test(lower) || !hasDc;
      var swOn = {};
      if (hasMaster) swOn.master = true;
      if (hasDc) {
        swOn.dc = true;
        swOn.master = true; // เปิดไฟทั้งกล่องด้วยเพื่อให้ไฟ DC 12V ติดจริง
      }
      var msgOn = (swOn.master && swOn.dc)
        ? 'เปิดไฟทั้งกล่อง (AC ~220V) และเปิดวงจร DC (~12.4V) เรียบร้อยแล้วครับ'
        : (swOn.dc ? 'เปิดสวิตช์วงจร DC และจ่ายไฟเรียบร้อยครับ' : 'เปิดไฟทั้งกล่องเรียบร้อยแล้วครับ (วงจร AC ทำงาน ~220V)');
      return msgOn + ' <powerbox_cmd>' + JSON.stringify({ set_switches: swOn }) + '</powerbox_cmd>';
    }

    // สั่งปิด/ตัดไฟ
    if (/(ปิด|ตัด|ดับ|สับลง|turn\s*off|off)/.test(lower) && /(ไฟ|dc|ac|ทั้งกล่อง|ทั้งหมด|ระบบ)/.test(lower)){
      var onlyDc = /dc|12v/.test(lower) && !/ทั้งกล่อง|ทั้งหมด|ac|220v/.test(lower);
      var swOff = onlyDc ? { dc: false } : { master: false };
      var msgOff = onlyDc
        ? 'ตัดไฟวงจร DC เรียบร้อยแล้วครับ (ไฟหลักฝั่ง AC ยังทำงานตามปกติ)'
        : 'ตัดไฟทั้งกล่องเรียบร้อยแล้วครับ ทั้งวงจร AC และ DC หยุดจ่ายไฟทั้งหมด (0V, 0A)';
      return msgOff + ' <powerbox_cmd>' + JSON.stringify({ set_switches: swOff }) + '</powerbox_cmd>';
    }

    // ทักทาย / ถามวิธีใช้งาน
    if (/^(สวัสดี|หวัดดี|ดีครับ|ดีค่ะ|hello|hi|ช่วยอะไร|ทำอะไรได้)/.test(lower)){
      return 'สวัสดีครับ! ผมคือสมองกลประจำกล่อง <b>PowerBox</b> สั่งงานผมได้เลย เช่น:<br>' +
        '• <i>"เปิดไฟทั้งหมด"</i> หรือ <i>"ปิดไฟ DC"</i><br>' +
        '• <i>"อีก 10 นาทีตัดไฟทั้งกล่อง"</i><br>' +
        '• <i>"ถ้าแบตต่ำกว่า 75% ให้ตัดไฟ DC"</i><br>' +
        '• <i>"สถานะตอนนี้เป็นอย่างไร"</i>';
    }

    // รายงานสถานะปัจจุบัน
    var schedSummary = schedules.length > 0
      ? ('<br>• ตารางงานอัตโนมัติที่รอทำงาน: <b>' + schedules.length + ' รายการ</b>')
      : '';
    return '<b>สถานะกล่อง PowerBox ปัจจุบัน:</b><br>' +
      '• สวิตช์ไฟทั้งกล่อง: ' + badge(state.master, 'ON (เปิด)', 'OFF (ปิด)') + '<br>' +
      '• วงจร AC: ' + badge(isAcActive(), 'ติด (' + fmtV(state.acV) + ' V)', 'ไม่ติด (0.0 V)') + '<br>' +
      '• วงจร DC: ' + badge(isDcActive(), 'ติด (' + fmtV(state.dcV) + ' V)', state.dc ? 'STANDBY (รอเปิดไฟทั้งกล่อง)' : 'ไม่ติด (0.0 V)') + '<br>' +
      '• กระแสรวม (AC+DC): <span class="mono">' + fmt(state.totalA) + ' A</span><br>' +
      '• แบตเตอรี่คงเหลือ: <span class="mono">' + Math.round(state.bat) + '%</span>' + schedSummary;
  }

  /* ── แชทผู้ช่วย AI ─────────────────────── */
  function pushMsg(role, html, extraClass){
    var wrap = document.createElement('div');
    wrap.className = 'msg ' + role + (extraClass ? ' ' + extraClass : '');
    var avatarText = role === 'ai' ? 'AI' : (role === 'sys' ? '⚡' : 'ฉัน');
    wrap.innerHTML =
      '<div class="avatar" aria-hidden="true">' + avatarText + '</div>' +
      '<div class="bubble">' + html + '</div>';
    chatBody.appendChild(wrap);
    chatBody.scrollTop = chatBody.scrollHeight;
    return wrap;
  }

  function formatMarkdownLite(text){
    return escapeHtml(text)
      .replace(/\*\*(.+?)\*\*/g, '<b>$1</b>')
      .replace(/`([^`]+)`/g, '<span class="mono">$1</span>')
      .replace(/\n/g, '<br>');
  }

  async function aiRespond(userText, isSystemTrigger){
    var t = pushMsg('ai', '<span class="typing"><i></i><i></i><i></i></span>', 'ajax');

    chatHistory.push({ role: 'user', content: userText });
    if (chatHistory.length > 16){
      chatHistory = chatHistory.slice(-16);
    }

    var messages = [{ role: 'system', content: buildSystemPrompt() }].concat(chatHistory);

    try {
      var rawReply = await callAiApi(messages);
      var cleanReply = parseAndExecuteAiCommands(rawReply);
      chatHistory.push({ role: 'assistant', content: cleanReply || rawReply });

      t.querySelector('.bubble').innerHTML = formatMarkdownLite(cleanReply || 'ดำเนินการเรียบร้อยครับ');
      t.classList.remove('ajax');
      if ($('ai-status-text')) $('ai-status-text').textContent = 'ออนไลน์ · เชื่อมต่อ AI จริง';
    } catch (_err){
      var fallbackRaw = localSmartFallback(userText);
      var fallbackClean = parseAndExecuteAiCommands(fallbackRaw);
      chatHistory.push({ role: 'assistant', content: fallbackClean });

      if ($('ai-status-text')) $('ai-status-text').textContent = 'ออนไลน์ · พร้อมสั่งงาน';
      t.querySelector('.bubble').innerHTML = fallbackClean;
      t.classList.remove('ajax');
    }

    chatBody.scrollTop = chatBody.scrollHeight;
    if (!chatOpen){
      unreadCount += 1;
      updateChatWindowUI();
    }
  }

  // ข้อความเปิด
  pushMsg('ai',
    '<b>สวัสดีครับ ผมคือระบบควบคุมอัจฉริยะประจำกล่อง PowerBox</b><br>' +
    'สั่งเปิด/ปิดไฟ ถามค่าแรงดัน–กระแส หรือตั้งเวลา/ตั้งเงื่อนไขล่วงหน้าได้เลย เช่น <i>"อีก 10 นาทีตัดไฟทั้งกล่องให้หน่อย"</i> หรือ <i>"ถ้าแบตต่ำกว่า 75% ให้ตัดไฟ DC"</i> ครับ');

  function sendChat(){
    var input = $('chat-input');
    var text = input.value.trim();
    if (!text) return;
    pushMsg('me', escapeHtml(text));
    input.value = '';
    aiRespond(text, false);
  }
  $('chat-send').addEventListener('click', sendChat);
  $('chat-input').addEventListener('keydown', function(e){
    if (e.key === 'Enter'){ e.preventDefault(); sendChat(); }
  });
  Array.prototype.forEach.call(document.querySelectorAll('.quick-btn'), function(btn){
    btn.addEventListener('click', function(){
      var q = btn.getAttribute('data-q');
      pushMsg('me', escapeHtml(q));
      aiRespond(q, false);
    });
  });

  /* ── หน้าต่างตั้งค่าระบบ (Settings Modal: Wi-Fi & AI API) ── */
  (function initSettingsModal(){
    var backdrop = $('settings-backdrop');
    var openBtn = $('settings-open-btn');
    var closeBtn = $('settings-close-btn');
    var cancelBtn = $('settings-cancel-btn');
    var saveBtn = $('settings-save-btn');
    var msgEl = $('settings-msg');
    var scanBtn = $('wifi-scan-btn');
    var scanList = $('wifi-scan-list');
    var resetApiBtn = $('api-reset-default-btn');
    if (!backdrop || !openBtn) return;

    var tabBtns = backdrop.querySelectorAll('.settings-tab');
    var paneWifi = $('pane-wifi');
    var paneApi = $('pane-api');

    function switchTab(tabName){
      Array.prototype.forEach.call(tabBtns, function(btn){
        var isMatch = btn.getAttribute('data-tab') === tabName;
        btn.classList.toggle('active', isMatch);
        btn.setAttribute('aria-selected', isMatch ? 'true' : 'false');
      });
      if (paneWifi) paneWifi.hidden = (tabName !== 'wifi');
      if (paneApi) paneApi.hidden = (tabName !== 'api');
      setMsg('', false);
    }

    function setMsg(text, isError){
      if (!msgEl) return;
      msgEl.textContent = text || '';
      msgEl.classList.toggle('err', !!isError);
    }

    Array.prototype.forEach.call(tabBtns, function(btn){
      btn.addEventListener('click', function(){
        switchTab(btn.getAttribute('data-tab'));
      });
    });

    function populateSettingsForm(){
      if ($('cfg-wifi-ssid')) $('cfg-wifi-ssid').value = WIFI_CONFIG.ssid || '';
      if ($('cfg-wifi-pass')) $('cfg-wifi-pass').value = WIFI_CONFIG.password || '';
      // ไม่ pre-fill รหัสผ่าน AP เพื่อไม่ให้เก็บ/แสดงรหัสในเครื่องเกินจำเป็น
      if ($('cfg-ap-pass')) $('cfg-ap-pass').value = '';
      applyWifiPanelStatus();

      if ($('cfg-api-url')) $('cfg-api-url').value = AI_CONFIG.AI_BASE_URL || '';
      if ($('cfg-api-key')) $('cfg-api-key').value = AI_CONFIG.API_KEY || '';
      if ($('cfg-api-model')) $('cfg-api-model').value = AI_CONFIG.AI_MODEL || '';
      setMsg('', false);
    }

    function openSettings(){
      populateSettingsForm();
      backdrop.classList.add('open');
      backdrop.setAttribute('aria-hidden', 'false');
      // ดึงสถานะจริงทุกครั้งที่เปิดหน้าต่าง (ไม่ใช้ค่าที่เดาไว้)
      fetchDeviceStatus().then(function(){
        applyWifiPanelStatus();
      });
    }

    function closeSettings(){
      backdrop.classList.remove('open');
      backdrop.setAttribute('aria-hidden', 'true');
      if (scanList) scanList.hidden = true;
    }

    openBtn.addEventListener('click', openSettings);
    if (closeBtn) closeBtn.addEventListener('click', closeSettings);
    if (cancelBtn) cancelBtn.addEventListener('click', closeSettings);

    backdrop.addEventListener('click', function(e){
      if (e.target === backdrop){
        closeSettings();
      }
    });

    document.addEventListener('keydown', function(e){
      if (e.key === 'Escape' && backdrop.classList.contains('open')){
        closeSettings();
      }
    });

    // ปุ่มแสดง/ซ่อนรหัสผ่าน & API Key
    Array.prototype.forEach.call(backdrop.querySelectorAll('[data-toggle-pass]'), function(btn){
      btn.addEventListener('click', function(){
        var targetId = btn.getAttribute('data-toggle-pass');
        var input = $(targetId);
        if (!input) return;
        input.type = input.type === 'password' ? 'text' : 'password';
      });
    });

    // ปุ่มสแกนหา Wi-Fi — ใช้ผลสแกนจริงจาก ESP32 เท่านั้น ไม่มีรายการจำลอง
    if (scanBtn && scanList){
      function showScanMessage(text){
        scanList.hidden = false;
        scanList.innerHTML = '<div class="wifi-scan-empty">' + escapeHtml(text) + '</div>';
      }

      scanBtn.addEventListener('click', async function(){
        if (!isDeviceUi()){
          showScanMessage('สแกนได้เฉพาะเมื่อเปิดหน้านี้จากตัวกล่อง PowerBox');
          return;
        }

        scanBtn.textContent = 'กำลังสแกน…';
        scanBtn.disabled = true;
        scanList.hidden = true;
        scanList.innerHTML = '';

        var networks = null;
        var errorText = '';
        try {
          var res = await fetch('/api/wifi/scan', { cache: 'no-store' });
          if (!res.ok) throw new Error('อุปกรณ์ตอบกลับ HTTP ' + res.status);
          var data = await res.json();
          if (!Array.isArray(data)) throw new Error('ข้อมูลจากอุปกรณ์ไม่ถูกต้อง');
          networks = data;
        } catch (e){
          errorText = (e && e.message) ? e.message : 'เชื่อมต่ออุปกรณ์ไม่ได้';
        }

        scanBtn.textContent = 'สแกนหา Wi-Fi';
        scanBtn.disabled = false;

        if (networks === null){
          showScanMessage('สแกนไม่สำเร็จ — ' + errorText);
          return;
        }
        if (networks.length === 0){
          showScanMessage('ไม่พบเครือข่าย Wi-Fi ในระยะ');
          return;
        }

        networks.sort(function(a, b){ return (b.rssi || -100) - (a.rssi || -100); });
        scanList.hidden = false;
        scanList.innerHTML = networks.map(function(n){
          return (
            '<div class="wifi-net-item" data-ssid="' + escapeHtml(n.ssid) + '">' +
              '<span>📶 <b>' + escapeHtml(n.ssid) + '</b></span>' +
              '<span class="rssi">' + (n.rssi || 0) + ' dBm · ' + escapeHtml(n.sec || '') + ' · เลือก</span>' +
            '</div>'
          );
        }).join('');
      });

      scanList.addEventListener('click', function(e){
        var item = e.target.closest('.wifi-net-item');
        if (!item) return;
        var pickedSsid = item.getAttribute('data-ssid');
        if ($('cfg-wifi-ssid')) $('cfg-wifi-ssid').value = pickedSsid;
        if ($('cfg-wifi-pass')) $('cfg-wifi-pass').focus();
        scanList.hidden = true;
      });
    }

    // ปุ่มคืนค่าเริ่มต้นของ AI API
    if (resetApiBtn){
      resetApiBtn.addEventListener('click', function(){
        if ($('cfg-api-url')) $('cfg-api-url').value = DEFAULT_AI_CONFIG.AI_BASE_URL;
        if ($('cfg-api-key')) $('cfg-api-key').value = DEFAULT_AI_CONFIG.API_KEY;
        if ($('cfg-api-model')) $('cfg-api-model').value = DEFAULT_AI_CONFIG.AI_MODEL;
        setMsg('คืนค่าเริ่มต้น API แล้ว (กดบันทึกเพื่อยืนยัน)', false);
      });
    }

    // บันทึกการตั้งค่าทั้งหมด — บันทึกจริงและรายงานผลจริงเท่านั้น
    if (saveBtn){
      saveBtn.addEventListener('click', async function(){
        var newSsid = $('cfg-wifi-ssid') ? $('cfg-wifi-ssid').value.trim() : '';
        var newPass = $('cfg-wifi-pass') ? $('cfg-wifi-pass').value : '';
        var newApiUrl = ($('cfg-api-url') ? $('cfg-api-url').value.trim() : '') || DEFAULT_AI_CONFIG.AI_BASE_URL;
        var newApiKey = ($('cfg-api-key') ? $('cfg-api-key').value.trim() : '');
        var newApiModel = ($('cfg-api-model') ? $('cfg-api-model').value.trim() : '') || DEFAULT_AI_CONFIG.AI_MODEL;
        var newApPass = $('cfg-ap-pass') ? $('cfg-ap-pass').value : '';

        if (isDeviceUi() && !newSsid){
          setMsg('กรุณากรอกชื่อ Wi-Fi (SSID) ก่อนบันทึก', true);
          eventLog('ยังไม่ได้บันทึก — ไม่มีชื่อ Wi-Fi');
          return;
        }

        if (newApPass && (newApPass.length < 8 || newApPass.length > 63)){
          setMsg('รหัสผ่าน PowerBox_AP ต้องยาว 8–63 ตัวอักษร', true);
          return;
        }

        // เก็บค่าที่ผู้ใช้กรอกไว้ใช้แสดงผลทันที (ค่าจริงยืนยันจากอุปกรณ์อีกที)
        WIFI_CONFIG.ssid = newSsid;
        WIFI_CONFIG.password = newPass;

        AI_CONFIG.AI_BASE_URL = newApiUrl;
        AI_CONFIG.API_KEY = newApiKey;
        AI_CONFIG.AI_MODEL = newApiModel;

        saveBtn.disabled = true;
        setMsg('กำลังบันทึก…', false);

        // ส่งค่าไปยัง ESP32 จริง และตรวจผลลัพธ์
        if (isDeviceUi()){
          try {
            var res = await fetch('/api/settings', {
              method: 'POST',
              headers: { 'Content-Type': 'application/json' },
              body: JSON.stringify({
                wifi: WIFI_CONFIG,
                ai: AI_CONFIG,
                apPassword: newApPass
              })
            });
            var payload = null;
            try { payload = await res.json(); } catch (_e){}
            if (!res.ok || !payload || payload.ok !== true){
              var reason = (payload && payload.error) ? payload.error : ('อุปกรณ์ตอบกลับ HTTP ' + res.status);
              saveBtn.disabled = false;
              setMsg('บันทึกไม่สำเร็จ — ' + reason, true);
              eventLog('บันทึกการตั้งค่าไม่สำเร็จ — ' + reason);
              return;
            }
          } catch (e){
            saveBtn.disabled = false;
            setMsg('บันทึกไม่สำเร็จ — ติดต่ออุปกรณ์ไม่ได้', true);
            eventLog('บันทึกการตั้งค่าไม่สำเร็จ — เชื่อมต่ออุปกรณ์ไม่ได้');
            return;
          }
        } else {
          setMsg('โหมดพรีวิวในเครื่อง — บันทึกเฉพาะค่าในเบราว์เซอร์', false);
        }

        try {
          localStorage.setItem(STORAGE_KEY_SETTINGS, JSON.stringify({
            wifiSsid: WIFI_CONFIG.ssid,
            wifiPass: WIFI_CONFIG.password,
            apiUrl: AI_CONFIG.AI_BASE_URL,
            apiKey: AI_CONFIG.API_KEY,
            apiModel: AI_CONFIG.AI_MODEL
          }));
        } catch (_e){}

        // เปลี่ยนรหัส PowerBox_AP → AP จะรีสตาร์ตหลังจากตอบกลับไปแล้ว
        // (เครื่องนี้จะหลุดจาก AP ต้องต่อใหม่ด้วยรหัสใหม่)
        if (isDeviceUi() && payload && payload.ap_restart){
          saveBtn.disabled = false;
          setMsg('บันทึกแล้ว — เปลี่ยนรหัส PowerBox_AP แล้ว ต้องเชื่อมต่อใหม่', false);
          eventLog('เปลี่ยนรหัส Wi-Fi ของกล่องแล้ว — กรุณาต่อ PowerBox_AP ใหม่');
          setTimeout(function(){
            closeSettings();
          }, 3000);
          return;
        }

        // รอผลการเชื่อมต่อจริงจากอุปกรณ์ (สูงสุด 20 วินาที) ก่อนบอกว่าสำเร็จ
        if (isDeviceUi()){
          setMsg('บันทึกแล้ว — กำลังเชื่อมต่อ Wi-Fi…', false);
          var joined = await waitForWifiJoin(newSsid, 20000);
          saveBtn.disabled = false;

          if (!joined){
            setMsg('บันทึกแล้ว แต่ยังเชื่อมต่อ "' + newSsid + '" ไม่ได้ — ตรวจรหัสผ่าน/สัญญาณ', true);
            eventLog('บันทึกแล้ว แต่เชื่อมต่อ Wi-Fi ไม่สำเร็จ');
            applyWifiPanelStatus();
            return;
          }
        }

        saveBtn.disabled = false;
        applyWifiPanelStatus();
        render();
        eventLog('บันทึกการตั้งค่าแล้ว — Wi-Fi: ' + (DEVICE_STATUS.ssid || WIFI_CONFIG.ssid) +
                 ' · โมเดล AI: ' + AI_CONFIG.AI_MODEL);
        setMsg(isDeviceUi()
          ? '✓ บันทึกและเชื่อมต่อ Wi-Fi สำเร็จ'
          : '✓ บันทึกค่าในเบราว์เซอร์แล้ว (โหมดพรีวิว)', false);
        setTimeout(function(){
          closeSettings();
        }, 1200);
      });
    }
  })();

  // ดึงสถานะจริงจาก ESP32 ทันทีที่โหลดหน้า แล้วรีเฟรชทุก 5 วินาที
  // (render() ที่วิ่งทุก 1 วินาทีจะอ่าน DEVICE_STATUS ไปแสดงผล)
  fetchDeviceStatus().then(function(){ render(); });
  if (isDeviceUi()){
    setInterval(function(){ fetchDeviceStatus(); }, 5000);
  }

})();
