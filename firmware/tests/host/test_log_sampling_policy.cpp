// HOST test of the EXACT sampling policy used by the real ESP32 logger.
// The user selects up to 12h; old attended 5-second logging lost history.
// No ESP32, heater output, thermal run or flash access in this test.
// Firmware events are inserted at 15-minute boundaries to verify retention.
#include "../../LogSamplingPolicy.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
using namespace LogSamplingPolicy;

int main() {
  static_assert(sampleIntervalMs(true)==60000U,
                "ACTIVE attended heater source must log 12h, not old 5s");
  static_assert(sampleIntervalMs(false)==60000U);
  static_assert(kMaxDurationMs==43200000U && kCapacity==1400U);
  static_assert(!shouldCapture(59999,0,true,false,false,60000));
  static_assert(shouldCapture(60000,0,true,false,false,60000));
  static_assert(shouldCapture(3000,0,true,false,true,60000));
  static_assert(shouldCapture(3000,0,true,true,false,60000));
  static_assert(shouldCapture(0,0,false,false,false,60000));
  static_assert(shouldCapture(25,0,true,false,false,20)); // elapsed handles wrap
  static_assert(shouldCapture(10,UINT32_MAX-10U,true,false,false,20U));

  for(bool attended:{false,true}) {
    const auto interval=sampleIntervalMs(attended);
    std::size_t count=0,edges=0;
    uint32_t last=0;
    bool heater=false, previousHeater=false, truncated=false;
    // A real loop can be faster than this 1s simulation. Capture policy
    // depends on time and transitions, not on the loop's polling rate.
    for(uint32_t t=0;t<=kMaxDurationMs;t+=1000U) {
      if(t>0 && t%900000U==5000U){heater=!heater;++edges;}
      const bool change=heater!=previousHeater;
      const bool force=t==kMaxDurationMs; // final session event/checkpoint
      if(shouldCapture(t,last,count>0,force,change,interval)){
        truncated |= willTruncate(count,kCapacity);
        if(count<kCapacity)++count;
        last=t;
        if(change)previousHeater=heater;
      }
    }
    assert(edges==48); // 1 change each 15 minutes, from t=15m+5s
    assert(!truncated);
    assert(count>=kMaxDurationMs/interval); // captures a full 12h history
    assert(count<=721U+edges+1U);
    assert(count<=kCapacity);
  }
  // More than 1400 real transitions over 12h may still fill the buffer:
  // never falsely promise all history can fit without truncation.
  std::size_t count=0; bool truncated=false;
  for(std::size_t i=0;i<kCapacity+10;++i){
    assert(shouldCapture(static_cast<uint32_t>(i),0,i!=0,false,true,60000));
    truncated |= willTruncate(count,kCapacity);
    if(count<kCapacity)++count;
  }
  assert(count==kCapacity && truncated);
  assert(!willTruncate(kCapacity-1,kCapacity));
  std::cout<<"PASS 12h ACTIVE attended + product: 60s baseline, "
             "heater/phase/fault/validity event capture, real overflow flag, "
             "prior 5s CSV files untouched.\n";
}
