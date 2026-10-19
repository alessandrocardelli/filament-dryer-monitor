#pragma once
#include <Arduino.h>

enum class AppState : uint8_t {
  Boot,
  Standby,
  Setup,
  Settings,
  Drying,
  Complete,
  Fault,
};

enum class SetupField : uint8_t {
  Temperature,
  Duration,
  Fan,
};

enum class SettingsField : uint8_t {
  BuzzerVolume,
  KeyClick,
};

enum class BuzzerVolume : uint8_t {
  Off = 0,
  Low = 1,
  Medium = 2,
  High = 3,
};

struct CycleSettings {
  int16_t targetTempC = 55;
  uint16_t durationMinutes = 240;
};

struct PersistentSettings {
  CycleSettings cycle{};
  BuzzerVolume buzzerVolume = BuzzerVolume::Low;
  bool keyClick = false;
};

struct SensorSnapshot {
  bool valid = false;
  float temperatureC = NAN;
  float humidityRh = NAN;
  uint32_t lastSuccessMs = 0;
};
