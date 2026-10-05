# Project state

Checkpoint date: **2026-10-04**  
Active hardware branch: **`main`**  
Fabrication-source checkpoint (before documentation updates): **`74a3000127371ac6c3b3b7197f9036dec5b58eef`** (`Production files`, 2026-09-17). Documentation-only commits after this checkpoint do not change the released hardware.

## Executive state

The L7987L + AutoEN revision was released for a **bare-board, manual-assembly prototype**. The stored current KiCad reports date from 2026-09-17: ERC **0 errors / 0 warnings**; DRC **0 active errors / 0 unconnected pads / 0 footprint errors**, with **one excluded** U4 B.Silkscreen-to-board-edge warning. The stored netlist was regenerated on 2026-09-17 and has the corrected USB-C D+/D− mapping. The released production BOM and position file are `hardware/production/Filament_Dryer_Monitor_bom.csv` and `hardware/production/Filament_Dryer_Monitor_positions.csv`; the unsuffixed `hardware/production/bom.csv` / `positions.csv` are legacy AP66200-era exports and **must not** be used for the current design.

Gerbers/drills were uploaded to JLCPCB for bare-PCB production. Its production/CAM package was received and checked against the uploaded Gerbers in the order discussion. **On 2026-10-01 the user reported that the bare PCBs and component shipment had arrived.** First-article hand assembly and staged bring-up are now in progress. Gate 1A, isolated buck, AutoEN normal-operation, 3V3_MCU rail-link, and Gate 3 MCU power/reset/boot checks have passed as recorded below. USB enumeration at Gate 3 currently fails at the device-descriptor stage. No PCBA/stencil service is planned; all components are hand-assembled.

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

The TME purchase was placed 2026-09-20; all **43 current `TME_IMPORT` supplier catalog codes** were found in the original order confirmation during a 2026-09-21 check (this does not verify shipment or received quantities). Its original confirmation lists C11 `CL21A226MAYNNNE` as `Disponibile a magazzino` (page 1, position 3), but TME later acknowledged the stocked unit is missing and is searching for it. Its week 48/2026 ETA is a supplier notice for **C11 alone**. Do not infer the delivery date of the remaining order from that notice. The supplier has **not confirmed** release of all other lines or resolution of C11. No cancellation/replacement has been authorized.

C11 is **required in the assembled design**: it is a 22 µF MLCC from `3V3_MCU` to GND. Current authorized MPN: Samsung `CL21A226MAYNNNE`, 25 V X5R 0805. The 16 V TDK candidate `C2012X5R1C226M125AC` / TME `C2012X5R1C226MAC` is a *candidate only*; verify stock, dimensions, DC-bias/usable capacitance and order resolution before installing or changing the purchasing BOM. There is no reason to change the PCB footprint based on the nominally identical 0805 size. Do **not** omit C11 for final assembly.

Do not treat the old unsuffixed `hardware/production/bom.csv` (legacy AP66200/CP2102N-era) as this design's current BOM. The suffixed export's `LCSC Part #` column is a mixed historical/supplier field; the **live Google Sheet's `Codice TME`** is authoritative for TME purchases. Reconcile actual deliveries (including supplier-imposed minimum quantities) against the TME order confirmation, not only the nominal per-board quantities.

Off-board OLED/J4, SHT45/J3, independent heater cutoff, mating housings/crimps/cable assemblies and mounting parts are **not included** in the 94-component PCB order. Treat them as inventory/assembly checks, not as newly selected PCB parts. See `hardware/docs/PROCUREMENT.md` for follow-up.

## Immediate handoff / next gates

1. Wait for TME's definitive result of the C11 warehouse search and a clear statement about shipping all other available order lines; do not cancel/reorder or silently replace the BOM until agreed.
2. Reconcile received TME packages against `BOM TME`, the confirmed order quantities and all 33 in-house refs. In particular, confirm the actual capacitor supplied for C11 before soldering.
3. Verify physical availability, pinout and fit of J4 OLED, external SHT45, thermal cutoff, mating connectors/cables and enclosure fasteners. The procurement sheet does not establish these are in house.
4. Inspect the received bare PCBs for mask, drills, component fit, exposed pads and assembly clearances; reconcile the received component packages/quantities against the released BOM before soldering.
5. Hand-assemble and bring up the first PCB following `docs/ASSEMBLY.md` (**R5 initially not populated**); verify 3.3 V, USB, sensor/display, buck temperature and fault handling before heater operation.
6. Keep experimental PCB changes and future-revision review separate from the already submitted fabrication revision; do not retroactively rewrite the released Gerber/netlist/BOM checkpoint without new evidence and a deliberate revision.

