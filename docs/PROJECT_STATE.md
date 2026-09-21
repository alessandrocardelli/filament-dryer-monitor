# Project state

Checkpoint date: **2026-09-21**  
Active hardware branch: **`pcb/l7987l-layout`**  
Fabrication-source checkpoint (before documentation updates): **`74a3000127371ac6c3b3b7197f9036dec5b58eef`** (`Production files`, 2026-09-17). Documentation-only commits after this checkpoint do not change the released hardware.

## Executive state

The L7987L + AutoEN revision was released for a **bare-board, manual-assembly prototype**. The stored current KiCad reports date from 2026-09-17: ERC **0 errors / 0 warnings**; DRC **0 active errors / 0 unconnected pads / 0 footprint errors**, with **one excluded** U4 B.Silkscreen-to-board-edge warning. The stored netlist was regenerated on 2026-09-17 and has the corrected USB-C D+/D− mapping. The released production BOM and position file are `hardware/production/Filament_Dryer_Monitor_bom.csv` and `hardware/production/Filament_Dryer_Monitor_positions.csv`; the unsuffixed `hardware/production/bom.csv` / `positions.csv` are legacy AP66200-era exports and **must not** be used for the current design.

Gerbers/drills were uploaded to JLCPCB for bare-PCB production. Its production/CAM package was received and checked against the uploaded Gerbers in the order discussion. The latest physical fab, production-approval and shipment status are **not recorded in GitHub** and should not be inferred from the existence of that package. No PCBA/stencil service is planned; all components are to be hand-assembled.

**Procurement (2026-09-21):** the live `BOM TME` reconciles to the current production BOM/netlist with 94 PCB-mounted component references for one assembled board: 61 references in 43 TME purchase rows and 33 references on 14 rows marked `In casa`. TME order was placed 2026-09-20; shipment and receipt are unconfirmed. **C11, Samsung CL21A226MAYNNNE (22 µF / 25 V X5R / 0805), is an open supply issue.** The order confirmation listed it as available; TME later reported its warehouse unit could not be located and is searching for it. TME offered a cancellation/refund or reordering route, but no cancellation, replacement or revised PCB MPN has been authorized. TME's later "week 48/2026" delivery notification does not establish that the rest of the order is delayed; dispatch of other in-stock items remains **unconfirmed**.

A candidate alternative mentioned by TME is TDK `C2012X5R1C226M125AC` (TME catalog code `C2012X5R1C226MAC`), 22 µF / 16 V X5R / 0805. It is **not ordered, not installed and not a committed BOM change**. The preferred next step is TME's definitive warehouse and shipping reply, not a second paid-shipping order. Keep the current C11 in schematic and BOM until an actual supply decision is reached.

**Outside the PCB purchasing BOM:** the OLED fitted at J4, off-board SHT45 sensor, independent heater thermal cutoff, mating connector housings/terminals/cables, and mounting hardware need a separate inventory check; their physical availability is not established by the 94-reference PCB BOM. J5 and 13 resistor rows are marked `In casa` in the sheet, but their actual packages/values/quantities still need confirming at assembly. Follow `docs/ASSEMBLY.md`: R5 is deliberately unpopulated at first 3.3 V power-up and fitted after the initial rail check.

The historical D012/C22 buck-bypass conflict remains closed (D018): C1 10 µF / 100 V covers VIN, C3 1 µF / 100 V covers VCC, and C22 now belongs exclusively to the CP2102 REGIN bypass. No circuit decision is changed in this documentation checkpoint.

## Sources inspected for this checkpoint

- Active branch `pcb/l7987l-layout`, fabrication-source commit `74a3000`; `AGENTS.md`, `README.md`, `docs/PROJECT_STATE.md`, `docs/DECISIONS.md`, `docs/TODO.md`, `docs/ASSEMBLY.md`, `docs/BUCK_L7987L_DESIGN.md`, `hardware/docs/PROCUREMENT.md`.
- Actual design/production source: `hardware/Power.kicad_sch`, `hardware/MCU.kicad_sch`, `hardware/Filament_Dryer_Monitor.net` (2026-09-17), `hardware/Filament_Dryer_Monitor.kicad_pro`, the `hardware/Filament_Dryer_Monitor.kicad_pcb` identity and released production files, `hardware/ERC.rpt` and `hardware/DRC.rpt` (2026-09-17). The whole PCB layout was not re-reviewed in this documentation-only checkpoint; the stored DRC and prior CAM review are the cited evidence.
- Live Google Sheet `Filament Dryer Monitor — BOM finale Mouser`, tabs `BOM TME` and `TME_IMPORT`, inspected 2026-09-21; original TME order confirmation dated 2026-09-20 and supplier correspondence of 2026-09-21.

