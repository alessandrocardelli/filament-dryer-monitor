#pragma once
#include "HeaterPulseCommissioning.h"

// STEP07-G: RAW NTC RESISTANCE GUARD QUALIFICATION, not V1 temperature
// regulation, not a calibrated Sanwa/NTC conversion.
// Origin: physically executed S6C, same FAN 100% and same pulse/time
// upper bounds. S6C reached R_NTC=30.32kΩ with Sanwa contact 53C;
// the earlier 15-Oct dynamic test already measured up to +6C indicated
// NTC-vs-Sanwa offset on heating, reversing sign during cooling.
// 40kΩ is a deliberately EARLIER software abort than the old 19.68kΩ,
// NOT evidence that 40kΩ corresponds to any fixed plate temperature.
// An operator must still watch Sanwa and verify contact, airflow and
// disconnect 24V on any anomaly. This test is not a heater setpoint.
namespace HeaterStep07GuardTrial {
constexpr uint32_t kPulseHardwareCeilingUs = 20500000; // ESP32 20.5s ISR
constexpr float kConservativeRawNtcTripOhms = 40000.0f;
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
  p.pulseCount = 6;
  p.maximumChamberC = 40.0f;    // SHT45 AIR experimental ceiling
  p.minimumNtcOhms = kConservativeRawNtcTripOhms; // early stop, no °C claim
  p.maximumNtcOhms = 250000.0f;
  return p;
}
} // namespace HeaterStep07GuardTrial
