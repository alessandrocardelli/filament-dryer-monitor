// GPIO19 host mock for the actual UI5 experiment. No ESP32 / heater power.
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
int highWrites=0,lowWrites=0,lastPinState=-1;
#include "../../SafetyController.h"
static NtcSnapshot ntc(uint32_t t,float C=60){
  NtcSnapshot n{};n.sampled=n.valid=true;n.fault=NtcFault::None;
  n.lastSampleMs=t;n.temperatureC=C;n.resistanceOhms=10000;return n;
}
static SensorSnapshot air(uint32_t t,float C=25){
  SensorSnapshot x{};x.valid=true;x.temperatureC=C;x.lastSuccessMs=t;return x;
}
static void tick(SafetyController &s,uint32_t t,float n=60,float a=25,
                 bool fan=true,bool active=true,float target=55){
  s.update(t,ntc(t,n),air(t,a),active,fan,t,target);
}
static ThermalSettings::Values good(){
  ThermalSettings::Values x{};x.ntcRearmC=80;x.ntcPauseC=85;
  x.ntcFaultC=95;x.airFaultC=60;return x; // HOST fixtures ONLY, not approved ratings
}
static void start(SafetyController &s){
  s.begin();
  assert(!s.heaterOn() && lastPinState==LOW);
  assert(s.heaterLocked());
  assert(!s.commissioningCanStart(0));
  assert(s.configureThermalSettings(good(),55));
  assert(!s.heaterLocked() && s.commissioningCanStart(0));
  tick(s,0);
  tick(s,4999);
  assert(!s.heaterOn());
  tick(s,5000);
  assert(s.heaterOn() && !s.faultActive() && lastPinState==HIGH);
}
int main(){
  static_assert(AppConfig::kHeaterStep09TrialMode,
                "Actual experimental UI5 must be HEATER-CAPABLE");
  static_assert(!AppConfig::kHeaterOutputEnabled &&
                !AppConfig::kHeaterProfileApproved &&
                !AppConfig::kHeaterNtcModelApproved,
                "No implicit production approval");
  static_assert(HeaterStep09AirFeedback::kHardwareWindowCeilingUs==20500000,
                "Preserve hardware fail-OFF deadline");
  {
    SafetyController s;start(s);
    tick(s,6000,85);assert(!s.heaterOn()&&!s.faultActive());
    assert(s.feedbackWaitReason(6000,0)==2);
    tick(s,7000,82);assert(!s.heaterOn());
    tick(s,8000,80);assert(s.heaterOn());
    tick(s,150000,80);assert(s.heaterOn()); // no automatic 20s OFF
    tick(s,151000,80,55);assert(!s.heaterOn());
    tick(s,152000,80,54.5f);assert(!s.heaterOn());
    tick(s,153000,80,54);assert(s.heaterOn());
    assert(s.feedbackTotalHeatMs()>100000);
    s.completeUserCycle(154000);assert(!s.heaterOn());
    assert(s.coolingRequired(154001)&&s.coolingSecondsRemaining(154001)>0);
    assert(!s.commissioningCanStart(154001));
  }
  {
    SafetyController s;start(s);
    tick(s,6000,95);assert(!s.heaterOn()&&s.faultActive());
    assert(s.commissioningFault()==
      static_cast<uint8_t>(HeaterProductAir::Fault::HeaterTooHot));
    assert(!s.commissioningCanStart(100000));
  }
  {
    SafetyController s;start(s);
    tick(s,6000,60,60);assert(!s.heaterOn()&&s.faultActive());
    assert(s.commissioningFault()==
      static_cast<uint8_t>(HeaterProductAir::Fault::AirTooHot));
  }
  {
    SafetyController s;start(s);
    tick(s,6000,60,25,false);assert(!s.heaterOn()&&s.faultActive());
  }
  {
    SafetyController s;start(s);
    auto n=ntc(6000);n.valid=false;n.fault=NtcFault::OpenCircuit;
    s.update(6000,n,air(6000),true,true,6000,55);
    assert(!s.heaterOn()&&s.faultActive());
  }
  {
    SafetyController s;start(s);
    auto n=ntc(1000);
    s.update(6000,n,air(6000),true,true,6000,55);
    assert(!s.heaterOn()&&s.faultActive()); // NTC stale
  }
  {
    SafetyController s;start(s);
    auto a=air(1000);
    s.update(7000,ntc(7000),a,true,true,7000,55);
    assert(!s.heaterOn()&&s.faultActive()); // SHT45 stale
  }
  {
    SafetyController s;start(s);s.stop();
    assert(!s.heaterOn()&&lastPinState==LOW);
    assert(s.coolingRequired(5100));
  }
  {
    SafetyController s;s.begin();
    auto invalid=good();invalid.ntcPauseC=invalid.ntcRearmC;
    assert(s.configureThermalSettings(invalid,55));
    assert(s.heaterLocked()&&!s.commissioningCanStart(0));
    invalid=good();invalid.airFaultC=55;
    assert(s.configureThermalSettings(invalid,55));
    assert(s.heaterLocked());
  }
  std::cout<<"PASS: UI5 GPIO HIGH with configured Vishay-Celsius limits, "
       <<"no automatic 20s OFF, pause/rearm, AIR hysteresis, STOP/cooldown, "
       <<"latched NTC/AIR/fan/stale/open faults; unset and incoherent menus block START\n";
}
