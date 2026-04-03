#include "PairLink.h"
#include "PLConfigStorage.h"
#include <string.h>
#include <stdarg.h>

// Helper to print debug messages (when debugMode is enabled)
#define DEBUG_PRINT(msg) if (_config.debugMode) { Serial.print("[PairLink] "); Serial.println(msg); }
#define DEBUG_PRINTF(fmt, ...) if (_config.debugMode) { Serial.printf("[PairLink] " fmt, ##__VA_ARGS__); }

// Helper to print general messages (always shown)
#define INFO_PRINT(msg) Serial.println(msg)
#define INFO_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)

static const char BOOT_MSGL[] = "---------------------------------------------------------------------------";
static const char BOOT_MSG0[] = "CONFIGURE CONTROLLER USING SERIAL MONITOR IN ARDUINO IDE";
static const char BOOT_MSG1[] = "Serial command to set WiFi Credentials:  wifi,SSID,PASSWORD";
static const char BOOT_MSG2[] = "Serial command to set localServer URL:   server,ws://XXX.XXX.XXX.XXX:8080/ws";
static const char BOOT_MSG3[] = "Serial command to save permanently:      save";

void PairLink::beginSimple(const char* ssid, const char* password, const char* serverUrl) {
  beginSimple(ssid, password, serverUrl, -1, -1);
}

void PairLink::beginSimple(const char* ssid, const char* password, const char* serverUrl,
                           int pairButtonPin, int statusLedPin) {
  PLConfig config;
  plConfigSetLocalOnly(config, ssid, password, serverUrl);
  config.pairButtonPin = pairButtonPin;     // Pass -1 to disable
  config.statusLedPin = statusLedPin;       // Pass -1 to disable
  begin(config);
}

void PairLink::begin(const PLConfig& config) {
  _serialConfigEnabled = config.enableSerialConfig;

  _config = config;
  if (_serialConfigEnabled) {
    PLConfigStorage::load(_config);
  }

  if (_config.defaultSsid) {
    strncpy(_configSsid, _config.defaultSsid, PL_CONFIG_SSID_MAX - 1);
    _configSsid[PL_CONFIG_SSID_MAX - 1] = '\0';
    _config.defaultSsid = _configSsid;
  }
  if (_config.defaultPassword) {
    strncpy(_configPassword, _config.defaultPassword, PL_CONFIG_PASSWORD_MAX - 1);
    _configPassword[PL_CONFIG_PASSWORD_MAX - 1] = '\0';
    _config.defaultPassword = _configPassword;
  } else {
    _configPassword[0] = '\0';
    _config.defaultPassword = _configPassword;
  }
  if (_config.localServerUrl) {
    strncpy(_configLocalUrl, _config.localServerUrl, PL_CONFIG_URL_MAX - 1);
    _configLocalUrl[PL_CONFIG_URL_MAX - 1] = '\0';
    _config.localServerUrl = _configLocalUrl;
  }
  if (_config.cloudServerUrl && _config.cloudServerUrl[0] != '\0') {
    strncpy(_configCloudUrl, _config.cloudServerUrl, PL_CONFIG_URL_MAX - 1);
    _configCloudUrl[PL_CONFIG_URL_MAX - 1] = '\0';
    _config.cloudServerUrl = _configCloudUrl;
  } else {
    _configCloudUrl[0] = '\0';
    _config.cloudServerUrl = nullptr;
  }

  _publishChannelCount = 0;
  _subscribeChannelCount = 0;
  _paired = false;
  _publishSeq = 0;
  _localRetryCount = 0;
  _lastLocalProbeMs = 0;
  memset(_sessionId, 0, sizeof(_sessionId));
  memset(_lastPublish, 0, sizeof(_lastPublish));

  setState(PL_STATE_BOOTING);

  _identity.begin(_config.identityMode);
  _pairing.begin(_config.pairButtonPin);
  _led.begin(_config.statusLedPin);

  _ws.onMessage([](void* ctx, const char* payload) {
    static_cast<PairLink*>(ctx)->handleIncomingMessage(payload);
  }, this);

  setState(PL_STATE_WIFI_CONNECTING);
  _wifi.begin(_config.defaultSsid, _config.defaultPassword);
}

