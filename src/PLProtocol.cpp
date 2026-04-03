#include "PLProtocol.h"
#include <ArduinoJson.h>

// Server expects every message to have type, msgId, ts, and payload (envelope).
static void setEnvelope(JsonDocument& doc, const char* type) {
  static unsigned long _msgCount = 0;
  char msgIdBuf[20];
  snprintf(msgIdBuf, sizeof(msgIdBuf), "m%lu", ++_msgCount);
  doc["type"] = type;
  doc["msgId"] = msgIdBuf;
  doc["ts"] = millis();
}

String PLProtocol::buildRegisterMessage(
  const PLConfig& config,
  const char* publishChannels[], int pubCount,
  const char* subscribeChannels[], int subCount,
  const char* deviceId,
  const char* identityMode
) {
  JsonDocument doc;
  setEnvelope(doc, "register");

  JsonObject payload = doc["payload"].to<JsonObject>();
  payload["identityMode"] = identityMode;
  payload["deviceType"] = config.deviceType;
  payload["deviceName"] = config.deviceName;
  if (config.groupName && config.groupName[0] != '\0') {
    payload["groupName"] = config.groupName;
  }
  if (deviceId && deviceId[0] != '\0') {
    payload["deviceId"] = deviceId;
  }

  JsonObject cap = payload["capabilities"].to<JsonObject>();
  JsonArray pub = cap["publish"].to<JsonArray>();
  for (int i = 0; i < pubCount; i++) {
    pub.add(publishChannels[i]);
  }
  JsonArray sub = cap["subscribe"].to<JsonArray>();
  for (int i = 0; i < subCount; i++) {
    sub.add(subscribeChannels[i]);
  }

  String output;
  serializeJson(doc, output);
  return output;
}

String PLProtocol::buildHeartbeatMessage() {
  JsonDocument doc;
  setEnvelope(doc, "heartbeat");
  (void)doc["payload"].to<JsonObject>();  // empty object {} so server does not receive null

  String output;
  serializeJson(doc, output);
  return output;
}

String PLProtocol::buildPairRequestMessage(PLPairingMode mode, unsigned long windowSec) {
  JsonDocument doc;
  setEnvelope(doc, "pair_request");

  const char* modeStr = "auto";
  if (mode == PL_PAIR_TEMPORARY)  modeStr = "temporary";
  if (mode == PL_PAIR_PERSISTENT) modeStr = "persistent";

  JsonObject payload = doc["payload"].to<JsonObject>();
  payload["requestedBindingMode"] = modeStr;
  payload["windowSec"] = windowSec;

  String output;
  serializeJson(doc, output);
  return output;
}

String PLProtocol::buildPublishValueMessage(const char* channel, float value, unsigned long seq) {
  JsonDocument doc;
  setEnvelope(doc, "publish_value");

  JsonObject payload = doc["payload"].to<JsonObject>();
  payload["channel"] = channel;
  payload["valueType"] = "float";
  payload["value"] = value;
  payload["seq"] = seq;

  String output;
  serializeJson(doc, output);
  return output;
}

const char* PLProtocol::parseMessageType(const char* json) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("[PLProto] Parse error: %s\n", err.c_str());
    return nullptr;
  }

  const char* type = doc["type"];
  if (!type) return nullptr;

  strncpy(_typeBuffer, type, sizeof(_typeBuffer) - 1);
  _typeBuffer[sizeof(_typeBuffer) - 1] = '\0';
  return _typeBuffer;
}

bool PLProtocol::parseSessionId(const char* json, char* outSessionId, size_t maxLen) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) return false;

  // Server sends register_ack with payload.sessionId
  JsonObject payload = doc["payload"].as<JsonObject>();
  const char* sid = !payload.isNull() ? payload["sessionId"].as<const char*>() : doc["sessionId"].as<const char*>();
  if (!sid) return false;

  strncpy(outSessionId, sid, maxLen - 1);
  outSessionId[maxLen - 1] = '\0';
  return true;
}

bool PLProtocol::parseDeliverValue(const char* json, char* outChannel, size_t channelMax, float& outValue) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) return false;

  // Server sends deliver_value with payload.channel and payload.value
  JsonObject payload = doc["payload"].as<JsonObject>();
  const char* ch = nullptr;
  if (!payload.isNull()) {
    ch = payload["channel"].as<const char*>();
    outValue = payload["value"].as<float>();
  }
  if (!ch) {
    ch = doc["channel"].as<const char*>();
    outValue = doc["value"] | 0.0f;
  }
  if (!ch) return false;

  strncpy(outChannel, ch, channelMax - 1);
  outChannel[channelMax - 1] = '\0';
  return true;
}

bool PLProtocol::parsePairConfirm(const char* json, char* outPeerId, size_t peerIdMax) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) return false;

  // Server sends pair_confirm with payload.partnerSessionId
  JsonObject payload = doc["payload"].as<JsonObject>();
  const char* peer = nullptr;
  if (!payload.isNull()) {
    peer = payload["partnerSessionId"].as<const char*>();
    if (!peer) peer = payload["partnerDeviceId"].as<const char*>();
  }
  if (!peer) peer = doc["peerId"].as<const char*>();
  if (!peer) return false;

  strncpy(outPeerId, peer, peerIdMax - 1);
  outPeerId[peerIdMax - 1] = '\0';
  return true;
}
