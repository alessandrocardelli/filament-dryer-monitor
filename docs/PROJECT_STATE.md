# Project state

Checkpoint date: **2026-09-07**  
Active hardware branch: **`pcb/l7987l-layout`**  
Last hardware/layout commit before this documentation checkpoint: **`6e74f76d8d6877fab63283c1aa520bdbfc149921`** (`Layout update`). Documentation-only commits after that SHA may advance the branch HEAD without changing the KiCad implementation.

## Executive state

The L7987L redesign is no longer only a schematic change. The PCB has been synchronized with the signed-off schematic and the legacy AP66200 stage is absent from the current board file. The project is now in the **PCB layout/routing phase for the L7987L + AutoEN block**.

The schematic electrical review remains **PASS**. The PCB is **not manufacturing-ready yet**: buck routing/grounding/thermal details, USB differential-pair geometry, final PCB review and a fresh DRC are still open. Existing production exports must be considered stale until regenerated from the final routed revision.

## Authoritative electrical state

Active schematic implementation: `hardware/Power.kicad_sch`.

Current exported netlist: `hardware/Filament_Dryer_Monitor.net`, generated with Eeschema 10.0.4 on 2026-09-04.

The four schematic closure gates completed on 2026-09-04 remain closed:

1. ERC reviewed and CI-enforced against the approved waiver/warning set.
2. L7987L current-limit / L1 engineering envelope reviewed.
3. Effective-capacitance / compensation sensitivity reviewed for the sourced capacitors.
4. AutoEN comparator and EN corner levels reviewed.

Current ERC report: one reviewed `power_pin_not_driven` modeling error on the externally-fed GND net plus 11 reviewed warnings. This is the state enforced by CI.

## L7987L implementation

Key final schematic values:

| Ref | Value / part | Function |
|---|---|---|
| U5 | ST L7987L | 24 V to 3.3 V asynchronous buck |
| U1 | TLV1701AIDBVR | AutoEN fault comparator |
| Q7 | MMBT3904 | EN pull-down |
| L1 | SRN6045-150M, 15 µH | Buck inductor |
| D7 | STPS2L60A | Catch Schottky |
| C1 | 10 µF, 100 V X7S, GRM32EC72A106KE05L | Main local input ceramic |
| C3 | 1 µF, 100 V | Local VIN/VCC bypass |
| C6 | 100 nF | BOOT-LX bootstrap |
| C10 | 47 µF, 10 V X7R | Main pre-bead output capacitor |
| C21 | 1 µF, 25 V | VBIAS/output bypass |
| R4 | 47 kΩ | FSW, about 516 kHz nominal |
| R29 | 47.5 kΩ | ILIM, about 1.705 A nominal |
| R33/C9/C8 | 16 kΩ / 18 nF / 39 pF | Type-III compensation branch |
| R35/C20 | 1.13 kΩ / 560 pF | Type-III compensation branch |
| R36/R34 | 49.9 kΩ / 16 kΩ | Feedback divider, about 3.295 V nominal |

Output architecture remains `3V3_BUCK -> FB1 -> 3V3_MCU`. `PGOOD` and `SYNCH` are intentionally NC. The existing upstream C5 = 100 µF / 50 V bulk reservoir remains.

Recorded simulation/design-review results remain approximately 59.1 kHz crossover, 64.9° phase margin and 19.6 dB gain margin. These are simulation results, not hardware measurements.

## Current PCB checkpoint

Current board file: `hardware/Filament_Dryer_Monitor.kicad_pcb`.

Verified at the 2026-09-07 checkpoint:

- old `AP66200` and `/Power/VCC_AP66200` are absent from the current PCB;
- U5 is the L7987L on **B.Cu**;
- buck and AutoEN components have been placed and their placement has undergone an initial review against ST/TI guidance;
- local B.Cu copper zones are already being developed for `24V_PROT`, `/Power/3V3_BUCK`, GND and the LX node `Net-(D7-K)`;
- U5 exposed pad is GND/`SGND_17`; thermal/GND via and final paste strategy are not yet closed;
- critical buck routing and final ground-via geometry are still in progress.

