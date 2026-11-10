// AUTO-GENERATED FILE - DO NOT EDIT BY HAND.
// Regenerate with: python tools/embed_assets.py
#pragma once
#include <pgmspace.h>

static const char POWERBOX_HTML[] PROGMEM = R"PBHTML(<!DOCTYPE html>
<html lang="th">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>PowerBox — ควบคุมไฟ</title>
<!-- Fonts: no external requests on purpose. The board's AP has no internet,
     and a blocking Google Fonts request makes the page render blank until it
     times out. IBM Plex is used when installed; system fonts otherwise. -->
<style>
  :root{
    --bg: oklch(0.155 0.008 25);
    --surface: oklch(0.205 0.010 25);
    --surface-2: oklch(0.250 0.012 25);
    --fg: oklch(0.93 0.004 25);
    --muted: oklch(0.66 0.008 25);
    --border: color-mix(in oklch, var(--fg) 9%, transparent);
    --accent: oklch(0.585 0.208 26);
    --accent-dim: color-mix(in oklch, var(--accent) 55%, var(--surface));
    --success: oklch(0.72 0.17 148);
    --warn: oklch(0.78 0.16 75);
    --ok-text: oklch(0.72 0.17 148);
    --font-ui: "IBM Plex Sans Thai", system-ui, sans-serif;
    --font-mono: "IBM Plex Mono", ui-monospace, monospace;
    --radius: 10px;
  }
  *{ margin:0; padding:0; box-sizing:border-box; }
  html{ height:100%; }
  body{
    font-family: var(--font-ui);
    background: var(--bg);
    color: var(--fg);
    min-height:100vh;
    font-size:15px;
    line-height:1.55;
  }
  ::selection{ background: var(--accent-dim); }

  .app{
    max-width: 1240px;
    margin: 0 auto;
    padding: 20px clamp(16px, 3vw, 32px) 32px;
  }

  /* ── Top bar ─────────────────────────── */
  .topbar{
    display:flex; align-items:center; gap:14px;
    padding: 14px 0 18px;
    border-bottom: 1px solid var(--border);
    margin-bottom: 22px;
  }
  .brand{ display:flex; align-items:center; gap:12px; }
  .brand-mark{
    width:38px; height:38px; border-radius:9px;
    background: linear-gradient(160deg, color-mix(in oklch, var(--accent) 26%, var(--surface-2)), var(--surface-2));
    border:1px solid var(--border);
    display:grid; place-items:center;
    color: var(--accent);
    flex:none;
  }
  .brand h1{ font-size:17px; font-weight:600; letter-spacing:0.01em; }
  .brand .sub{ font-size:12px; color:var(--muted); letter-spacing:0.02em; }
  .topbar .spacer{ flex:1; }
  .link-pill{
    display:inline-flex; align-items:center; gap:8px;
    font-size:12.5px; font-weight:500; letter-spacing:0.02em;
    color:var(--muted);
    border:1px solid var(--border);
    border-radius:999px; padding:7px 14px;
    background: var(--surface);
    white-space:nowrap;
  }
  .dot{ width:8px; height:8px; border-radius:50%; background:var(--success); flex:none; }
  .dot.off{ background: var(--muted); }
  .topbar-btn{
    display:inline-flex; align-items:center; gap:7px;
    font-family:inherit; font-size:12.5px; font-weight:500; letter-spacing:0.02em;
    color:var(--fg);
    border:1px solid var(--border);
    border-radius:999px; padding:7px 13px;
    background: var(--surface);
    cursor:pointer;
    transition: background .15s, border-color .15s, color .15s;
  }
  .topbar-btn svg{ color: var(--muted); transition: color .15s, transform .25s; }
  .topbar-btn:hover{
    background: var(--surface-2);
    border-color: color-mix(in oklch, var(--accent) 45%, var(--border));
  }
  .topbar-btn:hover svg{ color: var(--accent); transform: rotate(25deg); }
  .topbar-btn:focus-visible{ outline:2px solid var(--accent); outline-offset:2px; }
  @media (max-width: 520px){
    .topbar-btn span{ display:none; }
    .topbar-btn{ padding:8px; border-radius:9px; }
  }

  /* ── Settings Modal ─────────────────── */
  .modal-backdrop{
    position:fixed; inset:0; z-index:980;
    background: oklch(0 0 0 / 0.65);
    backdrop-filter: blur(8px);
    -webkit-backdrop-filter: blur(8px);
    display:grid; place-items:center;
    padding: 16px;
    opacity:0; visibility:hidden; pointer-events:none;
    transition: opacity .2s ease, visibility .2s ease;
  }
  .modal-backdrop.open{
    opacity:1; visibility:visible; pointer-events:auto;
  }
  .settings-modal{
    width: min(520px, 100%);
    max-height: calc(100vh - 32px);
    display:flex; flex-direction:column;
    background: var(--surface);
    border:1px solid color-mix(in oklch, var(--accent) 35%, var(--border));
    border-radius: 14px;
    box-shadow: 0 24px 64px oklch(0 0 0 / 0.7);
    transform: translateY(12px) scale(0.97);
    transition: transform .2s cubic-bezier(.2,.8,.2,1);
    overflow:hidden;
  }
  .modal-backdrop.open .settings-modal{
    transform: translateY(0) scale(1);
  }
  .settings-head{
    display:flex; align-items:center; justify-content:space-between; gap:12px;
    padding:15px 18px;
    border-bottom:1px solid var(--border);
    background: var(--surface-2);
  }
  .settings-head-title{
    display:flex; align-items:center; gap:9px;
    font-size:15px; font-weight:600;
  }
  .settings-head-title svg{ color: var(--accent); }
  .settings-tabs{
    display:grid; grid-template-columns:1fr 1fr; gap:6px;
    padding:10px 18px 0;
    border-bottom:1px solid var(--border);
    background: color-mix(in oklch, var(--surface-2) 55%, var(--surface));
  }
  .settings-tab{
    display:inline-flex; align-items:center; justify-content:center; gap:7px;
    font-family:inherit; font-size:13.5px; font-weight:600;
    color: var(--muted);
    background: transparent;
    border:none; border-bottom:2px solid transparent;
    padding:9px 12px 11px;
    cursor:pointer;
    transition: color .15s, border-color .15s;
  }
  .settings-tab:hover{ color: var(--fg); }
  .settings-tab.active{
    color: var(--accent);
    border-bottom-color: var(--accent);
  }
  .settings-body{
    padding:18px;
    overflow-y:auto;
    display:flex; flex-direction:column; gap:14px;
  }
  .settings-pane[hidden]{ display:none; }
  .form-group{
    display:flex; flex-direction:column; gap:6px;
    margin-bottom:13px;
  }
  .form-group:last-child{ margin-bottom:0; }
  .form-label{
    display:flex; align-items:center; justify-content:space-between; gap:8px;
    font-size:12.5px; font-weight:600; letter-spacing:0.02em;
    color: var(--muted);
  }
  .form-hint{
    font-size:11.5px; color: var(--muted);
  }
  .input-wrap{
    position:relative; display:flex; align-items:center;
  }
  .form-input{
    width:100%;
    font-family: inherit; font-size:14px;
    color: var(--fg);
    background: var(--bg);
    border:1px solid var(--border);
    border-radius:8px;
    padding:10px 12px;
    outline:none;
    transition: border-color .15s;
  }
  .form-input.mono{ font-family: var(--font-mono); font-size:13px; }
  .form-input.has-btn{ padding-right:42px; }
  .form-input:focus{ border-color: color-mix(in oklch, var(--accent) 60%, var(--border)); }
  .eye-btn{
    position:absolute; right:6px;
    width:30px; height:30px; border-radius:6px;
    border:none; background:transparent; color:var(--muted);
    display:grid; place-items:center; cursor:pointer;
  }
  .eye-btn:hover{ color:var(--fg); background:var(--surface-2); }
  .wifi-status-bar{
    display:flex; align-items:center; justify-content:space-between; gap:10px;
    padding:10px 12px;
    border-radius:8px;
    background: color-mix(in oklch, var(--surface-2) 65%, var(--bg));
    border:1px solid var(--border);
    font-size:12.5px;
    margin-bottom:13px;
  }
  .wifi-scan-list{
    display:flex; flex-direction:column; gap:6px;
    margin-top:6px;
    max-height:130px; overflow-y:auto;
  }
  .wifi-scan-list[hidden]{ display:none; }
  .wifi-net-item{
    display:flex; align-items:center; justify-content:space-between;
    padding:8px 11px; border-radius:7px;
    background: var(--bg); border:1px solid var(--border);
    font-size:13px; cursor:pointer;
    transition: border-color .15s, background .15s;
  }
  .wifi-net-item:hover{
    border-color: color-mix(in oklch, var(--accent) 45%, var(--border));
    background: var(--surface-2);
  }
  .wifi-net-item .rssi{ font-family:var(--font-mono); font-size:11.5px; color:var(--muted); }
  .wifi-scan-empty{
    padding:10px 12px; border-radius:7px;
    border:1px dashed var(--border);
    background: var(--bg);
    font-size:12.5px; color: var(--muted);
  }
  .btn-mini{
    font-family:inherit; font-size:11.5px; font-weight:500;
    padding:4px 9px; border-radius:6px;
    border:1px solid var(--border); background:var(--surface-2);
    color:var(--fg); cursor:pointer;
    transition: border-color .15s, background .15s;
  }
  .btn-mini:hover{ border-color: color-mix(in oklch, var(--accent) 50%, var(--border)); }
  .settings-foot{
    display:flex; align-items:center; justify-content:space-between; gap:10px;
    padding:14px 18px;
    border-top:1px solid var(--border);
    background: var(--surface-2);
  }
  .settings-msg{
    font-size:12.5px; color: var(--ok-text);
    min-height:18px;
  }
  .settings-msg.err{ color: var(--accent); }
  .settings-actions{ display:flex; align-items:center; gap:8px; }
  .btn-secondary, .btn-primary{
    font-family:inherit; font-size:13.5px; font-weight:600;
    padding:9px 15px; border-radius:8px; cursor:pointer;
    transition: background .15s, border-color .15s, transform .06s;
  }
  .btn-secondary{
    background: var(--bg); color: var(--muted);
    border:1px solid var(--border);
  }
  .btn-secondary:hover{ color: var(--fg); border-color: color-mix(in oklch, var(--fg) 22%, transparent); }
  .btn-primary{
    background: color-mix(in oklch, var(--accent) 85%, var(--bg));
    color: oklch(0.15 0.02 26);
    border:1px solid color-mix(in oklch, var(--accent) 70%, var(--bg));
  }
  .btn-primary:hover{ background: color-mix(in oklch, var(--accent) 95%, var(--bg)); }
  .btn-primary:active, .btn-secondary:active{ transform: translateY(1px); }

  /* ── Layout ─────────────────────────── */
  .layout{
    display:grid;
    grid-template-columns: 1fr;
    gap:18px;
    align-items:start;
    position:relative;
  }

  .panel{
    background: var(--surface);
    border:1px solid var(--border);
    border-radius: var(--radius);
  }
  .panel-head{
    display:flex; align-items:center; justify-content:space-between; gap:12px;
    padding:16px 20px;
    border-bottom:1px solid var(--border);
  }
  .panel-head h2{ font-size:13px; font-weight:600; letter-spacing:0.06em; text-transform:uppercase; color:var(--muted); }
  .panel-body{ padding:20px; }

  /* ── Controls ───────────────────────── */
  .controls{ display:grid; grid-template-columns:1fr 1fr; gap:18px; }
  @media (max-width: 560px){ .controls{ grid-template-columns:1fr; } }

  .switch-card{
    background: var(--surface-2);
    border:1px solid var(--border);
    border-radius: var(--radius);
    padding:18px 18px 20px;
    display:flex; flex-direction:column; gap:14px;
    transition: border-color .25s;
  }
  .switch-card.live{ border-color: color-mix(in oklch, var(--accent) 45%, var(--border)); }
  .switch-card .row-top{ display:flex; align-items:flex-start; gap:12px; }
  .switch-card .icon-box{
    width:40px; height:40px; border-radius:10px; flex:none;
    background: color-mix(in oklch, var(--accent) 10%, var(--bg));
    border:1px solid var(--border);
    display:grid; place-items:center;
    color: var(--muted);
    transition: color .25s, background .25s;
  }
  .switch-card.live .icon-box{ color: var(--accent); background: color-mix(in oklch, var(--accent) 18%, var(--bg)); }
  .switch-card h3{ font-size:15.5px; font-weight:600; letter-spacing:0.01em; }
  .switch-card .desc{ font-size:12.5px; color:var(--muted); margin-top:2px; }
  .switch-card .state-chip{
    margin-left:auto;
    font-family: var(--font-mono);
    font-size:11.5px; font-weight:500; letter-spacing:0.06em;
    padding:4px 10px; border-radius:5px;
    background: var(--bg); border:1px solid var(--border);
    color: var(--muted);
  }
  .switch-card.live .state-chip{ color:var(--ok-text); border-color: color-mix(in oklch, var(--success) 40%, var(--border)); }

  .toggle-btn{
    min-height:52px; border-radius:9px; cursor:pointer;
    font-family: inherit; font-size:15px; font-weight:600; letter-spacing:0.02em;
    color: var(--fg);
    background: var(--bg);
    border:1px solid var(--border);
    transition: background .15s, border-color .15s, transform .06s;
  }
  .toggle-btn:hover{ background: var(--surface-2); border-color: color-mix(in oklch, var(--fg) 18%, transparent); }
  .toggle-btn:active{ transform: translateY(1px); }
  .toggle-btn:focus-visible{ outline:2px solid var(--accent); outline-offset:2px; }
  .toggle-btn.on{
    background: color-mix(in oklch, var(--accent) 85%, var(--bg));
    border-color: color-mix(in oklch, var(--accent) 70%, var(--bg));
    color: oklch(0.15 0.02 26);
  }
  .toggle-btn.on:hover{ background: color-mix(in oklch, var(--accent) 95%, var(--bg)); }

  /* ── Metrics ────────────────────────── */
  .metrics{
    display:grid;
    grid-template-columns: repeat(4, 1fr);
    gap:14px;
    margin-top:18px;
  }
  @media (max-width: 1180px){ .metrics{ grid-template-columns: repeat(2, 1fr); } }
  @media (max-width: 560px){ .metrics{ gap:10px; } }

  .metric{
    background: var(--surface);
    border:1px solid var(--border);
    border-radius: var(--radius);
    padding:14px 16px 15px;
    min-width:0;
  }
  .metric .m-label{
    font-size:11.5px; font-weight:500;
    letter-spacing:0.06em; text-transform:uppercase;
    color:var(--muted);
  }
  .metric .m-value{
    font-family: var(--font-mono);
    font-variant-numeric: tabular-nums;
    font-size: clamp(24px, 3vw, 30px);
    font-weight:600;
    letter-spacing:-0.01em;
    line-height:1.15;
    margin-top:6px;
    color: var(--fg);
    white-space:nowrap;
    display:flex; align-items:baseline; gap:5px;
  }
  .metric .m-value .unit{ font-size:13px; font-weight:400; color:var(--muted); }
  .metric .m-foot{ font-size:11.5px; color:var(--muted); margin-top:4px; letter-spacing:0.01em; }
  .metric.alert{ border-color: color-mix(in oklch, var(--accent) 55%, var(--border)); }
  .metric.alert .m-value{ color: var(--accent); }
  .metric.idle .m-value{ color: color-mix(in oklch, var(--fg) 45%, var(--bg)); }

  /* battery bar */
  .bat-track{
    height:6px; border-radius:3px; background: var(--bg);
    margin-top:10px; overflow:hidden;
    border:1px solid var(--border);
  }
  .bat-fill{ height:100%; border-radius:3px; background: var(--ok-text); width:0%; transition: width .8s; }
  .bat-fill.warn{ background: var(--warn); }
  .bat-fill.low{ background: var(--accent); }

  /* ── Log strip & Schedules ──────────── */
  .logstrip{
    margin-top:18px;
    display:flex; align-items:center; gap:10px; flex-wrap:wrap;
    font-family: var(--font-mono);
    font-size:12px; letter-spacing:0.01em;
    color:var(--muted);
  }
  .logstrip .tag{
    font-size:10.5px; letter-spacing:0.08em;
    color: color-mix(in oklch, var(--accent) 80%, var(--fg));
    text-transform:uppercase;
  }
  #event-log{ min-height:18px; }

  .schedule-box{
    margin-top:16px;
    padding:12px 14px;
    border-radius: var(--radius);
    background: color-mix(in oklch, var(--surface-2) 65%, var(--bg));
    border:1px solid var(--border);
  }
  .schedule-box[hidden]{ display:none; }
  .schedule-head{
    display:flex; align-items:center; justify-content:space-between; gap:8px;
    font-size:11.5px; font-weight:600; letter-spacing:0.05em; text-transform:uppercase;
    color: var(--muted);
    margin-bottom:8px;
  }
  .schedule-list{
    display:flex; flex-direction:column; gap:8px;
  }
  .schedule-item{
    display:flex; align-items:center; justify-content:space-between; gap:10px; flex-wrap:wrap;
    padding:8px 12px;
    border-radius:8px;
    background: var(--bg);
    border:1px solid color-mix(in oklch, var(--accent) 30%, var(--border));
    font-size:13px;
  }
  .schedule-item .s-info{
    display:flex; align-items:center; gap:8px; flex-wrap:wrap;
  }
  .schedule-item .s-timer{
    font-family: var(--font-mono);
    font-size:12px;
    font-weight:600;
    color: var(--accent);
    background: color-mix(in oklch, var(--accent) 14%, var(--surface));
    padding:2px 8px;
    border-radius:5px;
  }
  .schedule-item .s-actions{
    display:flex; align-items:center; gap:6px;
  }
  .schedule-item button{
    font-family: inherit;
    font-size:11.5px;
    padding:4px 9px;
    border-radius:6px;
    border:1px solid var(--border);
    background: var(--surface-2);
    color: var(--muted);
    cursor:pointer;
    transition: color .15s, border-color .15s;
  }
  .schedule-item button:hover{
    color: var(--fg);
    border-color: color-mix(in oklch, var(--fg) 25%, transparent);
  }
  .msg.sys .avatar{
    background: color-mix(in oklch, var(--warn) 18%, var(--bg));
    color: var(--warn);
    border:1px solid color-mix(in oklch, var(--warn) 35%, var(--border));
  }
  .msg.sys .bubble{
    border-color: color-mix(in oklch, var(--warn) 35%, var(--border));
    font-size:12.5px;
    color: var(--muted);
  }

  /* ── Chat Floating Window & Launcher ── */
  .chat-fab{
    position:fixed;
    right: clamp(18px, 3vw, 32px);
    bottom: clamp(18px, 3vw, 28px);
    z-index: 950;
    width: 54px;
    height: 54px;
    border-radius: 50%;
    display: grid;
    place-items: center;
    border: 1px solid color-mix(in oklch, var(--accent) 65%, var(--border));
    background: linear-gradient(145deg, color-mix(in oklch, var(--accent) 90%, var(--bg)), color-mix(in oklch, var(--accent) 72%, var(--bg)));
    color: oklch(0.15 0.02 26);
    cursor: pointer;
    box-shadow: 0 12px 32px oklch(0 0 0 / 0.5), 0 0 24px color-mix(in oklch, var(--accent) 30%, transparent);
    transition: transform .2s cubic-bezier(.2,.8,.2,1), box-shadow .2s, background .15s;
  }
  .chat-fab:hover{
    transform: translateY(-2px);
    box-shadow: 0 16px 38px oklch(0 0 0 / 0.6), 0 0 30px color-mix(in oklch, var(--accent) 45%, transparent);
  }
  .chat-fab:active{ transform: translateY(0); }
  .chat-fab.active{
    background: color-mix(in oklch, var(--accent) 75%, var(--bg));
    color: oklch(0.15 0.02 26);
    border-color: color-mix(in oklch, var(--accent) 85%, var(--border));
  }
  .chat-badge{
    position: absolute;
    top: -4px;
    right: -4px;
    display: inline-grid;
    place-items: center;
    min-width: 20px;
    height: 20px;
    padding: 0 6px;
    border-radius: 999px;
    background: var(--bg);
    border: 1px solid color-mix(in oklch, var(--accent) 65%, var(--border));
    color: var(--fg);
    font-family: var(--font-mono);
    font-size: 11px;
    font-weight: 600;
  }
  .chat-badge[hidden]{ display:none; }

  .chat-panel{
    position: fixed;
    right: clamp(16px, 3vw, 32px);
    bottom: 84px;
    width: min(420px, calc(100vw - 32px));
    height: min(560px, calc(100vh - 120px));
    z-index: 940;
    display: flex;
    flex-direction: column;
    background: color-mix(in oklch, var(--surface) 92%, transparent);
    backdrop-filter: blur(18px);
    -webkit-backdrop-filter: blur(18px);
    border: 1px solid color-mix(in oklch, var(--accent) 35%, var(--border));
    border-radius: 14px;
    box-shadow: 0 24px 64px oklch(0 0 0 / 0.65), 0 0 0 1px oklch(1 0 0 / 0.04);
    opacity: 0;
    visibility: hidden;
    pointer-events: none;
    transform: translateY(18px) scale(0.96);
    transform-origin: bottom right;
    transition: opacity .22s cubic-bezier(.2,.8,.2,1), transform .22s cubic-bezier(.2,.8,.2,1), visibility .22s;
  }
  .chat-panel.open{
    opacity: 1;
    visibility: visible;
    pointer-events: auto;
    transform: translateY(0) scale(1);
  }
  .chat-panel.dragging{
    transition: none;
    user-select: none;
  }
  .chat-panel .panel-head{
    cursor: grab;
    user-select: none;
    background: color-mix(in oklch, var(--surface-2) 70%, transparent);
    border-top-left-radius: 13px;
    border-top-right-radius: 13px;
    padding: 13px 16px;
  }
  .chat-panel.dragging .panel-head{ cursor: grabbing; }
  .chat-head-left{
    display:flex; align-items:center; gap:10px;
  }
  .chat-head-left .ai-dot{
    width:9px; height:9px; border-radius:50%;
    background: var(--accent);
    box-shadow: 0 0 10px var(--accent);
  }
  .chat-head-actions{
    display:flex; align-items:center; gap:6px;
  }
  .chat-icon-btn{
    width:30px; height:30px; border-radius:7px;
    border:1px solid var(--border);
    background: var(--bg);
    color: var(--muted);
    display:grid; place-items:center;
    cursor:pointer;
    transition: color .15s, border-color .15s, background .15s;
  }
  .chat-icon-btn:hover{
    color: var(--fg);
    background: var(--surface-2);
    border-color: color-mix(in oklch, var(--fg) 22%, transparent);
  }
  .chat-body{
    flex:1; overflow-y:auto;
    padding:16px;
    display:flex; flex-direction:column; gap:12px;
    scrollbar-width: thin;
    scrollbar-color: color-mix(in oklch, var(--accent) 45%, var(--surface-2)) transparent;
  }
  .chat-body::-webkit-scrollbar,
  .schedule-list::-webkit-scrollbar{
    width: 6px;
    height: 6px;
  }
  .chat-body::-webkit-scrollbar-track,
  .schedule-list::-webkit-scrollbar-track{
    background: transparent;
    margin: 6px 0;
  }
  .chat-body::-webkit-scrollbar-thumb,
  .schedule-list::-webkit-scrollbar-thumb{
    background: color-mix(in oklch, var(--fg) 16%, var(--surface-2));
    border-radius: 999px;
    border: 1px solid var(--border);
  }
  .chat-body::-webkit-scrollbar-thumb:hover,
  .schedule-list::-webkit-scrollbar-thumb:hover{
    background: color-mix(in oklch, var(--accent) 65%, var(--surface-2));
    border-color: color-mix(in oklch, var(--accent) 45%, var(--border));
  }
  .chat-body::-webkit-scrollbar-thumb:active,
  .schedule-list::-webkit-scrollbar-thumb:active{
    background: var(--accent);
  }
  .msg{ display:flex; gap:10px; max-width:88%; }
  .msg .avatar{
    width:28px; height:28px; border-radius:8px; flex:none;
    display:grid; place-items:center;
    font-size:11px; font-weight:600;
    margin-top:2px;
  }
  .msg.ai .avatar{ background: color-mix(in oklch, var(--accent) 16%, var(--bg)); color: var(--accent); border:1px solid var(--border); }
  .msg.me{ align-self:flex-end; flex-direction:row-reverse; }
  .msg.me .avatar{ background: var(--surface-2); color: var(--muted); border:1px solid var(--border); }
  .msg .bubble{
    padding:10px 14px;
    border-radius:12px;
    font-size:14px; line-height:1.55;
    background: var(--surface-2);
    border:1px solid var(--border);
  }
  .msg.ai .bubble{ border-top-left-radius:4px; }
  .msg.me .bubble{
    border-top-right-radius:4px;
    background: color-mix(in oklch, var(--accent) 16%, var(--surface-2));
    border-color: color-mix(in oklch, var(--accent) 30%, var(--border));
  }
  .msg .bubble b{ font-weight:600; }
  .msg .bubble .mono{ font-family: var(--font-mono); font-size:13px; }
  .msg.ajax{ opacity:0.55; }
  .msg .bubble .typing{ display:inline-flex; gap:4px; align-items:center; height:16px; }
  .msg .bubble .typing i{
    width:5px; height:5px; border-radius:50%; background:var(--muted);
    animation: blink 1.2s infinite;
  }
  .msg .bubble .typing i:nth-child(2){ animation-delay:.2s; }
  .msg .bubble .typing i:nth-child(3){ animation-delay:.4s; }
  @keyframes blink{ 0%,60%,100%{ opacity:.25 } 30%{ opacity:1 } }

  .chat-quick{
    display:flex; gap:8px; flex-wrap:wrap;
    padding: 0 16px 12px;
  }
  .quick-btn{
    font-family:inherit; font-size:12.5px; font-weight:500; letter-spacing:0.01em;
    color: var(--muted);
    background: var(--bg);
    border:1px solid var(--border);
    border-radius:999px;
    padding:7px 13px; cursor:pointer;
    transition: color .15s, border-color .15s;
  }
  .quick-btn:hover{ color:var(--fg); border-color: color-mix(in oklch, var(--fg) 25%, transparent); }

  .chat-input-row{
    display:flex; gap:10px;
    padding:14px 16px;
    border-top:1px solid var(--border);
  }
  .chat-input-row input{
    flex:1; min-width:0;
    font-family:inherit; font-size:14.5px;
    color: var(--fg);
    background: var(--bg);
    border:1px solid var(--border);
    border-radius:9px;
    padding:12px 14px;
    outline:none;
    transition: border-color .15s;
  }
  .chat-input-row input::placeholder{ color: color-mix(in oklch, var(--muted) 75%, transparent); }
  .chat-input-row input:focus{ border-color: color-mix(in oklch, var(--accent) 55%, var(--border)); }
  .chat-input-row button{
    flex:none;
    min-width:52px; min-height:52px;
    border-radius:9px; border:none; cursor:pointer;
    background: color-mix(in oklch, var(--accent) 85%, var(--bg));
    color: oklch(0.15 0.02 26);
    display:grid; place-items:center;
    transition: background .15s, transform .06s;
  }
  .chat-input-row button:hover{ background: color-mix(in oklch, var(--accent) 95%, var(--bg)); }
  .chat-input-row button:active{ transform:translateY(1px); }
  .chat-input-row button:focus-visible{ outline:2px solid var(--fg); outline-offset:2px; }

  footer{
    margin-top:26px; padding-top:16px;
    border-top:1px solid var(--border);
    display:flex; justify-content:space-between; gap:12px; flex-wrap:wrap;
    font-size:12px; color:var(--muted); letter-spacing:0.01em;
  }
