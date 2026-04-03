#ifndef PLSERIALCONFIG_H
#define PLSERIALCONFIG_H

#include <Arduino.h>

class PairLink;

/**
 * Optional serial command parser for runtime WiFi/server config.
 * Commands (one per line): wifi,SSID,password | server,ws://host:port/path | save
 * In begin() prints a one-line boot instruction. Call update() in loop().
 */
class PLSerialConfig {
public:
  /** Serial = &Serial by default. Prints boot instruction once. */
  void begin(PairLink* pairLink, Stream* serial = &Serial);
  /** Non-blocking: read available input, parse complete lines, execute. */
  void update();

private:
  PairLink* _pairLink = nullptr;
  Stream* _serial = nullptr;
  static constexpr size_t _lineMax = 256;
  char _lineBuf[256] = {0};
  size_t _lineLen = 0;
  bool _bootPrinted = false;

  void printBootInstruction();
  void processLine();
};

#endif
