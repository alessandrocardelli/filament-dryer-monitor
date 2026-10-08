#pragma once
#include <Arduino.h>
#include "AppConfig.h"

enum class DiagnosticLedState : uint8_t { Boot, Normal, Warning, Fault };

class StatusLed {
 public:
  void begin() {
    pinMode(AppConfig::kPinStatusLed, OUTPUT);
    digitalWrite(AppConfig::kPinStatusLed, HIGH); // D021: solid ON during initialization.
  }

  void update(uint32_t nowMs, DiagnosticLedState state) {
    bool on = false;
    switch (state) {
      case DiagnosticLedState::Boot:
        on = true;
        break;
      case DiagnosticLedState::Normal:
        on = (nowMs % 2000U) < 100U;
        break;
      case DiagnosticLedState::Warning: {
        const uint32_t phase = nowMs % 2000U;
        on = phase < 100U || (phase >= 200U && phase < 300U);
        break;
      }
      case DiagnosticLedState::Fault:
        on = (nowMs % 500U) < 250U;
        break;
    }
    digitalWrite(AppConfig::kPinStatusLed, on ? HIGH : LOW);
  }
};
