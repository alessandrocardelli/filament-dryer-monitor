# Project working instructions

## Authority

GitHub repository `alessandrocardelli/filament-dryer-monitor` is the authoritative project state.
Chat history, memory, summaries and previous assistant messages are secondary and must not override the repository.

For electrical connectivity and PCB implementation, the actual KiCad files are authoritative. In particular, the current `.kicad_sch`, `.kicad_pcb`, `.kicad_pro` and a freshly exported netlist override prose documentation if they disagree.

Manufacturer datasheets, reference designs and standards control device-specific requirements. The project Hardware Design Manual is a source-agnostic review framework and must not be modified as part of normal repository work.

## Before working

Every new chat must complete this bootstrap before proposing circuit changes, component choices, architecture changes or implementation work:

1. Identify the active Git branch and current HEAD. If work is on a feature branch, inspect that branch rather than `main`.
2. Read this `AGENTS.md` completely.
3. Read `README.md`.
4. Read `docs/PROJECT_STATE.md`.
5. Read `docs/DECISIONS.md`.
6. Read `docs/TODO.md`.
7. Inspect the actual source files relevant to the task. For current PCB work this normally includes `hardware/Filament_Dryer_Monitor.kicad_pcb`, `hardware/Filament_Dryer_Monitor.kicad_pro`, the relevant schematic sheet(s), and the current netlist when connectivity matters.
8. Check whether documentation is stale against source before relying on it.

Do not silently revisit or replace an established decision. If new primary-source evidence conflicts with a recorded decision, state the conflict explicitly before changing the design and update `docs/DECISIONS.md` only after the new direction is agreed.

## PCB review rules

- For bottom-side footprints, do not infer physical pad direction by manually reading raw local footprint coordinates. Use KiCad/pcbnew absolute pad positions or a trustworthy board view/screenshot.
- Preserve the current four-layer reference-plane decision unless explicitly reopened: both inner layers are solid GND planes.
- Do not interpret the L7987L `PGND`/`SGND` layout guidance as two separate electrical GND nets or split inner planes. Follow `docs/DECISIONS.md` for the project implementation.
- Use the ST L7987L datasheet and STEVAL-ISA198V1 layout/Gerbers as the primary buck layout reference, adapted to this four-layer board and its actual footprints.
- Do not claim the current board is fabrication-ready from the historical `hardware/DRC.rpt`. Run and review a fresh DRC after routing is complete.
- Do not regenerate or treat production Gerbers/CPL/BOM exports as final until layout review and current DRC are closed.

## LATEST AUTHORITATIVE S9A PHYSICAL EVIDENCE — COMPLETED 2026-10-27

**The user has performed the real STEP09 S9A HEATER-ON 25°C chamber-feedback experiment and supplied BOTH manual Sanwa readings and the actual raw CSV.** The superseding source-of-truth files exist **on main AND on the experimental branch**, and the original byte-identical CSV was verified against Git blob SHA1 `9312a724743632e62e28368e27cba7241e55401f` and SHA256 `6632f79b9e815965fe7f78b6ed31ec39f4a4bcb35b3c4f0c17da655ab4b877e0`, 33,096 bytes, 778 samples.

- **[Full chronological Sanwa record and actual physical session](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md)**: initial Sanwa17°C; cycle1 ON17/OFF30; cycle2 29/36; cycle3 34/37; cycle4 35/37; cycle5 35/38; cycles6–12 each ON35/OFF38; cycle13 ON35 then cooling (no specific Sanwa OFF); final cooling Sanwa26°C. Never invent a reading at cycle13 OFF.
- [Original S9A physical CSV](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-raw.csv), [derived detailed analysis](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-analysis.md), [historic preregistered protocol](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/STEP09_REAL_HEAT_AIR_FEEDBACK_PROTOCOL_2026-10-27.md).
- **Actual result**: 13 software HEATER ON windows (12 normal OFF and 13th ON0.5s then COOLING), roughly120.64s total ON inferred from CSV transitions, cooling starts with SHT45 AIR22.59°C (user observed ~22.7°C on display), peak22.97°C in cooling and final22.93°C. Initial air17.64°C. R_NTC minimum48.7167kΩ, SOFT cutoff ~50kΩ effective; hard FAULT40kΩ never reached, no ABORTED phase. Fan duty 100% command only; not measured RPM. Sanwa reported max38°C during repeated stabilized cycles and26°C after cooling. **Target air25°C NOT reached, therefore SHT45 target-triggered thermostat OFF and hysteresis have NOT been physically validated.** The trial ended because the deliberately bounded cumulative HEAT budget120s was exhausted, NOT because the chamber hit25°C or faulted; overall600s window was not reached. Engineering review before further heating.
- **NEXT:** analyze the S9A budget vs local NTC soft50/65kΩ pause, decide whether to propose a different next experiment S9B that could test real SHT45 feedback on reaching25°C; retain existing NTC hard limit unless separately justified. Review physical setup/TCO105/material/fan before selecting any new bound. **No automatic new HEAT trial or new limits without a NEW documented/approved profile**, no user repetition of S9A or STEP03/08. PR#14 remains draft/unmerged. Firmware code not altered to register evidence.
- **Time fidelity:** session user report arrived27 Oct2026; exact physical wallclock time not supplied. Do NOT infer dates from `millis()`. Older AGENTS/README/PROJECT_STATE/TODO paragraphs saying S9A “NOT RUN” are **historical and superseded by this checkpoint**. The required WHAT/WHY/PREVIOUS/NEXT pre-test introduction remains mandatory for any new test.

