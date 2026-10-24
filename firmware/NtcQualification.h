#pragma once
#include <cmath>
#include <limits>

// Diagnostic hypotheses only. Do not use these conversions for GPIO19,
// temperature regulation, heater cutoffs, or approval of the bead model.
// The SHT45 is an air sensor: comparing it with the heater NTC requires
// actual cold thermal equilibrium; agreement does NOT validate hot behavior.
namespace NtcQualification {
struct Hypothesis {
  const char *name;
  float r25Ohms;
  float betaK;
};
// Historical donor-derived values (not a confirmed thermistor identity).
constexpr Hypothesis kDonor{"Donor 82.5k B4100", 82500.0f, 4100.0f};
// Alternative discussed in docs/NTC_CALIBRATION_ANALYSIS.md, NOT verified.
constexpr Hypothesis kAlternative{"Hypothesis 100k B3950", 100000.0f, 3950.0f};

inline float nan() { return std::numeric_limits<float>::quiet_NaN(); }

inline float expectedResistanceOhms(float temperatureC, const Hypothesis &m) {
  if (!std::isfinite(temperatureC) ||
      temperatureC <= -40.0f || temperatureC >= 125.0f ||
      !(m.r25Ohms > 0.0f) || !(m.betaK > 0.0f)) return nan();
  const float kelvin = temperatureC + 273.15f;
  const float result = m.r25Ohms *
      std::exp(m.betaK * (1.0f / kelvin - 1.0f / 298.15f));
  return std::isfinite(result) && result > 0.0f ? result : nan();
}

inline float estimatedBeadC(float ohms, const Hypothesis &m) {
  if (!std::isfinite(ohms) || !(ohms > 0.0f) ||
      !(m.r25Ohms > 0.0f) || !(m.betaK > 0.0f)) return nan();
  const float reciprocalK = 1.0f / 298.15f +
                            std::log(ohms / m.r25Ohms) / m.betaK;
  if (!(reciprocalK > 0.0f)) return nan();
  const float c = 1.0f / reciprocalK - 273.15f;
  return std::isfinite(c) ? c : nan();
}

// % error relative to resistance predicted at the *air* temperature.
// Meaningful as a model screen only if the bead is at that same temp.
inline float differencePercent(float observedOhms, float airC,
                               const Hypothesis &m) {
  if (!std::isfinite(observedOhms) || observedOhms <= 0.0f) return nan();
  const float expected = expectedResistanceOhms(airC, m);
  if (!std::isfinite(expected)) return nan();
  return 100.0f * (observedOhms / expected - 1.0f);
}
}  // namespace NtcQualification
