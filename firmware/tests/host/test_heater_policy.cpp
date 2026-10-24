// Native simulation only: g++ -std=c++17 -Wall -Wextra -Werror -pedantic ...
#include "../../HeaterControlPolicy.h"
#include <cassert>
#include <iostream>
using namespace HeaterControl;

static Limits simulatedLimits() {
  Limits c;
  // Synthetic fixture numbers are NOT approved operating temperatures.
  c.approved = true;
  c.maximumHeaterC = 90;
  c.maximumChamberC = 60;
  c.hysteresisC = 1;
  c.maximumNtcAgeMs = 2000;
  c.maximumChamberAgeMs = 10000;
  c.minimumFanPrerunMs = 1000;
  c.maximumContinuousHeatMs = 5000;
  c.maximumCycleMs = 60000;
  return c;
}
static Inputs simulatedInputs() {
  Inputs x;
  x.cycleRequested = true;
  x.fanCommandedRunning = true;
  x.ntcElectricalValid = true;
  x.ntcThermallyCalibrated = true;
  x.heaterTemperatureC = 45;
  x.ntcAgeMs = 100;
  x.chamberValid = true;
  x.chamberTemperatureC = 30;
  x.chamberAgeMs = 500;
  x.targetChamberC = 40;
  x.elapsedCycleMs = 1000;
  return x;
}
int main() {
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    assert(!p.evaluate(0,c,x));           // fan pre-run
    assert(!p.evaluate(999,c,x));
    assert(p.evaluate(1000,c,x));         // request, NOT physical energization
    x.chamberTemperatureC = 40;
    assert(!p.evaluate(1100,c,x));        // reaching setpoint turns request OFF
    x.chamberTemperatureC = 39.5f;
    assert(!p.evaluate(1200,c,x));        // no short cycling inside deadband
    x.chamberTemperatureC = 39.0f;
    assert(p.evaluate(1300,c,x));
    x.cycleRequested = false;
    assert(!p.evaluate(1400,c,x));        // stop clears requested heat
    x.cycleRequested = true;
    assert(!p.evaluate(1500,c,x));        // new fan pre-run; never auto-heat
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    c.approved = false;
    assert(!p.evaluate(0,c,x));
    assert(p.fault() == Fault::UnapprovedLimits);
    c.approved = true;
    assert(!p.evaluate(10000,c,x));       // latched even after valid config
    assert(!p.clearFault(false,true));
    assert(!p.clearFault(true,false));
    assert(p.clearFault(true,true));
    assert(!p.evaluate(11000,c,x));       // reset also requires new fan pre-run
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.ntcElectricalValid = false;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::NtcInvalid);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.ntcThermallyCalibrated = false;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::NtcInvalid);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.ntcAgeMs = 2001;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::NtcStale);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.chamberValid = false;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::ChamberInvalid);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.chamberAgeMs = 10001;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::ChamberStale);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.heaterTemperatureC = 90;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::HeaterTooHot);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.chamberTemperatureC = 60;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::ChamberTooHot);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    x.elapsedCycleMs = 60000;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::CycleTooLong);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    assert(!p.evaluate(0,c,x));
    assert(p.evaluate(1000,c,x));
    x.fanCommandedRunning = false;
    assert(!p.evaluate(1001,c,x) && p.fault() == Fault::FanLost);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    assert(!p.evaluate(0,c,x));
    assert(p.evaluate(1000,c,x));
    assert(p.evaluate(5999,c,x));
    assert(!p.evaluate(6000,c,x));
    assert(p.fault() == Fault::ContinuousHeatTooLong);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    c.maximumHeaterC = NAN;
    assert(!p.evaluate(0,c,x) && p.fault() == Fault::UnapprovedLimits);
  }
  {
    Policy p; auto c = simulatedLimits(); auto x = simulatedInputs();
    assert(!p.evaluate(UINT32_MAX - 499,c,x));
    assert(!p.evaluate(499,c,x));       // elapsed 999 across wrap
    assert(p.evaluate(500,c,x));        // elapsed 1000 across wrap
  }
  std::cout << "PASS: 14 native heater policy scenarios (no GPIO, no heat)\n";
}
