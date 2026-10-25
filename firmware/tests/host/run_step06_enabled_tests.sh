#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
echo "STEP06 HEAT: actual enabled config on native mock GPIO (NO PHYSICAL BOARD)"
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT/firmware/SafetyController.h" "$ROOT/firmware/HeaterControlPolicy.h" \
   "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" \
   "$ROOT/firmware/HeaterPulseCommissioning.h" \
   "$ROOT/firmware/HeaterStep06Candidate.h" \
   "$ROOT/firmware/tests/host/test_step06_enabled_gpio.cpp" \
   "$TMP/firmware/"
# test includes ../../SafetyController.h relative to fixture host directory.
mv "$TMP/firmware/test_step06_enabled_gpio.cpp" "$TMP/firmware/tests/host/"
cat >"$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT = 1, HIGH = 1, LOW = 0;
extern int highWrites, lowWrites, lastPinState;
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t value) {
  if (value == HIGH) ++highWrites;
  else ++lowWrites;
  lastPinState = value;
}
HDR
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I "$TMP/firmware" \
  "$TMP/firmware/tests/host/test_step06_enabled_gpio.cpp" -o "$TMP/test-enabled"
"$TMP/test-enabled"
echo "STEP06 HEAT: all historical controller tests with isolated cold fixtures"
bash "$ROOT/firmware/tests/host/run_step06_tests.sh"
echo "PASS: full Step06 experimental heater-on firmware policy tests, NO BOARD"
