# TODO

Current phase: **first-article hand assembly and staged bring-up** (2026-10-04). Active hardware branch: `main`. Fabrication-source checkpoint: `74a3000127371ac6c3b3b7197f9036dec5b58eef` (2026-09-17, `Production files`). This file tracks remaining work; read `AGENTS.md` and `docs/PROJECT_STATE.md` before changes. Do not silently alter the already submitted fabrication revision.

## Completed / recorded

- [x] Gate 5A real fan load test passed on 2026-10-08: actual 24 V fan started normally; bench input current peaked at ~102 mA and settled at ~89 mA. Static real-load switching path is validated; PWM characterization remains pending.

- [x] Gate 4D analog front end passed on 2026-10-05 with R28=47 kΩ and C19=100 nF: open NTC node ~3.3 V; temporary 100 kΩ to GND gave 2.253 V by DMM; GPIO34 ADC was stable at RAW ~2643-2647 / 2300-2302 mV. J7 and the real dryer NTC remain pending, as do final NTC calibration and fault limits.

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

- [ ] Connect and test the actual J4 1.54-inch SSD1309 OLED on the validated I2C bus.

- [ ] Gate 5A: finish PWM characterization by determining minimum reliable cold-start duty and minimum stable-running duty. Initial 25 kHz tests passed at 50% (~49 mA), 75% (~72 mA), and 100% (~89 mA); the fan whistles similarly even at 100%, indicating fan-inherent acoustic noise rather than a PWM-frequency issue.
