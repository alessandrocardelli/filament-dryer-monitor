# Project state

Checkpoint date: **2026-09-08**  
Active hardware branch: **`pcb/l7987l-layout`**  
Branch HEAD immediately before this handoff update: **`47232957429eb04c03334b39ef418ae120cec05e`**.  
Latest PCB-only commit at this checkpoint: **`e8741347ef5570948a1115719ae65fbc0de02ce6`** (`Update Filament_Dryer_Monitor.kicad_pcb`). Documentation commits after that SHA may advance branch HEAD without changing the KiCad implementation.

## Executive state

The L7987L redesign is integrated in both schematic and PCB. The legacy AP66200 stage is absent from the current board. Work is now in the **L7987L + AutoEN PCB layout/routing phase**.

The schematic electrical review remains **PASS**. The PCB is **not manufacturing-ready**: U5 exposed-pad via/paste implementation, local GND-via/current-return geometry, critical buck routing, USB differential geometry, final PCB review and a fresh DRC are still open.

Production Gerbers/BOM/CPL/netlist exports must be treated as stale until regenerated from the final routed revision.

## Sources inspected for this checkpoint

The 2026-09-08 handoff was based on the active branch and the current repository sources, including:

- `AGENTS.md`;
- `README.md`;
- `docs/PROJECT_STATE.md`;
- `docs/DECISIONS.md`;
- `docs/TODO.md`;
- `hardware/Filament_Dryer_Monitor.kicad_pcb`;
- `hardware/Filament_Dryer_Monitor.kicad_pro`;
- `hardware/Power.kicad_sch`;
- `hardware/Filament_Dryer_Monitor.net`.

The KiCad files remain authoritative if prose documentation disagrees.

## Authoritative electrical state

Active buck schematic implementation: `hardware/Power.kicad_sch`.

Current exported netlist: `hardware/Filament_Dryer_Monitor.net`, generated with Eeschema 10.0.4 on 2026-09-04.

The four schematic closure gates completed on 2026-09-04 remain closed:

1. ERC reviewed and CI-enforced against the approved waiver/warning set.
2. L7987L current-limit / L1 engineering envelope reviewed.
3. Effective-capacitance / compensation sensitivity reviewed for the sourced capacitors.
4. AutoEN comparator and EN corner levels reviewed.

Current reviewed ERC state remains one `power_pin_not_driven` modeling error on externally-fed GND plus eleven reviewed warnings, enforced by CI.

## L7987L implementation

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
| C21 | 1 µF, 25 V | VBIAS bypass |
| R4 | 47 kΩ | FSW, about 516 kHz nominal |
| R29 | 47.5 kΩ | ILIM, about 1.705 A nominal |
| R33/C9/C8 | 16 kΩ / 18 nF / 39 pF | Type-III compensation branch |
| R35/C20 | 1.13 kΩ / 560 pF | Type-III compensation branch |
| R36/R34 | 49.9 kΩ / 16 kΩ | Feedback divider, about 3.295 V nominal |

Output architecture remains `3V3_BUCK -> FB1 -> 3V3_MCU`. `PGOOD` and `SYNCH` are intentionally NC. Upstream C5 = 100 µF / 50 V remains.

Recorded simulation/design-review results remain approximately 59.1 kHz crossover, 64.9° phase margin and 19.6 dB gain margin. These are simulation results, not hardware measurements.

## Current PCB checkpoint

Current board file: `hardware/Filament_Dryer_Monitor.kicad_pcb`.

At the latest PCB checkpoint:

- old `AP66200` and `/Power/VCC_AP66200` are absent;
- U5 and the buck/AutoEN block are on **B.Cu**;
- the placement has been iterated against the ST L7987L / STEVAL-ISA198V1 layout and TI comparator guidance;
- in the current physical board view the **VIN/VCC side of U5 is the left side**;
- C3 is the closest local 1 µF VIN/VCC bypass; C1 is on the same VIN side, slightly more external;
- C21 is intentionally kept close to U5 pin 1 VBIAS and does not need to be moved to create a superficial PGND corridor;
- C6 is BOOT-to-LX and is not part of the GND network;
- C10 / FB1 / C11 remain the output-side sequence;
- FB/COMP parts are kept in the quiet region away from LX/high-current copper;
- local B.Cu copper/zones are being developed for `24V_PROT`, `/Power/3V3_BUCK`, GND and LX;
- U5 exposed pad is GND/`SGND_17`; **thermal/GND via count, diameter, spacing and paste strategy are the immediate open implementation item**;
- critical routing is not yet frozen.

