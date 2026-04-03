# PairLink Basic Sensor Example

A basic sensor node that reads an analog value and publishes it over PairLink.

If you are using PairLink for the first time, read these guides first:

- [`../../../../docs/setup/esp32-library-install.md`](../../../../docs/setup/esp32-library-install.md)
- [`../../../../docs/examples/two-device-sensor-actuator.md`](../../../../docs/examples/two-device-sensor-actuator.md)
- [`../../../../docs/setup/troubleshooting.md`](../../../../docs/setup/troubleshooting.md)

## What It Does

- Reads an analog input (e.g., potentiometer) on pin 34
- Publishes the value (0-100) on channel `sensor.value` at most every 50 ms
- Subscribes to `led.brightness` for optional feedback from paired actuators
- Uses a pair button (pin 12) and status LED (pin 2) for connection and pairing indication

## Wiring

| Component      | ESP32 Pin |
|----------------|-----------|
| Potentiometer  | Pin 34 (ADC1_CH6) |
| Potentiometer  | 3.3V and GND |
| Pair button    | Pin 12 and GND |
| Status LED     | Pin 2 (built-in on many boards) |

For the potentiometer: connect one outer leg to 3.3V, the other to GND, and the wiper to pin 34.

## Configuration

Edit these values in `setup()` to match your setup:

| Setting           | Description                          | Default                    |
|-------------------|--------------------------------------|----------------------------|
| `defaultSsid`     | Wi-Fi network name                   | `PAIRLINK_LOCAL`           |
| `defaultPassword` | Wi-Fi password                       | `pairlink123`              |
| `localServerUrl`  | PairLink server WebSocket URL        | `ws://192.168.1.100:8080/ws` |
| `pairButtonPin`   | GPIO for pairing button              | 12                         |
| `statusLedPin`    | GPIO for status LED                  | 2                          |

Ensure the local PairLink server is running and reachable at the given URL.

## Pairing

1. Power both the sensor and the actuator (or other subscriber).
2. Press the pair button on both devices within the pairing window.
3. Once paired, the sensor's `sensor.value` will be routed to the paired device(s).

## Dependencies

- PairLink library
- ArduinoJson
- ArduinoWebsockets
