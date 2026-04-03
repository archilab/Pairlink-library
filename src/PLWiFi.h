#ifndef PLWIFI_H
#define PLWIFI_H

#include <Arduino.h>
#include "PLConfig.h"

class PLWiFiManager {
public:
  void begin(const char* ssid, const char* password);
  /** Reconfigure and reconnect. Copies into internal buffers. Caller can pass nullptr for password. */
  void reconfigure(const char* ssid, const char* password);
  void update();
  bool isConnected() const;

private:
  enum WiFiState {
    WIFI_IDLE,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_DISCONNECTED
  };

  WiFiState _state = WIFI_IDLE;
  const char* _ssid = nullptr;
  const char* _password = nullptr;
  char _ssidBuf[PL_CONFIG_SSID_MAX] = {0};
  char _passwordBuf[PL_CONFIG_PASSWORD_MAX] = {0};

  unsigned long _connectStartMs = 0;
  unsigned long _backoffMs = 1000;
  unsigned long _lastAttemptMs = 0;

  static constexpr unsigned long MIN_BACKOFF_MS = 1000;
  static constexpr unsigned long MAX_BACKOFF_MS = 30000;
  static constexpr unsigned long CONNECT_TIMEOUT_MS = 15000;

  void startConnect();
  void resetBackoff();
  void increaseBackoff();
};

#endif
