#include "PLSerialConfig.h"
#include "PairLink.h"
#include "PLConfigStorage.h"
#include <string.h>

static const char BOOT_MSGL[] = "---------------------------------------------------------------------------";
static const char BOOT_MSG0[] = "CONFIGURE CONTROLLER USING SERILAL MONITOR IN ARDUINO IDE";
static const char BOOT_MSG1[] = "Serial command to set WiFi Credentials:  wifi,SSID,PASSWORD";
static const char BOOT_MSG2[] = "Serial command to set localServer URL:   server,ws://XXX.XXX.XXX.XXX:8080/ws";
static const char BOOT_MSG3[] = "Serial command to save permanently:      save";

void PLSerialConfig::begin(PairLink* pairLink, Stream* serial) {
  _pairLink = pairLink;
  _serial = serial;
  _lineLen = 0;
  _lineBuf[0] = '\0';
  _bootPrinted = false;
  printBootInstruction();
}

void PLSerialConfig::printBootInstruction() {
  if (_bootPrinted || !_serial) return;
  _bootPrinted = true;
  _serial->println(BOOT_MSGL);
  _serial->println(BOOT_MSG0);
  _serial->println(BOOT_MSGL);
  _serial->println(BOOT_MSG1);
  _serial->println(BOOT_MSG2);
  _serial->println(BOOT_MSG3);
  _serial->println(BOOT_MSGL);
  _serial->println(BOOT_MSGL);
}

void PLSerialConfig::update() {
  if (!_pairLink || !_serial) return;

  while (_serial->available()) {
    int c = _serial->read();
    if (c < 0) break;
    if (c == '\n' || c == '\r') {
      if (_lineLen > 0) {
        _lineBuf[_lineLen] = '\0';
        processLine();
        _lineLen = 0;
      }
      continue;
    }
    if (_lineLen < _lineMax - 1) {
      _lineBuf[_lineLen++] = (char)c;
    } else {
      _lineLen = 0;
    }
  }
}

void PLSerialConfig::processLine() {
  char* line = _lineBuf;
  while (*line == ' ' || *line == '\t') line++;
  if (*line == '\0') return;

  if (strncmp(line, "wifi,", 5) == 0) {
    char* rest = line + 5;
    char* comma = strchr(rest, ',');
    char ssid[PL_CONFIG_SSID_MAX] = {0};
    char password[PL_CONFIG_PASSWORD_MAX] = {0};
    if (comma) {
      size_t ssidLen = (size_t)(comma - rest);
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
    _pairLink->setNetworkAndServer(ssid, password,
      _pairLink->getLocalServerUrl(),
      _pairLink->getCloudServerUrl()[0] ? _pairLink->getCloudServerUrl() : nullptr);
    if (_serial) _serial->println("[PLSerialConfig] WiFi set, reconnecting");
    return;
  }

  if (strncmp(line, "server,", 7) == 0) {
    char* url = line + 7;
    while (*url == ' ') url++;
    if (*url != '\0') {
      _pairLink->setNetworkAndServer(
        _pairLink->getDefaultSsid(),
        _pairLink->getDefaultPassword(),
        url,
        _pairLink->getCloudServerUrl()[0] ? _pairLink->getCloudServerUrl() : nullptr);
      if (_serial) _serial->println("[PLSerialConfig] Server URL set, reconnecting");
    }
    return;
  }

  if (strcmp(line, "save") == 0) {
    PLConfigStorage::save(
      _pairLink->getDefaultSsid(),
      _pairLink->getDefaultPassword(),
      _pairLink->getLocalServerUrl(),
      _pairLink->getCloudServerUrl()[0] ? _pairLink->getCloudServerUrl() : nullptr);
    if (_serial) _serial->println("[PLSerialConfig] Config saved to NVS");
    return;
  }
}
