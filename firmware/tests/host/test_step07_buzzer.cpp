// Real Buzzer.h with fake LEDC: fault beep must FINISH by itself in trial mode.
// The D022 normal repeating alarm must remain available for non-trial mode.
#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>
uint32_t dutyNow=0, writeCount=0;
#include "../../Buzzer.h"
int main() {
  Buzzer b;
  b.begin();
  b.setVolume(BuzzerVolume::Low);
  b.playCommissionFaultOnce(0);
  assert(dutyNow>0);
  b.update(140); assert(dutyNow==0);
  b.update(230); assert(dutyNow>0);
  b.update(370); assert(dutyNow==0);
  b.update(1270); assert(dutyNow==0);
  const auto silentAfter=writeCount;
  b.update(1500); b.update(5000); b.update(100000);
  assert(dutyNow==0);
  assert(writeCount==silentAfter); // no endless alarm after experiment FAULT
  b.startFaultAlarm(200000); assert(dutyNow>0); // D022 still repeats in product
  b.update(200140);b.update(200230);b.update(200370);
  b.update(201270);assert(dutyNow>0); // repeating normal fault alarm
  b.muteFault();assert(dutyNow==0);
  const auto afterMute=writeCount;
  b.update(210000);assert(writeCount==afterMute);
  std::cout<<"PASS: commissioning FAULT alarm two beeps then SILENT; "
              "normal repeating D022 available; MODE-mute stops audio\n";
}
