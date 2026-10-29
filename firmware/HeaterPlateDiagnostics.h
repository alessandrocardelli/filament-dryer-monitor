#pragma once
#include <cmath>
#include <cstdint>

// Read-only diagnostics. Thermal thresholds are always passed by the caller,
// NEVER hidden here as historical defaults. NEVER used in SafetyController.
// The 7 original pairs are Sanwa PLATE/contact versus heater NTC ohms
// with BOTH covers OPEN, fan OFF and 120s holds (17 October 2026).
// They do not establish NTC bead temperature or safe operating limits.
// Source: docs/NTC_CALIBRATION_ANALYSIS.md
namespace HeaterPlateDiagnostics {

struct Margin {
  bool valid = false;
  float aboveSoftOhms = NAN;
  float aboveHardOhms = NAN;
};

inline Margin margin(float resistanceOhms,
                     float softOhms,
                     float hardOhms) {
  Margin m{};
  if (!std::isfinite(resistanceOhms) || resistanceOhms <= 0.0f ||
      !std::isfinite(softOhms) || !std::isfinite(hardOhms) ||
      softOhms <= hardOhms || hardOhms <= 0.0f) return m;
  m.valid = true;
  m.aboveSoftOhms = resistanceOhms - softOhms;
  m.aboveHardOhms = resistanceOhms - hardOhms;
  return m;
}

struct ContactProxy {
  bool valid = false;
  float sanwaEquivalentC = NAN; // historical comparison ONLY; NOT safety
};

// Sparse, log-R interpolated estimate of historical nearby Sanwa readings.
// Valid only inside *observed* 7.12-64.62kOhm. No extrapolation at cold end.
// Most importantly, DO NOT use this for heater control in a closed box.
inline ContactProxy historicalOpenBoxContact(float rOhms) {
  const float r[] = {64620.0f, 42270.0f, 28720.0f, 19680.0f,
                     13730.0f, 9810.0f, 7120.0f};
  const float sanwa[] = {30.0f, 39.0f, 47.0f, 56.0f,
                          64.0f, 74.0f, 85.0f};
  ContactProxy v{};
  if (!std::isfinite(rOhms) || rOhms > r[0] || rOhms < r[6]) return v;
  for (uint8_t i = 0; i < 6; ++i) {
    if (rOhms <= r[i] && rOhms >= r[i+1]) {
      const float fraction = std::log(r[i] / rOhms) /
                             std::log(r[i] / r[i+1]);
      v.valid = true;
      v.sanwaEquivalentC = sanwa[i] +
                           fraction * (sanwa[i+1] - sanwa[i]);
      return v;
    }
  }
  return v;
}

} // namespace HeaterPlateDiagnostics
