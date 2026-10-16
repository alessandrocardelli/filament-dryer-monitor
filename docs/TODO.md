# TODO

Current phase: **first-article hand assembly and staged bring-up** (2026-10-04). Active hardware branch: `main`. Fabrication-source checkpoint: `74a3000127371ac6c3b3b7197f9036dec5b58eef` (2026-09-17, `Production files`). This file tracks remaining work; read `AGENTS.md` and `docs/PROJECT_STATE.md` before changes. Do not silently alter the already submitted fabrication revision.

## Completed / recorded

- [x] Front-panel V1 interaction model specified on 2026-10-08 in `docs/UI_SPEC.md` and accepted as D022: standalone OLED/button operation, safe standby after boot, no automatic cycle resume, temperature/time/fan-AUTO V1 setup, RH displayed/logged but not a standalone completion trigger, fault-priority UI, buzzer event semantics and D3 remaining internal diagnostics.

- [x] Gate 5A real fan load test passed on 2026-10-08: actual 24 V fan started normally; bench input current peaked at ~102 mA and settled at ~89 mA. Static real-load switching path is validated; PWM characterization remains pending.

- [x] Gate 4D analog front end passed on 2026-10-05 with R28=47 kΩ and C19=100 nF. Real NTC/J7 path then passed on 2026-10-11: real NTC connected, 2.368 V at J7 pin 2, GPIO34 RAW 2779-2783 / 2409-2412 mV. Five-point 33.03k-99.9k substitute-resistor calibration was completed and the application now contains measured ADC correction plus NTC resistance/temperature conversion. Live-test of the integrated firmware and final thermal limits remain pending.

- [x] Gate 4C buzzer first-article test passed on 2026-10-05: BZ1/Q6/D6/R27/R37 populated; GPIO33 ~2.7 kHz tone and a three-note melody both worked. Reduced PWM duty cycle gives a quieter output than the initial ~50% drive; final firmware volume levels remain to be tuned.

- [x] Gate 4B buttons/status-LED first-article test passed on 2026-10-04: SW3/SW4/SW5/SW6 each measure ~3.3 V at rest and ~0 V when pressed. BTN_DOWN initially sat at ~0.91 V because of a poor R24 solder joint; reworking R24 restored the expected level. D3/R18 was then validated with a GPIO26 blink test. D3 is an internal diagnostic LED; firmware semantics are fixed by D021. Gate 4A remains blocked only on the external JST-PH SHT45 harness, so Gate 4C buzzer bring-up may proceed independently.

- [x] Gate 3 MCU/USB group fully populated on 2026-10-04. Unpowered rail/node checks passed; at 24 V with a 50 mA current limit the board drew ~8 mA and TP2/3V3_MCU, EN_ESP and IO0 were all 3.3 V. RESET and BOOT switches each pulled their node to ~2.5 mV while pressed. No concerning hotspot was seen on the thermal camera.
- [ ] Resolve Gate 3 USB enumeration failure before flashing diagnostic firmware: Windows reports Code 43 / `USB\\DEVICE_DESCRIPTOR_FAILURE`, unchanged with a different USB cable and host port. VBUS, divider/reset levels, U3 orientation/power, and D+/D− continuity from J2 through U3 to U2 have passed; inspect/verify remaining CP2102/QFN and signal-integrity possibilities before rework.

- [x] Gate 1A powered test passed on 2026-10-02: supply current decayed to ~0 after capacitor charging at 5 V, 12 V and 24 V; TP1 tracked the applied input voltage; Q1 gate measured 0 V at 5 V, 0 V at 12 V and 9.31 V at 24 V, consistent with D1 clamping Q1 |VGS| to about 14.7 V.

- [x] Bare-board rail/isolation preflight reported passed on 2026-10-01 before soldering: no short to GND on raw 24 V, protected 24 V, buck 3.3 V or MCU 3.3 V; FB1 pads isolated; J1 raw input isolated from TP1 with F1/Q1 absent.

- [x] Bare PCBs and component shipment reported received on 2026-10-01; physical inspection and package/quantity reconciliation are still pending and must not be treated as completed.

