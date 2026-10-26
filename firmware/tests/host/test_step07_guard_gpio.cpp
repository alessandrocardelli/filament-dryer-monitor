// STEP07-G: actual experimental config and SafetyController, fake GPIO.
// No board, no Sanwa model, no assertion of any absolute NTC temperature.
#include <cassert>
#include <cstdint>
#include <iostream>
int highWrites=0, lowWrites=0, lastPinState=-1;
#include "../../SafetyController.h"
static NtcSnapshot ntc(uint32_t ms,float ohms=90000.f){
 NtcSnapshot n{};n.sampled=true;n.valid=true;n.fault=NtcFault::None;
 n.lastSampleMs=ms;n.resistanceOhms=ohms;return n;
}
static SensorSnapshot air(uint32_t ms,float c=23.f){
 SensorSnapshot a{};a.valid=true;a.lastSuccessMs=ms;a.temperatureC=c;return a;
}
static void tick(SafetyController &s,uint32_t t,bool fan=true,float R=90000.f,float T=23.f){
 s.update(t,ntc(t,R),air(t,T),true,fan,t,40.f);
}
int main(){
 static_assert(AppConfig::kHeaterCommissioningMode,"experiment must use ON-restricted branch");
 static_assert(!AppConfig::kHeaterOutputEnabled,"continuous heater thermostat must remain OFF");
 static_assert(AppConfig::kFanBringupRunPercent==100,"previous fan100 command held");
 static_assert(HeaterStep07GuardTrial::kPulseHardwareCeilingUs==20500000,
               "per-window independent GPIO timeout must not increase");
 assert(HeaterStep07GuardTrial::parameters().minimumNtcOhms==40000.f);
 assert(HeaterStep07GuardTrial::parameters().pulseCount==6);
 assert(HeaterStep07GuardTrial::parameters().pulseMs==20000);
 assert(HeaterStep07GuardTrial::parameters().gapMs==12000);
 {
  SafetyController s;s.begin();assert(lastPinState==LOW&&highWrites==0);
  tick(s,0,false);tick(s,1000);tick(s,5999);assert(!s.heaterOn());
  tick(s,6000);assert(s.heaterOn()&&highWrites==1);
  tick(s,26000);assert(!s.heaterOn());
  tick(s,38000);assert(s.heaterOn()&&highWrites==2);
  tick(s,58000);assert(!s.heaterOn());
  tick(s,70000);assert(s.heaterOn()&&highWrites==3);
  // Historic S6C after pulse 3 R~39.16k, below this new early cut:
  tick(s,70900,true,39160.f);
  assert(s.faultActive()&&!s.heaterOn()&&lastPinState==LOW);
  assert(s.commissioningFault() ==
         static_cast<uint8_t>(HeaterCommissioning::Fault::NtcResistanceLimit));
  assert(s.coolingRequired(70901)&&s.fanPermitted());
  assert(s.coolingSecondsRemaining(70901)==90);
  assert(s.coolingSecondsRemaining(115900)>0);
  assert(s.coolingSecondsRemaining(160901)==0);
  const int onCount=highWrites;
  tick(s,120000,true,90000.f);
  assert(highWrites==onCount); // an abort is LATCHED, no restart
  s.stop();assert(!s.commissioningCanStart());
 }
 {
  SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
  assert(s.heaterOn());
  tick(s,6001,true,40000.f); // exact inclusive trip
  assert(s.faultActive()&&!s.heaterOn());
 }
 {
  SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
  assert(s.heaterOn());tick(s,6001,true,90000.f,40.f);
  assert(s.faultActive()&&!s.heaterOn()); // SHT45 air stop
 }
 {
  SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
  assert(s.heaterOn());tick(s,6001,false);
  assert(s.faultActive()&&!s.heaterOn()); // fan commanded missing
 }
 {
  SafetyController s;s.begin();tick(s,0,false);tick(s,1000);tick(s,6000);
  assert(s.heaterOn());s.stop();
  assert(!s.heaterOn()&&s.coolingRequired(6001));
  assert(!s.commissioningCanStart());
 }
 std::cout<<"PASS STEP07-G: strict 40k guard, previous max pulse budget preserved, "
             "heater OFF on NTC/air/fan faults, no rearm, cooling via MOCK GPIO\n";
}
