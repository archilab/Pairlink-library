#ifndef PLWEBSOCKET_H
#define PLWEBSOCKET_H

#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include "PLConfig.h"

typedef void (*PLWSMessageCallback)(void* userData, const char* payload);

enum PLWSTarget {
  PLWS_TARGET_PRIMARY,
  PLWS_TARGET_FALLBACK
};

class PLWebSocketClient {
public:
  void begin(const char* url);
  void beginWithFallback(const char* primaryUrl, const char* fallbackUrl);
  /** Reconfigure URLs and reconnect. Copies into internal buffers. fallbackUrl may be nullptr. */
  void reconfigure(const char* primaryUrl, const char* fallbackUrl = nullptr);
  void update();
  bool send(const char* message);
  bool isConnected() const;
  void disconnect();
  void onMessage(PLWSMessageCallback callback, void* userData = nullptr);

  PLWSTarget getActiveTarget() const;
  bool hasFallback() const;

  void switchTo(PLWSTarget target);

  int getConsecutiveFailures() const;

private:
  websockets::WebsocketsClient _client;
  const char* _primaryUrl = nullptr;
  const char* _fallbackUrl = nullptr;
  const char* _activeUrl = nullptr;
  char _primaryUrlBuf[PL_CONFIG_URL_MAX] = {0};
  char _fallbackUrlBuf[PL_CONFIG_URL_MAX] = {0};
  PLWSTarget _activeTarget = PLWS_TARGET_PRIMARY;
  bool _connected = false;
  bool _connecting = false;
  PLWSMessageCallback _messageCallback = nullptr;
  void* _messageUserData = nullptr;

  unsigned long _lastConnectAttemptMs = 0;
  unsigned long _backoffMs = 1000;
  int _consecutiveFailures = 0;

  static constexpr unsigned long MIN_BACKOFF_MS = 1000;
  static constexpr unsigned long MAX_BACKOFF_MS = 30000;

  void tryConnect();
  void resetBackoff();
  void increaseBackoff();
  void setupCallbacks();
};

#endif