## Documentation discipline

Keep the handoff files current:

- `docs/PROJECT_STATE.md`: factual current checkpoint and source/branch state.
- `docs/DECISIONS.md`: durable engineering decisions that should not be silently revisited.
- `docs/TODO.md`: immediate next actions and open gates.
- `README.md`: high-level public project status.
- `docs/BUCK_L7987L_DESIGN.md`: detailed L7987L design and layout record.
- `hardware/docs/PROCUREMENT.md`: sourcing/footprint/manufacturing-preparation state.

When a substantial checkpoint is reached, update these before handing work to a new chat.

## Latest S7U physical evidence status — 2026-10-26

**The S7U physical test HAS NOW BEEN PERFORMED and recorded** (earlier lines in this file stating "not physically tested" are historical pre-test information). The user provided **Sanwa17°C initial,17°C at first HEATER ON,36°C before third,43°C at FAULT `NTC R LIMIT`, falling normally to25°C when fan stopped**, plus the genuine **717-sample S7U CSV**. Its byte-identical repository copy was verified with original Git blob SHA1 `891ea18eb00efdb6690e7b7e3c0c6466e6a39674`, SHA256 `c417313414eede75f2269dbfb8717ac6d8246625172e315a007b8056def834ae`. Numeric data show commanded ON20s+20s+19.25s, `R_NTC=39789.3Ω` at cutoff, SHT45 air18.20→19.52°C at cutoff→20.89°C at last cooling sample. **G1 acquisition PASS limited, NOT V1 or hardware safety qualification. NEXT G2/G3 review without rerun.** Before making any new recommendation, read:
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-26-S7U-retest-raw.csv
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-26-S7U-retest-analysis.md
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/THERMAL_EXPERIMENT_LEDGER.md

Never conflate **Sanwa contact temperature** with SHT45 camera air or NTC resistance. Software PWM fan100 ≠ RPM; heater command ON/OFF ≠ independently measured current. Earlier S7G CSV is not required. No new HEAT run authorized by this document; preserve separation of HEAT-experimental PR#12 and cold/main.

## Current next physical test direction — user correction 2026-10-27

The user explicitly rejected a backward move to another physical `HEATER OFF` test after completing S6C/S7U heating and STEP03/STEP08 host simulation. **The next physical test to PREPARE is an actual supervised HEATER-ON, SHT45 chamber-feedback run**, not another fixed-pulse S7U retest. It must use the NTC heater as a separate protective input, maintain controlled fan/cooldown, retain operator Sanwa measurements and raw CSV, and begin with the required WHAT/WHY/PREVIOUS/NEXT introduction. Existing 40kΩ raw NTC shutdown was an intentionally early commissioning guard and would interrupt well before 40°C chamber air; do **not** naively lower its threshold or treat the OEM 105–118°C plate figures as approved test limits. Review actual NTC/Sanwa calibration uncertainty, TCO105 as currently installed, material/fan safety, and bounded timers, then prepare a test-specific firmware and **one real HEAT-ON protocol** with explicit approval. Brief offline compile/CI verification before flashing is a safety prerequisite, **not a user-facing new heater-OFF test phase**. Neither the direction nor this note is an authorization to energize, change the TCO, or override safety decisions. Canonical revised plan: [STEP09](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/STEP09_CLOSED_LOOP_ENGINEERING_GATE_2026-10-27.md).

## Latest STEP09 HEAT-capable source checkpoint — 2026-10-27

The user explicitly wants the **next PHYSICAL test to be a real HEATER-ON SHT45 chamber-feedback run**, not more cold-only physical retests. A candidate capable of energizing GPIO19 NOW EXISTS only on branch experiment/step09-real-sht45-feedback-2026-10-27, draft PR #14: https://github.com/alessandrocardelli/filament-dryer-monitor/pull/14 . Firmware identity HOT-261027-S9A / OLED H27-S9A; target CHAMBER25C fixed, heater NTC RAW ohms soft OFF <=50k, resume >=65k after12s, hard FAULT <=40k unchanged from S7U; 20s max continuous heat (ISR 20.5s), <=120s cumulative HEAT within10min, fan command100%, cooldown90s, Sanwa manual veto50C. This is a **candidate**, not final heater/air protection or V1 design. Unlike PR#13 mock replay this STEP09 path is actually capable of GPIO19 HIGH if compiled and flashed.

