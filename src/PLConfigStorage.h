#ifndef PLCONFIGSTORAGE_H
#define PLCONFIGSTORAGE_H

#include "PLConfig.h"

/**
 * Optional NVS persistence for WiFi and server config.
 * Uses namespace "pairlink" (same as PLIdentity). Keys: ssid, password, localServerUrl, cloudServerUrl.
 * Max lengths: PL_CONFIG_SSID_MAX, PL_CONFIG_PASSWORD_MAX, PL_CONFIG_URL_MAX.
 */
class PLConfigStorage {
public:
  /** Load stored values into config. Overwrites only ssid, password, localServerUrl, cloudServerUrl if keys exist. */
  static void load(PLConfig& config);

  /** Save to NVS. Pass nullptr or empty string for cloudUrl to clear. */
  static void save(const char* ssid, const char* password, const char* localUrl, const char* cloudUrl = nullptr);
};

#endif
