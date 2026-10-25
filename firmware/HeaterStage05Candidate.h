#pragma once
#include "HeaterPulseCommissioning.h"

// PROPOSED OFFLINE PROFILE ONLY. NO USER AUTHORIZATION TO HEAT.
// This header feeds a Sequence that is inert in the default firmware:
// kHeaterCommissioningMode == false and kHeaterStage05TrialReviewed == false.
// A future physical experiment requires separate review, explicit user
// permission, a uniquely identified image, preflight and stop criteria.
// These are temporary characterization aborts, NOT V1 thermal safety limits.
namespace HeaterStage05Candidate {
inline HeaterCommissioning::Parameters parameters() {
  HeaterCommissioning::Parameters p{};
  p.prerunMs = 5000;
  p.pulseMs = 1500;
  p.gapMs = 12000;
  p.cooldownMs = 60000;
  p.overallTimeoutMs = 140000;
  p.fanStartupTimeoutMs = 12000;
  p.maxNtcAgeMs = 1500;
  p.maxAirAgeMs = 5000;
  p.pulseCount = 4;             // candidate only: +1 pulse vs approved past test
  p.maximumChamberC = 35.0f;   // chamber AIR SHT45, not plate/bead
  p.minimumNtcOhms = 42270.0f; // OPEN-CASE 39C Sanwa proxy, not NTC temp
  p.maximumNtcOhms = 250000.0f;
  return p;
}
} // namespace HeaterStage05Candidate
