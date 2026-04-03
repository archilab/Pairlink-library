#include "PLWiFi.h"
#include <WiFi.h>
#include <string.h>

// Helper macros for WiFi status messages
#define WIFI_DEBUG(msg) DEBUG_PRINTF(msg)

void PLWiFiManager::begin(const char* ssid, const char* password) {
  _ssid = ssid;
  _password = password;
  _state = WIFI_IDLE;
  resetBackoff();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  Serial.printf("[PLWiFi] Starting WiFi connection to '%s'...\n", ssid);
  startConnect();
}

void PLWiFiManager::reconfigure(const char* ssid, const char* password) {
  WiFi.disconnect();
  if (ssid) {
    strncpy(_ssidBuf, ssid, PL_CONFIG_SSID_MAX - 1);
    _ssidBuf[PL_CONFIG_SSID_MAX - 1] = '\0';
    _ssid = _ssidBuf;
  }
  if (password) {
    strncpy(_passwordBuf, password, PL_CONFIG_PASSWORD_MAX - 1);
    _passwordBuf[PL_CONFIG_PASSWORD_MAX - 1] = '\0';
    _password = _passwordBuf;
  } else {
    _passwordBuf[0] = '\0';
    _password = _passwordBuf;
  }
  _state = WIFI_DISCONNECTED;
  _lastAttemptMs = millis();
  resetBackoff();
  startConnect();
}

void PLWiFiManager::update() {
  unsigned long now = millis();

  switch (_state) {
    case WIFI_IDLE:
      break;

    case WIFI_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        _state = WIFI_CONNECTED;
        resetBackoff();
        Serial.printf("[PLWiFi] Connected, IP: %s\n", WiFi.localIP().toString().c_str());
      } else if (now - _connectStartMs > CONNECT_TIMEOUT_MS) {
        Serial.println("[PLWiFi] Connection timeout");
        WiFi.disconnect();
        _state = WIFI_DISCONNECTED;
        _lastAttemptMs = now;
      }
      break;

    case WIFI_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[PLWiFi] Connection lost");
        _state = WIFI_DISCONNECTED;
        _lastAttemptMs = millis();
      }
      break;

    case WIFI_DISCONNECTED:
      if (now - _lastAttemptMs >= _backoffMs) {
        increaseBackoff();
        startConnect();
      }
      break;
  }
}

bool PLWiFiManager::isConnected() const {
  return _state == WIFI_CONNECTED;
}

void PLWiFiManager::startConnect() {
  Serial.printf("[PLWiFi] Connecting to %s (backoff %lums)\n", _ssid, _backoffMs);
  WiFi.begin(_ssid, _password);
  _connectStartMs = millis();
  _state = WIFI_CONNECTING;
}

void PLWiFiManager::resetBackoff() {
  _backoffMs = MIN_BACKOFF_MS;
}

void PLWiFiManager::increaseBackoff() {
  _backoffMs = min(_backoffMs * 2, MAX_BACKOFF_MS);
}