- [x] L7987L + AutoEN design basis, USB routing, two solid inner GND planes and U5's six peripheral thermal/GND vias accepted in `docs/DECISIONS.md`.
- [x] Updated schematic, 2026-09-17 netlist (correct USB-C D+/D− mapping), ERC (0 errors/0 warnings) and DRC (0 active errors/0 unconnected pads; one intentionally excluded U4 silkscreen-to-board-edge warning) are stored on the active branch.
- [x] Current production-source checkpoint contains `hardware/production/Filament_Dryer_Monitor_bom.csv` and `Filament_Dryer_Monitor_positions.csv`; unsuffixed legacy `bom.csv`/`positions.csv` must not be used.
- [x] Bare-PCB Gerbers/drills uploaded to JLCPCB; a production/CAM package was received and reviewed in the order discussion. Confirm actual manufacturing/shipping status from the supplier; it is not tracked here.
- [x] Live `BOM TME` checked against 94 PCB-mounted references and the current released BOM/netlist (2026-09-19); `TME_IMPORT` excludes in-house references.
- [x] TME order placed 2026-09-20 for the supplier-designated items for one assembled PCB. Original confirmation recorded C11 as in stock.

## Immediate: supplier and component availability

- [ ] **C11 — await TME's definitive warehouse answer.** Ordered Samsung `CL21A226MAYNNNE`, 22 µF / 25 V X5R / 0805, is currently unlocated in their stock. The later week 48/2026 estimate refers to C11 only. **No cancellation or replacement has been approved.**
- [ ] Ask/obtain explicit confirmation that other available TME items will ship without waiting for C11 and without extra fees; the supplier's latest message did not answer that question.
- [ ] If C11 cannot be found, settle shipment/cost/refund with TME before agreeing to any replacement or full-order cancellation. Candidate *only*: TDK `C2012X5R1C226M125AC` (TME `C2012X5R1C226MAC`), 22 µF / 16 V X5R / 0805. Verify exact part and effective capacitance/DC-bias before assembly; do not edit KiCad/BOM solely because it was discussed.
- [ ] On delivery, audit **actual shipped quantities/MPNs and packaging** against order confirmation, the live `BOM TME` and `hardware/production/Filament_Dryer_Monitor_bom.csv`. Supplier-imposed minimum packs may differ from the per-PCB need.
- [ ] Physically verify in-house stocks of J5 and the 13 resistor BOM rows (33 component refs marked `In casa`). In-house label is a planning declaration, **not** a verified physical inventory.
- [ ] Check the separate **off-board** assembly inventory: 1.54-inch SSD1309 OLED module for J4 and its physical fit/pinout, SHT45 sensor and J3 harness, independent heater thermal cutoff (TCO) in the HEATER+ wire, mating JST housings/contacts/leads for J1/J3/J6/J7 where needed, heater/fan/NTC wiring, mounting hardware and actual enclosure clearances. These are **not** covered by the 94-reference PCB BOM. Do not mark them ordered/in-house without evidence.

## Incoming bare PCB and assembly

- [ ] Confirm JLCPCB CAM approval/production status and actual shipment or receipt. Archive the final board-order stackup/production confirmation if useful; the uploaded Gerber/CAM ZIP is not maintained as a repository source file.
- [ ] Inspect incoming PCBs: 91 × 52 mm profile, board thickness, rounded corners, solder mask, plated holes, USB-C/JST footprints, component alignment and 4-layer continuity. Inspect U2's exposed pad and any open plated holes for hand-soldering suitability.
- [ ] Check physical fit/polarization of in-house J5 HALJIA, J4 OLED, J1/J3/J6/J7 mating connectors and BZ1 footprint.
- [ ] **Before assembly**, inspect the actual released PCB/netlist and current firmware, define a reference-by-reference staged population matrix, accessible test points, current limits and pass/fail criteria; prepare and compile a minimal safe-OFF serial diagnostic firmware. See the staged handoff in `docs/ASSEMBLY.md`.
- [ ] Hand assemble and test the first article **by functional groups**, preserving complete buck support circuitry and keeping **R5 unpopulated until initial 3.3 V is verified**. Flash the diagnostic firmware once ESP32/USB is populated and use it at later test gates. Do not omit C11 from the completed functional prototype merely to bypass the supply issue.
- [ ] First-article bring-up: check input protection/fuse, the 3.3 V rails, USB/UART enumeration and boot, I2C SHT45/OLED, buttons, fan/heat MOSFET defaults, buzzer and NTC input before enabling heating.
- [ ] Power/thermal tests: 3.3 V load steps, LX ringing and switch-loop stress, U5/D7/L1/capacitor temperatures, current-limit/foldback and AutoEN shutdown/restart.
- [ ] Install and verify the independent off-board heater TCO. Complete NTC characterization-to-firmware calibration and heater safety gates before normal closed-loop heating.

