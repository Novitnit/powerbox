# PowerBox Control

Web mockup for controlling a PowerBox device.

## Files

- `powerbox-control.html` — UI markup
- `powerbox-control.js` — client-side logic

## ESP32 firmware

The `esp32/` folder contains an Arduino sketch that serves this UI directly
from an ESP32 (Wi-Fi setup AP, AI proxy, settings, Wi-Fi scan). See
[`esp32/README.md`](esp32/README.md) for the port, upload commands and usage.

## Configuration

Create a `.env` file in the project root (it is git-ignored):

```
API_KEY=
AI_BASE_URL=
AI_MODEL=
```
