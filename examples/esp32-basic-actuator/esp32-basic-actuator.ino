/*
 * PairLink Basic Actuator Example
 *
 * Install these libraries in Arduino IDE (Sketch -> Include Library -> Manage Libraries):
 *   - PairLink (ESP32 library from this repo)
 *   - ArduinoJson (>= 7.0.0)
 *   - ArduinoWebsockets (>= 0.5.0)
 *
 * Receives values on "sensor.value" from a paired sensor and controls
 * LED brightness via PWM. Optionally publishes "led.brightness" for feedback.
 * Values are float; for buttons use 0.0 = off, 1.0 = on (check with value > 0.5f).
 */

#include <PairLink.h>

PairLink pairLink;
const int LED_PIN = 25;

void onStatusChanged(PLConnectionState state) {
  Serial.print("Status: ");
  Serial.println(plStateToString(state));
}

void onSensorValue(float value) {
  Serial.print("Received sensor.value: ");
  Serial.println(value);
  // Map 0-100 to 0-255 PWM
  int pwm = constrain((int)(value * 2.55f), 0, 255);
  analogWrite(LED_PIN, pwm);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  PLConfig config;
  plConfigSetLocalOnly(config, "PAIRLINK_LOCAL", "pairlink123", "ws://192.168.1.100:8080/ws");
  config.deviceType = "actuator-node";
  config.deviceName = "led-controller";
  config.pairButtonPin = 12;
  config.statusLedPin = 2;

  pairLink.begin(config);
  pairLink.addPublishChannel("led.brightness");
  pairLink.addSubscribeChannel("sensor.value");
  pairLink.onChannel("sensor.value", onSensorValue);
  pairLink.onStatus(onStatusChanged);
}

void loop() {
  pairLink.update();
}
