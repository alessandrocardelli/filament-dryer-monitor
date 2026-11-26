#pragma once
#include <cmath>

// Empirical END-TO-END J7 transfer correction, 2026-11-26 resistor data.
// Estimated from old reported ohms and the unchanged divider model.
// Not an independently measured ADC voltage offset or thermal calibration.
// See docs/THERMAL_EXPERIMENT_LEDGER.md. Physical verification pending.
namespace NtcReadoutCalibration {
inline constexpr float kResidualOffsetMv = 6.8f;

// Apply ONLY to resistance/temperature calculation, after the unchanged
// legacy open/short voltage fault checks. Preserve diagnostic correctedMv.
inline float resistanceOhms(float legacyMv, float supplyMv, float pullupOhms) {
  if (!std::isfinite(legacyMv) || !std::isfinite(supplyMv) ||
      !std::isfinite(pullupOhms) || legacyMv >= supplyMv ||
      pullupOhms <= 0.0f) return NAN;
  const float nodeMv = legacyMv - kResidualOffsetMv;
  if (nodeMv <= 0.0f || nodeMv >= supplyMv) return NAN;
  return pullupOhms * nodeMv / (supplyMv - nodeMv);
}
} // namespace NtcReadoutCalibration
