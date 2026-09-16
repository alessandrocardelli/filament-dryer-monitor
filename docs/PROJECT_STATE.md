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
2. Accepted decision D012 (second local 1 µF / 100 V L7987L VIN/VCC bypass) is **not present in the current Power schematic**. Current C22 is instead the CP2102 REGIN 1 µF / 25 V capacitor. This is an implementation/documentation conflict, not a silently superseded decision.
3. The stored netlist and ERC report predate the latest MCU schematic correction and must be regenerated.
4. The current DRC warnings still need deliberate review/closure or documented acceptance.
5. Production Gerbers/BOM/CPL/drill/netlist outputs must be regenerated from the released revision and reconciled with the live purchasing BOM.
6. The live `BOM TME` currently contains a stale duplicate reference: C22 appears in both the 1 µF / 25 V and 1 µF / 100 V rows.

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

### D012 implementation conflict

D012 accepted a second local 1 µF / 100 V bypass so VIN and VCC each have a physically local 1 µF ceramic. The current `Power.kicad_sch` contains only **C3** with MPN `GRJ21BC72A105KE11L` and function `C_vcc_buck1`; there is no second 1 µF / 100 V part and no `C_vin_byp1` in the current Power sheet.

This must be treated as an implementation regression/open decision conflict. Do not remove or rewrite D012 merely to match the current source. Restore the accepted function under a non-conflicting reference, or explicitly reopen D012 before fabrication.

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

The live Google Sheet `Filament Dryer Monitor — BOM finale Mouser`, tab `BOM TME`, currently lists:

- C15/C17/C21/C22 as 1 µF / 25 V / X7R / 0603 `GCM188R71E105KA64J`;
- a stale second row that still includes C22 with C3 as 1 µF / 100 V / 0805 `GRJ21BC72A105KE11L`.

The source implementation and purchasing sheet therefore need reconciliation before ordering/final BOM export.

## Immediate handoff / next gates

1. Resolve the D012 missing second 1 µF / 100 V VIN/VCC bypass without reusing C22, unless D012 is explicitly reopened.
2. Finalize U5 exposed-pad thermal vias and paste/stencil strategy.
3. Regenerate netlist and ERC from the latest schematic; verify USB connectivity against the current PCB.
4. Review the 14 active DRC warnings and either fix them or record justified exclusions.
5. Perform final engineering review of buck current loops/feedback/AutoEN routing, USB reference continuity, ESP32 antenna keepout, heater/fan/high-current paths and mechanical clearances.
6. Reconcile `BOM TME`, especially C22.
7. Regenerate final BOM/CPL/Gerbers/drills/netlist from the same released revision.
8. Perform fabrication/assembly review.
9. Follow `docs/ASSEMBLY.md` for first assembly and R5 sequencing.
10. Validate regulation, transients, switching stress, temperatures, current limit and AutoEN recovery on the first board.
