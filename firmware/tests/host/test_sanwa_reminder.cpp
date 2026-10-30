#include "../../SanwaReminder.h"
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
  SanwaReminder a;
  assert(!a.active() && !a.due(0) && a.secondsUntilNext(0)==0);
  a.start(10000);
  assert(a.stage()==SanwaReminder::Stage::Heating);
  assert(a.secondsUntilNext(10000)==60);
  assert(a.secondsUntilNext(69000)==1);
  assert(!a.due(69999));
  uint32_t previous=0;
  for(uint32_t minute=1;minute<=120;minute+=(minute<10?1:2)) {
    uint32_t now=10000+minute*60000U;
    assert(a.due(now));
    assert(a.secondsUntilNext(now)==0);
    a.consume(now);
    assert(!a.due(now)); // never duplicate a prompt
    assert(a.secondsUntilNext(now)==(minute<10?60U:120U));
    previous=minute;
  }
  assert(previous==120);
  // Skipping events must never produce backlogged beep cascades.
  a.start(4000);
  assert(a.due(4000+31U*60000U));
  a.consume(4000+31U*60000U);
  assert(!a.due(4000+31U*60000U));
  assert(a.secondsUntilNext(4000+31U*60000U)==60U); // next:32min
  // Millis wrap (unsigned modular arithmetic).
  a.start(UINT32_MAX-900U);
  assert(!a.due(58098));
  assert(a.due(59100));
  a.consume(59100);
  assert(!a.due(59100));
  // STOP restarts its own timer, never uses last PWM OFF pulse.
  a.start(500);
  a.stop(60500);
  assert(a.stage()==SanwaReminder::Stage::AfterStop);
  for(uint32_t i=1;i<=4;++i) {
    const uint32_t now=60500+i*30000U;
    assert(a.secondsUntilNext(now)==0 && a.due(now));
    a.consume(now);
    assert(i==4 ? !a.active() : a.active());
  }
  assert(!a.due(60500+500000U));
  // Starting a NEW session must not continue the old 2-minute sequence.
  a.start(900000);
  assert(a.secondsUntilNext(900000)==60U);
  a.cancel(); // FAULT cancels reminder permanently
  assert(!a.active() && !a.due(1100000));
  // Cold STOP without a session does nothing.
  a.stop(1100000);
  assert(!a.active());

  std::cout<<"PASS Sanwa reminders: 1..10min then 12..120min/2min, "
           "four post-STOP cues, skip missed deadlines, 32-bit wrap, "
           "fault cancellation, restart and countdown. No real buzzer.\n";
}
