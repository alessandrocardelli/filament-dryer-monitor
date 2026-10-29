# Next work

## One priority: repair STEP10A UI and session logging, NO HEATER ON

**Completed evidence recovery 29 October:** user supplied `filament-last-test (5).csv` but exact SHA256 `1e17e6e131ce7cf9074bfe8d23d31f5e8ea1a605833ff36c0a21fe6568314beb`, 44,461 bytes, 1,067 data rows,10 columns shows it was the **previous S9B180 file byte-identically**. No new 15-field STEP10A session CSV survived to the persistent `/last-test.csv` seen. The operator observed persistent `HEATER OFF` around cycle19, **fan actually rotating**, then powered down. Browser status link was a partial fragment, not the phase/fault snapshot. The installed firmware exact ID and actual phase/fault/Sanwa/air remain unknown. [Original session evidence](evidence/thermal/2026-10-29-STEP10A-OPERATOR-STOP.md). **Do not ask to redo this test to regenerate raw data.**

**ONLY NEXT ACTION: implement and host/ESP32-test a SOFTWARE-only correction on the single experimental branch** to clarify why heating is OFF (Phase3 requires >=12s OFF, R_NTC>=60kΩ and SHT45 AIR<=24°C; Phase4 denotes cooldown and Phase6 abort), fix the residual S9B labels/incorrect displayed10min versus actual12min max and15min outer cycle, and provide clear session state / saved-data provenance and safe final save on STOP/complete/cooldown. Any proposed persistence strategy must preserve the restriction on FLASH writes while HEATER may energize and must not stop fan prematurely. New hardware thermal exposure needs separate review; no changes to NTC40k fault, TCO105 or manual50°C veto just to avoid pauses. Validate no heating via host tests and actual ESP32 CI.

## Backlog

- Review reliable power-loss/event persistence and whether a safe post-heat checkpoint is possible; no blind flash writes during HEATER ON.
- Physically qualify local heater NTC/plate Sanwa model, hotspots/materials, TCO105 in series, fan behavior and chamber thermostat under separately approved tests.
- Finish V1 autonomous controls, fan AUTO, fault logic, stop/duration and hardware qualification.
- Continue general integration/OTA/UI polish once these issues are controlled.

See [PROJECT_STATE](PROJECT_STATE.md) for current code and [THERMAL_EXPERIMENT_LEDGER.md](THERMAL_EXPERIMENT_LEDGER.md) for original records.
