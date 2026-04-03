#ifndef PLIDENTITY_H
#define PLIDENTITY_H

#include <Arduino.h>
#include "PLConfig.h"

class PLIdentity {
public:
  void begin(PLIdentityMode mode);

  const char* getDeviceId() const;

  bool hasPersistentId() const;

  void resetPersistentId();

  const char* getIdentityModeString() const;

private:
  PLIdentityMode _mode = PL_IDENTITY_NONE;
  char _deviceId[48] = {0};  // "dev-" + UUID (36 chars) + null
  bool _hasPersistent = false;

  void loadOrGeneratePersistentId();
  void generateEphemeralId();
  void generateUUID(char* buffer, size_t bufferSize);
};

#endif
