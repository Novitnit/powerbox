#!/usr/bin/env python3
"""Regenerate the ESP32 asset header from the web UI files.

Usage (from the project root):
    python tools/embed_assets.py

Reads:
    powerbox-control.html
    powerbox-control.js

Writes:
    esp32/powerbox_esp32/powerbox_assets.h

The generated header stores both files as PROGMEM raw string literals so the
whole UI fits in flash and no LittleFS/SPIFFS upload step is required.
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HTML = ROOT / "powerbox-control.html"
JS = ROOT / "powerbox-control.js"
OUT = ROOT / "esp32" / "powerbox_esp32" / "powerbox_assets.h"


def raw_literal(name: str, text: str, delim: str) -> str:
    if f'){delim}"' in text:
        raise SystemExit(
            f"Asset contains the raw-string terminator ){delim}\"; "
            f"pick another delimiter in embed_assets.py."
        )
    return f'static const char {name}[] PROGMEM = R"{delim}({text}){delim}";\n'


def main() -> None:
    html = HTML.read_text(encoding="utf-8")
    js = JS.read_text(encoding="utf-8")

    header = (
        "// AUTO-GENERATED FILE - DO NOT EDIT BY HAND.\n"
        "// Regenerate with: python tools/embed_assets.py\n"
        "#pragma once\n"
        "#include <pgmspace.h>\n\n"
    )
    header += raw_literal("POWERBOX_HTML", html, "PBHTML")
    header += "\n"
    header += raw_literal("POWERBOX_JS", js, "PBJS")

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(header, encoding="utf-8", newline="\n")

    print(f"html: {len(html):>7} chars")
    print(f"js:   {len(js):>7} chars")
    print(f"wrote {OUT} ({OUT.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
