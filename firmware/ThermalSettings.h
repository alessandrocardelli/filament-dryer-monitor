#pragma once
#include <cstdint>
// Editable thermal thresholds; 0=UNCONFIGURED. Numeric edit ranges do not
// establish safety of the enclosure, air duct or installed TCO105.
namespace ThermalSettings {
struct Values {int16_t ntcPauseC=0,ntcRearmC=0,ntcFaultC=0,airFaultC=0;};
constexpr int16_t kNtcMinC=20,kNtcMaxC=150,kAirMinC=30,kAirMaxC=100;
constexpr bool allowed(int16_t x,int16_t a,int16_t b){
  return x==0||(x>=a&&x<=b);
}
inline void sanitize(Values &x){
  if(!allowed(x.ntcPauseC,kNtcMinC,kNtcMaxC))x.ntcPauseC=0;
  if(!allowed(x.ntcRearmC,kNtcMinC,kNtcMaxC))x.ntcRearmC=0;
  if(!allowed(x.ntcFaultC,kNtcMinC,kNtcMaxC))x.ntcFaultC=0;
  if(!allowed(x.airFaultC,kAirMinC,kAirMaxC))x.airFaultC=0;
}
constexpr bool validForTarget(Values x,int16_t target){
  return x.ntcRearmC>=kNtcMinC&&x.ntcRearmC<x.ntcPauseC&&
         x.ntcPauseC<x.ntcFaultC&&x.ntcFaultC<=kNtcMaxC&&
         x.airFaultC>=kAirMinC&&x.airFaultC<=kAirMaxC&&x.airFaultC>target;
}
constexpr int16_t adjust(int16_t x,int dir,int16_t a,int16_t b){
  return x==0?(dir>0?a:0):(dir>0?(x<b?x+1:b):(x<=a?0:x-1));
}
constexpr bool equal(Values a,Values b){
  return a.ntcPauseC==b.ntcPauseC&&a.ntcRearmC==b.ntcRearmC&&
         a.ntcFaultC==b.ntcFaultC&&a.airFaultC==b.airFaultC;
}
}
