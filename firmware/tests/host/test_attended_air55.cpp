// HOST ONLY. New closed-box/fan-corrected Sanwa contact 85C/9231.9ohm
// measured, 90C/~7923.8ohm estimated one degree above measured89C.
// Old 17 Oct open/fan-OFF 85/80 profile is historical.
// GPIO19 safety ISR verified separately by source checks + real ESP32 build.
#include "../../HeaterStep09AirFeedback.h"
#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>
using namespace HeaterStep09AirFeedback;
static Inputs good(float air=20.0f, float r=14000.0f,
                   float target=55.0f, bool active=true) {
  Inputs x{};
  x.cycleActive=active;
  x.fanRunning=x.ntcValid=x.airValid=true;
  x.airC=air; x.ntcOhms=r;
  x.ntcAgeMs=50; x.airAgeMs=300;
  x.selectedAirTargetC=target;
  return x;
}
int main() {
  const Parameters p=attendedSanwa90_85Parameters();
  assert(p.runPolicy==RunPolicy::DurationControlledDesign);
  assert(p.continuousLocalThermostat);
  const float r90=HeaterContactCurve::resistanceAtContactC(90.0f);
  const float r89=HeaterContactCurve::resistanceAtContactC(89.0f);
  const float r85=HeaterContactCurve::resistanceAtContactC(85.0f);
  assert(p.ntcSoftPauseOhms==r90 && r90>7900.0f && r90<7950.0f);
  assert(std::fabs(p.ntcResumeOhms-r85)<0.01f && std::fabs(r85-9231.9f)<0.2f);
  assert(r89>r90 && r85>r89);
  assert(p.ntcHardTripOhms==7120.0f && p.minimumOffMs==0);
  assert(p.airFaultCeilingC==60.0f);
  assert(kHardwareWindowCeilingUs==20500000U);
  { // 60-min user cycle, AIR55 then AIR54, continuous HEATER when requested.
    Controller c{p};
    bool sawTargetOff=false,sawHalfDegreeOff=false,sawAirRearm=false;
    uint64_t onMs=0;
    for(uint32_t now=0;now<=3600000UL+100000UL;now+=1000UL) {
      const bool running=now<3600000UL;
      const float air= now<2400000UL ? 35.0f :
                       now<2700000UL ? 55.0f :
                       now<3000000UL ? 54.5f :54.0f;
      const bool on=c.update(now,good(air,14000,55,running));
      assert(c.fault()==Fault::None);
      if(now>=5000 && now<2400000UL) assert(on);
      if(now>=2400000UL && now<3000000UL) assert(!on);
      if(now>=3000000UL && running) assert(on);
      if(!running) assert(!on);
      if(on) {onMs+=1000; if(now>=3000000UL) sawAirRearm=true;}
      if(now>=2400000UL && now<2700000UL && !on) sawTargetOff=true;
      if(now>=2700000UL && now<3000000UL && !on) sawHalfDegreeOff=true;
    }
    assert(onMs>360000 && sawTargetOff && sawHalfDegreeOff && sawAirRearm);
    assert(c.finished() && c.fault()==Fault::None);
  }
  { // At measured89C HEAT stays ON; estimated90C OFF; measured85C ON.
    Controller c{p};
    assert(!c.update(0,good()));
    assert(c.update(5000,good()));
    assert(c.update(30000,good(35,r89))); // No forced20s OFF
    assert(c.update(150000,good(35,r89))); // No forced 2min cap
    assert(!c.update(151000,good(35,r90))); // Normal thermostat OFF
    assert(c.fault()==Fault::None);
    assert(c.waitReason(151000,r90)==Controller::WaitReason::NtcRecovery);
    assert(!c.update(152000,good(35,8500))); // Still above 85C
    assert(!c.update(153000,good(35,9000)));
    assert(c.update(154000,good(35,r85))); // 85C restart
    assert(c.update(155000,good(35,8500))); // Stay ON in hysteresis band
    assert(!c.update(156000,good(35,r90))); // Normal OFF once more
    assert(c.fault()==Fault::None);
  }
  { // Independent existing backup still faults at R<7120ohm.
    Controller c{p};c.update(0,good());c.update(5000,good());
    assert(!c.update(6000,good(30,7119)));
    assert(c.fault()==Fault::NtcResistanceLimit);
    assert(!c.update(7000,good()));
  }
  { // HOT start cannot bypass lower NTC rearm.
    Controller c{p};
    assert(!c.update(0,good(30,8000)));
    assert(!c.update(5000,good(30,8000)));
    assert(c.waitReason(5000,8000)==Controller::WaitReason::NtcRecovery);
    assert(c.update(6000,good(30,r85)));
  }
  { // AIR independently reaches requested target before local limit.
    Controller c{p};c.update(0,good());assert(c.update(5000,good()));
    assert(!c.update(6000,good(55,14000)));
    assert(c.waitReason(6000,14000)==Controller::WaitReason::AirHysteresis);
    assert(!c.update(7000,good(54.5f,14000)));
    assert(c.update(8000,good(54,14000)));
    assert(!c.update(9000,good(55,r90))); // simultaneously hot and target
    assert(!c.update(10000,good(54,8000))); // rearm AIR but NOT local NTC
    assert(c.update(11000,good(54,r85)));
  }
  { Controller c{p};c.update(0,good());
    assert(!c.update(5000,good(40,14000,56)));
    assert(c.fault()==Fault::AirInvalid); }
  { Controller c{p};c.update(0,good());
    assert(!c.update(5000,good(60)));assert(c.fault()==Fault::AirLimit); }
  { Controller c{p};c.update(0,good());auto x=good();
    x.ntcAgeMs=1501;assert(!c.update(5000,x));
    assert(c.fault()==Fault::NtcStale); }
  { Controller c{p};c.update(0,good());auto x=good();
    x.airAgeMs=5001;assert(!c.update(5000,x));
    assert(c.fault()==Fault::AirStale); }
  { Controller c{p};c.update(0,good());c.update(5000,good());
    auto x=good();x.fanRunning=false;
    assert(!c.update(6000,x));assert(c.fault()==Fault::FanLost); }
  { Controller c{p};c.update(0,good());c.update(5000,good());
    c.emergencyStop(Fault::PulseWatchdog);
    assert(!c.heaterRequested() && c.fault()==Fault::PulseWatchdog); }
  { Controller c{p};c.update(0,good());c.update(5000,good());
    c.cancel();assert(!c.heaterRequested());
    assert(c.fault()==Fault::ManualStop);
    assert(!c.resetForNewSession(6000));
    assert(c.resetForNewSession(100000)); }
  { // Latched NTC thermostat persists for an AIR target cycle.
    Controller c{p};c.update(0,good());c.update(5000,good());
    c.update(6000,good(40,r90));
    assert(!c.update(7000,good(55,8000)));
    assert(!c.update(8000,good(54,8000)));
    assert(c.update(9000,good(54,r85)));
  }
  std::cout<<"PASS ATTENDED NTC90/85: 60min AIR55/54 cycle, >2min continuous "
           "HEATER ON with no forced gaps, local upper~7924/rearm9231.9, "
           "sub7120 latched fault, startup hot, fan/sensor/watchdog/STOP.\n";
}
