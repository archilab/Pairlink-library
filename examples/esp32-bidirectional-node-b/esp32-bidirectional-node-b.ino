/*
 * PairLink Bidirectional Multi-I/O Example - Node B
 *
 * Install these libraries in Arduino IDE (Sketch -> Include Library -> Manage Libraries):
 *   - PairLink (ESP32 library from this repo)
 *   - ArduinoJson (>= 7.0.0)
 *   - ArduinoWebsockets (>= 0.5.0)
 *
 * Node B publishes two sensor channels:
 * - nodeB.knob
 * - nodeB.button (0.0 = released, 1.0 = pressed)
 *
 * Node B subscribes to two channels from Node A:
 * - nodeA.knob   -> controls local PWM LED brightness
 * - nodeA.button -> controls local digital output LED (value > 0.5 = on)
 *
 * Uses per-channel callbacks so you don't compare channel strings yourself.
 * Matches Node A so both boards can be wired the same way.
 */

#include <PairLink.h>

PairLink pairLink;

const int PAIR_BUTTON_PIN = 12;
const int STATUS_LED_PIN = 2;
const int KNOB_PIN = 34;
const int INPUT_BUTTON_PIN = 27;
const int PWM_LED_PIN = 25;
const int DIGITAL_LED_PIN = 26;

float lastButtonValue = -1.0f;

void onStatusChanged(PLConnectionState state) {
  Serial.print("[Node B] Status: ");
  Serial.println(plStateToString(state));
}

void onNodeAKnob(float value) {
  int pwm = constrain((int)(value * 2.55f), 0, 255);
  analogWrite(PWM_LED_PIN, pwm);
  Serial.print("[Node B] nodeA.knob -> PWM ");
  Serial.println(pwm);
}

void onNodeAButton(float value) {
  bool on = value > 0.5f;
  digitalWrite(DIGITAL_LED_PIN, on ? HIGH : LOW);
  Serial.print("[Node B] nodeA.button -> ");
  Serial.println(on ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);
  Serial.println("PairLink Bidirectional Multi-I/O - Node B");

  pinMode(INPUT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PWM_LED_PIN, OUTPUT);
  pinMode(DIGITAL_LED_PIN, OUTPUT);

  PLConfig config;
  plConfigSetLocalOnly(config, "PAIRLINK_LOCAL", "pairlink123", "ws://192.168.1.100:8080/ws");
  config.deviceType = "bidirectional-node";
  config.deviceName = "node-b";
  config.pairButtonPin = PAIR_BUTTON_PIN;
  config.statusLedPin = STATUS_LED_PIN;

  pairLink.begin(config);

  pairLink.addPublishChannel("nodeB.knob");
  pairLink.addPublishChannel("nodeB.button");
  pairLink.addSubscribeChannel("nodeA.knob");
  pairLink.addSubscribeChannel("nodeA.button");

  pairLink.onChannel("nodeA.knob", onNodeAKnob);
  pairLink.onChannel("nodeA.button", onNodeAButton);
  pairLink.onStatus(onStatusChanged);
}

void loop() {
  pairLink.update();

  float knobPercent = (analogRead(KNOB_PIN) / 4095.0f) * 100.0f;
  pairLink.publish("nodeB.knob", knobPercent);  // default 50 ms throttle

  float buttonValue = digitalRead(INPUT_BUTTON_PIN) == LOW ? 1.0f : 0.0f;
  if (buttonValue != lastButtonValue) {
    pairLink.publishImmediate("nodeB.button", buttonValue);
    lastButtonValue = buttonValue;
  }
}
