#pragma once
#include <Arduino.h>
#include "AppConfig.h"

class SafetyController {
 public:
  void begin() {
    pinMode(AppConfig::kPinHeaterPwm, OUTPUT);
    forceHeaterOff();
  }

  void update() {
    forceHeaterOff();
  }

  bool heaterPermitted() const { return false; }
  bool heaterLocked() const { return true; }
  bool fanPermitted() const { return !faultActive_; }
  bool faultActive() const { return faultActive_; }

  void setFaultForFutureUse(bool active) {
    faultActive_ = active;
    if (faultActive_) forceHeaterOff();
  }

 private:
  bool faultActive_ = false;

  void forceHeaterOff() {
    digitalWrite(AppConfig::kPinHeaterPwm, LOW);
  }
};
