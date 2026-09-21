# Filament Dryer Monitor

Custom ESP32-based controller board that retrofits an **eSUN eBox** filament dryer with closed-loop humidity/temperature control, a local OLED interface, data logging and a web UI.

The PCB physically replaces the original front panel: display and buttons sit on the front face of the board, while the main electronics are on the back.

> **Current status — 2026-09-21:** hardware branch **pcb/l7987l-layout**, fabrication-source checkpoint **`74a3000127371ac6c3b3b7197f9036dec5b58eef`** (2026-09-17, production files). The stored current ERC (2026-09-17) reports **0 errors / 0 warnings**; DRC reports **0 active errors / 0 unconnected pads** and one consciously excluded U4 silkscreen/board-edge warning. The final Gerbers/drills were uploaded to JLCPCB; a CAM production file was received and reviewed for the **bare PCB, hand-assembly** prototype. The exact factory production/shipping status is not tracked in GitHub. Component procurement: TME order placed 2026-09-20; **C11 is unresolved pending TME's warehouse search**. See `docs/PROJECT_STATE.md` and `hardware/docs/PROCUREMENT.md`.

For a new work session, read AGENTS.md, docs/PROJECT_STATE.md, docs/DECISIONS.md and docs/TODO.md before changing the design.

---

## Features

- **SHT45** humidity/temperature sensor (I²C)
- **Characterized NTC thermistor** integrated into the heating element
- **1.54 in SSD1309 OLED** (128×64, I²C) + four front-panel buttons
- PWM heater control with hardware and firmware safety layers
- PWM fan control
- CSV logging to internal flash planned
- Web UI planned
- Powered from the dryer's 24 V supply; USB is used for programming/debug

## Safety concept

The project keeps three independent safety layers for the heater:

1. **Hardware TCO** in series with HEATER+, off-board and independent of the MCU.
2. **Firmware safety loop** using the heater NTC; an invalid/open sensor is treated as a fault and disables the heater.
3. **Compile-time gate**: heater drive is disabled until NTC calibration is explicitly enabled.

This is a personal project and not a certified product.

---

## Hardware

The board is a 4-layer KiCad 10 design intended for JLCPCB **bare-board fabrication and manual component assembly**. TME is the preferred prototype component supplier. The current KiCad schematic and PCB on the active branch are the authoritative implementation.

### Main blocks

| Block | Part | Notes |
|---|---|---|
| MCU | ESP32-WROOM-32E | Wi-Fi; final antenna copper/edge review remains a release item |
| Buck converter | **L7987L** | 24V_PROT -> 3.3 V, about 516 kHz, 1 A design target |
| Buck fault recovery | TLV1701AIDBVR + MMBT3904 | External AutoEN based on COMP fault detection |
| USB-UART | CP2102-GM | USB Full-Speed programming/debug |
| USB ESD | AKS1201 / USBLC6-2SC6-compatible topology | Two protected data channels |
| Humidity/temp | SHT45 | I²C 0x44 |
| Display | SSD1309 OLED 128×64 | I²C 0x3C |
| Heater driver | IRLR3636TRPBF | 60 V DPAK N-MOSFET, about 1.6 A heater load |
| Fan driver | IRLML2060TRPBF | 24 V fan low-side switch |
| Heater temperature | Integrated ~82 kΩ NTC | GPIO34 / ADC1, R28 = 47 kΩ, C19 = 100 nF |
| Buzzer | Passive buzzer + NPN | Defined startup state with base-emitter pull-down |

Heater and fan stay on 24V_PROT; they are not powered by the 3.3 V buck.

### L7987L electrical state

The active implementation is in hardware/Power.kicad_sch.

Key implemented parts/values:

| Ref | Value / part | Role |
|---|---|---|
| U5 | L7987L | Buck regulator |
| U1 | TLV1701AIDBVR | AutoEN comparator |
| Q7 | MMBT3904 | AutoEN EN pull-down |
| L1 | SRN6045-150M, 15 µH | Inductor |
| D7 | PMEG6030EP,115 | Catch diode, 60 V / 3 A Schottky |
| C1 | 10 µF / 100 V X7S | Main local input ceramic |
| C3 | 1 µF / 100 V X7S | Current local VIN/VCC bypass present in source |
| C6 | 100 nF | BOOT-LX bootstrap |
| C10 | 47 µF / 10 V X7R | Main pre-bead output capacitor |
| C21 | 1 µF / 25 V | VBIAS bypass |
| R4 | 47 kΩ | FSW, ~516 kHz |
| R29 | 47.5 kΩ | ILIM, ~1.705 A nominal |
| R36 / R34 | 49.9 kΩ / 16 kΩ | Feedback, ~3.295 V nominal |

Compensation: R33 = 16 kΩ, C9 = 18 nF, C8 = 39 pF, R35 = 1.13 kΩ, C20 = 560 pF.

Output architecture remains 3V3_BUCK -> FB1 -> 3V3_MCU. PGOOD and SYNCH are intentionally NC. The upstream C5 = 100 µF / 50 V bulk capacitor remains.

