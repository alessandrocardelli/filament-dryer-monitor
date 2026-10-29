# STEP12P — physical run, complete CSV, manual readings (29 October 2026)

**Evidence:** new user-supplied original `filament-last-test (7).csv`, byte-identical preserved [CSV](2026-10-29-STEP12P-raw.csv), **61,006 bytes**, SHA-256 `f208b4726f2d947292181a4d1b2bf4f1cee6039f9eb446cb6b9c109629ce0a5a`, git blob SHA1 `7a3aeaffddd09035f84e521eaab6548b6f89af42`. 1,089 observations, 15 fields including explicit loop/age/fault; all cells populated, ms strictly increasing. Raw file available as permanent Git object. **Do not delete or edit.**

**Firmware provenance:** submitted immediately after creating/compiling STEP12P candidate `HOT-261029-S12P25`, on experimental branch `experiment/step10-heater-diagnostics-air25-2026-10-28`, software Actions [#40942531127](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40942531127) success at `f0da9308ad4e3f3f95d8dfcb4b6e2b330576c70b`, 35/50/30k. The file does **not include a firmware-ID column or installed binary SHA**; phase/NTC transitions are consistent with STEP12P but **physical running-image identity was not independently verified**. Wall-clock start/end timestamps and electrical current not measured. Fan=100% is command, not tachometer/RPM.

**Chronology** (relative to first sample MCU uptime `40417 ms`, not civil time; source is the new CSV):

| Event | Relative time | SHT45 chamber | Raw NTC | Notes |
|---|---:|---:|---:|---|
| First sample | +0.000s |17.03°C |141.242kΩ |phase1 prerun fan100 |
| First HEATER ON | +6.039s |17.04°C |140.765kΩ |phase2 |
| ON window 4 ends | +122.061s |19.32°C |36.338kΩ |20.000s ON |
| ON window 5 ends | +151.561s |20.11°C |34.549kΩ |17.500s ON (local soft pause) |
| ON window 10 ends | +306.600s |23.57°C |34.601kΩ |14.000s ON |
| ON window 12 ends | +371.134s |24.60°C |34.596kΩ |13.500s ON |
| First target 25.00°C | **+400.232s** |25.00°C |37.554kΩ |heater OFF, 13th window ended after 9.598s; thermostat target proved to remove command |
| Max chamber SHT45 | +465.505s |**25.43°C** |CSV record at this timestamp |residual rise with heater OFF |
| Thermostat rearm HEATER ON | **+684.979s** |23.97°C |97.955kΩ |actual AIR 24°C rearm seen (previous STEP10B did not show rearm) |
| End of ON window 14 | +704.979s |23.92°C |46.863kΩ |20.000s ON |
| HEATER ON window 15 | +716.984s |23.97°C |59.245kΩ |heater commanded ON, only 3.016s |
| Absolute 12min session cutoff | **+720.000s** |23.99°C |55.152kΩ |heater OFF, phase4 COOLING |
| Last archived sample | +809.317s |23.95°C |89.837kΩ |phase4, cooldown sample; file does not show phase5 completion |

**Summary:** 15 HEATER ON transitions, **14 completed windows** (the 15th window truncated by global time cutoff), sum of commanded ON intervals **231.114s**, no `pulse_fault` or `ntc_fault` (all samples zero), air and NTC valid throughout, fan command100% throughout, chamber AIR17.03→25.43°C max. NTC minimum **34,482.9Ω**, observed after HEATER off at original MCU uptime `412301ms`, margin **4,482.9Ω** from provisional backup trip30k and **517.1Ω below the soft-pause35k** due post-OFF inertia. The previous STEP10B original CSV first reached AIR25°C at +563.196s; this run reached it at +400.232s, ~**28.9% less time** (163s faster; not literally half, and initial temperatures 17.28 vs17.03°C differed). The new AIR24→25 duty maintenance was entered near +685s but 12min budget interrupted the second on window; sustained setpoint holding still unqualified.

**Operator's manual chronological table, transcribed WITHOUT inventing column labels, units, measurement timestamps or physical probe identity:**

| Reported column 1 | Reported column 2 | Operator event |
| ---: | ---: | --- |
| 16 | 26 | |
| 27 | 34 | |
| 34 | 39 | |
| 38 | 42 | |
| 40 | 44 | |
| 41 | 44 | |
| 41 | 44 | |
| 41 | 44 | |
| 41 | 44 | |
| 41 | 44 | |
| 41 | 44 | |
| 41 | 43 | |
| | | `WAIT AIR 24C`; operator: heating restarts when chamber reaches24°C |
| 33 | | `Parte il cooling` |
| 26 | | |

These values are likely temperatures from context but no column headers/units were supplied. Therefore it is **NOT** defensible to name one as Sanwa PLATE, NTC bead, air or a second instrument. Previously documented Sanwa PLATE probe location remains in existing session files; do not ask user again for its location. Do not infer new physical plate max/hotspots from unlabeled columns. Exact timing of manual readings, measured fan RPM, current/power, temperatures at every thermal component, actual TCO contact state not observed. Manual cooling endpoint is not necessarily aligned with last CSV row.

**Engineering conclusion:** Changing 45k soft to 35k clearly improved warm-up: STEP12P real trace ON window5→12 declined to13.5–17.5s on local soft pause, but remaining heat budget 68.886s at end was not exhausted. The TEST TIMEOUT 720s now constrains useful observing of 25/24 thermostat rearm more than the heat-budget limit; the locally provisional 35k soft guard still clips the ON durations. Neither fact licenses removing the backup30k trip, changing installed TCO105, nor extrapolating to continuous 40–55°C-air qualification.

**NEXT distinct firmware experiment (proposal, not physical authorization):** STEP13A candidate uses AIR30/29°C, provisional SOFT33k/REARM50k/BACKUP30kΩ, 15min session /360s max cumulative ON, ON20s ISR20.5s OFF>=12s unchanged, fan command100%, air fault40°C unchanged, STOP/fault/cooldown intact. This is deliberately more permissive and higher-energy than STEP12P; it must first pass source-matched host GPIO test + ESP32 build and then independent physical safety preflight/approved operating criteria. No automatic HEATER ON. Contact-temperature rise cannot be predicted from an incomplete two-column manual table.