Do not use old chat coordinate/rotation notes as authoritative. Placement has changed. For B.Cu pad-direction review, use KiCad absolute pad positions or the actual board view.

## Stackup and inner planes

| Layer | Copper / spacing | Role |
|---|---|---|
| F.Cu | 35 µm | front components/signals |
| dielectric 1 | 0.10 mm FR4, Er 4.5 | F.Cu to In1 |
| In1.Cu | 35 µm | **solid GND plane** |
| core | 1.24 mm FR4, Er 4.5 | inner separation |
| In2.Cu | 35 µm | **solid GND plane** |
| dielectric 3 | 0.10 mm FR4, Er 4.5 | In2 to B.Cu |
| B.Cu | 35 µm | back components/signals + local buck/power copper |

This is a closed project decision: **In1 and In2 are both full GND planes. There is no internal 3V3 or 24 V power plane.**

For the B.Cu buck, In2 is the nearest reference plane at 0.10 mm.

## Buck PGND / SGND implementation

ST distinguishes power-ground and signal-ground current paths. In this project they are the **same KiCad `GND` net** and the inner planes are not split.

High-current/pulsed return group:

- C1 negative;
- D7 anode;
- C10 negative.

Quiet/signal return group:

- U5 pin 16 and exposed pad;
- C3 negative;
- C21 negative;
- sensitive feedback/compensation/control returns.

The practical distinction is **local B.Cu geometry and where the vias enter the common continuous GND planes**. The pulsed current must not be forced through the quiet local return region.

A continuous B.Cu GND strip from C1− to D7/C10− is **not required** merely because ST calls those paths PGND. C1−, D7 anode and C10− may each enter the common inner GND planes through very short nearby vias, with the planes providing the low-impedance common return. C21 therefore does not need to be displaced simply to create a B.Cu PGND corridor across the top of U5.

## Netclasses checkpoint

Power/default netclasses were reviewed and accepted at the end of the previous chat:

| Class | Clearance | Track width | Via diameter / drill |
|---|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.60 / 0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | 0.60 / 0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | 0.80 / 0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | 0.80 / 0.40 mm |
| USB | 0.20 mm | 0.25 mm | 0.60 / 0.30 mm |

Assignments include `3V3_MCU` and `/Power/3V3_BUCK` -> Power_3V3; `24V_PROT`, `/Power/JACK_24V_RAW`, `/Power/FUSE_OUT` -> Power_24V; `HEATER_SW` -> Power_Heat; `USB_DP`/`USB_DM` -> USB.

**USB differential-pair geometry remains open.** In the reviewed Board Setup screenshot, the USB class DP width/gap fields were still blank. The Default class contained 0.20 mm DP width / 0.25 mm DP gap, but these values must not be assumed valid for USB. Calculate/verify the USB pair against the actual B.Cu-to-In2 stackup before routing freeze.

## Exact handoff point for the next chat

The next chat must bootstrap from `AGENTS.md`, then inspect the current branch and current board rather than relying on this prose alone.

The immediate work sequence is:

1. **U5 exposed pad:** define/verify the GND/thermal via matrix and paste strategy against ST package guidance and the intended JLCPCB process. The previous chat ended while preparing to place these vias; do not assume a final via pattern is already committed unless the current PCB shows it.
2. Place the remaining local GND vias for the high-current group (C1−, D7 anode, C10−) and quiet group (U5 EP/pin16, C3−, C21−), all into the same solid inner GND planes.
3. Finish critical input/VIN, BOOT/LX/D7/L1, output and FB/COMP routing against the ST reference.
4. Finish AutoEN routing.
5. Decide DFT access before routing freeze.
6. Calculate and configure USB differential-pair geometry.
7. Complete full-board review, run a fresh DRC, then regenerate production outputs.

The historical `hardware/DRC.rpt` is not evidence that this PCB passes DRC.