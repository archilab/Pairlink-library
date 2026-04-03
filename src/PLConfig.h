#ifndef PLCONFIG_H
#define PLCONFIG_H

#include <Arduino.h>

/** Max lengths for runtime config (Serial/NVS). Used by PLWiFi, PLWebSocket, PairLink, PLConfigStorage. */
#define PL_CONFIG_SSID_MAX      32
#define PL_CONFIG_PASSWORD_MAX  64
#define PL_CONFIG_URL_MAX       128

enum PLIdentityMode {
  PL_IDENTITY_NONE,
  PL_IDENTITY_EPHEMERAL,
  PL_IDENTITY_PERSISTENT
};

enum PLPairingMode {
  PL_PAIR_AUTO,
  PL_PAIR_TEMPORARY,
  PL_PAIR_PERSISTENT
};

struct PLConfig {
  const char* deviceType = "controller";
  const char* deviceName = "pairlink-device";
  const char* groupName = nullptr;
  PLIdentityMode identityMode = PL_IDENTITY_NONE;
  PLPairingMode pairingMode = PL_PAIR_AUTO;
  const char* defaultSsid = "PAIRLINK_LOCAL";
  const char* defaultPassword = "";
  const char* localServerUrl = "ws://192.168.4.1:8080/ws";
  const char* cloudServerUrl = nullptr;
  int pairButtonPin = -1;
  int statusLedPin = -1;
  unsigned long heartbeatIntervalMs = 10000;
  unsigned long pairingWindowSec = 30;
  /** Enable built-in serial config (wifi/server commands + NVS) by default. */
  bool enableSerialConfig = true;
  /** Enable debug logging for connection events. Set to true for more verbose output. */
  bool debugMode = false;
};

/** Set config for local-only use: identity none, auto pairing. Call before begin(). */
inline void plConfigSetLocalOnly(PLConfig& config, const char* ssid, const char* password, const char* serverUrl) {
  config.identityMode = PL_IDENTITY_NONE;
  config.pairingMode = PL_PAIR_AUTO;
  config.defaultSsid = ssid;
  config.defaultPassword = password;
  config.localServerUrl = serverUrl;
}

#endif