**Software evidence VERIFIED PASS:** GitHub Actions https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40485007834 , host test real SafetyController with GPIO MOCK PASS; Arduino ESP32 core3.3.12 target compile PASS at 1,126,758 bytes FLASH (85%) and 92,096 bytes static RAM (28%). First run falsely green due tee pipeline missing pipefail and DRAM linker overflow5920 bytes from a temporary 3000-sample log buffer; corrected buffer to 1400 entries sampling Step09 at500ms (complete up to 600s run+90s cooling), and added explicit set -euo pipefail in CI. Do not cite false-green first run as PASS. Read https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step09-real-sht45-feedback-2026-10-27/docs/evidence/thermal/2026-10-27-S9A-CODE-AND-CI-REPORT.md .

**Physical test has NOT run, firmware has NOT been flashed.** Before operator executes it, follow the precise what/why/previous/next introduction, review assembly TCO105/Sanwa/fan/cabling and user approval of the fixed new 25C physical trial profile. Approved explicit protocol is not yet granted: https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step09-real-sht45-feedback-2026-10-27/docs/STEP09_REAL_HEAT_AIR_FEEDBACK_PROTOCOL_2026-10-27.md . Preserve old S6C/S7U measurements and their raw files; no regression to prior OFF-only tests and no implicit change to OEM temperatures, NTC conversion, TCO, or FAN AUTO. PR14 is DRAFT and must not be silently merged into main.

## Required introduction for every NEW test (user instruction, 2026-10-27)

**Every newly proposed test MUST start with a brief, comprehensible introduction explaining: (1) WHAT will be done, (2) WHY this test matters, (3) HOW it builds directly on the previous completed test and its actual results, and (4) WHICH future project gate/test it unlocks.** Then provide exact actionable steps, measurement timing and instrumentation, safety/abort criteria if physical, expected outputs, persistence paths, and next decisions. This requirement applies to software and analysis tests as well as physical tests.

Do NOT repackage an already completed test as the next task: STEP03 host thermostat simulation **already PASS**; STEP07 S7U heated physical test **already performed and raw CSV archived**. The prepared new STEP08 is a NEW data-grounded S6C/S7U comparison and integrated raw-NTC-guard/thermostat replay. It explicitly requires **no user device operation**, no cold-only retest, no additional heater actuation and no unapproved change of thermal thresholds. See [STEP08](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/STEP08_NEXT_TEST_2026-10-27.md). Keep real/claimed timestamps separated from protocol filenames: S7U measurement report and CSV were received in this chat on **2026-10-27**; the exact physical session timestamp was not independently provided.

## Mandatory experiment recording and cross-chat continuity (2026-10-26)

**User requirement: EVERY physical or software test must be reproducible from GitHub alone.** Do not rely on previous ChatGPT conversations for operating instructions or physical measurements. Summaries alone do not satisfy this rule.

**BEFORE each test**, commit or otherwise persist a concrete protocol in the repository, naming: engineering question and why the test is needed (avoid repeating already passed tests), exact branch/commit/firmware build, actual wiring/assembly, instrument make/model and probe contact LOCATION, power/enclosure/fan conditions, control inputs/buttons, expected chronological behavior, explicitly approved thresholds and abort actions, which values must be read at which events (including post-OFF temperature trend for thermal tests; never presume that a heater-contact probe must show a delayed peak), independent physical safety preflight, and data-file paths.

**DURING / AFTER each test**, capture and commit a structured run record, not only conclusions: timestamp/time zone, firmware ID, all numerical readings in chronological order and where/when read, explicitly missing or unobserved fields, actual OLED/serial/HTTP messages, operator interventions, abnormal behavior, outcome, original unedited CSV/log or a recorded reason why absent, raw-file hash/sample count when supplied, photos/video links when supplied, separate derived analysis and uncertainty, and the exact *next* engineering decision/gate. Never invent readings, complete a test from a CI pass, or call a prior file the new run. Sanwa/external-instrument readings cannot be inferred from the ESP32 CSV.

**HANDOFF GATE:** Before treating a test as finished or asking a user to start the next test, commit its record and update README, docs/PROJECT_STATE.md and docs/TODO.md (DECISIONS only if a durable decision actually changed). Record branch, commit/PR links, exact outcomes, and one next action. If data is unavailable, mark it explicitly rather than forcing unnecessary repetition. If write permission/tool fails, say so explicitly; do not claim the repository was updated.

**Current STEP07 context:** canonical physical S7G history in the operative branch `docs/HANDOFF_2026-10-26_STEP07_S7G_TO_S7U.md`. New S7U procedure plus pre-created evidence record are on `fix/step07-clear-test-feedback-2026-10-26`:
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/STEP07_S7U_PHYSICAL_RETEST_PROTOCOL_2026-10-26.md
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md

User specifically requested **a new supervised S7U acquisition**, not recovery of prior S7G CSV and not repeat of passed cold-only/OTA/menu/fan checks. The previously communicated S7G **manual Sanwa 50°C veto** must not be silently overwritten with **55°C** from the older S6C protocol; the new SOP records source/limits and requires explicit decision for changes. Neither S7U's passed CI nor this documentation is proof of a physical S7U run. Keep normal heater regulation disabled and do not silently merge HEAT-experimental firmware into main/COLD.
