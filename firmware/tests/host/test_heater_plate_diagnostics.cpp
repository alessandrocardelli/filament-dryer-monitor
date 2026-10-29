// STEP11D independent read-only diagnostic: NO GPIO, no HEATER control.
#include <cassert>
#include <cmath>
#include <iostream>
#include "../../HeaterPlateDiagnostics.h"

int main() {
  using namespace HeaterPlateDiagnostics;
  const auto invalid = margin(NAN);
  assert(!invalid.valid && !historicalOpenBoxContact(NAN).valid);
  assert(!historicalOpenBoxContact(100000.0f).valid);
  assert(!historicalOpenBoxContact(6000.0f).valid);
  const auto high = margin(45000.0f);
  assert(high.valid);
  assert(std::fabs(high.aboveSoftOhms) < 0.1f);
  assert(std::fabs(high.aboveHardOhms - 5000.0f) < 0.1f);
  const auto below = margin(43738.2f);
  assert(below.valid);
  assert(std::fabs(below.aboveSoftOhms + 1261.8f) < 0.2f);
  assert(std::fabs(below.aboveHardOhms - 3738.2f) < 0.2f);
  assert(!margin(45000.0f, 40000.0f, 45000.0f).valid);
  for (float r : {64620.0f, 42270.0f, 28720.0f, 19680.0f,
                  13730.0f, 9810.0f, 7120.0f}) {
    assert(historicalOpenBoxContact(r).valid);
  }
  const float s39 = historicalOpenBoxContact(42270.0f).sanwaEquivalentC;
  const float s47 = historicalOpenBoxContact(28720.0f).sanwaEquivalentC;
  assert(std::fabs(s39 - 39.0f) < 0.01f);
  assert(std::fabs(s47 - 47.0f) < 0.01f);
  const float mid = historicalOpenBoxContact(35000.0f).sanwaEquivalentC;
  assert(mid > 39.0f && mid < 47.0f);
  // This must never change protection: existing 40kOhm guard is still
  // established in HeaterStep09AirFeedback.h and SafetyController.
  std::cout << "PASS STEP11D historical proxy and real-ohm margins; "
               "NON-ACTUATING; no thermistor thermal qualification\n";
}