**VIN/VCC ceramic bypass — resolved:** ST requires a ceramic of 1 µF or higher across VIN-to-power-GND and another across VCC-to-IC-GND, close to the device. In the current design C1 = 10 µF / 100 V X7S already satisfies the VIN-side requirement, while C3 = 1 µF / 100 V X7S is the VCC bypass. The extra buck C22 added on 2026-09-09 was therefore redundant and was deliberately removed in commit `24a9170a`. Current C22 is unrelated: it is the CP2102 REGIN 1 µF / 25 V capacitor.

Detailed calculations and AutoEN rationale are in [docs/BUCK_L7987L_DESIGN.md](docs/BUCK_L7987L_DESIGN.md).

### Engineering review

The L7987L + AutoEN engineering review completed on **2026-09-04**. The accepted design basis remains:

- R29 = 47.5 kΩ -> ~1.705 A nominal current limit;
- conservative component-selection envelope ~1.42–2.15 A;
- actual sourced C1/C10 reviewed with class-II MLCC bias sensitivity;
- AutoEN reviewed COMP trip window ~1.56–2.07 V;
- recorded simulation ~59.1 kHz crossover, 64.9° phase margin, 19.6 dB gain margin.

These are engineering/simulation results; physical validation remains required.

The current `hardware/ERC.rpt` (2026-09-17) reports **0 errors and 0 warnings**. The existing GitHub ERC workflow contains the reviewed waiver logic, but its automatic push trigger currently names redesign/buck-sourcing, not the active pcb/l7987l-layout branch.

### Current PCB integration

The current board at hardware checkpoint `74a3000` contains the L7987L stage and AutoEN block on **B.Cu**. The old AP66200 stage and /Power/VCC_AP66200 are absent.

The saved 2026-09-17 DRC reports **0 active errors, 0 active warnings, 0 unconnected pads** and one **excluded** U4 B.Silkscreen-to-board-edge warning.

Local external copper/zones are present for 24V_PROT, /Power/3V3_BUCK, 3V3_MCU, HEATER_SW, LX and GND. Both internal layers remain solid GND.

The fabrication package has progressed to JLCPCB CAM review; this does **not** constitute physical validation of the assembled hardware.

### U5 exposed pad — resolved for planned hand assembly

U5 pad 17 is a 3.2 × 3.2 mm GND exposed pad on B.Cu. The accepted design intentionally uses **no via-in-pad**. Instead, six GND vias, each **0.60 mm diameter / 0.30 mm drill**, sit immediately outside the EP in two rows of three: 3 above and 3 below U5. This arrangement was implemented in commit `c9dc4129` and is present in the current PCB.

The peripheral vias provide a short thermal/GND path into the internal GND planes without placing open vias directly under the solderable exposed pad. For the planned prototype assembly, U5 is fitted by lightly pre-tinning the EP, applying flux and heating with hot air. Because this flow does not use a stencil for U5, the full B.Paste definition on pad 17 is not a prototype release blocker.

If the process later changes to stencil/reflow or external PCBA, paste-windowing and via treatment must be reopened for that process. First-board U5 temperature measurement remains part of validation. See decision D019 and `docs/ASSEMBLY.md`.

### CP2102-GM / USB

The classic CP2102-GM custom QFN footprint currently uses:

- 0.50 mm pitch;
- perimeter pads **0.95 × 0.28 mm**;
- exposed pad **3.25 × 3.25 mm**;
- solder-mask expansion **+0.06 mm**;
- **3 × 3** exposed-pad paste apertures, each 0.9 × 0.9 mm.

REGIN is on 3V3_MCU and now has a local **C22 = 1 µF / 25 V X7R 0603, Murata GCM188R71E105KA64J**, in addition to the local 100 nF decoupling.

USB differential routing:

- target: **90 Ω differential**;
- B.Cu referenced to In2.Cu;
- width: **0.20 mm**;
- gap: **0.25 mm**;
- KiCad tuning profile: **USB_90R**;
- long U2 -> U3 pair stays entirely on B.Cu with no vias;
- J2 A6/B6 = D+, A7/B7 = D−;
- U3 channel 1↔6 carries D+, channel 3↔4 carries D−;
- the short Type-C duplicated D+ fanout uses two signal vias and a short F.Cu bridge, with a nearby GND stitching via; D− stays on B.Cu.

The stored netlist was regenerated on 2026-09-17 after the MCU correction; USB pinout still requires first-board functional verification.

### Ground planes and stackup

| Layer | Role |
|---|---|
| F.Cu | Front components and signals |
| In1.Cu | **Solid GND plane** |
| In2.Cu | **Solid GND plane** |
| B.Cu | Back components/signals + local power copper |

Stackup entered in KiCad: 35 µm copper; 0.10 mm FR4 between F.Cu-In1 and In2-B.Cu; 1.24 mm FR4 core between inner layers; total board thickness 1.6 mm.

**Both inner layers are deliberately full GND.** There are no internal 3V3 or 24 V power planes.

For the L7987L, PGND and SGND are not separate project nets or split inner planes. They are current-return regions on the same GND net; local B.Cu geometry and via placement keep pulsed returns out of quiet control returns.

### Power distribution

