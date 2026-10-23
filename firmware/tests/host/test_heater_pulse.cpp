#include "../../HeaterPulseCommissioning.h"
#include <cassert>
#include <iostream>
using namespace HeaterCommissioning;

static Inputs ok() {
  Inputs v;
  v.cycleActive = true;
  v.fanRunning = true;
  v.ntcValid = true;
  v.ntcResistanceOhms = 122300.0f;
  v.ntcAgeMs = 100;
  v.airValid = true;
  v.chamberC = 20.0f;
  v.airAgeMs = 100;
  return v;
}

int main() {
  {
    Sequence s;
    auto v = ok();
    v.fanRunning = false;
    assert(!s.update(0,v));              // start: no fan running
    v.fanRunning = true;
    assert(!s.update(1000,v));           // fan starts
    assert(!s.update(5999,v));           // 5s fan pre-run
    assert(s.update(6000,v));            // first 1.5s pulse
    assert(s.update(7499,v));
    assert(!s.update(7500,v));           // gap: heater LOW
    assert(s.pulsesDone() == 1);
    assert(!s.update(19499,v));
    assert(s.update(19500,v));           // second pulse
    assert(!s.update(21000,v));
    assert(s.update(33000,v));           // third pulse
    assert(!s.update(34500,v));
    assert(s.pulsesDone() == 3);
    assert(!s.update(79500,v));          // completed after 45s cooldown
    assert(s.finished() && !s.heaterRequested());
    assert(!s.update(90000,v));          // no automatic repeat
  }
  {
    Sequence s; auto v=ok(); v.ntcValid=false;
    assert(!s.update(0,v) && s.fault()==Fault::NtcInvalid);
  }
  {
    Sequence s; auto v=ok(); v.ntcAgeMs=1501;
    assert(!s.update(0,v) && s.fault()==Fault::NtcStale);
  }
  {
    Sequence s; auto v=ok(); v.airValid=false;
    assert(!s.update(0,v) && s.fault()==Fault::AirInvalid);
  }
  {
    Sequence s; auto v=ok(); v.airAgeMs=5001;
    assert(!s.update(0,v) && s.fault()==Fault::AirStale);
  }
  {
    Sequence s; auto v=ok(); v.chamberC=35.0f;
    assert(!s.update(0,v) && s.fault()==Fault::AirLimit);
  }
  {
    Sequence s; auto v=ok(); v.ntcResistanceOhms=42270.0f;
    assert(!s.update(0,v) && s.fault()==Fault::NtcResistanceLimit);
  }
  {
    Sequence s; auto v=ok(); v.fanRunning=false;
    assert(!s.update(0,v));
    assert(!s.update(12000,v) && s.fault()==Fault::FanUnavailable);
  }
  {
    Sequence s; auto v=ok();
    assert(!s.update(0,v));
    assert(s.update(5000,v));
    v.fanRunning=false;
    assert(!s.update(5001,v) && s.fault()==Fault::FanLost);
    assert(s.cooldownNeeded(5001));
  }
  {
    Sequence s; auto v=ok();
    assert(!s.update(0,v));
    assert(!s.update(120000,v) && s.fault()==Fault::OverallTimeout);
  }
  {
    Sequence s; auto v=ok();
    assert(!s.update(0,v));
    assert(s.update(5000,v));
    s.cancel();
    assert(!s.heaterRequested() && s.fault()==Fault::None);
    assert(s.cooldownNeeded(5000));
    assert(!s.update(6000,v) && s.phase()==Phase::Aborted);
    assert(!s.canStart());
  }
  {
    Sequence s; auto v=ok();
    assert(!s.update(UINT32_MAX-5999,v));
    assert(s.update(UINT32_MAX-999,v));   // +5s rollover safe
    assert(!s.update(500,v));             // still in first 1.5s pulse
  }
  std::cout << "PASS: 12 first-heating commissioning scenarios (host only)\n";
}
