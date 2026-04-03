#include "PLIdentity.h"
#include <Preferences.h>

static const char* NVS_NAMESPACE = "pairlink";
static const char* NVS_KEY_DEVICE_ID = "deviceId";

void PLIdentity::begin(PLIdentityMode mode) {
  _mode = mode;
  _deviceId[0] = '\0';

  switch (mode) {
    case PL_IDENTITY_NONE:
      break;
    case PL_IDENTITY_EPHEMERAL:
      generateEphemeralId();
      break;
    case PL_IDENTITY_PERSISTENT:
      loadOrGeneratePersistentId();
      break;
  }
}

const char* PLIdentity::getDeviceId() const {
  return _deviceId;
}

bool PLIdentity::hasPersistentId() const {
  return _hasPersistent;
}

void PLIdentity::resetPersistentId() {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, false);
  prefs.remove(NVS_KEY_DEVICE_ID);
  prefs.end();

  _hasPersistent = false;
  _deviceId[0] = '\0';

  if (_mode == PL_IDENTITY_PERSISTENT) {
    loadOrGeneratePersistentId();
  }
}

const char* PLIdentity::getIdentityModeString() const {
  switch (_mode) {
    case PL_IDENTITY_EPHEMERAL: return "ephemeral";
    case PL_IDENTITY_PERSISTENT: return "persistent";
    default: return "none";
  }
}

void PLIdentity::loadOrGeneratePersistentId() {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, true);

  if (prefs.isKey(NVS_KEY_DEVICE_ID)) {
    String storedId = prefs.getString(NVS_KEY_DEVICE_ID, "");
    prefs.end();

    if (storedId.length() > 0) {
      strncpy(_deviceId, storedId.c_str(), sizeof(_deviceId) - 1);
      _deviceId[sizeof(_deviceId) - 1] = '\0';
      _hasPersistent = true;
      Serial.printf("[PLIdentity] Loaded persistent ID: %s\n", _deviceId);
      return;
    }
  }
  prefs.end();

  char uuid[40];
  generateUUID(uuid, sizeof(uuid));
  snprintf(_deviceId, sizeof(_deviceId), "dev-%s", uuid);

  prefs.begin(NVS_NAMESPACE, false);
  prefs.putString(NVS_KEY_DEVICE_ID, _deviceId);
  prefs.end();

  _hasPersistent = true;
  Serial.printf("[PLIdentity] Generated new persistent ID: %s\n", _deviceId);
}

void PLIdentity::generateEphemeralId() {
  char uuid[40];
  generateUUID(uuid, sizeof(uuid));
  snprintf(_deviceId, sizeof(_deviceId), "dev-%s", uuid);
  Serial.printf("[PLIdentity] Generated ephemeral ID: %s\n", _deviceId);
}

void PLIdentity::generateUUID(char* buffer, size_t bufferSize) {
  uint8_t bytes[16];
  for (int i = 0; i < 16; i++) {
    bytes[i] = (uint8_t)esp_random();
  }

  // RFC 4122 v4: set version and variant bits
  bytes[6] = (bytes[6] & 0x0F) | 0x40;
  bytes[8] = (bytes[8] & 0x3F) | 0x80;

  snprintf(buffer, bufferSize,
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    bytes[0], bytes[1], bytes[2], bytes[3],
    bytes[4], bytes[5],
    bytes[6], bytes[7],
    bytes[8], bytes[9],
    bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
}
