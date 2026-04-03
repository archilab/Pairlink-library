#include "PLWebSocket.h"
#include <string.h>

// Helper macros for WebSocket status messages
#define WS_DEBUG(msg) DEBUG_PRINTF(msg)

void PLWebSocketClient::begin(const char* url) {
  _primaryUrl = url;
  _fallbackUrl = nullptr;
  _activeUrl = url;
  _activeTarget = PLWS_TARGET_PRIMARY;
  _connected = false;
  _connecting = false;
  _consecutiveFailures = 0;
  Serial.printf("[PLWS] Starting WebSocket connection to %s...\n", url);
  resetBackoff();
  setupCallbacks();
  tryConnect();
}

void PLWebSocketClient::reconfigure(const char* primaryUrl, const char* fallbackUrl) {
  disconnect();
  if (primaryUrl) {
    strncpy(_primaryUrlBuf, primaryUrl, PL_CONFIG_URL_MAX - 1);
    _primaryUrlBuf[PL_CONFIG_URL_MAX - 1] = '\0';
    _primaryUrl = _primaryUrlBuf;
  }
  if (fallbackUrl && fallbackUrl[0] != '\0') {
    strncpy(_fallbackUrlBuf, fallbackUrl, PL_CONFIG_URL_MAX - 1);
    _fallbackUrlBuf[PL_CONFIG_URL_MAX - 1] = '\0';
    _fallbackUrl = _fallbackUrlBuf;
  } else {
    _fallbackUrlBuf[0] = '\0';
    _fallbackUrl = nullptr;
  }
  _activeUrl = _primaryUrl;
  _activeTarget = PLWS_TARGET_PRIMARY;
  _consecutiveFailures = 0;
  resetBackoff();
  setupCallbacks();
  tryConnect();
}

void PLWebSocketClient::beginWithFallback(const char* primaryUrl, const char* fallbackUrl) {
  _primaryUrl = primaryUrl;
  _fallbackUrl = fallbackUrl;
  _activeUrl = primaryUrl;
  _activeTarget = PLWS_TARGET_PRIMARY;
  _connected = false;
  _connecting = false;
  _consecutiveFailures = 0;
  resetBackoff();
  setupCallbacks();
  tryConnect();
}

void PLWebSocketClient::onMessage(PLWSMessageCallback callback, void* userData) {
  _messageCallback = callback;
  _messageUserData = userData;
}

void PLWebSocketClient::setupCallbacks() {
  _client.onMessage([this](websockets::WebsocketsMessage msg) {
    if (_messageCallback && msg.isText()) {
      _messageCallback(_messageUserData, msg.data().c_str());
    }
  });

  _client.onEvent([this](websockets::WebsocketsEvent event, String data) {
    switch (event) {
      case websockets::WebsocketsEvent::ConnectionOpened:
        Serial.printf("[PLWS] Connected to %s (%s)\n",
          _activeTarget == PLWS_TARGET_PRIMARY ? "primary" : "fallback",
          _activeUrl);
        _connected = true;
        _connecting = false;
        _consecutiveFailures = 0;
        resetBackoff();
        break;
      case websockets::WebsocketsEvent::ConnectionClosed:
        Serial.println("[PLWS] Disconnected");
        _connected = false;
        _connecting = false;
        _consecutiveFailures++;
        break;
      case websockets::WebsocketsEvent::GotPing:
        break;
      case websockets::WebsocketsEvent::GotPong:
        break;
    }
  });
}

void PLWebSocketClient::update() {
  if (_connected) {
    _client.poll();
    return;
  }

  if (_connecting) {
    _client.poll();
    return;
  }

  unsigned long now = millis();
  if (now - _lastConnectAttemptMs >= _backoffMs) {
    tryConnect();
  }
}

bool PLWebSocketClient::send(const char* message) {
  if (!_connected) return false;
  return _client.send(message);
}

bool PLWebSocketClient::isConnected() const {
  return _connected;
}

void PLWebSocketClient::disconnect() {
  _client.close();
  _connected = false;
  _connecting = false;
}

PLWSTarget PLWebSocketClient::getActiveTarget() const {
  return _activeTarget;
}

bool PLWebSocketClient::hasFallback() const {
  return _fallbackUrl != nullptr;
}

void PLWebSocketClient::switchTo(PLWSTarget target) {
  disconnect();
  _activeTarget = target;
  _activeUrl = (target == PLWS_TARGET_PRIMARY) ? _primaryUrl : _fallbackUrl;
  _consecutiveFailures = 0;
  resetBackoff();
  Serial.printf("[PLWS] Switching to %s: %s\n",
    target == PLWS_TARGET_PRIMARY ? "primary" : "fallback",
    _activeUrl);
  tryConnect();
}

int PLWebSocketClient::getConsecutiveFailures() const {
  return _consecutiveFailures;
}

void PLWebSocketClient::tryConnect() {
  Serial.printf("[PLWS] Connecting to %s (backoff %lums)\n", _activeUrl, _backoffMs);
  _connecting = true;
  _lastConnectAttemptMs = millis();
  _client.connect(_activeUrl);
  increaseBackoff();
}

void PLWebSocketClient::resetBackoff() {
  _backoffMs = MIN_BACKOFF_MS;
}

void PLWebSocketClient::increaseBackoff() {
  _backoffMs = min(_backoffMs * 2, MAX_BACKOFF_MS);
}