## Engineering items to verify on the first article / before any later revision

The 2026-09-17 reports replace the obsolete 2026-09-16 release checklist. The PCB has already been submitted for fabrication; **do not silently reclassify these follow-up checks as reasons to alter that released design**.

- [ ] Compare actual JLCPCB stackup/order details with the nominal KiCad USB reference geometry (B.Cu/In2.Cu, 0.20 mm width / 0.25 mm gap, 90 Ω design target). USB functionality needs real-board validation; a CAM comparison alone does not prove impedance.
- [ ] Verify U4 antenna-edge and keepout behavior, U2 pad/USB-C solderability, buck current-loop and quiet-feedback behavior, component-edge clearances, high-current heater/fan routing and all mating/assembly constraints during initial hardware tests.
- [ ] The one excluded U4 B.Silkscreen/edge warning remains documented, not magically fixed. If manufacturing or assembly reveals problems, record evidence and open a **new hardware revision** under `AGENTS.md` and `docs/DECISIONS.md`.
- [ ] Review GitHub CI branch triggers before expecting automatic ERC on future feature-branch PCB changes; for any actual future schematic/PCB revision, generate fresh netlist/ERC/DRC and all fabrication exports from the same revision.

- [x] First-board no-load power bring-up through 3V3_MCU rail (24 V in; ~1.8 mA; 3V3_BUCK and TP2 ~3.3 V). Load/ripple/thermal validation still pending.

- [ ] Gate 3 USB fault: Android second-host test confirmed with 5 V VBUS from the phone, but no device was detected. Windows-specific causes are now unlikely; inspect/reflow U2 CP2102 before replacement or design changes.

- [x] Validate ESP32-to-PC UART path: ROM boot log is readable on COM5 at 115200 after SW1 RESET.
- [ ] Validate PC-to-ESP32 UART path and bootloader/programming, including automatic DTR/RTS reset/BOOT behavior.

- [ ] Automatic upload currently fails at `Connecting...` with esptool `Invalid head of packet (0x65)`; test manual BOOT/RESET download mode next to separate UART RX from DTR/RTS auto-program behavior.

- [x] Manual BOOT/RESET reaches ESP32 ROM downloader and esptool reads chip type/revision/crystal/MAC, proving bidirectional UART.
- [ ] Resolve failure at `Uploading stub flasher...` (`Invalid head of packet (0x65)`): first verify bench current limit/3V3 stability, then test no-stub or alternate esptool version if power is clean.

- [x] Resolve stub-stage upload failure: raising the 24 V bench current limit from 50 mA to 100 mA allowed full esptool write/verify to complete successfully in manual download mode.
- [ ] Repeat upload at 100 mA without touching BOOT/RESET to validate DTR/RTS automatic programming entry.

- [ ] Auto-program remains open: at 100 mA, no-button upload gives `No serial data received`; manual BOOT/RESET upload succeeds. Scope TP4/EN_ESP and TP5/IO0 during automatic connect to identify the DTR/RTS/Q2/Q3 failure.

- [ ] Auto-program hardware defect identified: current Q2/Q3 DTR/RTS cross-coupled network swaps the intended EN/IO0 truth-table behavior versus Espressif reference. Agree and implement a first-article bodge, then open a deliberate schematic/PCB revision; manual BOOT/RESET programming remains usable meanwhile.


