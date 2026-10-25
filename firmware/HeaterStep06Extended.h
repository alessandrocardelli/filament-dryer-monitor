#pragma once
#include "HeaterPulseCommissioning.h"

// STEP06B — one authorized-development experimental candidate, NOT a V1
// air-temperature thermostat. This is a separate branch from the physically
// tested S6A and web-fixed S6B builds.
// A single test consists of 6 x 20s ON windows, 12s OFF between windows,
// fan at 100% PWM throughout, 90s cooldown and strict abort conditions.
// NTC limit is a RAW RESISTANCE trip, not an NTC bead temperature estimate.
// A handheld Sanwa is independent and MUST be monitored by the operator;
// the ESP32 does not read Sanwa or fan RPM/airflow.
namespace HeaterStep06Extended {
constexpr unsigned kPulseCount = 6;
constexpr uint32_t kPulseHardwareCeilingUs = 20500000; // max 20.5s HIGH per window
inline HeaterCommissioning::Parameters parameters() {
  HeaterCommissioning::Parameters p{};
  p.prerunMs = 5000;
  p.pulseMs = 20000;
  p.gapMs = 12000;
  p.cooldownMs = 90000;
  p.overallTimeoutMs = 330000;
  p.fanStartupTimeoutMs = 12000;
  p.maxNtcAgeMs = 1500;
  p.maxAirAgeMs = 5000;
  p.pulseCount = kPulseCount;
  p.maximumChamberC = 40.0f;    // SHT45 air limit, NOT heater temperature
  p.minimumNtcOhms = 19680.0f;  // old open-case Sanwa-near-heater ~56C proxy
  p.maximumNtcOhms = 250000.0f;
  return p;
}
} // namespace HeaterStep06Extended
