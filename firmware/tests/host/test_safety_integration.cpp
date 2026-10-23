#include <cassert>
#include <iostream>

// Uses the *actual* firmware headers. The harness supplies mocked Arduino
// pin functions and a synthetic copy of AppConfig for the second run.
int highWrites = 0;
int lowWrites = 0;
int lastPinState = -1;

#include "../../SafetyController.h"

int main() {
  NtcSnapshot ntc{};
  ntc.sampled = true;
  ntc.valid = true;
  ntc.fault = NtcFault::None;
  ntc.temperatureC = 45.0f;
  SensorSnapshot air{};
  air.valid = true;
  air.temperatureC = 30.0f;
  SafetyController safety;
  safety.begin();
  assert(!safety.heaterOn() && lastPinState == LOW);

  for (uint32_t ms = 0; ms < 3200; ms += 100) {
    ntc.lastSampleMs = ms;
    air.lastSuccessMs = ms;
    safety.update(ms, ntc, air, true, true, ms, 40.0f);
  }

#ifdef ENABLE_APPROVED_FIXTURE
  // Fixture limits/calibration are invented strictly for offline simulations.
  assert(!safety.heaterLocked());
  assert(safety.heaterOn() && highWrites > 0);
  safety.stop();
  assert(lastPinState == LOW && !safety.heaterOn());
  safety.update(3200, ntc, air, true, false, 3200, 40.0f);
  assert(lastPinState == LOW);
  ntc.fault = NtcFault::OpenCircuit;
  safety.update(3300, ntc, air, true, true, 3300, 40.0f);
  assert(safety.faultActive() && lastPinState == LOW);
#else
  // Production defaults: GPIO19 HIGH is NEVER issued.
  assert(safety.heaterLocked() && !safety.heaterOn());
  assert(highWrites == 0);
  ntc.fault = NtcFault::OpenCircuit;
  safety.update(3300, ntc, air, true, true, 3300, 40.0f);
  assert(safety.faultActive() && highWrites == 0 && lastPinState == LOW);
#endif
  std::cout << "PASS: integrated controller mock, GPIO HIGH writes="
            << highWrites << "\n";
}
