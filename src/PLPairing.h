#ifndef PLPAIRING_H
#define PLPAIRING_H

#include <Arduino.h>

class PLPairing {
public:
  void begin(int buttonPin);
  void update();

  bool isPressed();
  bool isLongPressed();

private:
  int _pin = -1;
  bool _enabled = false;

  bool _lastReading = HIGH;
  bool _stableState = HIGH;
  unsigned long _lastDebounceMs = 0;

  bool _shortPressFlag = false;
  bool _longPressFlag = false;
  bool _longPressHandled = false;
  unsigned long _pressStartMs = 0;

  static constexpr unsigned long DEBOUNCE_MS = 50;
  static constexpr unsigned long LONG_PRESS_MS = 2000;
};

#endif
