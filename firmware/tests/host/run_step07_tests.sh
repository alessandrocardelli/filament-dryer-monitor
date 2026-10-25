#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
echo "STEP07: all previous COLD and HEAT GPIO-mock regressions"
bash "$ROOT/firmware/tests/host/run_step06b_tests.sh"
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" \
  "$ROOT/firmware/HeaterPulseCommissioning.h" \
  "$ROOT/firmware/HeaterControlPolicy.h" \
  "$ROOT/firmware/HeaterStep07GuardTrial.h" \
  "$ROOT/firmware/SafetyController.h" "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_step07_guard_gpio.cpp" "$TMP/firmware/tests/host/"
cat >"$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT = 1,HIGH = 1,LOW = 0;
extern int highWrites,lowWrites,lastPinState;
inline void pinMode(uint8_t,uint8_t){}
inline void digitalWrite(uint8_t,uint8_t state){
 if(state==HIGH)++highWrites; else ++lowWrites;
 lastPinState=state;
}
HDR
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I"$TMP/firmware" \
 "$TMP/firmware/tests/host/test_step07_guard_gpio.cpp" -o "$TMP/guard-test"
"$TMP/guard-test"
echo "STEP07: replay the actual physical S6C CSV (652 samples)"
python3 "$ROOT/firmware/tests/host/test_step07_replay.py"
echo "PASS: Step07 guard trial tests passed (native mock only)"