void PairLink::setNetworkAndServer(const char* ssid, const char* password, const char* localUrl, const char* cloudUrl) {
  if (ssid && ssid[0] != '\0') {
    strncpy(_configSsid, ssid, PL_CONFIG_SSID_MAX - 1);
    _configSsid[PL_CONFIG_SSID_MAX - 1] = '\0';
    _config.defaultSsid = _configSsid;
  }
  if (password) {
    strncpy(_configPassword, password, PL_CONFIG_PASSWORD_MAX - 1);
    _configPassword[PL_CONFIG_PASSWORD_MAX - 1] = '\0';
    _config.defaultPassword = _configPassword;
  } else {
    _configPassword[0] = '\0';
    _config.defaultPassword = _configPassword;
  }
  if (localUrl && localUrl[0] != '\0') {
    strncpy(_configLocalUrl, localUrl, PL_CONFIG_URL_MAX - 1);
    _configLocalUrl[PL_CONFIG_URL_MAX - 1] = '\0';
    _config.localServerUrl = _configLocalUrl;
  }
  if (cloudUrl && cloudUrl[0] != '\0') {
    strncpy(_configCloudUrl, cloudUrl, PL_CONFIG_URL_MAX - 1);
    _configCloudUrl[PL_CONFIG_URL_MAX - 1] = '\0';
    _config.cloudServerUrl = _configCloudUrl;
  } else {
    _configCloudUrl[0] = '\0';
    _config.cloudServerUrl = nullptr;
  }

  _ws.disconnect();
  _wifi.reconfigure(_config.defaultSsid, _config.defaultPassword);
  setState(PL_STATE_WIFI_CONNECTING);
}

void PairLink::initiateConnection() {
  _localRetryCount = 0;
  if (_config.cloudServerUrl) {
    _ws.beginWithFallback(_config.localServerUrl, _config.cloudServerUrl);
  } else {
    _ws.begin(_config.localServerUrl);
  }
}

