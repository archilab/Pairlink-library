# PairLink Basic Actuator Example

A basic actuator that receives values from a paired sensor and controls an LED via PWM.

If you are using PairLink for the first time, read these guides first:

- [`../../../../docs/setup/esp32-library-install.md`](../../../../docs/setup/esp32-library-install.md)
- [`../../../../docs/examples/two-device-sensor-actuator.md`](../../../../docs/examples/two-device-sensor-actuator.md)
- [`../../../../docs/setup/troubleshooting.md`](../../../../docs/setup/troubleshooting.md)

## What It Does

- Subscribes to `sensor.value` from paired sensor nodes
- Maps incoming values (0-100) to LED PWM (0-255)
- Optionally publishes `led.brightness` for feedback
- Uses a pair button (pin 12) and status LED (pin 2) for connection and pairing indication

## Wiring

| Component   | ESP32 Pin |
|-------------|-----------|
| LED + resistor | Pin 25 and GND |
| Pair button | Pin 12 and GND |
| Status LED  | Pin 2 (built-in on many boards) |

Use a current-limiting resistor (e.g., 220 Ohm-330 Ohm) in series with the external LED.

## Configuration

Edit these values in `setup()` to match your setup:

| Setting           | Description                          | Default                    |
|-------------------|--------------------------------------|----------------------------|
| `defaultSsid`     | Wi-Fi network name                   | `PAIRLINK_LOCAL`           |
| `defaultPassword` | Wi-Fi password                       | `pairlink123`              |
| `localServerUrl`  | PairLink server WebSocket URL        | `ws://192.168.1.100:8080/ws` |
| `pairButtonPin`   | GPIO for pairing button              | 12                         |
| `statusLedPin`    | GPIO for status LED                  | 2                          |
| `LED_PIN`         | GPIO for the controlled LED          | 25                         |

## Pairing with the Sensor

1. Run the [esp32-basic-sensor](../esp32-basic-sensor) example on another ESP32.
2. Power both devices and ensure they connect to Wi-Fi and the PairLink server.
3. Press the pair button on both devices within the pairing window.
4. Turn the potentiometer on the sensor; the actuator's LED brightness should follow.

## Dependencies

- PairLink library
- ArduinoJson
- ArduinoWebsockets
