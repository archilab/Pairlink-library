/*
 * PairLink Basic Sensor Example
 *
 * Install these libraries in Arduino IDE (Sketch -> Include Library -> Manage Libraries):
 *   - PairLink (ESP32 library from this repo)
 *   - ArduinoJson (>= 7.0.0)
 *   - ArduinoWebsockets (>= 0.5.0)
 *
 * Reads an analog value (e.g., potentiometer on pin 34) and publishes it
 * as "sensor.value". Subscribes to "led.brightness" for optional feedback.
 * Uses local-only config preset; publish() uses default 50 ms throttle.
 */

#include <PairLink.h>

PairLink pairLink;

void onStatusChanged(PLConnectionState state) {
  Serial.print("Status: ");
  Serial.println(plStateToString(state));
}

void onLedBrightness(float value) {
  Serial.print("Received led.brightness: ");
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);

  PLConfig config;
  plConfigSetLocalOnly(config, "PAIRLINK_LOCAL", "pairlink123", "ws://192.168.1.100:8080/ws");
  config.deviceType = "sensor-node";
  config.deviceName = "analog-sensor";
  config.pairButtonPin = 12;
  config.statusLedPin = 2;

  pairLink.begin(config);
  pairLink.addPublishChannel("sensor.value");
  pairLink.addSubscribeChannel("led.brightness");
  pairLink.onChannel("led.brightness", onLedBrightness);
  pairLink.onStatus(onStatusChanged);
}

void loop() {
  pairLink.update();

  float value = (analogRead(34) / 4095.0f) * 100.0f;
  pairLink.publish("sensor.value", value);  // default 50 ms throttle
}