Do not use old coordinate/rotation notes from chats as authoritative: placement has changed during the layout pass. Inspect the current board.

## Stackup and ground planes

Current KiCad stackup:

| Layer | Copper / spacing | Current role |
|---|---|---|
| F.Cu | 35 µm | front components/signals |
| dielectric 1 | 0.10 mm FR4, Er 4.5 | F.Cu to In1 |
| In1.Cu | 35 µm | **solid GND plane** |
| core | 1.24 mm FR4, Er 4.5 | inner separation |
| In2.Cu | 35 µm | **solid GND plane** |
| dielectric 3 | 0.10 mm FR4, Er 4.5 | In2 to B.Cu |
| B.Cu | 35 µm | back components/signals + buck local copper |

This is a deliberate project decision: **both inner layers are full GND reference planes. There is no internal 3V3 or 24 V plane.**

For the B.Cu buck, In2 is the nearest reference plane at only 0.10 mm.

## Buck ground implementation

ST distinguishes power-ground and signal-ground current paths. In this project they remain the **same electrical `GND` net** and the inner planes are not split.

The practical distinction is local geometry and via entry points:

- high-current/pulsed return group: C1 negative, D7 anode, C10 negative;
- quiet/signal return group: U5 pin 16 + exposed pad, C3 negative, C21 negative and the sensitive control network.

Each group enters the same continuous internal GND planes with short local connections/vias. Layout must avoid forcing the pulsed buck return current through the quiet local return copper around the feedback/compensation circuitry.

## Placement principles already established

Use the ST L7987L datasheet and STEVAL-ISA198V1 Gerbers as the primary buck layout reference.

Important current interpretation:

- VIN/VCC are on the **left side of U5 in the current physical board view**;
- C3 is the local 1 µF bypass and gets highest proximity priority to VIN/VCC;
- C1 is the 10 µF main input ceramic and belongs on the same VIN side, slightly more external than C3 if necessary;
- C21 stays close to U5 pin 1 VBIAS and does not need to be moved merely to create a surface GND corridor;
- C6 is BOOT-to-LX and is not a GND component;
- FB/COMP components remain in the quiet area and must be kept away from high-current/LX copper.

For any bottom-side pad-orientation check, use KiCad absolute pad positions or the actual board view. Do not manually interpret raw local footprint coordinates.

## Netclasses

Current accepted power/default classes:

| Class | Clearance | Track width | Via diameter / drill |
|---|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.60 / 0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | 0.60 / 0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | 0.80 / 0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | 0.80 / 0.40 mm |
| USB | 0.20 mm | 0.25 mm | 0.60 / 0.30 mm |

Assignments include `3V3_MCU` and `/Power/3V3_BUCK` -> Power_3V3; `24V_PROT`, `/Power/JACK_24V_RAW`, `/Power/FUSE_OUT` -> Power_24V; `HEATER_SW` -> Power_Heat; `USB_DP`/`USB_DM` -> USB.

**Open item:** USB-class differential-pair width/gap fields are not yet finalized. The Default class currently contains 0.20 mm DP width / 0.25 mm DP gap, but these values must not be assumed valid for USB. Calculate/verify the USB pair against the actual 0.10 mm B.Cu-to-In2 GND stackup before routing freeze.

## DRC and manufacturing state

`hardware/DRC.rpt` is historical, dated 2026-08-06, from before the current L7987L PCB integration. It must not be used as evidence that the present PCB passes DRC.

A **fresh DRC is required** after current routing/layout work is complete.

Production Gerbers, CPL/position data, BOM exports and production netlists are not released for fabrication until:

- layout review is complete;
- current DRC is reviewed and closed;
- manufacturing outputs are regenerated from the same final revision;
- generated production data is reconciled against the final purchasing BOM.

## Immediate handoff

The next chat should start by reading `AGENTS.md`, this file, `docs/DECISIONS.md` and `docs/TODO.md`, then inspect the current PCB at the latest branch HEAD.

The immediate design task is **continue the L7987L PCB layout from the current B.Cu placement/zones**, beginning with U5 exposed-pad/GND via strategy and the local PGND/SGND current-return geometry, then finish critical buck routing.