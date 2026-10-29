# Next work

## One priority: determine STEP10A reported stop and whether any ORIGINAL CSV was saved — NO HEAT

**Observed report 29 October 2026:** operator reports that around cycle19 display remained `HEATER OFF`, then powered down. This follows STEP10A instructions, but actual flashed firmware identity, phase, sensor temperatures and fault code were **not** supplied. CSV existence is unverified. [Exact evidence and source-code possibilities](evidence/thermal/2026-10-29-STEP10A-OPERATOR-STOP.md).

**NEXT SINGLE ACTION:** after natural cooling, **only if the prototype can be powered safely without starting HEAT**, identify firmware using `/api/status` and check `/api/last-test.csv` (one-time read; no live test) for a genuinely NEW **15-column STEP10A** CSV. Older S9B persisted CSV has10 columns; `saved_log=true` can mean only the old file. A reboot destroys old RAM status/events. User need not repeat S9A, S9B or STEP10A just for missing data.

**Then:** if a new file exists, preserve byte-identical raw, hashes, derive phase3=waiting for NTC>=60k AND air<=24, phase4=scheduled cooling after300s cumulative ON/720s runtime, phase6=fault, and correlate with any available Sanwa report. If CSV absent, explicitly record no raw trace and inspect storage/stop and test usability in SOFTWARE (without HEATER ON). Do not assume a fault, attainment of25°C, the hardware state, or operator Sanwa readings. No changes to thermal limits/protections/TCO automatically; no HEATER ON until separate review.

## Backlog

- Improve robust crash/power-loss and operator-stop data persistence without flash writes under active heating; assess whether periodic checkpoints can be safely deferred or captured via other means.
- Qualify true heater-local thermal model, physical materials/hotspots, fan and installed TCO105 compatibility; distinguish plate Sanwa from chamber SHT45.
- Finish safe, autonomous chamber temperature regulation, fan AUTO, user duration, STOP/fault handling, and hardware validation.
- Continue firmware integration and deferred OLED/startup and future PCB revision only when relevant.

See [PROJECT_STATE](PROJECT_STATE.md) for the unique current checkpoint; [ledger](THERMAL_EXPERIMENT_LEDGER.md) for archived test evidence.
