#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
bash "$ROOT/firmware/tests/host/run_step05_tests.sh"
g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  "$ROOT/firmware/tests/host/test_step06_candidate.cpp" -o "$TMP/step06"
"$TMP/step06"
echo 'PASS: stage06 candidate tests are HOST ONLY, no GPIO, no flashing'
