#!/usr/bin/env python3
"""Contract checks on the actual STEP07 experiment UI/firmware source.
The ESP32 CI compile validates the full signatures. This test guards the
operator-facing wording that caused an unusable physical trial.
"""
from pathlib import Path
root=Path(__file__).resolve().parents[3]
ui=(root/"firmware/Ui.h").read_text()
ino=(root/"firmware/firmware.ino").read_text()
buzzer=(root/"firmware/Buzzer.h").read_text()
safety=(root/"firmware/SafetyController.h").read_text()
build=(root/"firmware/FirmwareBuild.h").read_text()

for part in [
    "FAULT / HEATER OFF", "SANWA: WATCH PEAK",
    "COOL DONE / CSV READY", "CSV SAVE FAILED",
    "COOL DONE / SAVING", "MODE = MUTE BEEP",
    "COOLING  %lu:%02lu",
    "HEATER ON %u/%u", "HEATER OFF", "NTC TRIP 40k",
]:
    assert part in ui, f"Operator feedback missing: {part}"
assert "AIR LIMIT 40C" in ui, "Configured SHT45 air bound is 40C"
assert "AIR LIMIT 35C" not in ui, "Stale misleading air fault label"

assert "safety.coolingSecondsRemaining(nowMs)" in ino
assert "csvSavedThisRun = thermalLog.saveWhenSafe();" in ino
assert 'network.note(csvSavedThisRun ? "NEW CSV READY"' in ino
assert "buzzer.playCommissionFaultOnce(nowMs);" in ino
assert "buzzer.startFaultAlarm(nowMs);" in ino  # D022 unchanged in normal mode
assert "void playCommissionFaultOnce(uint32_t nowMs)" in buzzer
assert "kFaultAlarm, countOf(kFaultAlarm), nowMs, false" in buzzer
assert "uint32_t coolingSecondsRemaining(uint32_t nowMs) const" in safety
assert "if (!coolingRequired(nowMs) || !everHeated_) return 0;" in safety
assert 'HOT-261026-S7U' in build
print("PASS: OLED tells operator heater OFF/ON, Sanwa peak, cooling countdown,"
      " new CSV ready and one-shot fault alert; normal D022 preserved")