void PairLink::update() {
  unsigned long now = millis();

  _wifi.update();
  _pairing.update();
  _led.update(_connectionState);

  switch (_connectionState) {
    case PL_STATE_BOOTING:
      break;

    case PL_STATE_WIFI_CONNECTING:
      if (_wifi.isConnected()) {
        setState(PL_STATE_LOCAL_CONNECTING);
        initiateConnection();
      }
      break;

    case PL_STATE_LOCAL_CONNECTING:
      _ws.update();
      if (_ws.isConnected()) {
        _localRetryCount = 0;
        setState(PL_STATE_LOCAL_CONNECTED);
        sendRegister();
      } else if (_ws.getConsecutiveFailures() >= MAX_LOCAL_RETRIES
                 && _ws.hasFallback()) {
        Serial.printf("[PairLink] Local failed %d times, switching to cloud\n",
          _ws.getConsecutiveFailures());
        _ws.switchTo(PLWS_TARGET_FALLBACK);
        setState(PL_STATE_CLOUD_CONNECTING);
      }
      if (!_wifi.isConnected()) {
        _ws.disconnect();
        setState(PL_STATE_WIFI_CONNECTING);
      }
      break;

    case PL_STATE_LOCAL_CONNECTED:
      _ws.update();

      if (!_ws.isConnected()) {
        setState(PL_STATE_RECONNECTING);
        break;
      }
      if (!_wifi.isConnected()) {
        _ws.disconnect();
        setState(PL_STATE_WIFI_CONNECTING);
        break;
      }

      if (now - _lastHeartbeat >= _config.heartbeatIntervalMs) {
        sendHeartbeat();
        _lastHeartbeat = now;
      }

      if (_pairing.isPressed()) {
        Serial.println("[PairLink] Pairing button short press");
        String msg = _protocol.buildPairRequestMessage(
          _config.pairingMode, _config.pairingWindowSec
        );
        _ws.send(msg.c_str());
      }
      if (_pairing.isLongPressed()) {
        Serial.println("[PairLink] Pairing button long press - persistent pair");
        String msg = _protocol.buildPairRequestMessage(
          PL_PAIR_PERSISTENT, _config.pairingWindowSec
        );
        _ws.send(msg.c_str());
      }
      break;

    case PL_STATE_CLOUD_CONNECTING:
      _ws.update();
      if (_ws.isConnected()) {
        setState(PL_STATE_CLOUD_CONNECTED);
        _lastLocalProbeMs = now;
        sendRegister();
      }
      if (!_wifi.isConnected()) {
        _ws.disconnect();
        setState(PL_STATE_WIFI_CONNECTING);
      }
      break;

    case PL_STATE_CLOUD_CONNECTED:
      _ws.update();
      if (!_ws.isConnected()) {
        setState(PL_STATE_RECONNECTING);
        break;
      }
      if (!_wifi.isConnected()) {
        _ws.disconnect();
        setState(PL_STATE_WIFI_CONNECTING);
        break;
      }

      if (now - _lastHeartbeat >= _config.heartbeatIntervalMs) {
        sendHeartbeat();
        _lastHeartbeat = now;
      }

      if (_pairing.isPressed()) {
        String msg = _protocol.buildPairRequestMessage(
          _config.pairingMode, _config.pairingWindowSec
        );
        _ws.send(msg.c_str());
      }
      if (_pairing.isLongPressed()) {
        String msg = _protocol.buildPairRequestMessage(
          PL_PAIR_PERSISTENT, _config.pairingWindowSec
        );
        _ws.send(msg.c_str());
      }

      if (now - _lastLocalProbeMs >= LOCAL_PROBE_INTERVAL_MS) {
        _lastLocalProbeMs = now;
        Serial.println("[PairLink] Probing local server availability...");
        _ws.switchTo(PLWS_TARGET_PRIMARY);
        setState(PL_STATE_LOCAL_CONNECTING);
      }
      break;

    case PL_STATE_RECONNECTING:
      if (!_wifi.isConnected()) {
        setState(PL_STATE_WIFI_CONNECTING);
        break;
      }
      _localRetryCount = 0;
      if (_config.cloudServerUrl) {
        _ws.beginWithFallback(_config.localServerUrl, _config.cloudServerUrl);
      } else {
        _ws.begin(_config.localServerUrl);
      }
      setState(PL_STATE_LOCAL_CONNECTING);
      break;

    case PL_STATE_OFFLINE_IDLE:
    case PL_STATE_ERROR:
      break;
  }

  if (_serialConfigEnabled) {
    handleSerialInput();
  }
}

void PairLink::publish(const char* channel, float value, unsigned long intervalMs) {
  // Check if we are connected to a server
  if (_connectionState != PL_STATE_LOCAL_CONNECTED &&
      _connectionState != PL_STATE_CLOUD_CONNECTED) {
    DEBUG_PRINTF("Cannot publish: not connected (state: %s)\n", 
      plStateToString(_connectionState));
    return;
  }

  // Check if the channel is registered for publishing
  int idx = findPublishChannelIndex(channel);
  if (idx < 0) {
    DEBUG_PRINTF("Cannot publish: channel '%s' not in publish list\n", channel);
    return;
  }

  unsigned long now = millis();
  if (intervalMs > 0 && (now - _lastPublish[idx]) < intervalMs) {
    return;
  }
  _lastPublish[idx] = now;

  String msg = _protocol.buildPublishValueMessage(channel, value, _publishSeq++);
  _ws.send(msg.c_str());
}

void PairLink::publishImmediate(const char* channel, float value) {
  publish(channel, value, 0);
}

