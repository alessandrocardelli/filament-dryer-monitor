#pragma once
#include <cmath>
#include <cstddef>

// Closed eBOX Lite, original fan correctly oriented, 31 Oct 2026.
// Sanwa probe is within 1 cm of embedded NTC. This curve estimates NEARBY
// Sanwa plate-contact C from measured NTC ohms; NOT bead C or TCO-body C.
// Raw paired data in docs/evidence/thermal/2026-10-31-NTC-SANWA-corrected-fan-pairs.csv
// At repeated Sanwa integer C values, use median ohms; full raw data retained.
// Log-R interpolation is used between knots. 90C requires 1C EXTRAPOLATION
// beyond the measured 89C endpoint: not a qualified physical safety limit.
// This header does not connect to heater GPIO or enable actual hardware.
namespace HeaterContactCurve {
struct Point {float contactC;float ntcOhms;};
inline constexpr Point kHeatingMedian[] = {
  {16.0f, 139928.7f},
  {45.0f, 38856.2f},
  {54.0f, 26568.5f},
  {59.0f, 21669.6f},
  {62.0f, 18757.8f},
  {66.0f, 16837.3f},
  {68.0f, 15395.0f},
  {71.0f, 14280.1f},
  {73.0f, 13412.2f},
  {74.0f, 12667.8f},
  {76.0f, 12093.8f},
  {78.0f, 11154.7f},
  {81.0f, 10527.4f},
  {82.0f, 10030.9f},
  {84.0f, 9624.8f},
  {85.0f, 9231.9f},
  {86.0f, 8942.2f},
  {87.0f, 8600.5f},
  {88.0f, 8355.7f},
  {89.0f, 8136.9f},
};
inline constexpr std::size_t kCount=sizeof(kHeatingMedian)/sizeof(kHeatingMedian[0]);
inline float resistanceAtContactC(float c) {
  if (!std::isfinite(c) || c<16.0f || c>90.0f) return NAN;
  for (std::size_t i=0;i+1<kCount;++i) {
    const auto a=kHeatingMedian[i],b=kHeatingMedian[i+1];
    if(c<=b.contactC) {
      const float f=(c-a.contactC)/(b.contactC-a.contactC);
      return std::exp(std::log(a.ntcOhms)+f*(std::log(b.ntcOhms)-std::log(a.ntcOhms)));
    }
  }
  const auto a=kHeatingMedian[kCount-2],b=kHeatingMedian[kCount-1];
  return std::exp(std::log(b.ntcOhms)+
    (c-b.contactC)*(std::log(b.ntcOhms)-std::log(a.ntcOhms))/(b.contactC-a.contactC));
}
}  // namespace HeaterContactCurve