## 2026-10-01 bare-board preflight

Bare-board preflight continuity/isolation checks were reported passed before assembly: no short to GND was observed on `JACK_24V_RAW`, `24V_PROT`, `3V3_BUCK` or `3V3_MCU`; the unpopulated FB1 pads were isolated from each other; and J1 pin 3 was isolated from TP1 with F1/Q1 still unpopulated. Exact resistance values were not recorded. Gate 1A assembly may proceed.

## 2026-10-02 Gate 1A powered bring-up

Gate 1A powered test passed on 2026-10-02: supply current decayed to ~0 after capacitor charging at 5 V, 12 V and 24 V; TP1 tracked the applied input voltage; Q1 gate measured 0 V at 5 V, 0 V at 12 V and 9.31 V at 24 V, consistent with D1 clamping Q1 |VGS| to about 14.7 V.

- 2026-10-03: first-board power bring-up reached the 3V3_MCU rail. With R5, C11 and FB1 fitted, 24 V input produced ~1.8 mA no-load input current, 3V3_BUCK ~3.3 V and TP2/3V3_MCU ~3.3 V. Gate 1 power-path/buck/rail-link checks passed at no load; load/ripple/thermal validation remains pending.


## 2026-10-04 Gate 3 MCU / USB status

Gate 3 was reported fully populated. With 24 V input and a 50 mA bench limit, the board drew about 8 mA; 3V3_MCU, EN_ESP and IO0 measured 3.3 V. SW1/RESET and SW2/BOOT each pulled their respective node to about 2.5 mV while pressed and returned to 3.3 V when released. Thermal-camera inspection found no concerning hotspot.

USB enumeration remains open: Windows creates an unknown USB device but fails the device-descriptor request with Code 43 (`USB\\DEVICE_DESCRIPTOR_FAILURE`), and no CP2102 COM port or VID/PID is exposed. Changing the USB cable and host USB port did not change the result. Hardware checks so far found USB VBUS present (~4.7 V), the CP2102 VBUS-sense divider active (~3.1 V at R7/R8), CP2102 reset high, U3 correctly powered/oriented, D+/D− continuity from J2 through U3 to U2, no D+/D− short, and approximately 3.3 V on the directly probed U2 VDD/REGIN/VBUS-sense-/RST pins. U2 orientation appears visually correct; hidden QFN joint quality is not yet proven. Continue diagnosis before any rework or hardware revision.

Oscilloscope follow-up on 2026-10-04: CP2102-side D+ rises to ~3.3 V while D− idles low; a 50 ms/div capture shows a host-driven USB reset (D+ forced low, then returning high). A 2 µs/div single-shot trigger on a D− rising edge did not trigger. Continue by distinguishing connector-side USB activity from U3/U2-side activity before any rework.

USB follow-up: no D− rising-edge trigger was obtained even with a single 10× probe on U3 pin 3 (connector side). This shifts the immediate diagnosis toward checking whether D− is being clamped/shorted low (or otherwise prevented from transitioning) before considering CP2102 rework.

USB fault isolation update: U3 pin 3 was observed to move physically under probe contact, indicating an unreliable solder joint on the connector-side D− path. Repair and re-inspect U3 before further USB measurements or any U2 rework.

U3 was re-soldered on all six pins after the loose pin-3 finding. USB enumeration remains unchanged (unknown device / descriptor request failed), so further USB signal diagnosis is still required before any U2 rework or design conclusion.

Post-rework USB scope update: after re-soldering all U3 pins, D− at U3 pin 3 (connector side) now triggers on a rising edge when USB is connected. The available capture was at 100 µs/div and is too slow to assess 12 Mbps waveform amplitude/shape. This confirms the U3 repair changed the D− signal path, but Windows still reports descriptor failure; continue with faster-timebase packet capture before further rework.

After re-soldering U3, oscilloscope probing now shows clear high-speed activity on D− at U3 pin 3 (connector side), including a resolved burst at 2.5 µs/div. This is a real change from the pre-rework no-trigger result and confirms the loose U3 pin-3 joint was affecting the USB signal path. Descriptor enumeration is still not yet confirmed fixed; compare the same activity at U3 pin 4 before considering U2 rework.

