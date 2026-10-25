// STEP 03: native-only SHT45 chamber thermostat and cycle simulation.
// Synthetic heater-local temperatures/NTC approval are fixture values, NOT
// a calibration, not validated thermal trip limits, and never GPIO commands.
#include "../../HeaterControlPolicy.h"
#include "../../CycleController.h"
#include <cassert>
#include <cstdint>
#include <iostream>

using HeaterControl::Fault;
using HeaterControl::Inputs;
using HeaterControl::Limits;
using HeaterControl::Policy;

static Limits syntheticLimits() {
  Limits l{};
  l.approved = true;  // ONLY for this host simulation
  l.maximumHeaterC = 90.0f;   // FAKE fixture values, never production limits
  l.maximumChamberC = 60.0f;
  l.hysteresisC = 1.0f;
  l.maximumNtcAgeMs = 2000;
  l.maximumChamberAgeMs = 10000;
  l.minimumFanPrerunMs = 1000;
  l.maximumContinuousHeatMs = 4000;
  l.maximumCycleMs = 180000;
  return l;
}

static Inputs nominal() {
  Inputs x{};
  x.cycleRequested = true;
  x.fanCommandedRunning = true;
  x.ntcElectricalValid = true;
  x.ntcThermallyCalibrated = true; // simulated; REAL NTC is NOT approved
  x.heaterTemperatureC = 45.0f;
  x.ntcAgeMs = 250;
  x.chamberValid = true;
  x.chamberTemperatureC = 30.0f;
  x.chamberAgeMs = 500;
  x.targetChamberC = 40.0f;
  x.elapsedCycleMs = 0;
  return x;
}

static void trace(uint32_t now, const Inputs &x, bool request) {
  std::cout << "SIM_ONLY,ms=" << now
            << ",SHT45_AIR_C=" << x.chamberTemperatureC
            << ",NTC_SIM_C=" << x.heaterTemperatureC
            << ",fan_command=" << (x.fanCommandedRunning ? 1 : 0)
            << ",HEAT_REQUEST_ONLY=" << (request ? 1 : 0)
            << ",GPIO19=NOT_CONNECTED\n";
}

int main() {
  const Limits l = syntheticLimits();
  int assertions = 0;

  {
    // Actual cycle-timer class and actual host-tested heater REQUEST policy.
    CycleController cycle;
    Policy p;
    CycleSettings settings{};
    settings.durationMinutes = 1;
    settings.targetTempC = 40;
    assert(!cycle.isRunning()); ++assertions; // initial boot/standby OFF
    cycle.start(settings, 0);
    bool completed = false;

    auto tick = [&](uint32_t now, float airC, bool fanCommanded) {
      if (cycle.update(now) == CycleEvent::Completed) completed = true;
      Inputs x = nominal();
      x.cycleRequested = cycle.isRunning();
      x.elapsedCycleMs = cycle.elapsedMs(now);
      x.chamberTemperatureC = airC;
      x.fanCommandedRunning = fanCommanded;
      const bool request = p.evaluate(now, l, x);
      trace(now, x, request);
      return request;
    };
    assert(!tick(0, 32, false)); ++assertions; // fan OFF
    assert(!tick(100, 32, true)); ++assertions; // fan starts
    assert(!tick(1099, 32, true)); ++assertions; // insufficient prerun
    assert(tick(1100, 32, true)); ++assertions;  // air below 39C -> HEAT request
    assert(!tick(1500, 40, true)); ++assertions; // reaches air setpoint -> OFF
    assert(!tick(1600, 39.5f, true)); ++assertions; // hysteresis, OFF
    assert(tick(1700, 39, true)); ++assertions;  // below 39 -> request
    assert(tick(2000, 39, true)); ++assertions;
    assert(!tick(2100, 40, true)); ++assertions; // stop heat at air setpoint
    cycle.stop();                               // front-panel STOP
    assert(!tick(2200, 30, true)); ++assertions;
    cycle.start(settings, 3000);               // new START must prerun
    assert(!tick(3000, 30, true)); ++assertions;
    assert(tick(4000, 30, true)); ++assertions;
    assert(!tick(63000, 40, true)); ++assertions; // duration elapsed
    assert(completed && !cycle.isRunning()); ++assertions;
    assert(!tick(65000, 30, true)); ++assertions; // no auto-resume
  }

  {
    Policy p; auto x = nominal();
    assert(!p.evaluate(0, l, x)); ++assertions;
    assert(p.evaluate(1000, l, x)); ++assertions;
    x.fanCommandedRunning = false; // commanded fan lost; no tachometer
    assert(!p.evaluate(1001, l, x) && p.fault() == Fault::FanLost); ++assertions;
    x.fanCommandedRunning = true;
    assert(!p.evaluate(5000, l, x)); ++assertions; // latch persists
    assert(!p.clearFault(false, true)); ++assertions;
    assert(!p.clearFault(true, false)); ++assertions;
    assert(p.clearFault(true, true)); ++assertions;
    x.cycleRequested = false; // operator STOP
    assert(!p.evaluate(5001, l, x)); ++assertions;
    x.cycleRequested = true;
    assert(!p.evaluate(5100, l, x)); ++assertions; // fresh prerun
    assert(!p.evaluate(6099, l, x)); ++assertions;
    assert(p.evaluate(6100, l, x)); ++assertions;
  }
  {
    Policy p; auto x = nominal();
    assert(!p.evaluate(0, l, x)); ++assertions;
    assert(p.evaluate(1000, l, x)); ++assertions;
    assert(!p.evaluate(5000, l, x) &&
           p.fault() == Fault::ContinuousHeatTooLong); ++assertions;
    assert(!p.evaluate(6000, l, x)); ++assertions; // latched
  }
  {
    Policy p; auto x = nominal(); x.ntcThermallyCalibrated = false;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::NtcInvalid);
    ++assertions; // real current NTC cannot pass this gate
  }
  {
    Policy p; auto x = nominal(); x.chamberValid = false;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::ChamberInvalid);
    ++assertions;
  }
  {
    Policy p; auto x = nominal(); x.chamberAgeMs = 10001;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::ChamberStale);
    ++assertions;
  }
  {
    Policy p; auto x = nominal(); x.ntcElectricalValid = false;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::NtcInvalid);
    ++assertions;
  }
  {
    Policy p; auto x = nominal(); x.ntcAgeMs = 2001;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::NtcStale);
    ++assertions;
  }
  {
    Policy p; auto x = nominal(); x.heaterTemperatureC = 90;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::HeaterTooHot);
    ++assertions; // TEMP synthetic, NOT a real heater limit
  }
  {
    Policy p; auto x = nominal(); x.chamberTemperatureC = 60;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::ChamberTooHot);
    ++assertions;
  }
  {
    Policy p; auto x = nominal(); x.elapsedCycleMs = 180000;
    assert(!p.evaluate(0, l, x) && p.fault() == Fault::CycleTooLong);
    ++assertions;
  }
  {
    Policy p; auto x = nominal(); x.cycleRequested = false;
    assert(!p.evaluate(0, l, x)); ++assertions;
    CycleController rebootedCycle;
    assert(!rebootedCycle.isRunning()); ++assertions; // BOOT standby, no auto-resume
  }

  std::cout << "PASS: STEP03 " << assertions
            << " assertions, scripted SHT45 air thermostat, fault latches, "
               "STOP/restart/expiry/no autoresume (request only, no GPIO)\n";
  return 0;
}
