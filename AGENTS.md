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

## Safety and branch discipline

- Read `docs/PROJECT_STATE.md` for the single current checkpoint; dated protocols and old handoffs are historical evidence, not a work queue.
- `main` is the non-heating reference. HEATER-capable experiments stay on the single active experimental branch named in PROJECT_STATE, isolated from main until separately approved and qualified.
- Do not change heater flags, limits, interlocks, calibration or approved decisions to resolve a merge. Test software separately from hardware; a successful build, CI or merge never constitutes a physical PASS.
- D029: an additional TCO is optional. The installed series 105°C TCO must not be bypassed, removed or rerated automatically. D020 belongs to the next deliberate hardware revision, not a rewrite of the fabricated board.
- Identify actual HEAD and working-tree changes before editing. Recheck remote heads before publishing, merging, retargeting or deleting refs. Never force over another contributor's work.
- Preserve unique source, raw data and historical checkpoints with verified reachable commits/tags before deleting branches; update branch-based links to permanent commit URLs. Do not use a closed PR as the only backup.
- Keep at most one thermal experiment line unless a concrete technical need is documented. Reuse it for small changes; do not create checkpoint/document-only branch chains.

## Anti-regression: experimental limits are NEVER production specifications (2026-10-29)

**Reason: user-requested correction after the STEP07 40kOhm carry-over failure.** The `R_NTC <= 40kOhm` guard was created as an intentionally *EARLY* one-shot STEP07 test stop. It is not a verified NTC temperature calibration, plate setpoint, material safety threshold, OEM specification, or permanent fault boundary. STEP06C previously used an experimental 19.68kOhm stop and observed 30.32kOhm / Sanwa PLATE53C (different conditions); the earlier open-covers/fan-OFF calibration reached 7.12kOhm with Sanwa85C / sketch-indicated90C. **Neither historical extreme independently establishes safe enclosed continuous operation.** See original sources, not a chat summary.

Before **every** HEATER firmware modification/reuse or thermal-test plan, make a short provenance table for EVERY numerical heater-related limit and operating setpoint (NTC pause/resume/trip, plate contact veto, air target/fault ceiling, pulse/total time, fan and TCO), explicitly classifying each as:
- `ELECTRICAL_OR_HARDWARE` (datasheet / actual wiring / measured, with source),
- `PROVISIONAL_EXPERIMENT` (which test and purpose; not inherited by default),
- `MEASURED_CHARACTERIZATION` (actual geometry, sensor/probe and airflow),
- `APPROVED_OPERATIONAL` (exact approving decision, firmware version and scope), or
- `UNKNOWN / UNQUALIFIED` (no guessed values).

**Mandatory engineering gate:** Whenever a provisional guard reduces reachable temperatures or prevents the design objective, identify it as a **known artificial limiting factor**, not as a natural/safe thermal maximum. Require an explicit justification or removal/replacement proposal before yet another trial/firmware. A prior CI PASS, a historical test's provisional number or a protection that happened to trip must NEVER be promoted silently to a permanent ceiling. Read original thermal CSV + external Sanwa evidence before changing limits; distinguish NTC bead resistance, Sanwa nearby plate-contact temperature and SHT45 chamber air. Do not invent Celsius calibration from raw NTC.

**Change review:** Any changed thermal envelope must be an explicit, named experimental proposal with source evidence, old vs new numbers, predicted exposure and residual risks, unchanged independent protective mechanisms, exact software test and separate physical authorization/preflight. A request to develop firmware is NOT in itself permission to energize the modified heater or bypass the actually installed TCO. No automatic physical test / no silent heater enable. Keep `main` non-heating. Archive the current physical evidence; preserve new experimental source on the existing experimental branch/PR. Use one concise current-state entry and one next action, not repetitive process documents.

**CI invariant:** Verify both the currently selected trial parameters **and their classification/provenance**. A unit test that merely asserts an old hard-coded threshold is not evidence that the threshold is a correct engineering decision. Old mock tests must be adapted when replacing a provisional envelope; do not let obsolete CI assertions freeze an arbitrary test value indefinitely. Maintain hard-time hardware watchdog, sensor invalid/stale stops, real STOP, fan-fault response, air ceiling, cooldown, and one-shot bounds independently of the chosen NTC profile.


**GitHub Actions evidence retrieval:** `fetch_commit_workflow_runs` is limited to PR-triggered runs and may return an empty list for a genuinely successful **push** run. Before declaring CI unavailable or asking anyone to re-run it, read the repository's `GET /repos/{owner}/{repo}/actions/runs?head_sha=<exact-SHA>&per_page=20` via the connected GitHub fetch action; verify `head_sha`, `event`, `conclusion`, job steps, artifact name, and actual OTA application binary SHA256 from job logs. Document both source commit and any later docs-only HEAD separately. Never interpret an empty filtered PR-run query as failed/missing CI.

## Mandatory separation: commissioning-energy budgets vs real multi-hour drying