- [x] Validate Gate 3 automatic programming on the first article: crossing the Q2/Q3 collector destinations (Q2 collector -> EN, Q3 collector -> IO0) restores no-button esptool upload and automatic RTS reset at a 100 mA bench limit.
- [ ] Open Serial Monitor at 115200 and confirm the newly flashed Gate 3 diagnostic sketch runs and prints its expected messages.
- [ ] Apply D020 in the next schematic/PCB revision: correct the Q2/Q3 auto-program output mapping in source, then regenerate netlist/ERC/DRC/production outputs.

- [x] Close Gate 3 MCU/USB/UART bring-up: automatic programming works after the validated Q2/Q3 collector-cross bodge, and the flashed diagnostic sketch runs correctly at 115200 baud.
- [ ] Gate 4A: populate and test J3 plus R14, R15, R16 and R17; verify the unloaded I2C bus first. J4 is the actual 1.54-inch SSD1309 OLED module (not a connector) and should be connected/tested separately after the bus checks, like the external SHT45 load.

- [x] Gate 4A initial population: J3 and R14-R17 installed; J4 OLED and SHT45 still disconnected.
- [ ] Verify unloaded J3 power/SDA/SCL continuity and powered idle levels before attaching I2C modules.

- [x] Verify unloaded J3 power/SDA/SCL continuity and powered idle levels before attaching I2C modules.
- [x] Verify the actual SHT45 board/harness pinout and JST-PH orientation, then connect and run an I2C/SHT45 diagnostic before adding the OLED. Passed 2026-10-07: sensor supply ~3.3 V, device detected at `0x44`, stable valid temperature/RH readings.

- [x] Obtain/prepare the J3 mating harness: JST-PH 2.00 mm, 4-way housing/contacts or equivalent pre-crimped PH lead set. Kit received and 4-way SHT45 harness assembled by 2026-10-07.
- [x] Before plugging the SHT45 into J3, verify the completed PH harness pin-for-pin against the PCB: J3-1=GND/black, J3-2=SDA/green, J3-3=SCL/yellow, J3-4=3V3/red. Powered diagnostic passed 2026-10-07.

- [x] Connect and test the actual J4 1.54-inch SSD1309 OLED on the validated I2C bus. Passed 2026-10-08 at `0x3C`; U8g2 graphical frame/text rendering confirmed with SHT45 simultaneously present at `0x44`. Gate 4A closed.

- [x] Gate 5A PWM characterization completed for the current fan at 25 kHz: 25% was the first tested duty that started from rest (20% did not); 15% was the lowest tested duty that sustained rotation (10% did not). Earlier current points: 50% ~49 mA, 75% ~72 mA, 100% ~89 mA. Final firmware should retain margin above these observed limits and may use a brief full-duty startup boost.


- [x] First-article J7 recovery completed by 2026-10-11: J7 remains installed and the original eSUN NTC leads were reterminated for the JST-GH connection; the real NTC is now connected and electrically validated through the PCB. Revisit the connector choice deliberately in the next hardware revision.


- [x] Gate 5B unloaded switching path passed 2026-10-08: Q4/D4/R20/R21 installed, J5 left unpopulated, safe-OFF gate 0 V, temporary 100 kΩ pull-up gave 24 V at HEATER_SW when OFF, GPIO19 HIGH gave 3.3 V at Q4 gate and ~1 mV at HEATER_SW. Real heater remains blocked pending NTC/J7 validation, firmware safety handling and TCO.

## Application firmware / front-panel UI

- [ ] Flash the longer Slewform boot hold: rootless emblem for 1.2 s, roots grow over 11×120 ms steps, full emblem held until 2.75 s, separate wordmark/chime at ~2.75 s and STANDBY at 4.15 s. Confirm the existing fixes (no vanishing roots or lost opening hold) remain effective.
- [ ] Investigate intermittent OLED horizontal lines / old frame at startup: user saw apparent improvement after `initDisplay()`/preloading but the effect returned without changing those calls. Compare **cold power-on** (power fully removed long enough for rails to decay), **rapid power-cycle**, and **ESP32 RESET while supply remains present**; note whether the artifact is seen strictly **before** the first emblem frame or after. Check power sequencing/reset behavior and actual OLED module wiring before considering an electrical revision. The software cannot guarantee suppressing content visible before it can initialize the SSD1309.

