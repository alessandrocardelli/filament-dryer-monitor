# TODO

Current phase: **released bare-PCB prototype procurement, incoming inspection and hand assembly** (2026-09-21). Active hardware branch: `pcb/l7987l-layout`. Fabrication-source checkpoint: `74a3000127371ac6c3b3b7197f9036dec5b58eef` (2026-09-17, `Production files`). This file tracks remaining work; read `AGENTS.md` and `docs/PROJECT_STATE.md` before changes. Do not silently alter the already submitted fabrication revision.

## Completed / recorded

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
