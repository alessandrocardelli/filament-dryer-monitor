// NEW attended AIR40..55 research controller, HOST ONLY, GPIO not connected.
// Real profile numbers are already OBSERVED NTC points paired with nearby
// Sanwa open/fanOFF contact: R13.73k/64C, R9.81k/74C, R7.12k/85C.
// NOT approved heater-bead Celsius, NOT a closed-box hot-spot prediction.
#include "../../HeaterStep09AirFeedback.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace HeaterStep09AirFeedback;
static Inputs good(float air=24.0f, float r=40000.0f, float target=55.0f,
                   bool active=true) {
  Inputs x{};
  x.cycleActive=active; x.fanRunning=true;
  x.ntcValid=x.airValid=true;
  x.ntcOhms=r; x.ntcAgeMs=100;
  x.airC=air; x.airAgeMs=500;
  x.selectedAirTargetC=target;
  return x;
}
int main() {
  const auto p=attendedAir55Parameters();
  assert(p.runPolicy==RunPolicy::DurationControlledDesign);
  assert(p.ntcResumeOhms==13730.0f && p.ntcSoftPauseOhms==9810.0f);
  assert(p.ntcHardTripOhms==7120.0f && p.airFaultCeilingC==60.0f);
  assert(p.minimumOffMs==2000U && p.maximumOnWindowMs==20000U);
  assert(kHardwareWindowCeilingUs==20500000U);
  {
    Controller c{p};
    uint64_t cumulative=0;
    bool sawReach=false,sawHysteresis=false,sawRearm=false,sawAfterHour=false;
    bool last=false;
    uint32_t began=0,off=0;
    for(uint32_t now=0;now<=2UL*3600000UL+100000UL;now+=1000UL) {
      const bool running=now<2UL*3600000UL;
      const float air=now<1800000UL ? 35.0f :
                      now<2700000UL ? 55.0f :
                      now<3600000UL ? 54.5f : 54.0f;
      bool on=c.update(now,good(air,40000.0f,55.0f,running));
      assert(c.fault()==Fault::None);
      if(on) {
        if(!last) {
          if(off>0) assert(now-off>=2000U);
          began=now;
        }
        assert(now-began<20000U);
        cumulative+=1000;
        if(now>3600000UL) sawRearm=true;
        if(now>5400000UL) sawAfterHour=true;
      } else {
        if(last) off=now;
        if(now>=1800000UL&&now<2700000UL)sawReach=true;
        if(now>=2700000UL&&now<3600000UL)sawHysteresis=true;
      }
      if(now>=1800000UL&&now<3600000UL) assert(!on);
      if(!running) assert(!on);
      last=on;
    }
    assert(cumulative>360000UL && sawReach && sawHysteresis);
    assert(sawRearm && sawAfterHour && c.finished());
    assert(c.fault()==Fault::None);
  }
  {
    Controller c{p};
    c.update(0,good());
    assert(c.update(5000,good()));
    assert(!c.update(6000,good(30,9809.0f)) && c.fault()==Fault::None);
    assert(!c.update(9000,good(30,11000.0f)));
    assert(c.update(10000,good(30,13730.0f)));
    assert(!c.update(11000,good(30,7119.0f)));
    assert(c.fault()==Fault::NtcResistanceLimit);
    assert(!c.update(20000,good()));
  }
  {
    Controller c{p};
    c.update(0,good());
    assert(!c.update(5000,good(30,40000,56)));
    assert(c.fault()==Fault::AirInvalid);
  }
  {
    Controller c{p};c.update(0,good());
    assert(!c.update(5000,good(60)));
    assert(c.fault()==Fault::AirLimit);
  }
  {
    Controller c{p};c.update(0,good());
    auto x=good();x.ntcAgeMs=1501;
    assert(!c.update(5000,x) && c.fault()==Fault::NtcStale);
  }
  {
    Controller c{p};c.update(0,good());
    auto x=good();x.airAgeMs=5001;
    assert(!c.update(5000,x) && c.fault()==Fault::AirStale);
  }
  {
    Controller c{p};c.update(0,good());c.update(5000,good());
    auto x=good();x.fanRunning=false;
    assert(!c.update(6000,x) && c.fault()==Fault::FanLost);
  }
  {
    Controller c{p};c.update(0,good());c.update(5000,good());
    c.cancel();assert(!c.heaterRequested());
    assert(c.fault()==Fault::ManualStop);
    assert(!c.resetForNewSession(6000)); // cooldown not complete
    assert(c.resetForNewSession(100000));
    assert(c.canStart());
  }
  std::cout<<"PASS ATTENDED AIR55: selected AIR target/hysteresis, 2h user timer, "
           ">360s total ON, 20s/2s windows, 9.81/13.73/7.12k NTC, "
           "fault latches, external STOP and cooldown. GPIO not connected.\n";
}
