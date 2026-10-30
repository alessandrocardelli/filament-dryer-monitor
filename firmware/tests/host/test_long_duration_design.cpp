// SOFTWARE-ONLY V1 architecture test; DOES NOT enable heater hardware.
// Proves real duration-controlled thermostat has no inherited cumulative-ON
// 300/360s timeout. All per-window, sensor, air and manual-stop checks remain.
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "../../HeaterStep09AirFeedback.h"

using namespace HeaterStep09AirFeedback;

static Inputs good(float airC, bool active = true) {
  Inputs in{};
  in.cycleActive = active;
  in.fanRunning = true;
  in.ntcValid = true;
  in.ntcOhms = 90000.0f;
  in.ntcAgeMs = 100;
  in.airValid = true;
  in.airC = airC;
  in.selectedAirTargetC = 40.0f;
  in.airAgeMs = 500;
  return in;
}

int main() {
  {
    const auto trial = parameters();
    assert(trial.runPolicy == RunPolicy::SupervisedTrial);
    assert(trial.maximumTotalHeatMs == 360000UL);
    assert(trial.maximumRunMs == 900000UL);
    // This is still the actual hardware configuration in SafetyController.
  }
  {
    const auto p = durationControlledDesignParameters();
    assert(p.runPolicy == RunPolicy::DurationControlledDesign);
    assert(p.maximumOnWindowMs == 20000UL);
    assert(p.minimumOffMs == 12000UL);
    assert(p.ntcHardTripOhms == 30000.0f);
    Controller c{p};
    uint64_t commandedOnMs = 0;
    bool previousOn = false, sawHighAfter3Hours = false;
    bool sawAirTargetOff = false, sawRearmAfter2Hours = false;
    bool sawSaturatedCounter = false;
    uint32_t onStarted = 0;
    constexpr uint32_t hour = 3600000UL;
    const uint32_t dryingDuration = 4UL * hour; // user duration, NOT heat budget
    for (uint32_t now = 0; now <= dryingDuration + 100000UL; now += 1000UL) {
      // Simulates a setpoint reached, then chamber cooling. No fake hardware.
      const float airC = now < hour ? 37.0f : now < 2*hour ? 40.0f : 39.0f;
      const bool cycleActive = now < dryingDuration;
      const bool heat = c.update(now, good(airC, cycleActive));
      assert(c.fault() == Fault::None);
      if (heat) {
        if (!previousOn) onStarted = now;
        assert(now - onStarted <= p.maximumOnWindowMs);
        if (now >= 3*hour) sawHighAfter3Hours = true;
        if (now >= 2*hour) sawRearmAfter2Hours = true;
        commandedOnMs += 1000;
      } else if (now >= hour && now < 2*hour && cycleActive) {
        sawAirTargetOff = true;
      }
      previousOn = heat;
      if (c.pulsesDone() == 255U) sawSaturatedCounter = true;
      if (!cycleActive) {
        assert(!heat);
        if (now == dryingDuration)
          assert(c.phase() == Phase::Cooldown && !c.finished());
      }
    }
    assert(commandedOnMs > 360000UL);
    assert(sawHighAfter3Hours && sawAirTargetOff && sawRearmAfter2Hours);
    assert(sawSaturatedCounter); // 8-bit CSV status saturates, not wraps
    assert(c.phase() == Phase::Complete && c.finished());
    assert(c.fault() == Fault::None); // normal user timer expiry, not STOP fault
  }
  // STOP remains a distinct latched manual abort.
  {
    Controller c{durationControlledDesignParameters()};
    c.update(0, good(37));
    c.update(5000, good(27));
    assert(c.heaterRequested());
    c.cancel();
    assert(!c.heaterRequested() && c.phase() == Phase::Aborted);
    assert(c.fault() == Fault::ManualStop);
    assert(!c.update(10000, good(27)));
  }
  // Independent backup raw NTC cutoff still works in duration mode.
  {
    Controller c{durationControlledDesignParameters()};
    c.update(0, good(27));
    c.update(5000, good(27));
    auto in = good(37);
    in.ntcOhms = 29999.0f;
    assert(!c.update(6000, in));
    assert(c.phase() == Phase::Aborted);
    assert(c.fault() == Fault::NtcResistanceLimit);
  }
  // Air 60 C abort is independent from the time policy.
  {
    Controller c{durationControlledDesignParameters()};
    c.update(0, good(27));
    auto in = good(60);
    assert(!c.update(6000, in));
    assert(c.fault() == Fault::AirLimit && c.phase() == Phase::Aborted);
  }
  std::cout << "PASS: HOST-ONLY 4h user timer, >360s accumulated ON, "
               "20s window, AIR30/29 rearm, clean timed cooldown, "
               "STOP/NTC/AIR faults, 8-bit pulse counter saturation. "
               "No heater hardware enabled.\n";
}