void PairLink::addPublishChannel(const char* channel) {
  if (_publishChannelCount >= MAX_CHANNELS) {
    Serial.println("[PairLink] Max publish channels reached");
    return;
  }
  _publishChannels[_publishChannelCount++] = channel;
}

void PairLink::addSubscribeChannel(const char* channel) {
  if (_subscribeChannelCount >= MAX_CHANNELS) {
    Serial.println("[PairLink] Max subscribe channels reached");
    return;
  }
  _subscribeChannels[_subscribeChannelCount++] = channel;
}

void PairLink::onValue(PLValueCallback callback) {
  _valueCallback = callback;
}

void PairLink::onChannel(const char* channel, PLChannelValueCallback callback) {
  for (int i = 0; i < _subscribeChannelCount; i++) {
    if (strcmp(_subscribeChannels[i], channel) == 0) {
      _channelCallbacks[i] = callback;
      return;
    }
  }
  Serial.printf("[PairLink] onChannel: '%s' not in subscribe list, call addSubscribeChannel first\n", channel);
}

void PairLink::onStatus(PLStatusCallback callback) {
  _statusCallback = callback;
}

PLConnectionState PairLink::getState() const {
  return _connectionState;
}

const char* PairLink::getSessionId() const {
  return _sessionId;
}

const char* PairLink::getDeviceId() const {
  return _identity.getDeviceId();
}

bool PairLink::isPaired() const {
  return _paired;
}

const char* PairLink::getDefaultSsid() const {
  return _config.defaultSsid ? _config.defaultSsid : "";
}

const char* PairLink::getDefaultPassword() const {
  return _config.defaultPassword ? _config.defaultPassword : "";
}

const char* PairLink::getLocalServerUrl() const {
  return _config.localServerUrl ? _config.localServerUrl : "";
}

const char* PairLink::getCloudServerUrl() const {
  return _config.cloudServerUrl ? _config.cloudServerUrl : "";
}

void PairLink::handleIncomingMessage(const char* payload) {
  const char* type = _protocol.parseMessageType(payload);
  if (!type) {
    Serial.println("[PairLink] Unknown message format");
    return;
  }

  if (strcmp(type, "register_ack") == 0) {
    if (_protocol.parseSessionId(payload, _sessionId, sizeof(_sessionId))) {
      Serial.printf("[PairLink] Registered, sessionId: %s\n", _sessionId);
    }
  } else if (strcmp(type, "pair_confirm") == 0) {
    char peerId[32] = {0};
    if (_protocol.parsePairConfirm(payload, peerId, sizeof(peerId))) {
      _paired = true;
      Serial.printf("[PairLink] Paired with: %s\n", peerId);
    }
  } else if (strcmp(type, "deliver_value") == 0) {
    char channel[64] = {0};
    float value = 0.0f;
    if (_protocol.parseDeliverValue(payload, channel, sizeof(channel), value)) {
      bool handled = false;
      for (int i = 0; i < _subscribeChannelCount; i++) {
        if (strcmp(_subscribeChannels[i], channel) == 0 && _channelCallbacks[i]) {
          _channelCallbacks[i](value);
          handled = true;
          break;
        }
      }
      if (!handled && _valueCallback) {
        _valueCallback(channel, value);
      }
    }
  } else if (strcmp(type, "error") == 0) {
    Serial.printf("[PairLink] Server error: %s\n", payload);
  } else {
    Serial.printf("[PairLink] Unhandled message type: %s\n", type);
  }
}

void PairLink::sendRegister() {
  String msg = _protocol.buildRegisterMessage(
    _config,
    _publishChannels, _publishChannelCount,
    _subscribeChannels, _subscribeChannelCount,
    _identity.getDeviceId(),
    _identity.getIdentityModeString()
  );
  _ws.send(msg.c_str());
  Serial.println("[PairLink] Sent register");
}

