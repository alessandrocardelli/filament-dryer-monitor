// STEP05: actual SafetyController, fake Arduino GPIO, no physical hardware.
// The approved-host fixture for this file is a TEMPORARY AppConfig copy.
#include <cassert>
#include <cstdint>
#include <iostream>

int highWrites = 0, lowWrites = 0, lastPinState = -1;
#include "../../SafetyController.h"

static NtcSnapshot ntcAt(uint32_t now) {
  NtcSnapshot n{};
  n.sampled = true;
  n.valid = true;
  n.fault = NtcFault::None;
  n.lastSampleMs = now;
  n.temperatureC = 45.0f; // HOST ONLY synthetic calibration
  n.resistanceOhms = 122300.0f;
  return n;
}

static SensorSnapshot airAt(uint32_t now) {
  SensorSnapshot a{};
  a.valid = true;
  a.temperatureC = 30.0f;
  a.lastSuccessMs = now;
  return a;
}

int main() {
  {
    SafetyController cold;
    cold.begin();
    assert(lastPinState == LOW && highWrites == 0);
    cold.update(0, ntcAt(0), airAt(0), true, true, 0, 40);
    cold.update(1200, ntcAt(1200), airAt(1200), true, true, 1200, 40);
#ifndef STEP05_HOST_APPROVED_FIXTURE
    assert(!cold.heaterOn() && highWrites == 0);
    assert(!cold.coolingRequired(1200));
    assert(cold.fanPermitted());
    cold.stop();
    assert(!cold.heaterOn() && !cold.coolingRequired(1201));
#else
    assert(cold.heaterOn() && highWrites > 0);
    assert(cold.coolingRequired(1200));
    cold.stop(); // manual STOP must preserve cooldown history
    assert(!cold.heaterOn() && lastPinState == LOW);
    assert(cold.coolingRequired(1201));
    assert(cold.coolingRequired(1200 + AppConfig::kPostHeatCooldownMs - 1));
    assert(!cold.coolingRequired(1200 + AppConfig::kPostHeatCooldownMs));

    SafetyController faulty;
    faulty.begin();
    faulty.update(200000, ntcAt(200000), airAt(200000), true, true, 0, 40);
    faulty.update(201200, ntcAt(201200), airAt(201200), true, true, 1200, 40);
    assert(faulty.heaterOn());
    auto openNtc = ntcAt(201201);
    openNtc.fault = NtcFault::OpenCircuit;
    openNtc.valid = false;
    faulty.update(201201, openNtc, airAt(201201), true, true, 1201, 40);
    assert(faulty.faultActive() && !faulty.heaterOn());
    assert(faulty.coolingRequired(201202) && faulty.fanPermitted());
    faulty.stop(); // transitioning to FAULT calls stop(); history preserved
    assert(faulty.coolingRequired(201203) && faulty.fanPermitted());
    assert(!faulty.coolingRequired(201201 + AppConfig::kPostHeatCooldownMs));
    assert(!faulty.fanPermitted()); // no endless fan after cooldown in fault

    SafetyController noFan;
    noFan.begin();
    noFan.update(300000, ntcAt(300000), airAt(300000), true, true, 0, 40);
    noFan.update(301200, ntcAt(301200), airAt(301200), true, true, 1200, 40);
    assert(noFan.heaterOn());
    noFan.update(301201, ntcAt(301201), airAt(301201), true, false, 1201, 40);
    assert(noFan.faultActive() && !noFan.heaterOn());
    assert(noFan.coolingRequired(301202) && noFan.fanPermitted());
#endif
  }
  std::cout << "PASS: STEP05 default cold GPIO LOW; approved HOST-ONLY fixture "
               "checks STOP/FAULT/fan-loss cooling without autorun\n";
}
