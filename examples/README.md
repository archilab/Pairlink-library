# PairLink ESP32 Library Examples



## Start Here

If you are new to PairLink, use the examples in this order:

1. `esp32-hello` – one channel in, one channel out; minimal config
2. `esp32-pairing-demo`
3. `esp32-basic-sensor`
4. `esp32-basic-actuator`
5. `esp32-bidirectional-node-a` together with `esp32-bidirectional-node-b`

Why this order:

- `esp32-hello` is the smallest sketch: local-only config preset, one publish, one subscribe, per-channel callback
- `esp32-pairing-demo` is the simplest way to understand pairing behavior
- `esp32-basic-sensor` shows how a board publishes values
- `esp32-basic-actuator` shows how another board receives those values
- the bidirectional node pair shows how both boards can publish and subscribe on several channels at the same time

## Before You Open An Example

Make sure you already have:

- Arduino IDE or PlatformIO with ESP32 support
- the PairLink library installed
- `ArduinoJson` installed
- `ArduinoWebsockets` installed
- a running PairLink local server

If you still need help with setup, read:

- `[../../../docs/setup/esp32-library-install.md](../../../docs/setup/esp32-library-install.md)`
- `[../../../docs/setup/local-development.md](../../../docs/setup/local-development.md)`

## Examples At A Glance


| Example                                                                                                               | What hardware is needed                                   | What it demonstrates                                                                        | What success looks like                                                                                           |
| --------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------- | ------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| [esp32-hello](esp32-hello/)                                                                                           | 1 ESP32, analog input (e.g. potentiometer on 34)          | minimal setup: one publish, one subscribe, local-only config, per-channel callback          | Serial shows status and received values; publish uses default throttle                                            |
| [esp32-pairing-demo](esp32-pairing-demo/)                                                                             | 1 ESP32, optional second ESP32                            | basic registration, pairing button flow, simple value exchange                              | serial output appears and pairing events become visible                                                           |
| [esp32-basic-sensor](esp32-basic-sensor/)                                                                             | 1 ESP32, analog input or potentiometer                    | sending `sensor.value`                                                                      | changing the analog input changes published values                                                                |
| [esp32-basic-actuator](esp32-basic-actuator/)                                                                         | 1 ESP32, LED on output pin                                | receiving values and driving an actuator                                                    | the LED brightness changes when values arrive                                                                     |
| [esp32-bidirectional-node-a](esp32-bidirectional-node-a/) + [esp32-bidirectional-node-b](esp32-bidirectional-node-b/) | 2 ESP32 boards, 2 potentiometers, 2 input buttons, 4 LEDs | true bidirectional multi-channel behavior with several sensors and actuators on both boards | both boards send values, both boards receive values, and each board visibly controls outputs on the other side    |


## Recommended First Hardware Tutorial

For the best beginner experience, follow the full walkthrough here:

- `[../../../docs/examples/two-device-sensor-actuator.md](../../../docs/examples/two-device-sensor-actuator.md)`

## Next Step After The Basic Tutorial

When you want to move beyond one sensor and one actuator, continue with:

- `[../../../docs/examples/two-device-bidirectional-multi-io.md](../../../docs/examples/two-device-bidirectional-multi-io.md)`

## If Something Does Not Work

Use the troubleshooting guide:

- `[../../../docs/setup/troubleshooting.md](../../../docs/setup/troubleshooting.md)`

