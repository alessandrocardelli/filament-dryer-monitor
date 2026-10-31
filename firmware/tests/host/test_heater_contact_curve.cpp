#include "../../HeaterContactCurve.h"
#include "../../HeaterStep09AirFeedback.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace HeaterStep09AirFeedback;
static Inputs ok(float r) {
  Inputs x{}; x.cycleActive=x.fanRunning=x.ntcValid=x.airValid=true;
  x.ntcOhms=r; x.ntcAgeMs=50;
  x.airC=40; x.airAgeMs=300; x.selectedAirTargetC=55;
  return x;
}
int main() {
  const float r85=HeaterContactCurve::resistanceAtContactC(85);
  const float r89=HeaterContactCurve::resistanceAtContactC(89);
  const float r90=HeaterContactCurve::resistanceAtContactC(90);
  assert(std::fabs(r85-9231.9f)<0.2f);
  assert(std::fabs(r89-8136.9f)<0.2f);
  assert(r90>7800.0f && r90<8000.0f && r90<r89);
  assert(std::isnan(HeaterContactCurve::resistanceAtContactC(90.1f)));
  float prior=1e9f;
  for(int step=160;step<=900;++step) {
    const float r=HeaterContactCurve::resistanceAtContactC(step/10.0f);
    assert(std::isfinite(r) && r<=prior);
    prior=r;
  }
  auto old=attendedAir55Parameters();
  assert(old.ntcSoftPauseOhms==7120.0f && old.ntcResumeOhms==8236.6f);
  auto p=candidateSanwa90_85Parameters();
  assert(p.continuousLocalThermostat);
  assert(std::fabs(p.ntcSoftPauseOhms-r90)<0.2f);
  assert(std::fabs(p.ntcResumeOhms-r85)<0.2f);
  assert(p.ntcHardTripOhms==7120 && p.airFaultCeilingC==60);
  Controller c{p};
  assert(!c.update(0,ok(130000)));
  assert(c.update(5000,ok(13000)));
  assert(c.update(20000,ok(r89)));
  assert(!c.update(21000,ok(r90)) && c.fault()==Fault::None);
  assert(!c.update(22000,ok(8500)));
  assert(c.update(23000,ok(r85)));
  assert(c.update(24000,ok(8500)));
  assert(!c.update(25000,ok(7119)) && c.fault()==Fault::NtcResistanceLimit);
  std::cout<<"PASS HOST ONLY: 90C estimated pause/85C measured rearm, active physical profile unchanged.\n";
}
