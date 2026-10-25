#pragma once
#include "HeaterPulseCommissioning.h"

// STEP06: PROPOSED supervised fan-on/heater-on test in final closed geometry.
// NOT authorized to run. The checked-in AppConfig has BOTH heat paths OFF.
// Four 10-second segments can show a measurable chamber-air trend; they do
// not certify the dryer at 40/50/55C or the embedded bead temperature.
//
// Historic NTC/Sanwa data came from OPEN enclosure, fan OFF: e.g. R=19.68k
// at nearby Sanwa 56C. This resistance is a PROVISIONAL stop guard, not a
// qualified closed-geometry plate/bead limit. Independent handheld Sanwa
// observation by an operator is required; firmware cannot read that probe.
namespace HeaterStep06Candidate {
constexpr unsigned kPulseCount = 4;
constexpr uint32_t kPulseHardwareCeilingUs = 10500000; // 10.5s hardware ISR
inline HeaterCommissioning::Parameters parameters() {
  HeaterCommissioning::Parameters p{};
  p.prerunMs = 5000;
  p.pulseMs = 10000;
  p.gapMs = 12000;
  p.cooldownMs = 60000;
  p.overallTimeoutMs = 155000;
  p.fanStartupTimeoutMs = 12000;
  p.maxNtcAgeMs = 1500;
  p.maxAirAgeMs = 5000;
  p.pulseCount = kPulseCount;
  p.maximumChamberC = 40.0f;    // air in spool chamber, not heater bead
  p.minimumNtcOhms = 19680.0f;  // historic 56C SANWA-nearby, not bead 56C
  p.maximumNtcOhms = 250000.0f;
  return p;
}
} // namespace HeaterStep06Candidate
