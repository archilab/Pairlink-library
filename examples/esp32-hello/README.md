# PairLink Hello Example

Minimal one-channel example: one publish, one subscribe, local-only config.

If you are using PairLink for the first time, read these guides first:

- [`../../../docs/setup/esp32-library-install.md`](../../../docs/setup/esp32-library-install.md)
- [`../../../docs/setup/troubleshooting.md`](../../../docs/setup/troubleshooting.md)

## What It Does

- Publishes analog pin 34 (0–100%) on channel `sensor.value` with default 50 ms throttle
- Subscribes to `led.brightness` and prints received values to Serial
- Uses `plConfigSetLocalOnly()` so you only set WiFi and server URL (no identity/pairing mode choice)
- Uses `onChannel()` so you don't compare channel strings in code

## Wiring

| Component     | ESP32 Pin |
|---------------|-----------|
| Potentiometer | Pin 34, 3.3V, GND |
| Pair button   | Pin 12 and GND (optional) |
| Status LED    | Pin 2 (optional) |

## Configuration

Edit in `setup()`: `plConfigSetLocalOnly(config, "YOUR_SSID", "YOUR_PASSWORD", "ws://YOUR_SERVER_IP:8080/ws");`

## Data Types

Values are float. For buttons use 0.0 = off, 1.0 = on; in a callback use `value > 0.5f` for "on".
