#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT"/firmware/{AppConfig.h,AppTypes.h,ThermalSettings.h,MaterialPresets.h,HeaterControlPolicy.h,HeaterProductAir.h,HeaterPulseCommissioning.h,HeaterStep07GuardTrial.h,HeaterStep09AirFeedback.h,HeaterContactCurve.h,SafetyController.h} "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_ui5_heater_gpio.cpp" "$TMP/firmware/tests/host/"
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
    -I"$TMP/firmware" "$TMP/firmware/tests/host/test_ui5_heater_gpio.cpp" \
    -o "$TMP/ui5-gpio-mock"
"$TMP/ui5-gpio-mock"
echo "UI5 GPIO mock ONLY. No physical firmware upload or heater authorization."
