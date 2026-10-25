#!/usr/bin/env bash
set -euo pipefail
# Native Step 03 is GPIO-FREE; the real firmware still compiles independently.
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cat >"$TMP/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
HDR

echo "STEP03: existing heater policy/integration/pulse tests (real source headers)"
bash "$ROOT/firmware/tests/host/run_heater_tests.sh"

echo "STEP03: host-only scripted thermostat/cycle trace"
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I "$TMP" \
    "$ROOT/firmware/tests/host/test_thermostat_cycle.cpp" \
    -o "$TMP/thermostat-scripted-NO-GPIO"
"$TMP/thermostat-scripted-NO-GPIO"

echo "PASS: Step03 native suite completed; simulator has no physical heater output"