Power remains on the external layers.

- 24V_PROT: wide traces/local zones are appropriate where current requires them.
- 3V3_BUCK: compact local copper around the buck output/Cout/FB1 path is preferred.
- 3V3_MCU: may remain mainly track-distributed, with local copper where useful.
- LX and other fast switched nodes: keep copper only as large as needed.

The capacitance from external power copper to the nearby GND plane is not a reason to avoid local power pours; the important restriction is minimizing fast switch-node area.

### Netclasses

| Class | Clearance | Track | DP width / gap | Via dia/drill |
|---|---:|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.20 / 0.25 mm default | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | — | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | — | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | — | 0.80/0.40 mm |
| USB | 0.20 mm | **0.20 mm** | **0.20 / 0.25 mm** | 0.60/0.30 mm |

USB data nets are USB_D+, USB_D−, USB_CONN_D+, USB_CONN_D−.

### Procurement

The working purchasing BOM is the Google Sheet **Filament Dryer Monitor — BOM finale Mouser**, tab **BOM TME**.

The live **BOM TME** was checked against the current production BOM/netlist: **94 mounted component references**, of which **61 items / 43 TME purchase rows** and **33 references marked in-house** for one assembled board. TME order was placed 2026-09-20. **C11 (22 µF, 25 V, X5R, 0805)** is outstanding: TME confirmed a stock-location problem and is checking its warehouse. No replacement/cancellation has been approved. The display, external sensor, heater thermal cutoff and mating cabling are outside this PCB purchasing BOM; their actual inventory remains to be checked.

See [hardware/docs/PROCUREMENT.md](hardware/docs/PROCUREMENT.md).

### Heater NTC characterization

The original eSUN heater includes an NTC whose exact manufacturer/part number is unknown. It was characterized in situ during cooldown after disconnecting it from the original controller.

| Temperature | Resistance |
|---:|---:|
| 25 °C | 82.5 kΩ |
| 28 °C | 75.5 kΩ |
| 30 °C | 67.4 kΩ |
| 35 °C | 51.7 kΩ |
| 40 °C | 43.6 kΩ |
| 44 °C | 35.93 kΩ |
| 50 °C | 28.8 kΩ |

The measurements are consistent with about 82 kΩ at 25 °C and a single-beta approximation near β ≈ 4100 K over the measured range. These are empirical measurements, not manufacturer specifications.

PCB interface: R28 = 47 kΩ 1%, C19 = 100 nF, GPIO34 / ADC1.

### GPIO map

| Function | GPIO |
|---|---|
| SDA / SCL | IO21 / IO22 |
| UART TX0 / RX0 | IO1 / IO3 |
| BOOT / RESET | IO0 / EN |
| Status LED | IO26 |
| Fan PWM | IO16 |
| Buzzer | IO33 |
| Heater PWM | IO19 |
| NTC | IO34 / ADC1 |
| Button ON/OFF | IO35 |
| Button M | IO32 |
| Button UP | IO14 |
| Button DOWN | IO27 |

Do not run a project-wide annotation reset when changing a hardware block; preserve established references or annotate only selected new circuitry.

---

## Firmware

Firmware uses the ESP32 Arduino core 3.x and is intended to remain cooperative/non-blocking in normal operation.

| Block | Status |
|---|---|
| SHT45 driver | Done |
| Fan PWM | Done; kickstart/duty floor provisional |
| Heater + NTC safety | NTC characterized; conversion/calibration/safety implementation pending |
| OLED | Pending |
| Buttons + UI state machine | Pending |
| LittleFS + CSV logging | Pending |
| Web UI | Pending |
| Buzzer / LED | Pending |
| Non-blocking SHT45 conversion | Pending |

---

## Repository handoff files

~~~text
AGENTS.md                       Mandatory workflow before project work
docs/PROJECT_STATE.md           Current authoritative checkpoint
docs/DECISIONS.md               Durable engineering decisions
docs/TODO.md                    Immediate work list
docs/BUCK_L7987L_DESIGN.md      Detailed L7987L design/layout record
docs/ASSEMBLY.md                Assembly/bring-up sequencing
hardware/docs/PROCUREMENT.md    Sourcing/footprint/manufacturing state
hardware/*.kicad_*              Actual KiCad implementation
~~~

## Current next steps

1. Await TME's definitive response about **C11** and confirmation that the remaining available parts will ship; do not change the KiCad MPN or procurement BOM until an actual replacement/cancellation is agreed.
2. Verify the physical delivery, identities, quantities and packages of the TME order against the live `BOM TME` / `TME_IMPORT`; verify the stock of all items marked `In casa`.
3. Check availability and mechanical/electrical fit of off-board items (SSD1309 OLED, SHT45 sensor, independent heater thermal cutoff, mating connectors/cables and mounting hardware).
4. On receipt of the bare PCBs, inspect fabrication/assembly details and follow `docs/ASSEMBLY.md`; **leave R5 unfitted for initial 3.3 V bring-up**.
5. Perform first-board buck, USB, thermal and fault-recovery testing before normal heater operation. See `docs/PROJECT_STATE.md` and `docs/TODO.md`.
