// EXPERIMENT STEP06: GPIO integration with synthetic sensors ONLY.
// Compiles actual committed heater-enabled AppConfig and SafetyController.
// The IO functions below are mocks: no physical board or real temperature.
#include <cassert>
#include <cstdint>
#include <iostream>
int highWrites=0, lowWrites=0, lastPinState=-1;
#include "../../SafetyController.h"
static NtcSnapshot ntc(uint32_t ms,float ohms=90000.0f) {
  NtcSnapshot v{};v.sampled=true;v.valid=true;v.fault=NtcFault::None;
  v.lastSampleMs=ms;v.resistanceOhms=ohms;return v;
}
static SensorSnapshot air(uint32_t ms,float temp=23.0f) {
  SensorSnapshot v{};v.valid=true;v.lastSuccessMs=ms;v.temperatureC=temp;return v;
}
static void tick(SafetyController &s,uint32_t ms,bool fan=true,float ohms=90000.0f,float temp=23.0f) {
  s.update(ms,ntc(ms,ohms),air(ms,temp),true,fan,ms,40.0f);
}
int main(){
  static_assert(AppConfig::kHeaterCommissioningMode,"This fixture MUST compile the real HEAT-mode config");
  static_assert(AppConfig::kHeaterStage05TrialReviewed && AppConfig::kHeaterStep06TrialReviewed,
                "Separate authorized test flags required");
  static_assert(!AppConfig::kHeaterOutputEnabled && !AppConfig::kHeaterNtcModelApproved,
                "Do not enable unqualified continuous thermostat");

  {
    SafetyController s;s.begin();
    assert(lastPinState==LOW&&!s.heaterOn());
    tick(s,0,false);assert(!s.heaterOn()&&highWrites==0);
    tick(s,1000);assert(!s.heaterOn()); // five second fan prerun STARTS here
    tick(s,5999);assert(!s.heaterOn());
    tick(s,6000);assert(s.heaterOn()&&highWrites==1&&lastPinState==HIGH);
    tick(s,15999);assert(s.heaterOn());
    tick(s,16000);assert(!s.heaterOn()&&lastPinState==LOW);
    tick(s,27999);assert(!s.heaterOn());
    tick(s,28000);assert(s.heaterOn()&&highWrites==2);
    tick(s,38000);assert(!s.heaterOn());
    tick(s,50000);assert(s.heaterOn()&&highWrites==3);
    tick(s,60000);assert(!s.heaterOn());
    tick(s,72000);assert(s.heaterOn()&&highWrites==4);
    tick(s,82000);assert(!s.heaterOn());
    assert(s.coolingRequired(82001));
    tick(s,142000);assert(s.commissioningComplete()&&!s.heaterOn());
    tick(s,150000);assert(highWrites==4&&lastPinState==LOW);
    assert(!s.commissioningCanStart());
  }
  {
    SafetyController s;s.begin();
    tick(s,100,false);tick(s,1100);tick(s,6100);
    assert(s.heaterOn());int before=highWrites;
    tick(s,6101,true,19680.0f);
    assert(s.faultActive()&&!s.heaterOn()&&lastPinState==LOW);
    assert(s.coolingRequired(6102)&&s.fanPermitted());
    s.stop();assert(s.coolingRequired(6103));
    tick(s,9100,true);assert(highWrites==before&&!s.heaterOn());
  }
  {
    SafetyController s;s.begin();
    tick(s,100,false);tick(s,1100);tick(s,6100);
    assert(s.heaterOn());tick(s,6101,true,90000,40.0f);
    assert(s.faultActive()&&!s.heaterOn()); // >=40 C chamber stop
  }
  {
    SafetyController s;s.begin();
    tick(s,100,false);tick(s,1100);tick(s,6100);
    assert(s.heaterOn());tick(s,6101,false);
    assert(s.faultActive()&&!s.heaterOn()); // lost fan COMMAND, no tach
  }
  {
    SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
    assert(s.heaterOn());s.stop();
    assert(!s.heaterOn()&&lastPinState==LOW&&s.coolingRequired(6001));
    assert(!s.commissioningCanStart()); // STOP cannot restart without reboot
  }
  std::cout<<"PASS: actual Step06 HEAT-mode config on host mock: exactly four 10s windows, "
              "fan prerun, NTC/air/fan abort, STOP and cooldown (NO HARDWARE)\n";
}