</style>
</head>
<body>
<div class="app">

  <header class="topbar" data-od-id="topbar">
    <div class="brand">
      <div class="brand-mark" aria-hidden="true">
        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linejoin="round" stroke-linecap="round">
          <path d="M13 2 4 14h5l-1 8 9-12h-5z"/>
        </svg>
      </div>
      <div>
        <h1>PowerBox</h1>
        <div class="sub">แผงควบคุมและมอนิเตอร์ระบบไฟ</div>
      </div>
    </div>
    <div class="spacer"></div>
    <div class="link-pill" data-od-id="device-connection" id="connection-pill">
      <span class="dot" id="conn-dot"></span>
      <span id="conn-text">เชื่อมต่อกับกล่องแล้ว · ออนไลน์</span>
    </div>
    <button class="topbar-btn" id="settings-open-btn" type="button" data-od-id="settings-open-btn" aria-haspopup="dialog" aria-controls="settings-backdrop" title="ตั้งค่า Wi-Fi และ AI API">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
        <circle cx="12" cy="12" r="3"/>
        <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/>
      </svg>
      <span>ตั้งค่า</span>
    </button>
  </header>

  <main class="layout">

    <!-- ── คอลัมน์ซ้าย: ควบคุมไฟ + ค่าสถานะ ── -->
    <section class="left-col" data-od-id="control-and-status">

      <div class="panel" data-od-id="light-control-panel">
        <div class="panel-head">
          <h2>ควบคุมไฟ</h2>
          <span class="logstrip" style="margin:0">
            <span id="circuit-summary">ไฟปิดอยู่ทั้งหมด</span>
          </span>
        </div>
        <div class="panel-body">
          <div class="controls">

            <div class="switch-card" data-od-id="master-light-card" id="master-card">
              <div class="row-top">
                <div class="icon-box" aria-hidden="true">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                    <rect x="4" y="7" width="16" height="10" rx="2"/>
                    <path d="M9 7V5a3 3 0 0 1 6 0v2M8 12h2m4 0h2"/>
                  </svg>
                </div>
                <div>
                  <h3>ไฟทั้งกล่อง</h3>
                  <div class="desc">เปิด: AC ติด และดูว่า DC เปิดหรือปิด — ปิด: AC และ DC ไม่ติด</div>
                </div>
                <span class="state-chip" id="master-chip">OFF</span>
              </div>
              <button class="toggle-btn" id="master-btn" data-od-id="master-light-toggle">เปิดไฟทั้งกล่อง</button>
            </div>

            <div class="switch-card" data-od-id="dc-light-card" id="dc-card">
              <div class="row-top">
                <div class="icon-box" aria-hidden="true">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                    <path d="M12 3v3m0 12v3m-5.5-9H4m16 0h-2.5M7 7l2 2m6 6 2 2M7 17l2-2m6-6 2-2"/>
                    <circle cx="12" cy="12" r="3.5"/>
                  </svg>
                </div>
                <div>
                  <h3>ไฟวงจร DC</h3>
                  <div class="desc">สวิตช์วงจร 12V — จะติดเมื่อเปิดไฟทั้งกล่องและเปิดสวิตช์ DC</div>
                </div>
                <span class="state-chip" id="dc-chip">OFF</span>
              </div>
              <button class="toggle-btn" id="dc-btn" data-od-id="dc-light-toggle">เปิดไฟวงจร DC</button>
            </div>

          </div>

          <!-- ค่าสถานะ -->
          <div class="metrics" data-od-id="metrics-grid">
            <div class="metric" data-od-id="metric-ac-voltage">
              <div class="m-label">แรงดัน AC</div>
              <div class="m-value"><span id="ac-volt">0.0</span><span class="unit">V</span></div>
              <div class="m-foot" id="ac-foot">วงจร AC ปิดอยู่</div>
            </div>
            <div class="metric" data-od-id="metric-dc-voltage">
              <div class="m-label">แรงดัน DC</div>
              <div class="m-value"><span id="dc-volt">0.0</span><span class="unit">V</span></div>
              <div class="m-foot" id="dc-foot">วงจร DC ปิดอยู่</div>
            </div>
            <div class="metric" data-od-id="metric-total-current">
              <div class="m-label">กระแสรวม (AC + DC)</div>
              <div class="m-value"><span id="total-val">0.00</span><span class="unit">A</span></div>
              <div class="m-foot" id="total-foot">ไม่มีโหลดในขณะนี้</div>
            </div>
            <div class="metric" data-od-id="metric-battery">
              <div class="m-label">แบตเตอรี่</div>
              <div class="m-value"><span id="bat-val">--</span><span class="unit">%</span></div>
              <div class="bat-track" aria-hidden="true"><div class="bat-fill" id="bat-fill"></div></div>
              <div class="m-foot" id="bat-foot">อ่านค่ารอบแรก…</div>
            </div>
          </div>

          <!-- รายการตั้งเวลาและเงื่อนไขที่ AI สร้างไว้ -->
          <div class="schedule-box" id="schedule-box" hidden data-od-id="ai-schedule-box">
            <div class="schedule-head">
              <span>ตารางงาน / เงื่อนไขอัตโนมัติของ AI</span>
              <span id="schedule-count">0 รายการ</span>
            </div>
            <div class="schedule-list" id="schedule-list"></div>
          </div>

          <div class="logstrip" data-od-id="event-log-strip">
            <span class="tag">บันทึก</span>
            <span id="event-log">พร้อมใช้งาน</span>
          </div>
        </div>
      </div>

    </section>

    <!-- ── หน้าต่างแชทผู้ช่วย AI (แบบลอย ซ้อนทับส่วนควบคุม เปิด/ปิดและลากย้ายได้) ── -->
    <aside class="panel chat-panel" id="chat-panel" data-od-id="ai-chat-panel" aria-hidden="true">
      <div class="panel-head" id="chat-drag-handle" title="ลากเพื่อย้ายตำแหน่งหน้าต่าง">
        <div class="chat-head-left">
          <span class="ai-dot" aria-hidden="true"></span>
          <h2>ผู้ช่วย AI</h2>
          <span class="logstrip" style="margin:0"><span id="ai-status-text">ลากย้ายได้</span></span>
        </div>
        <div class="chat-head-actions">
          <button class="chat-icon-btn" id="chat-close-btn" type="button" title="ย่อ/ปิดหน้าต่างแชท (Esc)" aria-label="ปิดหน้าต่างแชท">
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
              <path d="M18 6 6 18M6 6l12 12"/>
            </svg>
          </button>
        </div>
      </div>
      <div class="chat-body" id="chat-body" data-od-id="chat-message-list"></div>
      <div class="chat-quick" data-od-id="chat-quick-prompts">
        <button class="quick-btn" data-q="สถานะตอนนี้เป็นอย่างไร">สถานะตอนนี้</button>
        <button class="quick-btn" data-q="เปิดไฟทั้งกล่องและไฟ DC ให้หน่อย">เปิดไฟทั้งหมด</button>
        <button class="quick-btn" data-q="อีก 10 นาทีตัดไฟทั้งกล่องให้หน่อย">อีก 10 นาทีตัดไฟ</button>
        <button class="quick-btn" data-q="ถ้าแบตต่ำกว่า 75% ให้ตัดไฟ DC ให้หน่อย">ตั้งเงื่อนไขแบต &lt; 75%</button>
        <button class="quick-btn" data-q="ยกเลิกตารางงานทั้งหมดให้หน่อย">ยกเลิกตารางงาน</button>
      </div>
      <div class="chat-input-row" data-od-id="chat-input-row">
        <input type="text" id="chat-input" placeholder="พิมพ์คำถาม หรือสั่งเปิด/ปิดไฟได้เลย…" aria-label="ข้อความถึงผู้ช่วย AI" autocomplete="off">
        <button id="chat-send" data-od-id="chat-send-btn" aria-label="ส่งข้อความ">
          <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M5 12h14M13 6l6 6-6 6"/>
          </svg>
        </button>
      </div>
    </aside>

    <!-- ปุ่มลอยเปิด/ปิดหน้าต่างแชท (ไอคอนอย่างเดียว) -->
    <button class="chat-fab" id="chat-fab" type="button" data-od-id="chat-fab-btn" aria-label="แชทผู้ช่วย AI" aria-expanded="false" aria-controls="chat-panel">
      <svg id="chat-fab-icon" width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.1" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
        <path d="M21 15a2 2 0 0 1-2 2H7l-4 4V5a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2z"/>
      </svg>
      <span class="chat-badge" id="chat-badge">1</span>
    </button>

  </main>

  <!-- ── หน้าต่างตั้งค่า (Settings Modal: Wi-Fi & AI API) ── -->
  <div class="modal-backdrop" id="settings-backdrop" data-od-id="settings-modal" aria-hidden="true" role="dialog" aria-modal="true" aria-labelledby="settings-modal-title">
    <div class="settings-modal">
      <div class="settings-head">
        <div class="settings-head-title" id="settings-modal-title">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <circle cx="12" cy="12" r="3"/>
            <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/>
          </svg>
          <span>ตั้งค่าระบบ PowerBox</span>
        </div>
        <button class="chat-icon-btn" id="settings-close-btn" type="button" title="ปิดหน้าต่าง (Esc)" aria-label="ปิดหน้าต่างตั้งค่า">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M18 6 6 18M6 6l12 12"/>
          </svg>
        </button>
      </div>

      <div class="settings-tabs" role="tablist">
        <button class="settings-tab active" type="button" role="tab" aria-selected="true" data-tab="wifi" id="tab-btn-wifi">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="M5 12.55a11 11 0 0 1 14.08 0"/>
            <path d="M1.42 9a16 16 0 0 1 21.16 0"/>
            <path d="M8.53 16.11a6 6 0 0 1 6.95 0"/>
            <line x1="12" y1="20" x2="12.01" y2="20"/>
          </svg>
          <span>ตั้งค่า Wi-Fi</span>
        </button>
        <button class="settings-tab" type="button" role="tab" aria-selected="false" data-tab="api" id="tab-btn-api">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="m21 2-2 2m-7.61 7.61a5.5 5.5 0 1 1-7.778 7.778 5.5 5.5 0 0 1 7.777-7.777zm0 0L15.5 7.5m0 0 3 3L22 7l-3-3m-3.5 3.5L19 4"/>
          </svg>
          <span>ตั้งค่า AI API</span>
        </button>
      </div>

      <div class="settings-body">
        <!-- แท็บ 1: ตั้งค่า Wi-Fi -->
        <div class="settings-pane" id="pane-wifi" role="tabpanel">
          <div class="wifi-status-bar">
            <div style="display:flex; align-items:center; gap:8px;">
              <span class="dot off" id="wifi-status-dot"></span>
              <span id="wifi-status-label">สถานะ Wi-Fi: <b class="mono" id="wifi-cur-ssid">กำลังตรวจสอบ…</b></span>
            </div>
            <button type="button" class="btn-mini" id="wifi-scan-btn">สแกนหา Wi-Fi</button>
          </div>

          <div class="wifi-scan-list" id="wifi-scan-list" hidden></div>

          <div class="form-group">
            <label class="form-label" for="cfg-wifi-ssid">
              <span>ชื่อเครือข่าย Wi-Fi (SSID)</span>
              <span class="form-hint">รองรับ 2.4 GHz สำหรับ ESP32</span>
            </label>
            <input type="text" class="form-input" id="cfg-wifi-ssid" placeholder="เช่น Home_WiFi_2.4G" autocomplete="off">
          </div>

          <div class="form-group">
            <label class="form-label" for="cfg-wifi-pass">
              <span>รหัสผ่าน Wi-Fi (Password)</span>
            </label>
            <div class="input-wrap">
              <input type="password" class="form-input has-btn" id="cfg-wifi-pass" placeholder="กรอกรหัสผ่าน Wi-Fi…" autocomplete="new-password">
              <button type="button" class="eye-btn" data-toggle-pass="cfg-wifi-pass" title="แสดง/ซ่อนรหัสผ่าน" aria-label="แสดงหรือซ่อนรหัสผ่าน">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/></svg>
              </button>
            </div>
          </div>
          <div class="form-group">
            <label class="form-label" for="cfg-ap-pass">
              <span>รหัสผ่าน PowerBox_AP</span>
              <span class="form-hint">8–63 ตัวอักษร · เปลี่ยนแล้วต้องต่อใหม่</span>
            </label>
            <div class="input-wrap">
              <input type="password" class="form-input has-btn" id="cfg-ap-pass" placeholder="เว้นว่าง = ไม่เปลี่ยน" autocomplete="new-password">
              <button type="button" class="eye-btn" data-toggle-pass="cfg-ap-pass" title="แสดง/ซ่อนรหัสผ่าน" aria-label="แสดงหรือซ่อนรหัสผ่าน">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/></svg>
              </button>
            </div>
          </div>
        </div>

        <!-- แท็บ 2: ตั้งค่า AI API -->
        <div class="settings-pane" id="pane-api" role="tabpanel" hidden>
          <div class="form-group">
            <label class="form-label" for="cfg-api-url">
              <span>API Base URL (OpenAI Compatible)</span>
              <button type="button" class="btn-mini" id="api-reset-default-btn" title="คืนค่าเริ่มต้น">คืนค่าเริ่มต้น</button>
            </label>
            <input type="url" class="form-input mono" id="cfg-api-url" placeholder="https://api.tokenme.limited/api/v1/chat/completions" autocomplete="off">
          </div>

          <div class="form-group">
            <label class="form-label" for="cfg-api-key">
              <span>API Key</span>
              <span class="form-hint">เก็บไว้ในเครื่อง (LocalStorage)</span>
            </label>
            <div class="input-wrap">
              <input type="password" class="form-input mono has-btn" id="cfg-api-key" placeholder="ใส่ API Key (เช่น tm_... หรือ sk-...)" autocomplete="new-password">
              <button type="button" class="eye-btn" data-toggle-pass="cfg-api-key" title="แสดง/ซ่อน API Key" aria-label="แสดงหรือซ่อน API Key">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/></svg>
              </button>
            </div>
          </div>

          <div class="form-group">
            <label class="form-label" for="cfg-api-model">
              <span>ชื่อโมเดล AI (Model ID)</span>
              <span class="form-hint">เช่น google/gemma-4-31b-it</span>
            </label>
            <input type="text" class="form-input mono" id="cfg-api-model" placeholder="google/gemma-4-31b-it" autocomplete="off">
          </div>
        </div>
      </div>

      <div class="settings-foot">
        <span class="settings-msg" id="settings-msg"></span>
        <div class="settings-actions">
          <button type="button" class="btn-secondary" id="settings-cancel-btn">ยกเลิก</button>
          <button type="button" class="btn-primary" id="settings-save-btn">บันทึกการตั้งค่า</button>
        </div>
      </div>
    </div>
  </div>

  <footer data-od-id="page-footer">
    <span>PowerBox · ระบบจำลองการทำงานสำหรับต้นแบบ</span>
    <span>ค่ากระแสและแบตเตอรี่อัปเดตทุกวินาที</span>
  </footer>

</div>

<script src="powerbox-control.js?v=8"></script>
</body>
</html>
)PBHTML";

static const char POWERBOX_JS[] PROGMEM = R"PBJS((function(){
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
)PBJS";
