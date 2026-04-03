#include "PLConfigStorage.h"
#include <Preferences.h>
#include <string.h>

static const char* NVS_NAMESPACE = "pairlink";
static const char* NVS_KEY_SSID = "ssid";
static const char* NVS_KEY_PASSWORD = "password";
static const char* NVS_KEY_LOCAL_URL = "localServerUrl";
static const char* NVS_KEY_CLOUD_URL = "cloudServerUrl";

void PLConfigStorage::load(PLConfig& config) {
  static char ssidBuf[PL_CONFIG_SSID_MAX];
  static char passwordBuf[PL_CONFIG_PASSWORD_MAX];
  static char localUrlBuf[PL_CONFIG_URL_MAX];
  static char cloudUrlBuf[PL_CONFIG_URL_MAX];

  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, true);

  if (prefs.isKey(NVS_KEY_SSID)) {
    String s = prefs.getString(NVS_KEY_SSID, "");
    if (s.length() > 0) {
      strncpy(ssidBuf, s.c_str(), PL_CONFIG_SSID_MAX - 1);
      ssidBuf[PL_CONFIG_SSID_MAX - 1] = '\0';
      config.defaultSsid = ssidBuf;
    }
  }
  if (prefs.isKey(NVS_KEY_PASSWORD)) {
    String s = prefs.getString(NVS_KEY_PASSWORD, "");
    strncpy(passwordBuf, s.c_str(), PL_CONFIG_PASSWORD_MAX - 1);
    passwordBuf[PL_CONFIG_PASSWORD_MAX - 1] = '\0';
    config.defaultPassword = passwordBuf;
  }
  if (prefs.isKey(NVS_KEY_LOCAL_URL)) {
    String s = prefs.getString(NVS_KEY_LOCAL_URL, "");
    if (s.length() > 0) {
      strncpy(localUrlBuf, s.c_str(), PL_CONFIG_URL_MAX - 1);
      localUrlBuf[PL_CONFIG_URL_MAX - 1] = '\0';
      config.localServerUrl = localUrlBuf;
    }
  }
  if (prefs.isKey(NVS_KEY_CLOUD_URL)) {
    String s = prefs.getString(NVS_KEY_CLOUD_URL, "");
    if (s.length() > 0) {
      strncpy(cloudUrlBuf, s.c_str(), PL_CONFIG_URL_MAX - 1);
      cloudUrlBuf[PL_CONFIG_URL_MAX - 1] = '\0';
      config.cloudServerUrl = cloudUrlBuf;
    }
  }

  prefs.end();
}

void PLConfigStorage::save(const char* ssid, const char* password, const char* localUrl, const char* cloudUrl) {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, false);

  if (ssid && ssid[0] != '\0') {
    prefs.putString(NVS_KEY_SSID, ssid);
  } else {
    prefs.remove(NVS_KEY_SSID);
  }
  if (password) {
    prefs.putString(NVS_KEY_PASSWORD, password);
  } else {
    prefs.putString(NVS_KEY_PASSWORD, "");
  }
  if (localUrl && localUrl[0] != '\0') {
    prefs.putString(NVS_KEY_LOCAL_URL, localUrl);
  } else {
    prefs.remove(NVS_KEY_LOCAL_URL);
  }
  if (cloudUrl && cloudUrl[0] != '\0') {
    prefs.putString(NVS_KEY_CLOUD_URL, cloudUrl);
  } else {
    prefs.remove(NVS_KEY_CLOUD_URL);
  }

  prefs.end();
}
