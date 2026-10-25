// STEP06B: actual enabled SafetyController source with MOCK GPIO only.
// Tests the new 6x20s profile; no electrical heater and no ESP32 board here.
#include <cassert>
#include <cstdint>
#include <iostream>
int highWrites=0, lowWrites=0, lastPinState=-1;
#include "../../SafetyController.h"
static NtcSnapshot ntc(uint32_t ms,float ohms=90000.f) {
  NtcSnapshot n{};n.sampled=true;n.valid=true;n.fault=NtcFault::None;
  n.lastSampleMs=ms;n.resistanceOhms=ohms;return n;
}
static SensorSnapshot air(uint32_t ms,float c=23.f) {
  SensorSnapshot a{};a.valid=true;a.lastSuccessMs=ms;a.temperatureC=c;return a;
}
static void tick(SafetyController &s,uint32_t t,bool fan=true,float R=90000.f,float C=23.f){
  s.update(t,ntc(t,R),air(t,C),true,fan,t,40.f);
}
int main(){
  static_assert(AppConfig::kHeaterCommissioningMode,
                "Run these tests ONLY with the actual enabled experimental source");
  static_assert(!AppConfig::kHeaterOutputEnabled &&
                !AppConfig::kHeaterNtcModelApproved &&
                !AppConfig::kHeaterProfileApproved,
                "No normal thermostat permitted");
  static_assert(AppConfig::kFanBringupRunPercent==100,
                "Fan must command 100% throughout Step06B trial");
  static_assert(HeaterStep06Extended::kPulseHardwareCeilingUs==20500000,
                "Each heating window has a 20.5s hardware cutoff");
  const auto p=HeaterStep06Extended::parameters();
  assert(p.pulseCount==6&&p.pulseMs==20000&&p.gapMs==12000);
  assert(p.minimumNtcOhms==19680&&p.maximumChamberC==40);
  assert(p.cooldownMs==90000&&p.overallTimeoutMs==330000);
  {
    SafetyController s;s.begin();assert(lastPinState==LOW);
    tick(s,0,false);tick(s,1000);
    assert(!s.heaterOn()&&highWrites==0);
    const uint32_t on[]={6000,38000,70000,102000,134000,166000};
    for(unsigned i=0;i<6;i++){
      tick(s,on[i]-1);assert(!s.heaterOn());
      tick(s,on[i]);assert(s.heaterOn()&&lastPinState==HIGH);
      assert(highWrites==static_cast<int>(i)+1);
      tick(s,on[i]+19999);assert(s.heaterOn());
      tick(s,on[i]+20000);assert(!s.heaterOn()&&lastPinState==LOW);
      if(i!=5)tick(s,on[i]+31999);
    }
    assert(s.coolingRequired(186001));
    tick(s,276000);
    assert(s.commissioningComplete()&&!s.heaterOn());
    tick(s,279000);assert(highWrites==6&&lastPinState==LOW);
    assert(!s.commissioningCanStart());
  }
  {
    SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
    assert(s.heaterOn());
    tick(s,6001,true,19680);assert(!s.heaterOn()&&s.faultActive());
    assert(s.coolingRequired(6002)&&s.fanPermitted());
    s.stop();assert(!s.commissioningCanStart());
  }
  {
    SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
    assert(s.heaterOn());
    tick(s,6001,true,90000,40);assert(!s.heaterOn()&&s.faultActive());
  }
  {
    SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
    assert(s.heaterOn());
    tick(s,6001,false);assert(!s.heaterOn()&&s.faultActive());
  }
  {
    SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
    assert(s.heaterOn());
    s.stop();assert(!s.heaterOn()&&s.coolingRequired(6001));
    assert(!s.commissioningCanStart());
  }
  std::cout<<"PASS STEP06B ACTUAL ENABLED CONFIG, MOCK GPIO: 6x20s max, "
              "fan100, cool, NTC/air/fan abort, STOP, one-shot\n";
}
