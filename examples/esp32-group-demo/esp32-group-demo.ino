/*
 * PairLink Group Demo – auto group-based routing
 *
 * Set config.groupName to the same value on multiple devices
 * (e.g. "lab-group-1") to have them automatically exchange
 * values within that group without manual pairing.
 *
 * Use this sketch on two ESP32 boards with the same groupName.
 * Both publish "sensor.value" and subscribe to "sensor.value"
 * so you can see values flowing between group members.
 */

#include <PairLink.h>

PairLink pairLink;

void onStatusChanged(PLConnectionState state) {
  Serial.print("Status: ");
  Serial.println(plStateToString(state));
}

void onSensorValue(float value) {
  Serial.print("Received sensor.value from group peer: ");
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);
  Serial.println("PairLink Group Demo");

  PLConfig config;
  plConfigSetLocalOnly(config, "PAIRLINK_LOCAL", "pairlink123", "ws://192.168.1.100:8080/ws");
  config.deviceType = "group-node";
  config.deviceName = "group-demo";
  config.groupName = "lab-group-1";
  config.pairButtonPin = -1;
  config.statusLedPin = 2;

  pairLink.begin(config);
  pairLink.addPublishChannel("sensor.value");
  pairLink.addSubscribeChannel("sensor.value");
  pairLink.onChannel("sensor.value", onSensorValue);
  pairLink.onStatus(onStatusChanged);
}

void loop() {
  pairLink.update();

  float value = (analogRead(34) / 4095.0f) * 100.0f;
  pairLink.publish("sensor.value", value);
}

