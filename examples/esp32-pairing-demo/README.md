# PairLink Pairing Demo

A demo that shows both publishing and subscribing on a single device. Uses the built-in LED and BOOT button with no external wiring required.

If you are new to PairLink, this is a good first hardware example. You may also want to read:

- [`../../../../docs/setup/esp32-library-install.md`](../../../../docs/setup/esp32-library-install.md)
- [`../../../../docs/examples/two-device-sensor-actuator.md`](../../../../docs/examples/two-device-sensor-actuator.md)
- [`../../../../docs/setup/troubleshooting.md`](../../../../docs/setup/troubleshooting.md)

## What It Does

- Publishes an incrementing counter on `demo.counter` every second
- Subscribes to `demo.counter` and prints received values
- Uses the BOOT button (pin 0) for pairing
- Uses the built-in LED (pin 2) for connection status

When two demo devices are paired, each will receive the other's counter values and print them to Serial.

## Wiring

No external wiring needed. Uses:

- BOOT button (GPIO 0) for pairing
- built-in LED (GPIO 2) for status indication

## Pairing Flow

1. Start the local server and ensure it is reachable at `localServerUrl`.
2. Flash this sketch on two ESP32 boards.
3. Wait for both devices to connect to Wi-Fi and the server.
4. Press the BOOT button on both devices within the pairing window.
5. Open Serial Monitor at `115200 baud` to see incoming values.

## Configuration

Edit these values in `setup()` to match your setup:

| Setting           | Description                             | Default                    |
|-------------------|-----------------------------------------|----------------------------|
| `defaultSsid`     | Wi-Fi network name                      | `PAIRLINK_LOCAL`           |
| `defaultPassword` | Wi-Fi password                          | `pairlink123`              |
| `localServerUrl`  | PairLink server WebSocket URL           | `ws://192.168.1.100:8080/ws` |
| `pairButtonPin`   | GPIO for pairing (BOOT on many boards)  | 0                          |
| `statusLedPin`    | GPIO for status LED                     | 2                          |

## Pairing Modes

- Short press means temporary pairing
- Long press means persistent pairing

## Dependencies

- PairLink library
- ArduinoJson
- ArduinoWebsockets
