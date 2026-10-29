# Current project state

Updated 2026-10-29. **This is the sole current-state snapshot.** Dated protocols, old handoffs and archived branch documents describe their own checkpoints; their old NEXT/NOT RUN text does not override this file. No physical tests were performed during consolidation.

## Code and firmware identities

| Line / image | Exact source and status |
| --- | --- |
| `main` | SAFE BRING-UP, non-heating. Runtime baseline `a60804d2aadc9cc5c2d0c291c59c54fee0d2c072`; consolidation adds only the existing read-only CycleController elapsed accessor and host tests. `firmware.ino`, AppConfig and SafetyController are unchanged. No Wi-Fi/OTA/CSV integration on main. |
| Active experiment | `experiment/step10-heater-diagnostics-air25-2026-10-28`, [PR #18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18), now a cumulative experiment against main, not permission to merge it. Thermal source is unchanged from `05d4c36d2378be9b09664545f37243a5dfa9f477`; later consolidation changes only docs, offline replay tests and CI references. |
| Latest experimental candidate | `HOT-261028-S10A25` / OLED `H28-S10A`. [Exact original source](https://github.com/alessandrocardelli/filament-dryer-monitor/tree/05d4c36d2378be9b09664545f37243a5dfa9f477/firmware). Software PASS; **no physical STEP10A run evidenced**. |
| Latest physically run firmware | `HOT-261027-S9B180` / `H27-B180`, confirmed by recovered device status. Source line preserved at `2cefe18089175694a7d689259d1acbff7d00156f`; tested source/build provenance in [S9B180 software record](evidence/thermal/2026-10-27-S9B180-FINAL-SOFTWARE-CHECKPOINT.md). The exact binary installed was not independently hashed. |

Original heads for all 22 branches are preserved by `archive/2026-10-29/<original-branch>` tags; [audit](BRANCH_CLEANUP_2026-10-29.md) and [ref manifest](evidence/repository/2026-10-29-refs.json). Use `git rev-parse HEAD` on the relevant branch for the current documentation commit; never infer running-device firmware from a branch name.

## Hardware actually built and tested

- Four-layer L7987L + AutoEN board assembled in the eBOX Lite; fabrication source `74a3000127371ac6c3b3b7197f9036dec5b58eef`. Inner layers remain solid GND. No schematic, PCB, BOM or production output changed during consolidation.
- Physical first-article PASS, within the recorded scope: no-load 3.3 V power path, MCU/USB/UART/programming after Q2/Q3 collector-cross bodge, OLED/I2C/SHT45, four buttons, buzzer, electrical NTC path/open fault, fan and short real-heater switching. [Historical measurement record](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/a60804d2aadc9cc5c2d0c291c59c54fee0d2c072/docs/PROJECT_STATE.md), [assembly matrix](FIRST_ARTICLE_TEST_MATRIX.md).
- D020 correction exists on the physical prototype; the released KiCad source still represents the pre-bodge board. Apply it only in a future deliberate revision. Stored 2026-09-17 ERC/DRC are historical checks, not a new release approval.
- Fan fault was corrected by repairing the U4 physical pin27/GPIO16 solder joint. LEDC channel0 attempt is unproven and archived, not merged. Fan startup100%→40%, short sustained run/STOP and no auto-resume were user-reported PASS in the bring-up application. Final FAN AUTO is open.
- Heater J5 remains connected by user report. One-shot TCO105 is reported installed in series; trip/DC suitability/material protection are not qualified. D029 optional enhancement and prohibition on automatic bypass remain in force.

## Physical thermal evidence

| Test | Observed result | Validation limit |
| --- | --- | --- |
| Seven old heater/Sanwa plateaus | Sanwa30–85°C with recorded NTC resistance | Both covers open, fan OFF; not bead calibration or closed-dryer qualification. [Analysis](NTC_CALIBRATION_ANALYSIS.md). |
| S6C | Six commanded20s windows, SHT45 air peak23.98°C; manual Sanwa peak53°C | Dynamics only; no production thermostat. Raw provenance/hash caveat in audit. |
| S7U | Two20s +19.25s, raw NTC trip near39.79kΩ, Sanwa43°C at FAULT; air19.52°C at cutoff | Acquisition/guard observation PASS limited; safety qualification open. |
| S9A | Target25°C not reached; cumulative120s heat budget ended normally; air22.59°C at cooling, peak22.97°C; Sanwa max reported38°C | Physical target-triggered OFF/hysteresis **not validated**. |
| S9B180 | Physical run completed as an experiment, **ABORTED**; air23.54°C at abort, peak23.77°C, Sanwa35–37°C in later cycles; 19 ON fronts in CSV vs20 cycles reported | Recovered `pulse_fault=4 AirStale`, event575852ms `FAULT ntc=0 pulse=4`. Not normal budget completion or25°C thermostat PASS. |
| STEP10A report, received29 Oct | Operator reports `HEATER OFF` remained after approximately cycle19 and manually powered down; **exact installed firmware ID not independently verified** | New physical observation recorded; phase, fault, temperatures, fan, timeline and CSV unknown. **NOT PASS; cause unknown.** [Evidence](evidence/thermal/2026-10-29-STEP10A-OPERATOR-STOP.md). |

[Ledger with original CSV, complete Sanwa readings, protocols and analyses](THERMAL_EXPERIMENT_LEDGER.md). Do not repeat S9A/S9B or retrieve the already recovered S9B fault again.

**Measurement roles:** SHT45 = chamber AIR temperature/RH; NTC = heater thermistor resistance; Sanwa RD701/K = independent local PLATE contact temperature near the NTC. None are interchangeable. CSV heater/fan fields are commands, not current, RPM or airflow measurements. RH decrease alone is not proof of filament drying.

## Software evidence and experimental envelope

- STEP03 host policy/cycle simulation previously PASS; now preserved and rerun on main. Main GPIO mock verifies hard LOW, including electrical NTC fault latch. See [consolidation verification](BRANCH_CLEANUP_2026-10-29.md).
- STEP08 offline replay:717 S7U samples,237 ON records and8 injected faults PASS; fixture approvals/temperatures are synthetic, no GPIO. Unique branch replay is retained on the single experimental line, not treated as hardware qualification.
- STEP10A [original Actions run40737797336 SUCCESS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40737797336), exact SHA `05d4c36d2378be9b09664545f37243a5dfa9f477`: real SafetyController GPIO-mock plus ESP32 core3.3.12/U8g2 2.36.19 compile/link PASS. Flash1,177,662 bytes89%; static RAM108,904 bytes33%. [Full record](evidence/thermal/2026-10-28-STEP10A-HOST-ESP32-CI.md).
- Original OTA app `HOT-261028-S10A25-OTA-app.bin`, SHA256 `c8544793fefecf88b59b6aeabbcf5462e86e495a85c7a35d1f0671546119526b`, artifact12837437883. This identifies that artifact, not a flashed device; Actions artifacts may expire, source remains preserved.
- Candidate only: AIR25/24°C, NTC soft45k/rearm60k/hard40kΩ, ON20s/ISR20.5s, OFF≥12s, total ON300s, session720s, fan command100%, cooldown90s, operator Sanwa PLATE veto50°C. TCO105 unchanged. These values are **not durable production decisions**. [Complete proposed protocol](STEP10A_HEATER_AND_LOGGING_TEST_2026-10-28.md).

Consolidation verification: [main post-merge CI40865389451](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40865389451) SUCCESS at source `bbee1b5ebf9fb8e4692d152cba169334299727a6`; before/after flash347,115 bytes, RAM25,360 bytes. [Experimental CI40864787188](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40864787188) SUCCESS at `615e8945d1c4ee203a1bd0c22f271be69758b550`, including preserved STEP08 replay. Final source/CI reference commit `470f1e5181c9b327292af6a581c0bb407b25591d`: [CI40865576489](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40865576489) SUCCESS, with identical thermal code. Later audit receipts are docs-only. Remote inventory verified: **2 branches, only PR18 DRAFT open**. Full operations and remaining E integration: [audit](BRANCH_CLEANUP_2026-10-29.md).

## Open issues and next action

1. NEW 29 Oct STEP10A-context observation: heater OFF persistent around cycle19, operator powered down. No measured phase/fault, air/NTC/Sanwa, exact flashed ID or verified new CSV. **Heater OFF is not inherently failure:** Phase3 waits for R_NTC>=60k and air<=24 after>=12s, phase4 denotes cooldown after 300s cumulative ON/720s session; phase6 fault needs code. [Session record](evidence/thermal/2026-10-29-STEP10A-OPERATOR-STOP.md). No automatic retest. After user can safely cold-start the device, READ ONLY /api/status and /api/last-test.csv; compare 15 columns of STEP10A vs10 of older S9B. A power cut before cooldown ended might have lost volatile new log.
2. S9B immediate fault **identified** as AirStale. Synchronous expanding HTTP CSV streaming is a plausible contributor to loop gaps up to4.670s, **not conclusively proven**. STEP10A blocks live CSV streaming and records fault/ages/loop gaps; the physical fix is not validated.
2. Normal V1 heater regulation remains disabled. Actual NTC conversion/thermal envelope, local hotspots/materials, installed TCO compatibility, fan AUTO/cooldown and fault acceptance remain unqualified. No tachometer is fitted.
3. General Wi-Fi/logging/safety integration remains on the archived cold/experimental history. Latest cold Wi-Fi branch CI failed; importing its full mixed diff is deferred rather than treating earlier PASS as current validity.
4. Intermittent OLED startup artifacts, extended resilience/storage/OTA acceptance, long-duration drying and optional future PCB revision remain backlog.

**Single next activity: investigate the already-reported 29 October STEP10A-context stop WITHOUT heating.** Determine whether a genuinely NEW 15-column CSV was persistently saved from the completed/incomplete session; if available analyze the last samples (`phase`, `pulse_fault`, `air_age_ms`, `ntc_ohms`, `air_c`, `loop_gap_ms`). If not, mark trace lost; inspect firmware save/stop architecture offline and do not rerun simply to regenerate a CSV. Do not alter thermal thresholds or restart heating. See [TODO](TODO.md).
