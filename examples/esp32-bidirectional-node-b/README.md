# PairLink Bidirectional Multi-I/O Example - Node B

This example is the second half of the two-board bidirectional PairLink demo. It uses the same hardware layout as Node A so beginners can build both boards in the same way.

Use this sketch together with:

- [`../esp32-bidirectional-node-a/`](../esp32-bidirectional-node-a/)

If you are new to PairLink, read these guides first:

- [`../../README.md`](../../README.md)
- [`../README.md`](../README.md)
- [`../esp32-bidirectional-node-a/README.md`](../esp32-bidirectional-node-a/README.md)

## What It Does

Node B has two local inputs and two local outputs:

- reads a potentiometer on pin `34`
- reads a push button on pin `27`
- drives a PWM LED on pin `25`
- drives a digital LED on pin `26`

Published channels:

- `nodeB.knob`
- `nodeB.button`

Subscribed channels:

- `nodeA.knob`
- `nodeA.button`

Interaction model:

- turning the Node B potentiometer changes the PWM LED on Node A
- pressing the Node B button changes the digital LED on Node A
- Node B also reacts to Node A in the same way in the opposite direction

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
| `deviceName` | Name shown by the device | `node-b` |

Important:

- replace the Wi-Fi values with your own network
- replace `localServerUrl` with the real IP address of the machine running PairLink
- do not use `localhost` in an ESP32 sketch

## How To Use It

1. Upload this sketch to the second ESP32.
2. Upload [`esp32-bidirectional-node-a`](../esp32-bidirectional-node-a/) to the first ESP32.
3. Open both Serial Monitors at `115200 baud`.
4. Wait until both boards connect to Wi-Fi and the PairLink server.
5. Press the pair button on both boards within the pairing window.
6. Turn the potentiometer on Node B and watch the PWM LED on Node A.
7. Press the input button on Node B and watch the digital LED on Node A.

## What Success Looks Like

- both boards appear in the PairLink dashboard
- one binding connects the two boards
- Node B serial output shows local publishing
- Node B serial output also shows incoming `nodeA.*` values
- Node A visibly reacts when Node B inputs change

## One Pairing, Many Channels

Even though this example uses four logical channels, PairLink still creates a single paired relationship between the two boards. That makes it a good template for larger installations with several sensors and actuators per node.

## Dependencies

- PairLink library
- ArduinoJson
- ArduinoWebsockets
