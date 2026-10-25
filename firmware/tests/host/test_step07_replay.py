#!/usr/bin/env python3
"""Replays REAL S6C data as a constraint on the next safety trial.

This is an evidence check, not a thermistor temperature calibration.
Source is the text-normalized CSV copied from the actual user-uploaded file.
"""
from pathlib import Path
import csv

ROOT = Path(__file__).resolve().parents[3]
p = ROOT / "docs" / "evidence" / "thermal" / "2026-10-25-S6C-fan100.csv"
with p.open(newline="") as fp:
    rows = list(csv.DictReader(fp))
assert len(rows) == 652, len(rows)
assert all(x["air_valid"] == "1" and x["ntc_valid"] == "1" for x in rows)
assert all(int(x["fan_percent"]) == 100 for x in rows)
assert int(rows[-1]["ms"]) - int(rows[0]["ms"]) == 275664
start_ms = int(rows[0]["ms"])
guard = 40000.0
historic_trip = 19680.0
assert all(float(x["ntc_ohms"]) > historic_trip for x in rows)
crossings = [x for x in rows if float(x["ntc_ohms"]) <= guard]
assert crossings, "New guard would not have tripped on the prior trace"
first = crossings[0]
first_s = (int(first["ms"]) - start_ms) / 1000
assert 85 < first_s < 95, first_s
assert int(first["phase"]) == 2 and int(first["pulses"]) == 2
assert int(first["heater_command"]) == 1
assert min(float(x["ntc_ohms"]) for x in rows) < guard
assert max(float(x["air_c"]) for x in rows) < 40
print("PASS real-S6C replay: previous 19.68k guard never fired; "
      f"proposed 40k would fire at t={first_s:.3f}s during heater pulse 3. "
      "Thermal lag and Sanwa degrees remain UNQUANTIFIED.")