**User-reported design problem (2026-10-29):** Never treat early STEP09/10/12/13 trial cumulative HEATER ON ceilings (120/300/360 seconds) or one-off run windows (12/15 minutes) as requirements for the finished dryer. These numbers were conservative experimental exposure limits only. They do NOT represent manufacturer heater duty ratings, overtemperature safety mechanisms, or product cycle timers. A normal dryer must run the full **user-selected duration (15 minutes to 12 hours currently available in CycleController)**, cycling the heater on SHT45 temperature demand, independently stopping on actual calibrated thermal/fault conditions; accumulated ON seconds do not terminate drying.

**Architecture review gate for all future firmware:** Explicitly document whether a limit is (a) supervised-trial-only, (b) normal user cycle duration, (c) independent per-window watchdog, or (d) a verified sensor/material overheat/fault boundary. CI must exercise multi-hour duration-controlled demand and thermostat rearm with cumulative heater ON longer than 360 seconds; prove user timer ends cleanly with cooldown, manual STOP remains distinct, and safety guards still fault. Never silently carry provisional trial caps to product thermostat, and never silently remove experimental trial bounds before physical authorization. **Hardware execution of the multi-hour design is NOT permitted until independent thermal qualification; host-only tests are safe.** Retain main HEATER locked off and the experimental hardware trial separate.

**Programming workflow:** The user uses **Arduino IDE** to open the correct Git branch's `firmware/firmware.ino` and compile/upload it. Do not instruct them to download an Actions ZIP/OTA application BIN or use browser OTA; such artifacts are build evidence only unless they explicitly choose another method.

## Required introduction for every NEW test (user instruction, 2026-10-27)

**Every newly proposed test MUST start with a brief, comprehensible introduction explaining: (1) WHAT will be done, (2) WHY this test matters, (3) HOW it builds directly on the previous completed test and its actual results, and (4) WHICH future project gate/test it unlocks.** Then provide exact actionable steps, measurement timing and instrumentation, safety/abort criteria if physical, expected outputs, persistence paths, and next decisions. This requirement applies to software and analysis tests as well as physical tests.

## Mandatory experiment recording and cross-chat continuity (2026-10-26)

**User requirement: EVERY physical or software test must be reproducible from GitHub alone.** Do not rely on previous ChatGPT conversations for operating instructions or physical measurements. Summaries alone do not satisfy this rule.

**BEFORE each test**, commit or otherwise persist a concrete protocol in the repository, naming: engineering question and why the test is needed (avoid repeating already passed tests), exact branch/commit/firmware build, actual wiring/assembly, instrument make/model and probe contact LOCATION, power/enclosure/fan conditions, control inputs/buttons, expected chronological behavior, explicitly approved thresholds and abort actions, which values must be read at which events (including post-OFF temperature trend for thermal tests; never presume that a heater-contact probe must show a delayed peak), independent physical safety preflight, and data-file paths.

**DURING / AFTER each test**, capture and commit a structured run record, not only conclusions: timestamp/time zone, firmware ID, all numerical readings in chronological order and where/when read, explicitly missing or unobserved fields, actual OLED/serial/HTTP messages, operator interventions, abnormal behavior, outcome, original unedited CSV/log or a recorded reason why absent, raw-file hash/sample count when supplied, photos/video links when supplied, separate derived analysis and uncertainty, and the exact *next* engineering decision/gate. Never invent readings, complete a test from a CI pass, or call a prior file the new run. Sanwa/external-instrument readings cannot be inferred from the ESP32 CSV.

**HANDOFF GATE:** Before treating a test as finished or asking a user to start the next test, commit its record and update README, docs/PROJECT_STATE.md and docs/TODO.md (DECISIONS only if a durable decision actually changed). Record branch, commit/PR links, exact outcomes, and one next action. If data is unavailable, mark it explicitly rather than forcing unnecessary repetition. If write permission/tool fails, say so explicitly; do not claim the repository was updated.

## Documentation discipline

- `README.md`: short overview and navigation.
- `docs/PROJECT_STATE.md`: the only current factual state, with source refs, exact tested image, software/physical/untested distinctions, open problems and next action.
- `docs/TODO.md`: one priority, reason, prerequisites, completion criterion and short backlog.
- `docs/DECISIONS.md`: preserve D001–D029 and later approved decisions; update only on explicit durable agreement.
- `docs/THERMAL_EXPERIMENT_LEDGER.md`: index of protocols, immutable raw evidence and analyses.
- `docs/EXECUTION_PLAN_2026-10-24.md`: overall route to V1, not another current-state snapshot.
- Detailed hardware/design/procurement records remain in `docs/BUCK_L7987L_DESIGN.md`, `docs/ASSEMBLY.md`, `hardware/docs/PROCUREMENT.md` and the KiCad sources.

Replace stale current-state paragraphs instead of prepending another checkpoint. Preserve unique historical content in its existing evidence file or immutable Git snapshot. Do not make a new handoff document when these files suffice. Do not repeat completed tests without a new technical question.
