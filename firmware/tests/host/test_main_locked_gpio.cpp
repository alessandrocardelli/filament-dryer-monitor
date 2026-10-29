// Actual main SafetyController, native GPIO mock only. No device connection.
#include "../../SafetyController.h"
#include <cassert>
#include <iostream>
int highWrites=0, lowWrites=0, lastPinState=-1;
int main() {
  SafetyController safety;
  safety.begin();
  assert(highWrites==0 && lastPinState==LOW && safety.heaterLocked());
  NtcSnapshot ntc{};
  for (int i=0; i<100; ++i) safety.update(ntc);
  ntc.sampled=true; ntc.valid=true; ntc.fault=NtcFault::None;
  for (int i=0; i<100; ++i) safety.update(ntc);
  for (auto fault: {NtcFault::OpenCircuit,NtcFault::ShortCircuit,NtcFault::InvalidReading}) {
    SafetyController fresh; fresh.begin();
    ntc.fault=fault; fresh.update(ntc);
    assert(fresh.faultActive() && !fresh.fanPermitted());
    ntc.fault=NtcFault::None; fresh.update(ntc);
    assert(fresh.faultActive()); // no silent rearm
    assert(!fresh.heaterPermitted() && fresh.heaterLocked());
  }
  assert(highWrites==0 && lastPinState==LOW && lowWrites>=210);
  std::cout<<"PASS: actual main SafetyController stays LOW before samples, with valid NTC and latched electrical faults; no GPIO HIGH writes\n";
}