The actual KiCad source files override descriptions, and supplier order/ship status must be confirmed directly rather than inferred from purchase-list availability.

## Electrical design state

The accepted L7987L design basis remains:

| Ref | Value / part | Function |
|---|---|---|
| U5 | ST L7987L | 24 V to 3.3 V asynchronous buck |
| U1 | TLV1701AIDBVR | AutoEN fault comparator |
| Q7 | MMBT3904 | EN pull-down |
| L1 | SRN6045-150M, 15 µH | Buck inductor |
| D7 | PMEG6030EP,115 | Catch Schottky, Nexperia 60 V / 3 A, CFP5/SOD-128 |
| C1 | 10 µF / 100 V X7S | Main local input ceramic |
| C3 | 1 µF / 100 V X7S | Current local VIN/VCC bypass present in source |
| C10 | 47 µF / 10 V X7R | Main pre-bead output capacitor |
| C21 | 1 µF / 25 V | VBIAS bypass |
| R4 | 47 kΩ | FSW, about 516 kHz nominal |
| R29 | 47.5 kΩ | ILIM, about 1.705 A nominal |
| R33/C9/C8 | 16 kΩ / 18 nF / 39 pF | Type-III compensation branch |
| R35/C20 | 1.13 kΩ / 560 pF | Type-III compensation branch |
| R36/R34 | 49.9 kΩ / 16 kΩ | Feedback divider, about 3.295 V nominal |

Output architecture remains `3V3_BUCK -> FB1 -> 3V3_MCU`. `PGOOD` and `SYNCH` remain intentionally NC.

The 2026-09-04 design calculations/review remain accepted: nominal ILIM about 1.705 A, engineering selection envelope about 1.42–2.15 A, and recorded compensation simulation about 59.1 kHz crossover / 64.9° phase margin / 19.6 dB gain margin.

### VIN/VCC local ceramic requirement — resolved

The original D012 interpretation treated ST's VIN and VCC bypass guidance as requiring two additional dedicated 1 µF / 100 V parts. The later review corrected that interpretation.

ST requires:

- a ceramic capacitor of **1 µF or higher** across VIN and power GND, close to U5;
- a ceramic capacitor of **1 µF or higher** across VCC and IC/signal GND, close to U5.

Current implementation:

- **C1 = 10 µF / 100 V X7S** is the local VIN ceramic and therefore already exceeds the VIN-side minimum;
- **C3 = 1 µF / 100 V X7S** is the local VCC bypass.

The extra 1 µF / 100 V buck C22 added in commit `6c6414b3` was redundant and was deliberately removed in commit `24a9170a`. Decision D012 is retained only as historical context and is superseded by D018 in `docs/DECISIONS.md`.

Current C22 belongs to the MCU sheet and is the CP2102 REGIN 1 µF / 25 V capacitor.

## MCU / USB state

### CP2102-GM

U2 is the classic CP2102-GM QFN28 implementation.

Current custom footprint state:

- perimeter pads: **0.95 × 0.28 mm**;
- exposed pad: **3.25 × 3.25 mm**;
- footprint solder-mask expansion: **+0.06 mm**;
- exposed-pad paste: **3 × 3 apertures, 0.9 × 0.9 mm**;
- U2 pad 3 and exposed pad 29 are GND.

The new REGIN decoupling capacitor is **C22 = 1 µF / 25 V / X7R / 0603, Murata `GCM188R71E105KA64J`**, connected between `3V3_MCU` and GND.

### USB differential routing

Accepted/configured USB geometry:

- signal layer: **B.Cu**;
- reference: **In2.Cu**;
- B.Cu-to-In2 dielectric: **0.10 mm FR4, Er 4.5**;
- target: **90 Ω differential**;
- track width: **0.20 mm**;
- differential gap: **0.25 mm**;
- tuning profile: **`USB_90R`**.

Current PCB mapping:

- U2 pin 4 = `USB_D+`;
- U2 pin 5 = `USB_D-`;
- U3 pin 1 ↔ pin 6 = D+ channel;
- U3 pin 3 ↔ pin 4 = D− channel;
- J2 A6/B6 = `USB_CONN_D+`;
- J2 A7/B7 = `USB_CONN_D-`;
- J2 GND pins and shield are GND.

