# Project state

Checkpoint date: **2026-09-16**  
Active hardware branch: **`pcb/l7987l-layout`**  
Hardware HEAD immediately before this documentation refresh: **`9da46e7953f801073882fd1934802fa8ace1f1c2`** (`updated layout`).

## Executive state

The L7987L + AutoEN redesign is integrated and the PCB is now substantially routed. The latest board has a fresh DRC dated **2026-09-16 20:12:47** with:

- **0 DRC errors**;
- **0 unconnected pads**;
- **14 active warnings**;
- **1 excluded silkscreen/board-edge warning**.

The remaining active warnings are six library-footprint mismatch warnings (J4, U4, J6, J1, J5, J3) and eight BZ1 silkscreen-over-copper warnings.

USB routing is now configured and routed for a **90 Ω differential target** on B.Cu referenced to In2.Cu. The CP2102-GM footprint, solder-mask expansion and exposed-pad paste pattern were also updated, and the new 1 µF REGIN bypass is present.

The PCB is **not manufacturing-ready**. Release blockers/open gates are:

1. U5 exposed-pad thermal-via and paste/stencil implementation is still not finalized.
2. The stored netlist and ERC report predate the latest MCU schematic correction and must be regenerated.
3. The current DRC warnings still need deliberate review/closure or documented acceptance.
4. Production Gerbers/BOM/CPL/drill/netlist outputs must be regenerated from the released revision and reconciled with the live purchasing BOM.

The previously documented D012/C22 buck-bypass conflict is **closed**: the later design review recognized that C1 = 10 µF / 100 V already satisfies ST's ≥1 µF VIN-to-power-GND ceramic requirement, while C3 = 1 µF / 100 V satisfies the VCC-to-IC-GND requirement. The redundant buck C22 was deliberately removed in commit `24a9170a`.

The live **BOM TME** was also rechecked on 2026-09-16: C22 appears only in the 1 µF / 25 V CP2102 REGIN row, while C3 is the sole 1 µF / 100 V entry. The earlier duplicate-reference warning is obsolete.

## Sources inspected for this checkpoint

This checkpoint was verified against the active branch and current repository sources:

- `AGENTS.md`;
- `README.md`;
- `docs/PROJECT_STATE.md`;
- `docs/DECISIONS.md`;
- `docs/TODO.md`;
- `docs/BUCK_L7987L_DESIGN.md`;
- `docs/ASSEMBLY.md`;
- `hardware/docs/PROCUREMENT.md`;
- `hardware/Filament_Dryer_Monitor.kicad_pcb`;
- `hardware/Filament_Dryer_Monitor.kicad_pro`;
- `hardware/Power.kicad_sch`;
- `hardware/MCU.kicad_sch`;
- `hardware/Filament_Dryer_Monitor.net`;
- `hardware/DRC.rpt`;
- `hardware/ERC.rpt`;
- `.github/workflows/kicad-erc.yml`.

The KiCad files remain authoritative if prose documentation disagrees.

## Electrical design state

The accepted L7987L design basis remains:

| Ref | Value / part | Function |
|---|---|---|
| U5 | ST L7987L | 24 V to 3.3 V asynchronous buck |
| U1 | TLV1701AIDBVR | AutoEN fault comparator |
| Q7 | MMBT3904 | EN pull-down |
| L1 | SRN6045-150M, 15 µH | Buck inductor |
| D7 | STPS2L60A | Catch Schottky |
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

`hardware/Filament_Dryer_Monitor.net` was generated at **2026-09-16T18:12:49** and was not regenerated after the final MCU schematic/layout correction committed in `9da46e7`.

It still reports J2 A6/B6 on `USB_CONN_D-` and A7/B7 on `USB_CONN_D+`, while the current PCB has the corrected physical mapping above. Therefore the stored netlist must not be used as proof of current USB schematic connectivity until regenerated.

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

### U5 exposed pad remains open

U5 pad 17 is a **3.2 × 3.2 mm** GND/SGND exposed pad. Inspection of the current board shows no thermal/GND via located inside the exposed-pad area. The current footprint also applies B.Paste over the EP as a single full-size pad.

The final EP strategy must therefore still define:

- via count and placement;
- via finished/drill size;
- whether vias are tented/filled/plugged for the intended assembly process;
- paste-window pattern/coverage;
- solder-wicking risk and JLCPCB manufacturability.

Nearby GND vias do not by themselves close the exposed-pad thermal implementation.

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

Stored `hardware/ERC.rpt` is dated **2026-09-16 18:13:37** and contains:

- 1 `power_pin_not_driven` error on `#PWR02` GND;
- 0 warnings.

That report predates the latest MCU schematic commit and must be regenerated.

The existing ERC workflow still recognizes the external-input GND error as the reviewed modeling waiver and allows warning counts only to decrease. However, its current `push` trigger is limited to **`redesign/buck-sourcing`**, so it does **not** automatically gate pushes to `pcb/l7987l-layout`. Treat the current branch as requiring an explicit ERC run until the workflow trigger is updated or the branch is merged into a covered branch.

## DRC state

Current `hardware/DRC.rpt` is dated **2026-09-16 20:12:47** and matches the latest PCB checkpoint.

It reports no errors and no unconnected pads. Remaining warnings:

- footprint-library mismatch: J4, U4, J6, J1, J5, J3;
- eight BZ1 silkscreen-over-copper warnings;
- one ESP32 silkscreen/board-edge warning already excluded.

The fresh DRC is meaningful evidence for connectivity/clearance at this checkpoint, but it is not a manufacturing release by itself.

## Procurement/BOM reconciliation

The live Google Sheet `Filament Dryer Monitor — BOM finale Mouser`, tab `BOM TME`, was rechecked on 2026-09-16.

Relevant rows are now consistent with KiCad:

- **C1** -> 10 µF / 100 V X7S, Murata `GRM32EC72A106KE05L`;
- **C3** -> 1 µF / 100 V X7S, Murata `GRJ21BC72A105KE11L`;
- **C15,C17,C21,C22** -> 1 µF / 25 V X7R 0603, Murata `GCM188R71E105KA64J`.

C22 no longer appears in the 100 V row. The previous C22 duplicate-reference issue is closed. Final production exports still need the normal release-time reconciliation against this live sheet.

## Immediate handoff / next gates

1. Finalize U5 exposed-pad thermal vias and paste/stencil strategy.
2. Regenerate netlist and ERC from the latest schematic; verify USB connectivity against the current PCB.
3. Review the 14 active DRC warnings and either fix them or record justified exclusions.
4. Perform final engineering review of buck current loops/feedback/AutoEN routing, USB reference continuity, ESP32 antenna keepout, heater/fan/high-current paths and mechanical clearances.
5. Regenerate final BOM/CPL/Gerbers/drills/netlist from the same released revision and reconcile them with the live BOM TME.
6. Perform fabrication/assembly review.
7. Follow `docs/ASSEMBLY.md` for first assembly and R5 sequencing.
8. Validate regulation, transients, switching stress, temperatures, current limit and AutoEN recovery on the first board.
