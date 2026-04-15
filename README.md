# PairLink ESP32 Library

Arduino library for ESP32 controllers to connect to PairLink servers and exchange values with paired devices.

PairLink is open source under the MIT License. The library is especially intended to be approachable for education, workshops, teaching labs, and rapid prototyping on ESP32 hardware.

If you are new to Arduino IDE or ESP32 setup, start with these in-repo guides:

- [`examples/README.md`](examples/README.md)
- [`examples/esp32-hello/README.md`](examples/esp32-hello/README.md)
- [`examples/esp32-pairing-demo/README.md`](examples/esp32-pairing-demo/README.md)

## Dependencies

- ArduinoJson (>= 7.0.0)
- ArduinoWebsockets (>= 0.5.0)

## Quick Start

```cpp
#include <PairLink.h>

PairLink pairLink;  // do not name the variable "link" — it conflicts with POSIX link()

void onValueReceived(const char* channel, float value) {
  Serial.printf("Received %s = %.2f\n", channel, value);
}

void onStatusChanged(PLConnectionState state) {
  Serial.printf("State: %s\n", plStateToString(state));
}

void setup() {
  Serial.begin(115200);

  pairLink.begin({
    .deviceType = "sensor-node",
    .deviceName = "imu-01",
    .identityMode = PL_IDENTITY_NONE,
    .defaultSsid = "PAIRLINK_LOCAL",
    .defaultPassword = "secret",
    .localServerUrl = "ws://192.168.4.1:8080/ws",
    .pairButtonPin = 12,
    .statusLedPin = 2
  });

  pairLink.addPublishChannel("imu.pitch");
  pairLink.addPublishChannel("imu.roll");
  pairLink.addSubscribeChannel("led.brightness");

  pairLink.onValue(onValueReceived);
  pairLink.onStatus(onStatusChanged);
}

void loop() {
  pairLink.update();

  float pitch = analogRead(34) * 0.1;
  pairLink.publish("imu.pitch", pitch, 50);
}
```

## Included Examples

The ESP32 example sketches are available inside the library's own `examples/` folder:

- `examples/esp32-hello/` – minimal one-channel example
- `examples/esp32-pairing-demo/`
- `examples/esp32-basic-sensor/`
- `examples/esp32-basic-actuator/`
- `examples/esp32-bidirectional-node-a/`
- `examples/esp32-bidirectional-node-b/`
- `examples/esp32-group-demo/`

If you install the library in Arduino IDE, these examples are surfaced through the normal **Library Examples** workflow.

For each example under `examples/<scenario>/`, there is a **mirrored PlatformIO project** under:

- `platformio-examples/<scenario>/`

Each PlatformIO project contains:

- a `platformio.ini` for the example, and
- a `src/main.cpp` file that uses the same code as the Arduino sketch.

This makes it easy to switch between Arduino IDE and PlatformIO without changing the example logic.

For the repository overview and walkthrough links, also see:

- `examples/README.md` inside this library

## API Reference

### Initialization

- `begin(config)` -- Initialize with a `PLConfig` struct.

### Main Loop

- `update()` -- Call in `loop()`. Handles WiFi, WebSocket, pairing, heartbeat, and LED -- all non-blocking.

### Publishing and Subscribing

- `addPublishChannel(channel)` -- Register a channel name this device will publish on.
- `addSubscribeChannel(channel)` -- Register a channel name this device will receive values on.
- `publish(channel, value, intervalMs)` -- Send a value on a channel. Default throttle 50 ms; use `publishImmediate(channel, value)` for buttons or when you need no throttle.
- `publishImmediate(channel, value)` -- Send a value with no throttle (e.g. for button press/release).

### Callbacks

- `onValue(callback)` -- Called when a value arrives on a subscribed channel (if no per-channel callback is set). Signature: `void(const char* channel, float value)`.
- `onChannel(channel, callback)` -- Set a callback for one subscribed channel; no need to compare channel strings in code. Call after `addSubscribeChannel`. Signature: `void(float value)`.
- `onStatus(callback)` -- Called when the connection state changes. Signature: `void(PLConnectionState state)`.

### Status

- `getState()` -- Returns the current `PLConnectionState`.
- `getSessionId()` -- Returns the session ID assigned by the server, or empty string if not registered.
- `getDeviceId()` -- Returns the device ID (ephemeral or persistent when enabled).
- `isPaired()` -- Returns `true` if currently paired with another device.

### Runtime config (no reflash)

- `setNetworkAndServer(ssid, password, localUrl, cloudUrl)` -- Copy into internal buffers and reconnect. Omit or pass `nullptr` for `cloudUrl` for local-only. Buffer limits: SSID 32, password 64, URL 128 chars (see `PL_CONFIG_SSID_MAX` etc. in `PLConfig.h`).
- `getDefaultSsid()`, `getDefaultPassword()`, `getLocalServerUrl()`, `getCloudServerUrl()` -- Current config (for partial updates).

Optional **serial config** (built into `PairLink` by default):