- [ ] Validate real fan application control on the first article: with the actual fan connected, start a DRYING test cycle and confirm ~1 s at 100% followed by steady 40% at 25 kHz; confirm long ON/OFF stop and cycle completion both turn the fan fully OFF. Keep the heater disconnected/locked and use an adequate bench current limit.
- [ ] Validate Preferences/NVS persistence across reset/power cycle: change cycle time/temperature UI values and buzzer settings, exit with M-long or start a cycle, reset, and confirm values return; confirm no active cycle auto-resumes.
- [ ] Validate SETTINGS UI: from STANDBY hold M, test buzzer OFF/LOW/MED/HIGH preview and KEY CLICK ON/OFF, then hold M to save/back.

- [ ] Verify on first article that the cleaned-up layout is restored and M-long exits SETUP. Contextual button hints were rejected because they competed with the 128×64 screen content.

- [ ] Reflash after first live UI corrections and verify: Slewform splash now remains visible 3.5 s, pre-logo horizontal/random lines are eliminated or reduced to an unavoidable power-up transient, all temperature fields show `°C`, and heater/fan pins remain LOW.

- [x] Split cycle, fan and safety responsibilities into `CycleController`, `FanController` and `SafetyController`; heater remains hard-locked OFF. Logging and web-interface modules remain future work.
- [x] Implement and live-test debounced active-low button handling with short press, ~1.5 s long press, UP/DOWN autorepeat and wake-consume behavior. First-article interaction test passed 2026-10-08.
- [ ] Complete the D022 state model. BOOT -> STANDBY -> SETUP -> DRYING-demo -> COMPLETE is implemented in `030cb5c` with no auto-resume; real FAULT entry/reset remains pending the safety controller.
- [ ] Complete SSD1309 pages from `docs/UI_SPEC.md`. Slewform boot splash, standby, setup, drying main, technical/system, complete and generic fault screens are implemented in `030cb5c`; final diagnostics and safety-specific fault content remain.
- [ ] Live-test the expanded buzzer manager: persisted OFF/LOW/MED/HIGH levels, optional key click, warning pattern and repeating fault pattern. Code is implemented; acoustic tuning and real safety-fault triggering remain to be validated.
- [x] Implement D021 diagnostic LED patterns in the application firmware (`030cb5c`).
- [ ] Decide and validate target-temperature range/step, cycle-duration range/step, defaults and persistence only after heater/NTC/safety validation supports safe values.
- [ ] Define final fan AUTO policy using the measured first-article fan thresholds with adequate startup/run margin; decide whether a post-cycle cooldown state is required.
- [ ] Keep material profiles and humidity/trend-based automatic completion deferred until their values/algorithm are deliberately designed and validated.


- [x] Live-test calibrated NTC application firmware on the first article with heater power disconnected: stable serial NTC diagnostics, heater-temperature display on SYSTEM page, and deliberate J7 unplug correctly entering latched open-circuit FAULT. Heater remained hard-locked OFF. Passed 2026-10-11.
- [ ] Define and validate final heater thermal fault thresholds/control limits only after the NTC conversion live test; do not treat the current open/short electrical windows as final thermal safety limits.
- [x] Procure and install the independent off-board TCO in series with HEATER+: first article now uses a one-shot 105 °C / 15 A / 250 V cutoff on J5 pin 2 / HEATER+, with post-install path continuity confirmed by the ~15 ohm complete heater resistance. Short controlled real-heater pulses passed on 2026-10-14. The TCO's destructive 105 °C trip point was not intentionally tested.
- [x] Extend heater test to the empirical NTC range upper point: 40 °C target PASS in 26.000 s and 50 °C target PASS in 42.800 s (50.14 °C peak; final NTC OK), first article 2026-10-15. These results validate diagnostic NTC response and shutdown, not independent case/heater surface temperature accuracy.
- [ ] Before testing above 50 °C, check nearby case/plastic and off-board TCO/J5 wires/crimps for overheating and verify heater/critical enclosure thermal behavior with an independent temperature measurement; the current NTC table is empirical only to 50 °C and extrapolates above it.
- [ ] Define and validate final thermal fault thresholds and closed-loop heater control before enabling heater operation in normal application firmware. Diagnostic temperature cutoff/60 s timeout are test-only, not production safety settings.

