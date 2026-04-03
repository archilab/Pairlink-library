/*
 * PairLink Pairing Demo
 *
 * Install these libraries in Arduino IDE (Sketch -> Include Library -> Manage Libraries):
 *   - PairLink (ESP32 library from this repo)
 *   - ArduinoJson (>= 7.0.0)
 *   - ArduinoWebsockets (>= 0.5.0)
 *
 * Demonstrates both publishing and subscribing on the same device.
 * Uses the built-in LED and BOOT button for pairing. Publish an incrementing
 * counter and receive values from paired devices.
 */

#include <PairLink.h>

PairLink pairLink;
unsigned long lastPrint = 0;

void onStatusChanged(PLConnectionState state) {
  Serial.print("[PairLink] State changed: ");
  Serial.println(plStateToString(state));
}

void onValueReceived(const char* channel, float value) {
  Serial.printf("[PairLink] %s = %.2f\n", channel, value);
}

void setup() {
  Serial.begin(115200);
  Serial.println("PairLink Pairing Demo");
  Serial.println("Press the pair button to pair with another device.");

  // Use beginSimple() so BOOT (GPIO0) pairing is enabled.
  pairLink.beginSimple("PAIRLINK_LOCAL","pairlink123","ws://192.168.1.100:8080/ws",0,2); // pairLink.beginSimple(SSID,password,serverUrl,pairButtonPin,statusLedPin)
  pairLink.addPublishChannel("demo.counter");
  pairLink.addSubscribeChannel("demo.counter");
  pairLink.onValue(onValueReceived);
  pairLink.onStatus(onStatusChanged);
}

void loop() {
  pairLink.update();

  // Publish an incrementing counter every second
  if (millis() - lastPrint > 1000) {
    static float counter = 0;
    counter += 1.0;
    pairLink.publish("demo.counter", counter);
    lastPrint = millis();
  }
}
