# HOT-261030-SANWA — actual physical AIR55 session, 30 October 2026

**Source-of-truth note:** data recorded from one user-supplied CSV and the user's independently filled Sanwa Google Sheet. This is the real test immediately after the operator-requested Sanwa-beep feature. The prior `docs/NTC_CALIBRATION_ANALYSIS.md` is a separate **17 October** seven-point characterization; do not conflate conditions. The operator reported an unsatisfactory temperature plateau during the session. **The AIR55 setpoint was NOT achieved. No successful physical qualification is claimed.**

## Exact experiment and inputs

- Firmware user-selected: `HOT-261030-SANWA`; experimental source test/ESP32 compile PASS at `b21036ae959802a71c4833e3830dd9d6a611f4e0` ([Actions #41128992631](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/41128992631)). The CSV has no firmware-build-ID field; identity comes from the user's external Sheet, **not a binary hash read from the device**.
- Active branch `experiment/step10-heater-diagnostics-air25-2026-10-28` / PR#18 DRAFT; **main is HEATER-OFF**.
- User input: SHT45 **chamber AIR target55°C**, user cycle duration **60 minutes**. Operator explicitly ran this attended experiment. Exact wall-clock time in Europe/Rome **not supplied**; calendar date 2026-10-30 from conversation and original spreadsheet, no fabricated start/end clock.
- Hardware: existing ESP32 PCB and eSUN eBOX Lite heating assembly. Independent **Sanwa RD701/K** thermocouple at previously recorded heater PLATE near its embedded NTC; thermocouple is NOT wired to ESP32. SHT45 measures chamber air; raw NTC reads resistance through the GPIO34 ADC divider. Actual Sanwa mounting geometry was already established; no new physical repositioning reported for this run. Fan PWM **100% commanded**, physical RPM/flow **not logged**. Existing one-shot **105°C TCO reportedly in series**; fresh actual mounting/body temperature, continuity and DC break qualification **not recorded**; neither bypass nor modification reported. Enclosure state during this specific run **not explicitly restated in the supplied record** (prior planned procedure was closed enclosure); do not label it verified closed.
- Operator controls: press short ON/OFF to start after user menu setup; 1..10min then every2min Sanwa reading/buzzer as documented, 60min timed completion. No STOP or FAULT user intervention recorded, but operator confirmation of button events and actual audible cues not separately captured. Last MCU row phase4 at the 60-min end is compatible with normal expiry; **no final COMPLETE phase / full postheat cooling evidence in this CSV**.
- Software-only experimental guards in this exact build (NOT independently qualified operating limits): AIR target40–55°C selectable; AIR thermostat1°C rearm; AIR fault60°C; NTC soft pause<=**9.81kΩ**, NTC rearm>=**13.73kΩ**, latched backup<=**7.12kΩ**, NTC open250kΩ; per-ON window20s, GPIO watchdog ISR20.5s, minimum OFF2s; 5s fan prerun, commanded fan100%, fan cooling90s, no cumulative heater-on budget. Sensor stale/invalid, STOP, fan-command and TCO protections remain as originally implemented. Sanwa contact85°C manual research veto was **proposed**, not a certified safe maximum.

## Original evidence preserved

1. [Unedited 850-row MCU CSV](2026-10-30-AIR55-SANWA-raw.csv) — `filament-last-test(2).csv` user upload, **47,793 bytes**, 15 columns, **SHA256 `5cbb1235ff14cb319de943aeb5b45cb6202c1bac61eb3ad005e7bd61c0c720da`**. Original byte-identical line terminators LF and terminal newline were preserved. Explicit file commit: [`2e4a3e41`](https://github.com/alessandrocardelli/filament-dryer-monitor/commit/2e4a3e41c3c2ff8f0d66304c6f16d7f92fb9a9ec).
2. [Operator's original Google Sheet — Sanwa measurements](https://docs.google.com/spreadsheets/d/1I5utkzGP1zvD168jLghpYB_y_myGjxMb1ccLens4Cks/edit). Manual entries below are the **2026-10-30 snapshot**, retained here so a future sheet edit does not remove the original sample values. The Sanwa was not auto-recorded by the ESP32.
3. Previously completed independently measured 7-point NTC/Sanwa characterization, 2026-10-17: [source and conditions](../../NTC_CALIBRATION_ANALYSIS.md), both box covers open and fan OFF. **Sanwa85°C at NTC7.12kΩ, previous standalone sketch indicated90°C.** This is already measured: do not ask to repeat it.

## All operator Sanwa readings (PLATE contact, °C)

| Elapsed since START | Sanwa °C | User note |
| --- | ---: | --- |
| Prima di START | 17.0 | — |
| 0:00 | 17.0 | — |
| 1:00 | 38.0 | — |
| 2:00 | 48.0 | — |
| 3:00 | 53.0 | — |
| 4:00 | 53.0 | — |
| 5:00 | 53.0 | — |
| 6:00 | 55.0 | — |
| 7:00 | 57.0 | — |
| 8:00 | 59.0 | — |
| 9:00 | 61.0 | — |
| 10:00 | 63.0 | — |
| 12:00 | 64.0 | — |
| 14:00 | 65.0 | — |
| 16:00 | 66.0 | — |
| 18:00 | 66.0 | — |
| 20:00 | 66.0 | — |
| 22:00 | 67.0 | — |
| 24:00 | 67.0 | — |
| 26:00 | 68.0 | — |
| 28:00 | 67.0 | — |
| 30:00 | 68.0 | — |
| 32:00 | 68.0 | — |
| 34:00 | 68.0 | — |
| 36:00 | 68.0 | — |
| 38:00 | 68.0 | — |
| 40:00 | 69.0 | — |
| 42:00 | 68.0 | — |
| 44:00 | 68.0 | — |
| 46:00 | 69.0 | — |
| 48:00 | 68.0 | — |
| 50:00 | 69.0 | — |
| 52:00 | 68.0 | — |
| 54:00 | 69.0 | — |
| 56:00 | 68.0 | — |
| 58:00 | 68.0 | — |
| 60:00 | 69.0 | — |

The manual sheet shows **37 populated readings including two initial 17°C entries**, maximum **69°C** at several scheduled observations near end. This is a **plate-contact** reference, not an NTC-bead Celsius estimate. Post-STOP +30/+60/+90/+120s temperatures are **absent from the supplied sheet** and must remain missing; the uploaded CSV stops at the first phase4 COOLDOWN record.

## MCU CSV quantitative analysis (source: raw file only)

| Metric | Actual result |
| --- | --- |
| Raw acquisition | 850 data records, 15 columns, device uptime 68,882..3,668,906ms (span **3,600.024s**) |
| SHT45 chamber AIR | **17.28°C initial; 41.12°C last; maximum 41.16°C** |
| Final AIR plateau | Last 10min records ranged approximately **41.07..41.16°C**, still ~13.9°C below55°C setpoint |
| Embedded NTC raw | **min11,726.7Ω** (11.727k), so no recorded soft trip at9.81kΩ and no hard trip at7.12kΩ |
| HEATER command | **146 OFF→ON fronts**, all 146 matched to OFF fronts; **145/146 ON windows exactly20s**, final one18.348s at timer end |
| Heater cumulative commanded ON | **2,918.348s = 48min38.348s** computed from recorded switching edges; not a current/power measurement |
| Inter-window OFF gaps | 145 intervals, **minimum2.0s, mean4.6596s, median5.5s, maximum6.297s** |
| Fan | field `fan_percent=100` for all rows; physical fan RPM/airflow not measured |
| Logged faults | `pulse_fault=0`, `ntc_fault=0`, all 850 records; electrical and thermal safety **not qualified** by this fact |
| End of CSV | `phase=4` cooldown entry, last AIR41.12°C; **no cooldown tail or finish proof**, physical temperature peak after OFF unknown |

### Verified reason for poor performance (CODE + EVIDENCE, not conjecture)

`firmware/HeaterStep09AirFeedback.h`, current `attendedAir55Parameters()`: `maximumOnWindowMs=20000` forcibly closes each heat window; in `Phase::Gap`, restart requires both `nowMs-phaseSinceMs>=minimumOffMs` AND `in.ntcOhms>=ntcResumeOhms` (**13.73kΩ**) AND demand from SHT45. The same NTC-recovery gate applies **after a 20-second watchdog test window**, even though `in.ntcOhms<=ntcSoftPauseOhms` (9.81kΩ) did **not occur at all** in this run.

Thus the code explicitly introduces artificial HEATER-OFF intervals and a thermal recovery requirement **without a genuine NTC-local soft-pause event**. The 20s/20.5s safety-time concepts originated in supervised electrical qualification, not an approved continuous heater rating. This is a confirmed **architectural limitation**, not proof that the modified heater is physically incapable of reaching AIR55; actual physically attainable temperature after correcting control is not yet known.

## Historical reference — DO NOT discard or pretend absent

The 17 October research sequence already provides the full physical measured pairs:

| Sanwa near-plate contact | R_NTC |
| ---: | ---: |
|30°C|64.62kΩ|
|39°C|42.27kΩ|
|47°C|28.72kΩ|
|56°C|19.68kΩ|
|64°C|13.73kΩ|
|74°C|9.81kΩ|
|**85°C**|**7.12kΩ**|

**Measured-characterization**, not thermistor-bead calibration or cross-enclosure guarantee; the old standalone sketch indicated **90°C** when the nearby Sanwa read85°C. The user's proposed plate thermostat target ~90°C is **not already measured** by that experiment; the available dataset does not support extrapolating resistance below7.12kΩ. Installed 105°C TCO does not automatically imply 90°C at the *heater* is safe: TCO body temperature, location and protection capability differ.

## Outcome / next gate

**Outcome: test completed for ~60-minute command interval, target NOT achieved, heat-control architecture is defective for the design goal.** No repeat of an unchanged 20s/NTC-gap test.

**Only next action:** revise control **on this same experimental branch** to implement demand-driven NTC-local thermostat/hysteresis and independent AIR target control without artificial periodic heater-off gaps. Preserve independent GPIO emergency-off watchdog and sensor/fan/STOP faults, avoid turning any unqualified NTC-ohm pair into an approved final safety limit. Before any changed hot run, review explicitly whether any additional local thermal measurement/limit qualification and the installed TCO105/material exposure are needed. First verify behavior with host mocks (actual long commanded ON without forced20s OFF; genuinely NTC-driven STOP/REARM; AIR55/54; ISR expiry; fault and timer STOP), compile ESP32, then separately request operator permission and perform physical preflight. **No physical heater test automatically authorized.**
