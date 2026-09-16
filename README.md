# Filament Dryer Monitor

Custom ESP32-based controller board that retrofits an **eSUN eBox** filament dryer with closed-loop humidity/temperature control, a local OLED interface, data logging and a web UI.

The PCB physically replaces the original front panel: display and buttons sit on the front face of the board, while the main electronics are on the back.

> **Current status — 2026-09-16:** hardware work is on branch **pcb/l7987l-layout**. Latest hardware checkpoint before this documentation refresh: **9da46e7953f801073882fd1934802fa8ace1f1c2** (updated layout). L7987L + AutoEN routing is electrically complete enough for the current DRC to report **0 errors and 0 unconnected pads**. USB routing is configured for a **90 Ω differential target** on B.Cu/In2.Cu using **0.20 mm width / 0.25 mm gap**. The board is **not manufacturing-ready**: U5 exposed-pad thermal-via/paste implementation remains open, stored netlist/ERC must be regenerated after the latest MCU edit, current DRC warnings need review, and production outputs must be regenerated/reconciled before release.

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

The board is a 4-layer KiCad 10 design intended for JLCPCB fabrication/assembly. TME is the preferred prototype component supplier. The current KiCad schematic and PCB on the active branch are the authoritative implementation.

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
| Fan driver | CJ2310 | 24 V fan low-side switch |
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
| D7 | STPS2L60A | Catch diode |
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

The stored hardware/ERC.rpt currently reports **1 power_pin_not_driven error on external GND and 0 warnings**, but it predates the latest MCU schematic edit and must be regenerated. The existing GitHub ERC workflow contains the reviewed waiver logic, but its automatic push trigger currently names redesign/buck-sourcing, not the active pcb/l7987l-layout branch.

### Current PCB integration

The current board at hardware checkpoint 9da46e7 contains the L7987L stage and AutoEN block on **B.Cu**. The old AP66200 stage and /Power/VCC_AP66200 are absent.

The fresh 2026-09-16 DRC reports:

- **0 errors**;
- **0 unconnected pads**;
- 14 active warnings;
- 1 excluded warning.

Local external copper/zones are present for 24V_PROT, /Power/3V3_BUCK, 3V3_MCU, HEATER_SW, LX and GND. Both internal layers remain solid GND.

Routing connectivity is therefore complete, but the final engineering/manufacturing review remains open.

### U5 exposed pad

U5 pad 17 is a 3.2 × 3.2 mm GND exposed pad. In the current board there are **no thermal vias inside the EP**, and the current footprint exposes the full EP as B.Paste.

Before fabrication the project still needs a deliberate EP implementation: via matrix/process, paste-window strategy, solder-wicking review and JLCPCB capability check.

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

The stored netlist is stale relative to the last MCU edit and still contains the pre-correction J2 D+/D− assignment. Regenerate it before using it for sign-off.

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

The live **BOM TME** was rechecked on 2026-09-16 and is already consistent with the current references: C1 is the 10 µF / 100 V VIN ceramic, C3 is the 1 µF / 100 V VCC bypass, and C22 appears only in the 1 µF / 25 V row for CP2102 REGIN. No C22 duplicate remains.

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

1. finalize U5 exposed-pad thermal-via and paste/stencil strategy;
2. regenerate the netlist and ERC from the latest schematic and verify USB mapping;
3. review/close the current DRC warnings;
4. complete final buck, USB, antenna, high-current and mechanical PCB review;
5. regenerate production BOM/CPL/Gerbers/drills/netlist from the same final revision and reconcile them with the already-updated live BOM TME;
6. perform fabrication/assembly review;
7. follow docs/ASSEMBLY.md for first power-up and AutoEN R5 sequencing;
8. perform first-board electrical, thermal, switching-stress and fault-recovery validation.
