# PairLink Bidirectional Multi-I/O Example - Node A

This example is one half of a two-board PairLink setup where both ESP32 boards send and receive multiple values at the same time.

Use this sketch together with:

- [`../esp32-bidirectional-node-b/`](../esp32-bidirectional-node-b/)

If you are new to PairLink, read these guides first:

- [`../../../../docs/setup/esp32-library-install.md`](../../../../docs/setup/esp32-library-install.md)
- [`../../../../docs/examples/two-device-bidirectional-multi-io.md`](../../../../docs/examples/two-device-bidirectional-multi-io.md)
- [`../../../../docs/setup/troubleshooting.md`](../../../../docs/setup/troubleshooting.md)

## What It Does

Node A has two local inputs and two local outputs:

- reads a potentiometer on pin `34`
- reads a push button on pin `27`
- drives a PWM LED on pin `25`
- drives a digital LED on pin `26`

Published channels:

- `nodeA.knob`
- `nodeA.button`

Subscribed channels:

- `nodeB.knob`
- `nodeB.button`

Interaction model:

- turning the Node A potentiometer changes the PWM LED on Node B
- pressing the Node A button changes the digital LED on Node B
- Node A also reacts to Node B in the same way in the opposite direction

## Wiring

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| Potentiometer wiper | `34` | outer legs go to `3.3V` and `GND` |
| Input button | `27` | wire the other side to `GND`, uses `INPUT_PULLUP` |
| PWM LED | `25` | use a resistor in series |
| Digital LED | `26` | use a resistor in series |
| Pair button | `12` | wire the other side to `GND` |
| Status LED | `2` | built-in on many ESP32 boards |

## Configuration

Edit these values in `setup()` before uploading:

| Setting | Description | Default |
|---------|-------------|---------|
| `defaultSsid` | Wi-Fi network name | `PAIRLINK_LOCAL` |
| `defaultPassword` | Wi-Fi password | `pairlink123` |
| `localServerUrl` | PairLink server WebSocket URL | `ws://192.168.1.100:8080/ws` |
| `deviceName` | Name shown by the device | `node-a` |

Important:

- replace the Wi-Fi values with your own network
- replace `localServerUrl` with the real IP address of the machine running PairLink
- do not use `localhost` in an ESP32 sketch

## How To Use It

1. Upload this sketch to the first ESP32.
2. Upload [`esp32-bidirectional-node-b`](../esp32-bidirectional-node-b/) to the second ESP32.
3. Open both Serial Monitors at `115200 baud`.
4. Wait until both boards connect to Wi-Fi and the PairLink server.
5. Press the pair button on both boards within the pairing window.
6. Turn the potentiometer on Node A and watch the PWM LED on Node B.
7. Press the input button on Node A and watch the digital LED on Node B.

## What Success Looks Like

- both boards appear in the PairLink dashboard
- one binding connects the two boards
- Node A serial output shows local publishing
- Node A serial output also shows incoming `nodeB.*` values
- Node B visibly reacts when Node A inputs change

## One Pairing, Many Channels

PairLink still creates one paired relationship between the two boards. Inside that paired relationship, several logical channels can flow at the same time. This example makes that visible with one knob channel and one button channel in each direction.

## Dependencies

- PairLink library
- ArduinoJson
- ArduinoWebsockets
