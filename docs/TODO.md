# Next work

## Unica priorità: CI reale STEP12P e qualificazione del nuovo envelope NTC

**Current GitHub source:** single experiment `experiment/step10-heater-diagnostics-air25-2026-10-28`, PR#18 DRAFT; candidate ID **`HOT-261029-S12P25` / OLED `H29-S12P`**, not flashed, new exact CI PASS not yet verified. Main remains HEATER forced LOW.

**User-reported root mistake, now fixed in candidate:** 40kΩ originated as **provisional intentionally EARLY STEP07 one-shot abort**, not the heater Tmax. STEP10B air25.10°C achieved with heater plate Sanwa38–39°C and 21 pulses; the old 45/60/40k software envelope, not a hardware ceiling, repeatedly stopped heating. The 17 Oct open-box trial already reached heater-sketch90°C / Sanwa85°C at R7.12k; S6C closed fan100 reached Sanwa53°C and R_NTC30.32k with provisional19.68k test abort. Distinct conditions; don't misstate plate Sanwa temperature as calibrated NTC bead temperature. Original [bench study](NTC_CALIBRATION_ANALYSIS.md), [S6C](STEP06B_FAN100_PHYSICAL_RESULTS_2026-10-25.md), [STEP10B original](evidence/thermal/2026-10-29-STEP10B-PHYSICAL-SESSION.md).

**New strictly EXPERIMENTAL proposal:** **NTC soft pause35kΩ, rearm50kΩ, BACKUP trip30kΩ**; not a certified plate setpoint nor independently approved material safety limit. The 35k pause is above S6C minimum30.32k; its actual piastra maximum cannot be predicted from Ohms alone. Previous45/60/40k frozen-guard errors removed from active code, API/UI and their CI assertions. **Unchanged:** SHT45 AIR target25/hysteresis24 and fault ceiling40°C, 20s ON + independent hardware20.5s watchdog, min12s OFF, max300s total commanded ON, max720s elapsed, sensors validity/freshness, fan PWM100, STOP/FAULT, cool-down and physically installed series TCO105 (no bypass/removal). Once flashed this is HEATER-capable but could run HOTTER; **NO HEATER ON / flashing for a hot test until approved separately**.

**What has been done on GitHub:** new controller values in `firmware/HeaterStep09AirFeedback.h`, updated `HeaterPlateDiagnostics.h` (no hidden old default), `NetworkService.h` (live candidate limits/margins), `Ui.h` (35/50/30, 50k wait), GPIO-mock with test ensuring40k no longer stops, numerical diagnostic unit tests, `FirmwareBuild.h`, serial identity, and single CI workflow. Historical `HeaterStep07GuardTrial.h` labelled historical/obsolete to discourage reuse. **AGENTS.md updated on both branches**: mandatory provenance classification, explicit warning on artificial limits, no repeated experiments, no hidden promotion of test thresholds to production.

**Immediate software test** (no hardware): Actions [STEP12P workflow](../.github/workflows/step10-heater-diagnostic-ci.yml) must report a real SUCCESS on exact source (native GPIO-mock; native diagnostic C++; STEP08 replay; actual ESP32 core3.3.12/U8g2 2.36.19 compile); record run ID/commit and verified OTA .bin SHA. A prior STEP10A or old STEP11D success is irrelevant. Current connector returns no corresponding completed run and no local full-sketch compile was performed: **do not claim verified PASS**.

**Next distinct physical gate** (NOT authorized by code/CI): propose one short, supervised new STEP12P test for the revised NTC envelope only after complete written protocol/preflight approval; observe Sanwa RD701/K on the **already documented PLATE contact near NTC**, SHT45, heater-off rebound and full CSV. Before energizing check TCO105 physically still in series, fan actually rotating, connections/thermal materials and immediate isolated24V cutoff; choose explicit manual cutoff/margins and verify stopping behavior. Previous S6C Sanwa53°C and the old veto55°C are **not** blanket authorization. Do not repeat STEP10B. The further V1 40/50/55°C air and duration/autonomy is a later phase.

## Backlog breve

- Prove sensor error/hysteresis while live and transition to V1 with a qualified local temperature model/independent safe fault ceiling.
- Fan AUTO, duration, saved settings, loss-of-power CSV/OTA, material review with appropriate optional TCO handling.
- Other PCB/production revisions only on new evidence and explicit decision.

[PROJECT_STATE](PROJECT_STATE.md) is the sole current snapshot. [AGENTS](../AGENTS.md) controls provenance before every thermal change. [Evidence ledger](THERMAL_EXPERIMENT_LEDGER.md) remains unmodified (no new physical run).
