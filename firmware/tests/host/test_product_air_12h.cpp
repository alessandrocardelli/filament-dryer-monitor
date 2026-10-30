// PRODUCT CONTROL DESIGN, simulated ONLY. No GPIO, no heater enable.
// Numeric thermistor *Celsius* values below are synthetic host test stimuli,
// NOT the independent SANWA contact 85/80C example or calibrated NTC conversion.
#include "../../HeaterProductAir.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using namespace HeaterProductAir;
static Limits fixture() {
  Limits l{};
  l.qualified=true;       // test fixture ONLY, firmware approval flags remain FALSE
  l.ntcRearmC=80;         // SYNTHETIC 5C hysteresis; not physical calibration
  l.ntcSoftC=85;          // SYNTHETIC thermostat high
  l.ntcHardC=95;          // SYNTHETIC fault injection boundary
  l.airFaultC=60;         // SYNTHETIC test
  l.airHysteresisC=1;
  l.fanPrerunMs=5000;
  l.maxNtcAgeMs=1500;
  l.maxAirAgeMs=5000;
  return l;
}
static Inputs healthy(float air=30,float ntc=60,float target=55,bool active=true) {
  Inputs x{};
  x.cycleActive=active;
  x.fanCommandedRunning=true;
  x.ntcElectricalValid=x.ntcThermallyQualified=true;
  x.heaterTempC=ntc;
  x.ntcAgeMs=100;
  x.airValid=true;
  x.airC=air;
  x.airAgeMs=500;
  x.targetAirC=target;
  return x;
}
int main(){
  { Controller blocked; assert(!blocked.update(0,healthy())); 
    assert(blocked.fault()==Fault::Unqualified); }
  for(float target : {40.0f,55.0f}){
    Controller c{fixture()};
    constexpr uint32_t hour=3600000UL;
    uint64_t accumulated=0;
    bool reached=false,held=false,rearmed=false,late=false;
    for(uint32_t now=0;now<=12*hour+1000;now+=1000){
      const float air = now<2*hour?target-5:
                        now<3*hour?target:
                        now<4*hour?target-0.5f:target-1;
      const bool active=now<12*hour;
      const bool on=c.update(now,healthy(air,60,target,active));
      assert(c.fault()==Fault::None);
      if(now>=5000 && now<2*hour)
        assert(on); // continuous 2h: across 20s, 60s, 120s, NO periodic gaps
      if(now>=2*hour && now<4*hour)
        assert(!on); // reached target and 0.5C drop: still OFF
      if(now>=4*hour && active)
        assert(on); // AIR target-1C -> immediate continuous demand
      if(!active) assert(!on);
      if(on){
        accumulated+=1000;
        if(now>=4*hour) rearmed=true;
        if(now>=11*hour) late=true;
      }else{
        if(now>=2*hour&&now<3*hour) reached=true;
        if(now>=3*hour&&now<4*hour) held=true;
      }
    }
    assert(accumulated>360000UL && reached && held && rearmed && late);
    assert(!c.heaterRequested());
  }
  { // Heater-local upper/lower thermostat only; no arbitrary time gap.
    Controller c{fixture()};
    assert(!c.update(0,healthy()));
    assert(c.update(5000,healthy()));
    assert(c.update(65000,healthy(30,84))); // still ON after 65s
    assert(!c.update(66000,healthy(30,85))); // upper limit reached
    assert(!c.update(67000,healthy(30,82))); // wait while still above lower
    assert(!c.update(68000,healthy(30,80.01f)));
    assert(c.update(69000,healthy(30,80)));  // rearm at lower threshold
    assert(c.update(70000,healthy(30,82)));  // hysteresis stays ON
    assert(!c.update(71000,healthy(30,85))); // pause again
  }
  { // AIR demand independent of local readiness, no target-1C premature ON.
    Controller c{fixture()};
    assert(!c.update(0,healthy()));
    assert(c.update(5000,healthy()));
    assert(!c.update(10000,healthy(55)));
    assert(!c.update(11000,healthy(54.5f)));
    assert(c.update(12000,healthy(54)));
  }
  { Controller c{fixture()};c.update(0,healthy());c.update(5000,healthy());
    assert(!c.update(6000,healthy(30,95))&&c.fault()==Fault::HeaterTooHot);
    assert(!c.update(7000,healthy())); }
  { Controller c{fixture()};c.update(0,healthy());c.update(5000,healthy());
    auto x=healthy();x.fanCommandedRunning=false;
    assert(!c.update(6000,x)&&c.fault()==Fault::FanLost); }
  { Controller c{fixture()};c.update(0,healthy());
    auto x=healthy();x.ntcAgeMs=1501;
    assert(!c.update(5000,x)&&c.fault()==Fault::NtcStale); }
  { Controller c{fixture()};c.update(0,healthy());
    auto x=healthy();x.airAgeMs=5001;
    assert(!c.update(5000,x)&&c.fault()==Fault::AirStale); }
  { Controller c{fixture()};c.update(0,healthy());
    assert(!c.update(5000,healthy(60))&&c.fault()==Fault::AirTooHot); }
  { Controller c{fixture()};c.update(0,healthy());
    assert(c.update(5000,healthy()));
    c.stop();
    assert(!c.heaterRequested());
    assert(!c.update(6000,healthy(30,60,55,false))); }
  { Controller c{fixture()};assert(!c.update(0,healthy(30,60,56)));
    assert(c.fault()==Fault::Unqualified); }
  std::cout<<"PASS HOST-ONLY: continuous 2h/12h AIR+NTC thermostat (no20s OFF), "
              "NTC85/80 SIMULATION ONLY, AIR55/54 rearm, "
              "hard/sensor/fan/air faults and STOP. No GPIO.\n";
}