U2 -> U3 D+/D− stays on B.Cu with **no vias**. On the short Type-C breakout, D+ uses **two signal vias** for the duplicated-pad crossover; D− remains on B.Cu. A nearby GND stitching via supports the reference-layer transition.

### Stored netlist is stale

`hardware/Filament_Dryer_Monitor.net` was regenerated on **2026-09-17 21:13:01** from the current schematic, after the earlier USB correction. The actual netlist contains J2 A6/B6 on `/MCU/USB_CONN_D+` and J2 A7/B7 on `/MCU/USB_CONN_D-`, consistent with the current schematic/board mapping above. The older warning about the 2026-09-16 reversed USB netlist is historical and **resolved**. USB enumeration and signal integrity still require first-board testing.

## PCB state

Current board file: `hardware/Filament_Dryer_Monitor.kicad_pcb`.

Current B.Cu/local-zone implementation includes:

- `24V_PROT` zones;
- local `/Power/3V3_BUCK` zone;
- local `3V3_MCU` zone;
- `/IO_Power/HEATER_SW` zone;
- controlled LX / `Net-(D7-K)` zone;
- local GND zones;
- full-board GND planes on In1.Cu and In2.Cu.

All nets are presently connected according to the fresh DRC. That closes the **connectivity/ratsnest** phase, not the engineering review/release phase.

### Power-distribution policy

Decision D017 records the power-copper policy for the current layout:

- `/Power/3V3_BUCK` stays a **compact local B.Cu copper area** around the buck output/C10/L1/FB1 path;
- `3V3_MCU` may remain **primarily 0.50 mm Power_3V3 traces after FB1**. The local zone currently present in the PCB is optional layout copper, not a requirement for a broad 3.3 V plane;
- `24V_PROT` may use wide traces and local pours where current/voltage-drop needs justify them;
- `LX` and other switched nodes remain geometrically compact;
- both inner layers remain uninterrupted GND.

Capacitance from an external DC power area to the nearby GND plane is not considered a reason to avoid the area. The relevant parasitic/EMI concern is excessive copper on fast switched nodes, plus any optional pour crowding the USB pair or ESP32 antenna keepout.

### U5 exposed pad — resolved for planned assembly

U5 is centered at approximately `(174.649, 85.116)` on B.Cu. Pad 17 is the **3.2 × 3.2 mm** GND/SGND exposed pad.

The accepted implementation is deliberately **not via-in-pad**. Six GND vias, each **0.60 mm diameter / 0.30 mm drill**, are placed immediately outside the pad in two rows of three:

- one row at approximately y = 82.9 mm;
- one row at approximately y = 87.35 mm;
- x positions around 173.75 / 174.65 / 175.55 mm.

This geometry was implemented in commit `c9dc4129` (`updated vias on U5 and D7`) and remains present in the current PCB. It gives the EP a short thermal/GND path to the internal GND planes without open vias directly under the solderable pad.

For the planned prototype process, U5 is hand assembled with a very light pre-tin on the exposed pad, flux and hot air. The current full-size B.Paste definition therefore does not control solder deposition and is **not a fabrication blocker** for this flow. If the process later changes to stencil/reflow or external PCBA, the paste aperture and via-treatment strategy must be reopened for that process.

First-board measurement of U5 temperature remains required, but that is prototype validation rather than unfinished PCB implementation. Decision D019 records this closure.

## Stackup / planes

| Layer | Copper / spacing | Role |
|---|---|---|
| F.Cu | 35 µm | front components/signals |
| dielectric 1 | 0.10 mm FR4, Er 4.5 | F.Cu to In1 |
| In1.Cu | 35 µm | **solid GND plane** |
| core | 1.24 mm FR4, Er 4.5 | inner separation |
| In2.Cu | 35 µm | **solid GND plane** |
| dielectric 3 | 0.10 mm FR4, Er 4.5 | In2 to B.Cu |
| B.Cu | 35 µm | back components/signals + local power copper |

Closed decision: **both internal layers remain continuous GND planes**. Power distribution stays on external layers.

## Netclasses

| Class | Clearance | Track | DP width / gap | Via dia/drill |
|---|---:|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.20 / 0.25 mm default | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | — | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | — | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | — | 0.80/0.40 mm |
| USB | 0.20 mm | **0.20 mm** | **0.20 / 0.25 mm** | 0.60/0.30 mm |

USB data nets are assigned to the USB class and use tuning profile `USB_90R`.