USB propagation update: after U3 rework, D− activity measured at U3 pin 4 (CP2102 side) is practically identical to pin 3 (connector side), indicating the signal is propagating through U3. Continue diagnosis toward U2/USB protocol behavior rather than U3 continuity.

Gate 3 completeness was re-audited against the current MCU source and CP2102 datasheet: no additional essential component is missing from the populated USB/UART group. Required VDD/REGIN bypassing, /RST pull-up, VBUS sensing, USB-C CC resistors and ESD/data path are present. A post-rework 100 ns/div D− capture shows full-swing high-speed transitions; this confirms bit-level bus activity but does not yet prove successful CP2102 descriptor response.

CP2102 ground verification: U2 pin 3 to TP3/GND measured ~0.3 Ω unpowered, confirming the peripheral GND connection. The hidden exposed-pad joint remains unverified visually, but loss of the chip's only ground path is excluded.

Android cross-check: an Android USB device-info app reported no device detected when the powered board was connected to the phone. Treat this as provisional until phone-side host/OTG operation is confirmed by measuring USB VBUS at J2/R7 during the connection.

Android host validation: USB_VBUS measured only ~3.3 V with the phone connected, rather than the ~5 V expected from an active USB host. The Android no-device result is therefore inconclusive and must not be treated as an independent confirmation of the Windows descriptor failure.

USB VBUS backfeed check: with USB disconnected and only 24 V applied, the USB_VBUS node measured ~1 mV. The PCB is not backfeeding VBUS; the earlier ~3.3 V with the phone was external to the board and did not establish valid USB host mode.

Android host test confirmed valid: USB_VBUS measured 5 V from the phone while connected, yet no USB device was detected. Since Windows also fails at the device-descriptor stage, the remaining leading suspects are the CP2102 device or its QFN soldering/hidden pad rather than host software.

U2 GND clarification: pin 3 and exposed pad 29 share GND, and the PCB EP copper is tied to the GND network. The ~0.3 Ω pin-3-to-TP3 reading confirms board-side grounding; only the hidden package-EP solder bond remains unobservable. With peripheral GND working, perimeter U2 pins (especially USB/power/reset pins 4–9) are the next rework focus rather than the EP connection itself.

Gate 3 USB is now operational: following U2 perimeter-pin rework, Windows enumerates U2 as **Silicon Labs CP210x USB to UART Bridge (COM5)**. The prior descriptor failure is resolved. This strongly indicates a marginal U2 solder joint was the remaining assembly fault; exact pin not identified.

Gate 3 UART progress: ESP32 ROM boot output is readable on COM5 at 115200 after SW1 RESET, confirming U4 TXD0 -> CP2102 -> USB and ESP32 reset/boot operation. Reverse UART and programming/auto-reset remain to be validated.

Automatic programming is not yet passing: esptool on COM5 reaches `Connecting...` but reports `Invalid head of packet (0x65)`. Since ESP32 ROM TX output is readable, next isolate automatic DTR/RTS boot control from the PC-to-ESP32 UART path using manual BOOT/RESET download mode.

Manual BOOT/RESET proves bidirectional UART and ROM download mode: esptool identifies ESP32-D0WD-V3 rev 3.1 and reads chip information, but then fails while uploading/starting the RAM stub with `Invalid head of packet (0x65)`. Focus now shifts from UART connectivity to rail stability during stub start and possible esptool 5.3.x stub behavior.

Gate 3 programming update: with the 24 V bench current limit raised from 50 mA to 100 mA, manual BOOT/RESET programming completes successfully; esptool stub runs, flash writes and hash verification pass. The prior stub-stage corruption was caused by the too-low 50 mA current limit. Auto-download entry still needs a separate no-button upload test.

At 100 mA, automatic no-button upload still fails with `No serial data received`, while manual BOOT/RESET flashing succeeds. This isolates the remaining Gate 3 issue to the DTR/RTS automatic boot-control path. Capture TP4/EN_ESP and TP5/IO0 simultaneously during esptool connection.

Gate 3 auto-program root cause is now identified from scope plus source review: the released Q2/Q3 DTR/RTS cross-coupled network implements the opposite EN/IO0 truth table from Espressif's ESP32 auto-download reference. Manual BOOT/RESET works, but automatic esptool entry cannot generate the required GPIO0-low-at-reset-release condition. Treat this as a released-hardware design defect pending an agreed prototype bodge and later source correction; do not silently alter the schematic/PCB.


