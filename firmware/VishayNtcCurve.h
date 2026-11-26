#pragma once
#include <cmath>
#include <cstddef>
// Vishay NTCS0603E3104FXT, document 29056 (25-Feb-2013), p.5.
// 39 NOMINAL manufacturer R(T) anchors, every 5C from -40 to 150C.
// Not the unidentified original OEM sensor. Not a qualified safety limit.
namespace VishayNtcCurve {
inline constexpr float kOhms[] = {
  3921252.0f, // -40 C
  2774565.0f, // -35 C
  1988706.0f, // -30 C
  1442861.0f, // -25 C
  1058901.0f, // -20 C
  785573.0f, // -15 C
  588793.0f, // -10 C
  445602.0f, // -5 C
  340346.0f, // 0 C
  262229.0f, // 5 C
  203723.0f, // 10 C
  159522.0f, // 15 C
  125851.0f, // 20 C
  100000.0f, // 25 C
  80003.0f, // 30 C
  64422.0f, // 35 C
  52200.0f, // 40 C
  42548.0f, // 45 C
  34879.0f, // 50 C
  28749.0f, // 55 C
  23820.0f, // 60 C
  19835.0f, // 65 C
  16597.0f, // 70 C
  13951.0f, // 75 C
  11780.0f, // 80 C
  9988.4f, // 85 C
  8504.3f, // 90 C
  7269.4f, // 95 C
  6237.5f, // 100 C
  5371.7f, // 105 C
  4642.5f, // 110 C
  4025.9f, // 115 C
  3502.7f, // 120 C
  3057.1f, // 125 C
  2676.4f, // 130 C
  2350.1f, // 135 C
  2069.5f, // 140 C
  1827.4f, // 145 C
  1617.9f, // 150 C
};
static_assert(sizeof(kOhms)/sizeof(kOhms[0])==39);
inline float temperatureC(float r){
  if(!std::isfinite(r)||r>kOhms[0]||r<kOhms[38])return NAN;
  for(std::size_t i=0;i<38;++i){
    if(r<=kOhms[i]&&r>=kOhms[i+1]){
      const float fraction=(std::log(r)-std::log(kOhms[i]))/
          (std::log(kOhms[i+1])-std::log(kOhms[i]));
      return -40.0f+5.0f*(static_cast<float>(i)+fraction);
    }
  }
  return r==kOhms[38]?150.0f:NAN;
}
}