- [ ] Revalidate heater NTC resistance/temperature curve: at rest 122.3 kOhm by DMM with J7 disconnected versus 124.4 kOhm ADC-derived when connected, SHT45 and Sanwa ~20 C, but NTC beta extrapolation ~16.35 C; heating-phase Sanwa/NTC offset reaches 6 C and reverses during cooling, compatible with reference-probe thermal lag. Do not fit this transient data as steady-state calibration; establish stable co-located references and recheck thermocouple response after OFF before extending above 50 C. See PROJECT_STATE.md (2026-10-15). Do not repeat already recorded resistor checks.

- [ ] Complete a 50 C NTC steady-state supervised hold (maximum 120 s HOLD, independent Sanwa readings and NTC kOhm, active NTC/time aborts) before fitting any new thermistor conversion. Already recorded: Sanwa 30 C -> 64.62 kOhm (old curve 30.89 C), Sanwa 39 C -> 42.27 kOhm (old curve 40.69 C), room Sanwa/SHT45 ~20 C -> directly measured J7-disconnected NTC 122.3 kOhm (old extrapolation ~16.6 C). 35 C intermediate is optional; avoid redundant measurements.

- [ ] CRITICAL identification before setting final heater range: manufacturer original eBOX is 12 V/3 A with 50/60/70/80 C plate, eBOX Lite is 24 V/2 A with 105/116/118 C plate. Prototype heater is 24 V ~1.59 A (~38 W), suggesting Lite but donor model not independently confirmed. Do NOT treat prior 80 C as a proven maximum or 50 C as final heater limit. The currently installed 105 C one-shot TCO might open during OEM Lite hot-plate settings; review model, coupling and protection philosophy before any higher-temperature testing or revision, no bypass/replacement without explicit decision. See PROJECT_STATE.md 2026-10-15.
- [ ] NTC curve analytical checkpoint: stabilized Sanwa 30 C -> 64.62 kOhm, 39 C -> 42.27 kOhm imply local beta ~4463 K and R25 ~82.7 kOhm (consistent with original 82.5 kOhm), but predict R20 ~106.8 kOhm, while directly measured NTC disconnected from J7 was 122.3 kOhm around Sanwa/SHT45 20 C. Resolve low-temperature inconsistency before committing production NTC curve. Sanwa showed stable plateau well before 120 s; do not dismiss data as insufficient hold. No blanket correction and no unnecessary repetition of already completed checks.

- [x] Identify donor dryer model from primary purchase records: 3DJake confirmed purchased eSUN **eBOX Lite** (item ESUNCN-6922572201521) on 2024-11-25 and shipment on 2024-11-26. **Original eBOX 80 C plate reference is inapplicable.** eSUN eBOX Lite manual references heater plate 105/116/118 C, inside air 40/50/55 C (conditions dependent).
- [ ] Before raising heater test target above previously checked values, review the installed **105 C one-shot series TCO** against eBOX Lite's nominal plate range 105-118 C; verify physical thermocouple/NTC/TCO positions and protective behavior and establish deliberate safe operating range. Never disable/bypass/rerate TCO on the basis of the nominal table alone. Do not confuse heater NTC with SHT45 chamber air.