Gate 3 auto-program fix validated on the first article (2026-10-04): the Q2/Q3 collector destinations were crossed so Q2 collector drives EN and Q3 collector drives IO0. After this bodge, no-button Arduino/esptool programming succeeds through COM5, including stub start, flash verification and automatic RTS reset. This experimentally confirms the released PCB's auto-download truth table was reversed. The design sources have not yet been silently altered; the correction is recorded in D020 for the next hardware revision. Runtime output from the newly flashed diagnostic sketch remains to be checked before closing Gate 3.

Gate 3 is closed on the first article (2026-10-04). After the Q2/Q3 collector-cross bodge, automatic USB/UART programming works without manual buttons, and the flashed diagnostic sketch runs correctly at 115200 baud (startup banner after reset, then periodic `alive`). Next assembly/test gate is Gate 4A I2C support: J3 plus R14-R17 first, with the unloaded bus checked before separately attaching the actual J4 SSD1309 OLED module and the external SHT45.

Gate 4A hardware population has begun: J3 and R14-R17 are fitted; J4 OLED and SHT45 are not connected yet. J3 mapping is 1=GND, 2=SDA, 3=SCL, 4=3V3_MCU. Unloaded I2C bus checks are next.

Gate 4A unloaded I2C bus validation passed: J3 power, SDA and SCL continuity/idle levels are correct with no off-board modules attached. Next hold point is physical pinout/orientation verification of the actual SHT45 board and J3 harness before connection.

Gate 4A harness status (2026-10-04): the external SHT45 is still **not connected** to J3. The sensor currently has four loose color-coded leads: red = 3V3, yellow = SCL, green = SDA, black = GND. The first-article PCB already has J3 fitted as JST PH 4-way `B4B-PH-SM4-TB` (2.00 mm pitch), with released-netlist mapping pin 1 = GND, pin 2 = SDA, pin 3 = SCL, pin 4 = 3V3_MCU. The user currently has JST-XH connector parts, which do not mate with J3. A JST-PH 2.00 mm assortment with 2/3/4/5/6-way housings, pre-crimped 22 AWG leads and female crimp terminals is under consideration; do **not** mark it ordered/received until confirmed. The intended next bench step is to terminate the SHT45 leads in a 4-way PH mating housing with the above pin order, verify orientation/continuity with a meter, and only then connect/power the sensor for the I2C diagnostic. Historical README wording mentioning a STEMMA QT cable is not the released PCB interface; the fabricated PCB interface at J3 is JST-PH.

## 2026-10-04 Gate 4B buttons + diagnostic LED

Gate 4B is **PASSED** on the first article. SW3, SW4, SW5 and SW6 all read approximately 3.3 V at rest and pull their respective input net to approximately 0 V when pressed.

During bring-up, SW6 / BTN_DOWN initially measured about 0.91 V at rest. The cause was a poor solder joint on R24, not the switch or ESP32. Reworking R24 restored the expected ~3.3 V idle level.

D3/R18 was tested by flashing a temporary GPIO26 blink sketch; the LED blinked correctly, confirming the GPIO26 -> R18 -> D3 path. Because D3 is not visible outside the enclosure, D021 defines it as an **internal diagnostic/service LED**, not a user-facing status indicator.

Gate 4A remains at the JST-PH SHT45 harness hold point. Gate 4C buzzer population/testing can proceed independently while the PH harness is pending.

## 2026-10-05 Gate 4C buzzer

Gate 4C is **PASSED** on the first article. BZ1, Q6, D6, R27 and R37 were populated and the buzzer path was verified from ESP32 GPIO33.

A temporary firmware test drove the passive buzzer at approximately 2.7 kHz and produced the expected audible tone. A second test played a three-note ascending melody once at startup, confirming that firmware can vary pitch and timing.

The initial 50% duty-cycle test was subjectively too loud. Future firmware should therefore expose buzzer loudness as a software-controlled setting using reduced PWM duty cycle (for example OFF / LOW / MED / HIGH after final tuning), with the understanding that this is an approximate acoustic level control rather than a calibrated linear volume control.

Proceed to Gate 4D NTC input before fan/heater power-output bring-up.
