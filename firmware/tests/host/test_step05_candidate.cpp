// Candidate ONLY: tests the proposed 4-pulse envelope without GPIO.
#include "../../HeaterStage05Candidate.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace HeaterCommissioning;

static Inputs good() {
  Inputs x{};
  x.cycleActive = true;
  x.fanRunning = true;
  x.ntcValid = true;
  x.ntcResistanceOhms = 122300.0f;
  x.ntcAgeMs = 100;
  x.airValid = true;
  x.chamberC = 20.0f;
  x.airAgeMs = 100;
  return x;
}

int main() {
  const auto p = HeaterStage05Candidate::parameters();
  assert(p.pulseCount == 4);
  assert(p.pulseMs == 1500 && p.gapMs >= 12000);
  assert(p.prerunMs >= 5000 && p.cooldownMs >= 60000);
  assert(p.overallTimeoutMs >= 107000);
  assert(p.minimumNtcOhms == 42270.0f && p.maximumChamberC == 35.0f);
  {
    Sequence s{p}; auto x=good();
    assert(!s.update(0, x));
    assert(!s.update(4999, x));
    assert(s.update(5000, x)); // candidate pulse 1
    assert(!s.update(6500, x));
    assert(s.update(18500, x)); // 2
    assert(!s.update(20000, x));
    assert(s.update(32000, x)); // 3
    assert(!s.update(33500, x));
    assert(s.update(45500, x)); // 4
    assert(!s.update(47000, x));
    assert(s.pulsesDone() == 4);
    assert(!s.update(106999, x));
    assert(!s.update(107000, x));
    assert(s.finished() && !s.heaterRequested());
    assert(!s.canStart() && !s.update(110000, x));
  }
  {
    Sequence s{p}; auto x=good();
    x.ntcResistanceOhms=42270.0f;
    assert(!s.update(0, x) && s.fault()==Fault::NtcResistanceLimit);
  }
  {
    Sequence s{p}; auto x=good();
    x.chamberC=35.0f;
    assert(!s.update(0, x) && s.fault()==Fault::AirLimit);
  }
  {
    Sequence s{p}; auto x=good();
    x.ntcAgeMs=1501;
    assert(!s.update(0, x) && s.fault()==Fault::NtcStale);
  }
  {
    Sequence s{p}; auto x=good();
    assert(!s.update(0, x));
    assert(s.update(5000, x));
    x.fanRunning=false;
    assert(!s.update(5001, x) && s.fault()==Fault::FanLost);
    assert(s.cooldownNeeded(5001));
  }
  {
    Sequence s{p}; auto x=good();
    assert(!s.update(0, x));
    assert(s.update(5000, x));
    s.cancel(); // manual stop cannot rearm in same boot
    assert(!s.heaterRequested() && !s.canStart());
    assert(s.cooldownNeeded(5001));
  }
  std::cout << "PASS: STEP05 4-pulse CANDIDATE offline envelope, "
               "strict aborts, one-shot and cooldown (NO HEATER)\n";
}
