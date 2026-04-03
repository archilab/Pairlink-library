#ifndef PLPROTOCOL_H
#define PLPROTOCOL_H

#include <Arduino.h>
#include "PLConfig.h"

class PLProtocol {
public:
  // Buffer size for most outgoing messages (512 bytes).
  // register messages with many channels may need the full buffer.
  static constexpr size_t MSG_BUF_SIZE = 512;

  String buildRegisterMessage(
    const PLConfig& config,
    const char* publishChannels[], int pubCount,
    const char* subscribeChannels[], int subCount,
    const char* deviceId = "",
    const char* identityMode = "none"
  );

  String buildHeartbeatMessage();

  String buildPairRequestMessage(PLPairingMode mode, unsigned long windowSec);

  String buildPublishValueMessage(const char* channel, float value, unsigned long seq);

  const char* parseMessageType(const char* json);

  bool parseSessionId(const char* json, char* outSessionId, size_t maxLen);

  bool parseDeliverValue(const char* json, char* outChannel, size_t channelMax, float& outValue);

  bool parsePairConfirm(const char* json, char* outPeerId, size_t peerIdMax);

private:
  // Reusable parse buffer kept in the class to avoid repeated stack allocation.
  // 512 bytes covers all expected incoming messages.
  char _typeBuffer[64] = {0};
};

#endif
