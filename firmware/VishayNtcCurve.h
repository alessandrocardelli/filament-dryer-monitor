#pragma once
#include <cmath>
#include <cstddef>
// Vishay NTCS0603E3104FXT, document 29056 (25-Feb-2013), p.5.
// 39 NOMINAL manufacturer R(T) anchors, every 5C from -40 to 150C.
// Not the unidentified original OEM sensor. Not a qualified safety limit.
namespace VishayNtcCurve {
inline constexpr float kOhms[] = {
  3921252f, // -40 C
  2774565f, // -35 C
  1988706f, // -30 C
  1442861f, // -25 C
  1058901f, // -20 C
  785573f, // -15 C
  588793f, // -10 C
  445602f, // -5 C
  340346f, // 0 C
  262229f, // 5 C
  203723f, // 10 C
  159522f, // 15 C
  125851f, // 20 C
  100000f, // 25 C
  80003f, // 30 C
  64422f, // 35 C
  52200f, // 40 C
  42548f, // 45 C
  34879f, // 50 C
  28749f, // 55 C
  23820f, // 60 C
  19835f, // 65 C
  16597f, // 70 C
  13951f, // 75 C
  11780f, // 80 C
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