void PairLink::sendHeartbeat() {
  String msg = _protocol.buildHeartbeatMessage();
  _ws.send(msg.c_str());
}

void PairLink::setState(PLConnectionState newState) {
  if (_connectionState == newState) return;

  Serial.printf("[PairLink] State: %s -> %s\n",
    plStateToString(_connectionState), plStateToString(newState));

  _connectionState = newState;

  if (_statusCallback) {
    _statusCallback(newState);
  }
}

int PairLink::findPublishChannelIndex(const char* channel) const {
  for (int i = 0; i < _publishChannelCount; i++) {
    if (strcmp(_publishChannels[i], channel) == 0) {
      return i;
    }
  }
  return -1;
}

void PairLink::handleSerialInput() {
  if (!_serial) return;

  if (!_serialBootPrinted) {
    _serialBootPrinted = true;
    _serial->println(BOOT_MSGL);
    _serial->println(BOOT_MSG0);
    _serial->println(BOOT_MSGL);
    _serial->println(BOOT_MSG1);
    _serial->println(BOOT_MSG2);
    _serial->println(BOOT_MSG3);
    _serial->println(BOOT_MSGL);
    _serial->println(BOOT_MSGL);
  }

  while (_serial->available()) {
    int c = _serial->read();
    if (c < 0) break;
    if (c == '\n' || c == '\r') {
      if (_serialLineLen > 0) {
        _serialLineBuf[_serialLineLen] = '\0';
        processSerialLine();
        _serialLineLen = 0;
      }
      continue;
    }
    if (_serialLineLen < _serialLineMax - 1) {
      _serialLineBuf[_serialLineLen++] = static_cast<char>(c);
    } else {
      _serialLineLen = 0;
    }
  }
}

void PairLink::processSerialLine() {
  char* line = _serialLineBuf;
  while (*line == ' ' || *line == '\t') line++;
  if (*line == '\0') return;

  if (strncmp(line, "wifi,", 5) == 0) {
    char* rest = line + 5;
    char* comma = strchr(rest, ',');
    char ssid[PL_CONFIG_SSID_MAX] = {0};
    char password[PL_CONFIG_PASSWORD_MAX] = {0};
    if (comma) {
      size_t ssidLen = static_cast<size_t>(comma - rest);
      if (ssidLen >= PL_CONFIG_SSID_MAX) ssidLen = PL_CONFIG_SSID_MAX - 1;
      strncpy(ssid, rest, ssidLen);
      ssid[ssidLen] = '\0';
      const char* passStart = comma + 1;
      strncpy(password, passStart, PL_CONFIG_PASSWORD_MAX - 1);
      password[PL_CONFIG_PASSWORD_MAX - 1] = '\0';
    } else {
      strncpy(ssid, rest, PL_CONFIG_SSID_MAX - 1);
      ssid[PL_CONFIG_SSID_MAX - 1] = '\0';
    }
    setNetworkAndServer(
      ssid,
      password,
      getLocalServerUrl(),
      getCloudServerUrl()[0] ? getCloudServerUrl() : nullptr
    );
    if (_serial) _serial->println("[PairLink] WiFi set, reconnecting");
    return;
  }

  if (strncmp(line, "server,", 7) == 0) {
    char* url = line + 7;
    while (*url == ' ') url++;
    if (*url != '\0') {
      setNetworkAndServer(
        getDefaultSsid(),
        getDefaultPassword(),
        url,
        getCloudServerUrl()[0] ? getCloudServerUrl() : nullptr
      );
      if (_serial) _serial->println("[PairLink] Server URL set, reconnecting");
    }
    return;
  }

  if (strcmp(line, "save") == 0) {
    PLConfigStorage::save(
      getDefaultSsid(),
      getDefaultPassword(),
      getLocalServerUrl(),
      getCloudServerUrl()[0] ? getCloudServerUrl() : nullptr
    );
    if (_serial) _serial->println("[PairLink] Config saved to NVS");
    return;
  }
}
