// Physical-experiment source logic (HOST ONLY): independent AIR and NTC
// thermostats. Original 17-Oct OPEN/FAN-OFF Sanwa pairs:
// Sanwa85C/NTC7120 ohm measured; Sanwa80C/NTC8236.6 ohm interpolated.
// These numbers are research references, NOT calibrated NTC bead C.
// GPIO19 safety ISR verified separately by source checks + real ESP32 build.
#include "../../HeaterStep09AirFeedback.h"
#include <cassert>
#include <cstdint>
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
  const Parameters p=attendedAir55Parameters();
  assert(p.runPolicy==RunPolicy::DurationControlledDesign);
  assert(p.continuousLocalThermostat);
  assert(p.ntcSoftPauseOhms==7120.0f);
  assert(p.ntcResumeOhms>8236.0f && p.ntcResumeOhms<8237.0f);
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
  { // Local 85C/contact max at exactly7120 -> NORMAL OFF, 80C->ON.
    Controller c{p};
    assert(!c.update(0,good()));
    assert(c.update(5000,good()));
    assert(c.update(30000,good(35,7121))); // NO forced20s/2s cycle
    assert(c.update(150000,good(35,7121))); // NO forced 2min cutoff
    assert(!c.update(151000,good(35,7120))); // local upper latch
    assert(c.fault()==Fault::None);
    assert(c.waitReason(151000,8000)==Controller::WaitReason::NtcRecovery);
    assert(!c.update(152000,good(35,7500)));
    assert(!c.update(153000,good(35,8200)));
    assert(c.update(154000,good(35,8237))); // interpolated80C rearm
    assert(c.update(155000,good(35,8000))); // stays ON across hysteresis
    assert(!c.update(156000,good(35,7120))); // stop again
    assert(c.fault()==Fault::None);
  }
  { // R below highest measured hot endpoint: FAULT even in OFF gap.
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
    assert(c.update(6000,good(30,8237)));
  }
  { // AIR independently reaches requested target before local limit.
    Controller c{p};c.update(0,good());assert(c.update(5000,good()));
    assert(!c.update(6000,good(55,14000)));
    assert(c.waitReason(6000,14000)==Controller::WaitReason::AirHysteresis);
    assert(!c.update(7000,good(54.5f,14000)));
    assert(c.update(8000,good(54,14000)));
    assert(!c.update(9000,good(55,7120))); // simultaneously hot and target
    assert(!c.update(10000,good(54,8000))); // rearm AIR but NOT local NTC
    assert(c.update(11000,good(54,8237)));
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
    c.update(6000,good(40,7120));
    assert(!c.update(7000,good(55,8000)));
    assert(!c.update(8000,good(54,8000)));
    assert(c.update(9000,good(54,8237)));
  }
  std::cout<<"PASS ATTENDED NTC85/80: 60min AIR55/54 cycle, >2min continuous "
           "HEATER ON with no forced gaps, local upper7120/rearm8236.6, "
           "sub7120 latched fault, startup hot, fan/sensor/watchdog/STOP.\n";
}
