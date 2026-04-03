#ifndef PLLED_H
#define PLLED_H

#include <Arduino.h>
#include "PLState.h"

class PLLed {
public:
  void begin(int pin);
  void update(PLConnectionState state);

private:
  int _pin = -1;
  bool _enabled = false;
  unsigned long _lastToggleMs = 0;
  bool _ledOn = false;
  int _blinkPhase = 0;

  void blink(unsigned long intervalMs);
  void multiBlink(int count, unsigned long onMs, unsigned long offMs, unsigned long pauseMs);
  void setLed(bool on);
};

#endif
