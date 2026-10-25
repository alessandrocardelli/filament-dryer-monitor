// STEP06: no GPIO, pure C++ checks for UNAPPROVED fan-on thermal run.
#include "../../HeaterStep06Candidate.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace HeaterCommissioning;

static Inputs valid() {
  Inputs x{};
  x.cycleActive=true;
  x.fanRunning=true;
  x.ntcValid=true;
  x.ntcResistanceOhms=90000.0f;
  x.ntcAgeMs=100;
  x.airValid=true;
  x.chamberC=23.0f;
  x.airAgeMs=100;
  return x;
}
int main() {
  auto p=HeaterStep06Candidate::parameters();
  static_assert(HeaterStep06Candidate::kPulseHardwareCeilingUs > 10000000,
                "Hardware deadline must exceed normal requested 10s interval");
  static_assert(HeaterStep06Candidate::kPulseHardwareCeilingUs <= 11000000,
                "Hardware deadline must not permit extended heat pulses");
  assert(p.pulseCount==4 && p.pulseMs==10000 && p.gapMs==12000);
  assert(p.minimumNtcOhms==19680 && p.maximumChamberC==40);
  assert(p.overallTimeoutMs==155000 && p.cooldownMs==60000);
  assert(p.pulseCount*p.pulseMs==40000); // max nominal heater ON, 40 seconds
  {
    Sequence s(p); auto x=valid();
    assert(!s.update(0,x));
    assert(!s.update(4999,x));
    assert(s.update(5000,x)); // heat 1 starts
    assert(s.update(14999,x));
    assert(!s.update(15000,x));
    assert(s.update(27000,x)); // heat 2 starts
    assert(!s.update(37000,x));
    assert(s.update(49000,x)); // heat 3 starts
    assert(!s.update(59000,x));
    assert(s.update(71000,x)); // heat 4 starts
    assert(!s.update(81000,x)); // heat off and cooldown
    assert(s.pulsesDone()==4 && !s.heaterRequested());
    assert(s.cooldownNeeded(81000) && !s.finished());
    assert(!s.update(140999,x));
    assert(s.update(141000,x)==false);
    assert(s.finished() && !s.cooldownNeeded(141000));
    assert(!s.canStart() && !s.update(145000,x)); // no-repeat
  }
  {
    Sequence s(p); auto x=valid();x.fanRunning=false;
    assert(!s.update(0,x)); assert(!s.update(12000,x));
    assert(s.fault()==Fault::FanUnavailable);
  }
  {
    Sequence s(p);auto x=valid();assert(!s.update(0,x));
    assert(s.update(5000,x));
    x.fanRunning=false;
    assert(!s.update(5001,x) && s.fault()==Fault::FanLost);
    assert(s.cooldownNeeded(5001));
  }
  {
    Sequence s(p);auto x=valid();x.ntcResistanceOhms=19680;
    assert(!s.update(0,x) && s.fault()==Fault::NtcResistanceLimit);
  }
  {
    Sequence s(p);auto x=valid();x.chamberC=40;
    assert(!s.update(0,x) && s.fault()==Fault::AirLimit);
  }
  {
    Sequence s(p);auto x=valid();x.ntcAgeMs=1501;
    assert(!s.update(0,x) && s.fault()==Fault::NtcStale);
  }
  {
    Sequence s(p);auto x=valid();x.airAgeMs=5001;
    assert(!s.update(0,x) && s.fault()==Fault::AirStale);
  }
  {
    Sequence s(p);auto x=valid();
    assert(!s.update(0,x));assert(s.update(5000,x));s.cancel();
    assert(!s.heaterRequested() && s.cooldownNeeded(5001));
    assert(!s.canStart());
  }
  {
    Sequence s(p);auto x=valid();assert(!s.update(0,x));
    assert(!s.update(155000,x));
    assert(s.fault()==Fault::OverallTimeout);
  }
  std::cout << "PASS: Stage06 distinct fan-on real-heat CANDIDATE only; "
               "4x10s, fan prerun, cutoff, sensor faults, timer and no rearm\n";
}
