// V1 product controller offline simulation only. Numeric limits below are
// SYNTHETIC FIXTURE INPUTS, not heater-material qualification or permissions.
#include "../../HeaterProductAir.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace HeaterProductAir;

static Limits fixture() {
  Limits l{};
  l.qualified = true; // SIMULATION ONLY
  l.ntcRearmC = 70;
  l.ntcSoftC = 90;
  l.ntcHardC = 110;
  l.airFaultC = 65;
  l.airHysteresisC = 1;
  l.fanPrerunMs = 5000;
  l.maxOnWindowMs = 20000;
  l.minOffMs = 12000;
  l.maxNtcAgeMs = 1500;
  l.maxAirAgeMs = 5000;
  return l;
}
static Inputs healthy(float air, float target = 55, bool active = true) {
  Inputs x{};
  x.cycleActive = active;
  x.fanCommandedRunning = true;
  x.ntcElectricalValid = x.ntcThermallyQualified = true;
  x.heaterTempC = 60;
  x.ntcAgeMs = 200;
  x.airValid = true;
  x.airC = air;
  x.airAgeMs = 1000;
  x.targetAirC = target;
  return x;
}
int main() {
  {
    Controller blocked; // must fail closed without qualified local limits
    assert(!blocked.update(0, healthy(35)));
    assert(blocked.fault() == Fault::Unqualified);
    assert(!blocked.update(5000, healthy(35)));
  }
  for (float target : {40.0f, 55.0f}) {
    Controller c{fixture()};
    constexpr uint32_t h = 3600000UL;
    uint64_t onMs = 0;
    uint32_t startedOn = 0;
    uint32_t lastOff = 0;
    bool prevOn = false, reachedOff = false, heldHysteresis = false;
    bool returnedOn = false, lateOn = false;
    for (uint32_t now = 0; now <= 12UL*h+1000UL; now += 1000UL) {
      // A plateau at exact target followed by 0.5C below it MUST stay OFF;
      // 1C below target MUST rearm even after many hours.
      const float air = now < 2*h ? target - 5 :
                        now < 3*h ? target :
                        now < 4*h ? target - 0.5f : target - 1.0f;
      const bool active = now < 12*h;
      const bool on = c.update(now, healthy(air, target, active));
      assert(c.fault() == Fault::None);
      if (on) {
        if (!prevOn) {
          assert(now >= 5000);
          if (lastOff > 0) assert(now - lastOff >= 12000);
          startedOn = now;
        }
        assert(now - startedOn < 20000);
        if (now >= 4*h) returnedOn = true;
        if (now >= 11*h) lateOn = true;
        onMs += 1000;
      } else {
        if (prevOn) lastOff = now;
        if (now >= 2*h && now < 3*h) reachedOff = true;
        if (now >= 3*h && now < 4*h) heldHysteresis = true;
      }
      if (now >= 2*h && now < 4*h) assert(!on);
      if (!active) assert(!on);
      prevOn = on;
    }
    assert(onMs > 360000UL && reachedOff && heldHysteresis);
    assert(returnedOn && lateOn); // real user-cycle timer, no global heat cap
  }
  {
    Controller c{fixture()};
    assert(!c.update(0, healthy(30)));
    assert(c.update(5000, healthy(30)));
    auto hot=healthy(30); hot.heaterTempC=110;
    assert(!c.update(6000, hot) && c.fault() == Fault::HeaterTooHot);
    assert(!c.update(20000, healthy(30))); // latched
  }
  {
    Controller c{fixture()};
    c.update(0, healthy(30));
    assert(c.update(5000, healthy(30)));
    auto hot=healthy(30); hot.heaterTempC=90;
    assert(!c.update(6000, hot) && c.fault()==Fault::None); // local pause
    hot.heaterTempC=80;
    assert(!c.update(20000, hot)); // above70 rearm
    hot.heaterTempC=70;
    assert(c.update(21000, hot)); // recovery after minimum OFF
  }
  {
    Controller c{fixture()}; c.update(0,healthy(30)); c.update(5000,healthy(30));
    auto x=healthy(30); x.fanCommandedRunning=false;
    assert(!c.update(6000,x) && c.fault()==Fault::FanLost);
  }
  {
    Controller c{fixture()}; c.update(0,healthy(30));
    auto x=healthy(30); x.ntcAgeMs=1600;
    assert(!c.update(5000,x) && c.fault()==Fault::NtcStale);
  }
  {
    Controller c{fixture()}; c.update(0,healthy(30));
    auto x=healthy(30); x.airAgeMs=5001;
    assert(!c.update(5000,x) && c.fault()==Fault::AirStale);
  }
  {
    Controller c{fixture()}; c.update(0,healthy(30));
    assert(!c.update(5000,healthy(65)) && c.fault()==Fault::AirTooHot);
  }
  {
    Controller c{fixture()}; c.update(0,healthy(30));
    assert(c.update(5000,healthy(30)));
    c.stop(); assert(!c.heaterRequested());
    assert(!c.update(6000,healthy(30,false)));
  }
  {
    Controller c{fixture()}; auto x=healthy(30,56);
    assert(!c.update(0,x) && c.fault()==Fault::Unqualified);
  }
  std::cout << "PASS: V1 40/55C target, 12h timed regulation, >360s ON, "
               "20s ON/12s OFF, 1C hysteresis, independent local soft/rearm/"
               "hard, latched sensor/fan/air faults and STOP; NO GPIO.\n";
}
