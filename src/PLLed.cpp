#include "PLLed.h"

void PLLed::begin(int pin) {
  _pin = pin;
  if (_pin < 0) {
    _enabled = false;
    return;
  }
  _enabled = true;
  pinMode(_pin, OUTPUT);
  setLed(false);
}

void PLLed::update(PLConnectionState state) {
  if (!_enabled) return;

  switch (state) {
    case PL_STATE_BOOTING:
      blink(500);  // 1 Hz
      break;

    case PL_STATE_WIFI_CONNECTING:
      blink(125);  // 4 Hz
      break;

    case PL_STATE_LOCAL_CONNECTING:
      multiBlink(2, 100, 100, 600);
      break;

    case PL_STATE_LOCAL_CONNECTED:
      setLed(true);
      break;

    case PL_STATE_CLOUD_CONNECTING:
      multiBlink(3, 100, 100, 600);
      break;

    case PL_STATE_CLOUD_CONNECTED: {
      unsigned long cycle = millis() % 3000;
      setLed(cycle > 100);  // solid on, brief off every 3s
      break;
    }

    case PL_STATE_RECONNECTING: {
      // Alternating fast/slow: 2s fast then 2s slow
      unsigned long phase = (millis() / 2000) % 2;
      blink(phase == 0 ? 100 : 400);
      break;
    }

    case PL_STATE_OFFLINE_IDLE:
      setLed(false);
      break;

    case PL_STATE_ERROR:
      blink(50);  // 10 Hz
      break;
  }
}

void PLLed::blink(unsigned long intervalMs) {
  unsigned long now = millis();
  if (now - _lastToggleMs >= intervalMs) {
    _lastToggleMs = now;
    _ledOn = !_ledOn;
    setLed(_ledOn);
  }
}

void PLLed::multiBlink(int count, unsigned long onMs, unsigned long offMs, unsigned long pauseMs) {
  unsigned long cycleLen = (unsigned long)count * (onMs + offMs) + pauseMs;
  unsigned long pos = millis() % cycleLen;

  for (int i = 0; i < count; i++) {
    unsigned long blinkStart = (unsigned long)i * (onMs + offMs);
    if (pos >= blinkStart && pos < blinkStart + onMs) {
      setLed(true);
      return;
    }
  }
  setLed(false);
}

void PLLed::setLed(bool on) {
  if (!_enabled) return;
  _ledOn = on;
  digitalWrite(_pin, on ? HIGH : LOW);
}
