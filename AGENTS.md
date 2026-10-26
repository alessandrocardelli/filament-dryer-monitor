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

## Mandatory experiment recording and cross-chat continuity (2026-10-26)

**User requirement: EVERY physical or software test must be reproducible from GitHub alone.** Do not rely on previous ChatGPT conversations for operating instructions or physical measurements. Summaries alone do not satisfy this rule.

**BEFORE each test**, commit or otherwise persist a concrete protocol in the repository, naming: engineering question and why the test is needed (avoid repeating already passed tests), exact branch/commit/firmware build, actual wiring/assembly, instrument make/model and probe contact LOCATION, power/enclosure/fan conditions, control inputs/buttons, expected chronological behavior, explicitly approved thresholds and abort actions, which values must be read at which events (including post-OFF temperature trend for thermal tests; never presume that a heater-contact probe must show a delayed peak), independent physical safety preflight, and data-file paths.

**DURING / AFTER each test**, capture and commit a structured run record, not only conclusions: timestamp/time zone, firmware ID, all numerical readings in chronological order and where/when read, explicitly missing or unobserved fields, actual OLED/serial/HTTP messages, operator interventions, abnormal behavior, outcome, original unedited CSV/log or a recorded reason why absent, raw-file hash/sample count when supplied, photos/video links when supplied, separate derived analysis and uncertainty, and the exact *next* engineering decision/gate. Never invent readings, complete a test from a CI pass, or call a prior file the new run. Sanwa/external-instrument readings cannot be inferred from the ESP32 CSV.

**HANDOFF GATE:** Before treating a test as finished or asking a user to start the next test, commit its record and update README, docs/PROJECT_STATE.md and docs/TODO.md (DECISIONS only if a durable decision actually changed). Record branch, commit/PR links, exact outcomes, and one next action. If data is unavailable, mark it explicitly rather than forcing unnecessary repetition. If write permission/tool fails, say so explicitly; do not claim the repository was updated.

**Current STEP07 context:** canonical physical S7G history in the operative branch `docs/HANDOFF_2026-10-26_STEP07_S7G_TO_S7U.md`. New S7U procedure plus pre-created evidence record are on `fix/step07-clear-test-feedback-2026-10-26`:
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/STEP07_S7U_PHYSICAL_RETEST_PROTOCOL_2026-10-26.md
- https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md

User specifically requested **a new supervised S7U acquisition**, not recovery of prior S7G CSV and not repeat of passed cold-only/OTA/menu/fan checks. The previously communicated S7G **manual Sanwa 50°C veto** must not be silently overwritten with **55°C** from the older S6C protocol; the new SOP records source/limits and requires explicit decision for changes. Neither S7U's passed CI nor this documentation is proof of a physical S7U run. Keep normal heater regulation disabled and do not silently merge HEAT-experimental firmware into main/COLD.