- [x] Complete third supervised heater NTC plateau reference: nominal 50 C indicated, after 120 s hold Sanwa contact thermocouple read 47 C and J7 ADC-derived NTC 28.72 kOhm (old firmware curve about 50.0 C), on 2026-10-15. Together with 30 C / 64.62 kOhm and 39 C / 42.27 kOhm, provisional three-point local fit beta ~4624 K / R25 ~83.86 kOhm, not yet accepted as firmware calibration; Sanwa absolute accuracy, sensor-placement differences, and unresolved rest 20 C / 122.3 kOhm remain. No need to repeat completed plateau points. See PROJECT_STATE.md.


- [x] Fourth stabilized Sanwa versus heater-NTC comparison: 2026-10-16 at nominal 60 C NTC / 120 s hold, Sanwa 56 C and J7 ADC-derived NTC 19.68 kOhm. Together with 30/64.62, 39/42.27 and 47/28.72 (Sanwa C/kOhm), four-point exploratory beta fit about 4585.8 K, R25 about 83.49 kOhm, max mathematical temperature residual about 0.23 C. This is NOT reference absolute accuracy. Predicts ~108.5 kOhm at 20 C, versus direct room-state NTC 122.3 kOhm. Resolve low-end disagreement and thermocouple/Ntc placement uncertainty; do not repeat four plateau readings or change final firmware/safety limits without validation. See PROJECT_STATE.md.

- [x] Fifth stabilized heater comparison (2026-10-16): nominal NTC diagnostic target 70 C, after 120-second hold Sanwa heater contact **64 C**, J7 NTC **13.73 kOhm**, old 50-C-anchored beta=4100 conversion **70.04 C** (+6.04 C vs Sanwa). Five stabilized points 30/64.62, 39/42.27, 47/28.72, 56/19.68, 64/13.73 (Sanwa C/NTC kOhm); exploratory beta fit beta ~4645.8 K, R25 ~84.20 kOhm, maximum back-fit discrepancy ~0.33 C. Room-state Sanwa ~20 C / NTC 122.3 kOhm remains inconsistent with fitted 20 C prediction ~109.83 kOhm. User reported 105 C series TCO restored before this test; do not assume its trip function was verified. No firmware or accepted safety limit changed. See PROJECT_STATE.md.

- [ ] Redo only the incomplete 80 C nominal NTC plateau. First 80 C test auto-entered COOLDOWN because diagnostic PREHEAT_TIMEOUT_MS was prematurely set to 150 s; user confirmed this as abort reason, **not** an NTC failure or a completed 80 C reading. Revised standalone test uses 300 s bounded preheat, 120 s hold only once target reached, continuous NTC fault/83 C cap, and a test-duration-derived active watchdog; keep old curve unchanged. Note preheat 300 s is a safety time cap, not measured heater thermal performance. Confirm final Sanwa C and NTC kOhm before fitting further.

- [x] Complete repeated nominal 80 C NTC hold (2026-10-16), with 300 s preheat cap rather than unjustified 150 s. After 120 s hold: Sanwa 74 C / NTC 9.81 kOhm (legacy diagnostic maps to 79.97 C, +5.97 C difference). Six stable Sanwa-C/NTC-kOhm points: 30/64.62;39/42.27;47/28.72;56/19.68;64/13.73;74/9.81. Exploratory fit beta 4560.81 K, R25 82.98 kOhm, worst fit residual ~0.74 C; unresolved prior ambient 20 C / 122.3 kOhm inconsistent with model R20 ~107.72 kOhm. Next possible supervised 90 C *old NTC-indicated* 120 s plateau, after inspection of nearby housing, heater and independent series TCO, with bounded firmware and supply disconnect; no accepted final safety range or production conversion yet. Do not repeat previous plateau tests by default.

- [ ] Nominal 90 C NTC / 120 s hold diagnostic **timed out on preheat** (user reported timeout; temporary sketch used 300 s preheat cap), so no seventh Sanwa/R data point exists. Next: obtain last logged NTC C and supply current near shutdown; if current remained at heater-load level, assess gradual thermal equilibrium; if current collapsed despite HEATER=ON, check 105 C one-shot TCO and heater circuit continuity with supply removed. Do not raise diagnostic preheat cap or bypass the independent TCO without evidence.
