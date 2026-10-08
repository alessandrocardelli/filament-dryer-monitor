#pragma once
#include <Arduino.h>

enum class AppState : uint8_t {
  Boot,
  Standby,
  Setup,
  Drying,
  Complete,
  Fault,
};

enum class SetupField : uint8_t {
  Temperature,
  Duration,
  Fan,
};

struct CycleSettings {
  int16_t targetTempC = 55;       // UI prototype value only; not a validated control limit.
  uint16_t durationMinutes = 240; // UI prototype value only.
};

struct SensorSnapshot {
  bool valid = false;
  float temperatureC = NAN;
  float humidityRh = NAN;
  uint32_t lastSuccessMs = 0;
};