## ERC / CI state

Current stored `hardware/ERC.rpt` is dated **2026-09-17 22:18:03** and reports **0 errors, 0 warnings** (with the report's listed ignored check classes). No new ERC is claimed for documentation-only commits.

The branch-specific CI workflow trigger should be checked before assuming automatic ERC will run on documentation or future hardware changes; the successful stored report is the last confirmed check.

## DRC state

Current stored `hardware/DRC.rpt` is dated **2026-09-17 22:18:19**. It reports **0 active errors, 0 active warnings, 0 unconnected pads and 0 footprint errors**. One `silk_edge_clearance` warning for U4 B.Silkscreen touching Edge.Cuts is **explicitly excluded**, not an active defect. Check the released fabrication layers/board edge in the physical prototype; no DRC was rerun for documentation-only commits. The earlier 14-active-warning report from 2026-09-16 is obsolete.

The released bare PCB still requires incoming inspection and all first-board electrical, thermal and fault-recovery tests.

## Procurement/BOM reconciliation

The live Google Sheet `Filament Dryer Monitor — BOM finale Mouser`, tab `BOM TME`, was read on **2026-09-21**. Its 94 PCB-mounted references match the current released `hardware/production/Filament_Dryer_Monitor_bom.csv` and schematic netlist; no PCB-mounted ref or MPN discrepancy was found in the previous full 2026-09-19 audit. The sheet currently lists **61 PCB items across 43 supplier rows**, with **33 refs marked in house**; one PCB is selected for assembly. `TME_IMPORT` is the dynamically generated 43-row purchasing/export tab, **not an order-status ledger**.

The TME purchase was placed 2026-09-20. Its original confirmation lists C11 `CL21A226MAYNNNE` as `Disponibile a magazzino` (page 1, position 3), but TME later acknowledged the stocked unit is missing and is searching for it. Its week 48/2026 ETA is a supplier notice for **C11 alone**. Do not infer the delivery date of the remaining order from that notice. The supplier has **not confirmed** release of all other lines or resolution of C11. No cancellation/replacement has been authorized.

C11 is **required in the assembled design**: it is a 22 µF MLCC from `3V3_MCU` to GND. Current authorized MPN: Samsung `CL21A226MAYNNNE`, 25 V X5R 0805. The 16 V TDK candidate `C2012X5R1C226M125AC` / TME `C2012X5R1C226MAC` is a *candidate only*; verify stock, dimensions, DC-bias/usable capacitance and order resolution before installing or changing the purchasing BOM. There is no reason to change the PCB footprint based on the nominally identical 0805 size. Do **not** omit C11 for final assembly.

Do not treat the old unsuffixed `hardware/production/bom.csv` (legacy AP66200/CP2102N-era) as this design's current BOM. The suffixed export's `LCSC Part #` column is a mixed historical/supplier field; the **live Google Sheet's `Codice TME`** is authoritative for TME purchases. Reconcile actual deliveries (including supplier-imposed minimum quantities) against the TME order confirmation, not only the nominal per-board quantities.

Off-board OLED/J4, SHT45/J3, independent heater cutoff, mating housings/crimps/cable assemblies and mounting parts are **not included** in the 94-component PCB order. Treat them as inventory/assembly checks, not as newly selected PCB parts. See `hardware/docs/PROCUREMENT.md` for follow-up.

## Immediate handoff / next gates

1. Wait for TME's definitive result of the C11 warehouse search and a clear statement about shipping all other available order lines; do not cancel/reorder or silently replace the BOM until agreed.
2. Reconcile received TME packages against `BOM TME`, the confirmed order quantities and all 33 in-house refs. In particular, confirm the actual capacitor supplied for C11 before soldering.
3. Verify physical availability, pinout and fit of J4 OLED, external SHT45, thermal cutoff, mating connectors/cables and enclosure fasteners. The procurement sheet does not establish these are in house.
4. Track JLCPCB bare-board production/shipment outside GitHub; inspect the actual PCBs for mask, drills, component fit, exposed pads and assembly clearances on arrival.
5. Hand-assemble and bring up the first PCB following `docs/ASSEMBLY.md` (**R5 initially not populated**); verify 3.3 V, USB, sensor/display, buck temperature and fault handling before heater operation.
6. Keep experimental PCB changes and future-revision review separate from the already submitted fabrication revision; do not retroactively rewrite the released Gerber/netlist/BOM checkpoint without new evidence and a deliberate revision.
