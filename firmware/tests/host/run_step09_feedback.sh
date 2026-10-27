#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" \
   "$ROOT/firmware/HeaterControlPolicy.h" "$ROOT/firmware/HeaterPulseCommissioning.h" \
   "$ROOT/firmware/HeaterStep07GuardTrial.h" "$ROOT/firmware/HeaterStep09AirFeedback.h" \
   "$ROOT/firmware/SafetyController.h" "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_step09_feedback_gpio.cpp" "$TMP/firmware/tests/host/"
cat >"$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT=1,HIGH=1,LOW=0;
extern int highWrites,lowWrites,lastPinState;
inline void pinMode(uint8_t,uint8_t){}
inline void digitalWrite(uint8_t,uint8_t x){
  if(x==HIGH)++highWrites; else ++lowWrites;
  lastPinState=x;
}
HDR
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic \
    -I"$TMP/firmware" "$TMP/firmware/tests/host/test_step09_feedback_gpio.cpp" \
    -o "$TMP/step09-HEATER-GPIO-MOCK"
"$TMP/step09-HEATER-GPIO-MOCK"
echo "PASS STEP09 native fixture. NO ESP32 connected or heater energized."