- With the default `PLConfig` settings, `PairLink` will:
  - load stored WiFi and server settings from NVS at boot (if present),
  - print a short instruction banner to the Serial Monitor once,
  - listen for `wifi,SSID,PASSWORD`, `server,ws://host:port/path`, and `save` commands on Serial during `update()`,
  - apply changes at runtime and persist them on `save`.
- To opt out (for advanced use), set `config.enableSerialConfig = false;` before calling `pairLink.begin(config);`. In that case, no serial banner is printed and no NVS read/write occurs.

## Minimal setup

For local-only use you only need to set WiFi and server URL. Use the preset so you don't have to choose identity or pairing mode:

```cpp
PLConfig config;
plConfigSetLocalOnly(config, "PAIRLINK_LOCAL", "pairlink123", "ws://192.168.1.100:8080/ws");
config.pairButtonPin = 12;   // optional
config.statusLedPin = 2;     // optional
pairLink.begin(config);
```

Optional: set `config.deviceType`, `config.deviceName` if you want; other fields already have safe defaults.

## Data types

Values are sent and received as **float**. For buttons or on/off state use **0.0 = off, 1.0 = on**; in the callback treat `value > 0.5f` as true. Example: `pairLink.publish("button", digitalRead(PIN) == LOW ? 1.0f : 0.0f);` and `bool pressed = value > 0.5f;`.

## Changing WiFi and server without reflashing

You can change WiFi credentials and server URL at runtime over **Serial** (and optionally persist to NVS for the next boot):

1. At boot, the device prints one line to Serial, e.g.:  
   `PairLink config: wifi,SSID,password | server,ws://host:port/path | save`
2. Open Serial Monitor (115200) and send:
   - `wifi,YourSSID,YourPassword` — set WiFi and reconnect
   - `server,ws://192.168.1.100:8080/ws` — set server URL and reconnect
   - `save` — write current config to NVS (used on next boot)

**Buffer limits:** SSID 32 chars, password 64 chars, server URL 128 chars (`PL_CONFIG_SSID_MAX`, `PL_CONFIG_PASSWORD_MAX`, `PL_CONFIG_URL_MAX` in `PLConfig.h`).

## Configuration

The `PLConfig` struct supports:

| Field | Default | Description |
|---|---|---|
| `deviceType` | `"controller"` | Device type string sent to server |
| `deviceName` | `"pairlink-device"` | Human-readable name |
| `identityMode` | `PL_IDENTITY_NONE` | `NONE`, `EPHEMERAL`, or `PERSISTENT` |
| `pairingMode` | `PL_PAIR_AUTO` | `AUTO`, `TEMPORARY`, or `PERSISTENT` |
| `groupName` | `nullptr` | Optional group label for auto group-based routing |
| `defaultSsid` | `"PAIRLINK_LOCAL"` | WiFi SSID to connect to |
| `defaultPassword` | `""` | WiFi password |
| `localServerUrl` | `"ws://192.168.4.1:8080/ws"` | Local server WebSocket URL |
| `cloudServerUrl` | `nullptr` | Optional cloud/relay server URL |
| `pairButtonPin` | `-1` | GPIO for pairing button (-1 = disabled) |
| `statusLedPin` | `-1` | GPIO for status LED (-1 = disabled) |
| `heartbeatIntervalMs` | `10000` | Heartbeat interval in ms |
| `pairingWindowSec` | `30` | Pairing window duration in seconds |

## Connection States

| State | LED Pattern |
|---|---|
| `PL_STATE_BOOTING` | Slow blink (1 Hz) |
| `PL_STATE_WIFI_CONNECTING` | Fast blink (4 Hz) |
| `PL_STATE_LOCAL_CONNECTING` | Double blink |
| `PL_STATE_LOCAL_CONNECTED` | Solid on |
| `PL_STATE_CLOUD_CONNECTING` | Triple blink |
| `PL_STATE_CLOUD_CONNECTED` | Solid with brief off every 3s |
| `PL_STATE_RECONNECTING` | Alternating fast/slow |
| `PL_STATE_OFFLINE_IDLE` | Off |
| `PL_STATE_ERROR` | Very fast blink (10 Hz) |

## Pairing

If a `pairButtonPin` is configured:

- **Short press** -- Sends a pair request using the configured `pairingMode`.
- **Long press (>2s)** -- Sends a persistent pair request.

## Architecture

The library is split into focused modules:

- `PLWiFi` -- Non-blocking WiFi connection with exponential backoff; supports `reconfigure()` for runtime config
- `PLWebSocket` -- WebSocket client wrapper with reconnection; supports `reconfigure()` for runtime URL change
- `PLProtocol` -- JSON message serialization/parsing (ArduinoJson)
- `PLPairing` -- Debounced button handler for short/long press
- `PLLed` -- Status LED patterns per connection state
- `PLState` -- Connection state machine enum
- `PLConfig` -- Configuration struct and buffer size constants
- `PLConfigStorage` -- Optional NVS persistence for WiFi/server config (namespace `pairlink`)
- `PLSerialConfig` -- Optional serial command parser and boot instruction for runtime config
