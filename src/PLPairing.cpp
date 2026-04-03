#include "PLPairing.h"

void PLPairing::begin(int buttonPin) {
  _pin = buttonPin;
  if (_pin < 0) {
    _enabled = false;
    return;
  }
  _enabled = true;
  pinMode(_pin, INPUT_PULLUP);
  _stableState = digitalRead(_pin);
  _lastReading = _stableState;
}

void PLPairing::update() {
  if (!_enabled) return;

  bool reading = digitalRead(_pin);
  unsigned long now = millis();

  if (reading != _lastReading) {
    _lastDebounceMs = now;
  }
  _lastReading = reading;

  if ((now - _lastDebounceMs) < DEBOUNCE_MS) return;

  if (reading != _stableState) {
    bool wasPressed = (_stableState == LOW);
    _stableState = reading;
    bool isNowPressed = (_stableState == LOW);

    if (isNowPressed && !wasPressed) {
      _pressStartMs = now;
      _longPressHandled = false;
    }

    if (!isNowPressed && wasPressed) {
      if (!_longPressHandled) {
        _shortPressFlag = true;
      }
    }
  }

  if (_stableState == LOW && !_longPressHandled) {
    if ((now - _pressStartMs) >= LONG_PRESS_MS) {
      _longPressFlag = true;
      _longPressHandled = true;
    }
  }
}

bool PLPairing::isPressed() {
  if (_shortPressFlag) {
    _shortPressFlag = false;
    return true;
  }
  return false;
}

bool PLPairing::isLongPressed() {
  if (_longPressFlag) {
    _longPressFlag = false;
    return true;
  }
  return false;
}
