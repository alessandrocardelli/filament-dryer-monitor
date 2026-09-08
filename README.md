# Filament Dryer Monitor

Custom ESP32-based controller board that retrofits an **eSUN eBox** filament dryer with closed-loop humidity/temperature control, a local OLED interface, data logging and a web UI.

The PCB physically replaces the original front panel: display and buttons sit on the front face of the board, while the main electronics are on the back.

> **Current status — 2026-09-08:** hardware work is on branch **`pcb/l7987l-layout`**. The L7987L + AutoEN schematic passed the project engineering schematic review on 2026-09-04 and the PCB is synchronized to that design. The legacy AP66200 stage is absent. The project is in the **L7987L PCB layout/routing phase**. Latest PCB-only checkpoint recorded for this handoff is `e8741347ef5570948a1115719ae65fbc0de02ce6`. Both inner layers are solid GND planes and the power/default netclasses are accepted. The immediate open implementation is the U5 exposed-pad GND/thermal-via and paste strategy, followed by buck local GND-via/current-return geometry and critical routing. USB differential-pair geometry and final DRC remain open. The board is **not manufacturing-ready**.

For a new work session, read `AGENTS.md`, `docs/PROJECT_STATE.md`, `docs/DECISIONS.md` and `docs/TODO.md` before changing the design.

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

1. **Hardware TCO** in series with `HEATER+`, off-board and independent of the MCU.
2. **Firmware safety loop** using the heater NTC; an invalid/open sensor is treated as a fault and disables the heater.
3. **Compile-time gate**: heater drive is disabled until NTC calibration is explicitly enabled.

This is a personal project and not a certified product.

---

## Hardware

The board is a 4-layer KiCad 10 design intended for JLCPCB fabrication/assembly. TME is the preferred prototype component supplier. The current KiCad schematic, PCB and project files on the active branch are the authoritative implementation.

### Main blocks

| Block | Part | Notes |
|---|---|---|
| MCU | ESP32-WROOM-32E | Wi-Fi; final antenna copper keepout still part of PCB review |
| Buck converter | **L7987L** | `24V_PROT` -> 3.3 V, about 516 kHz, 1 A design target |
| Buck fault recovery | TLV1701AIDBVR + MMBT3904 | External AutoEN based on COMP fault detection |
| USB-UART | CP2102-GM | USB used for programming/debug |
| Humidity/temp | SHT45 | I²C `0x44` |
| Display | SSD1309 OLED 128×64 | I²C `0x3C` |
| Heater driver | IRLR3636TRPBF | 60 V DPAK N-MOSFET, about 1.6 A heater load |
| Fan driver | CJ2310 | 24 V fan low-side switch |
| Heater temperature | Integrated ~82 kΩ NTC | GPIO34 / ADC1, R28 = 47 kΩ, C19 = 100 nF |
| Buzzer | Passive buzzer + NPN | Defined startup state with base-emitter pull-down |

Heater and fan stay on `24V_PROT`; they are not powered by the 3.3 V buck.

### L7987L electrical state

The active implementation is in `hardware/Power.kicad_sch`.

Key parts/values:

| Ref | Value / part | Role |
|---|---|---|
| U5 | L7987L | Buck regulator |
| U1 | TLV1701AIDBVR | AutoEN comparator |
| Q7 | MMBT3904 | AutoEN EN pull-down |
| L1 | SRN6045-150M, 15 µH | Inductor |
| D7 | STPS2L60A | Catch diode |
| C1 | 10 µF / 100 V X7S | Main local input ceramic |
| C3 | 1 µF / 100 V | Local VIN/VCC bypass |
| C6 | 100 nF | BOOT-LX bootstrap |
| C10 | 47 µF / 10 V X7R | Main pre-bead output capacitor |
| C21 | 1 µF / 25 V | VBIAS bypass |
| R4 | 47 kΩ | FSW, ~516 kHz |
| R29 | 47.5 kΩ | ILIM, ~1.705 A nominal |
| R36 / R34 | 49.9 kΩ / 16 kΩ | Feedback, ~3.295 V nominal |

Compensation: R33 = 16 kΩ, C9 = 18 nF, C8 = 39 pF, R35 = 1.13 kΩ, C20 = 560 pF.

Output architecture remains `3V3_BUCK -> FB1 -> 3V3_MCU`. `PGOOD` and `SYNCH` are intentionally NC. The upstream C5 = 100 µF / 50 V bulk capacitor remains.

Detailed calculations, source hierarchy, AutoEN rationale and schematic sign-off are in [`docs/BUCK_L7987L_DESIGN.md`](docs/BUCK_L7987L_DESIGN.md).

### Schematic sign-off

The Hardware Design Manual-based review completed on **2026-09-04**. Four electrical gates are closed:

1. **ERC:** current reviewed state is CI-enforced. The report contains one reviewed `power_pin_not_driven` modeling error on external GND plus eleven reviewed warnings.
2. **ILIM/L1:** R29 = 47.5 kΩ gives ~1.705 A nominal. A conservative engineering envelope of roughly 1.42–2.15 A was used for component-selection review; the upper estimate remains below the Bourns 2.3 A Isat rating. It is not claimed as an ST-guaranteed 47.5 kΩ limit.
3. **Capacitance/stability:** actual sourced C1 and C10 were reviewed including class-II MLCC bias sensitivity. No invented guaranteed `Ceff,min` is used.
4. **AutoEN:** reviewed COMP trip window is about 1.56–2.07 V over the defined engineering corners; EN-high margin remains ample.

Recorded simulation results with final compensation are approximately **59.1 kHz crossover, 64.9° phase margin and 19.6 dB gain margin**. These are design/simulation results; physical validation remains required.

### Current PCB integration

The current `hardware/Filament_Dryer_Monitor.kicad_pcb` contains the L7987L stage and AutoEN block. The old AP66200 stage and `/Power/VCC_AP66200` are absent.

The buck is on **B.Cu**. Placement has been iterated against ST/TI guidance. In the current physical board view the U5 VIN/VCC side is the **left side**; C3 is the closest local VIN/VCC bypass, C1 is on the same side more externally, and C21 remains close to pin 1 VBIAS.

Primary buck layout reference: ST L7987L datasheet plus STEVAL-ISA198V1 Gerbers/layout. The ST reference is used for topology/current-return intent, adapted to this project's four-layer stackup and footprints.

### Ground planes and stackup

| Layer | Role |
|---|---|
| F.Cu | Front components and signals |
| In1.Cu | **Solid GND plane** |
| In2.Cu | **Solid GND plane** |
| B.Cu | Back components/signals + local buck/power copper |

Stackup entered in KiCad: 35 µm copper; 0.10 mm FR4 between F.Cu-In1 and In2-B.Cu; 1.24 mm FR4 core between inner layers; total board thickness 1.6 mm.

**Both inner layers are deliberately full GND.** There are no internal 3V3 or 24 V power planes.

For the L7987L, PGND and SGND are not separate project nets or separate internal planes. They are different **current-return regions on the same `GND` net**. High-current returns (C1−, D7 anode, C10−) and quiet returns (U5 pin16/EP, C3−, C21− and sensitive control returns) use local B.Cu geometry and short vias into the common solid GND planes so switching current is not forced through the quiet return region. A continuous B.Cu PGND corridor joining the two sides of U5 is not required.

See `docs/DECISIONS.md` before changing this architecture.

### Netclasses

| Class | Clearance | Track | Via dia/drill |
|---|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | 0.80/0.40 mm |
| USB | 0.20 mm | 0.25 mm | 0.60/0.30 mm |

`USB_DP` and `USB_DM` are assigned to the USB class, but **USB differential-pair width/gap is still open**. In the last reviewed Board Setup the USB DP fields were blank. Do not assume the Default-class 0.20/0.25 mm DP settings are correct.

### Procurement

The working purchasing BOM is the Google Sheet **`Filament Dryer Monitor — BOM finale Mouser`**, tab `BOM TME`.

TME sourcing, KiCad manufacturer/MPN synchronization and the footprint/pinout audit are closed for the current schematic. See [`hardware/docs/PROCUREMENT.md`](hardware/docs/PROCUREMENT.md).

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

Do not run a project-wide annotation reset when changing a hardware block; preserve established references or annotate only the selected new block.

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

```text
AGENTS.md                       Mandatory workflow before project work
docs/PROJECT_STATE.md           Current authoritative checkpoint
docs/DECISIONS.md               Durable engineering decisions
docs/TODO.md                    Immediate work list
docs/BUCK_L7987L_DESIGN.md      Detailed L7987L design/layout record
hardware/docs/PROCUREMENT.md    Sourcing/footprint/manufacturing state
hardware/*.kicad_*              Actual KiCad implementation
```

## Current next steps

Continue from the current PCB rather than resynchronizing from scratch. The immediate sequence is:

1. finalize U5 exposed-pad thermal/GND via and paste strategy;
2. place/review high-current and quiet local GND vias into the common inner GND planes;
3. finish L7987L critical input, LX/BOOT/diode/inductor, output and FB/COMP routing against ST guidance;
4. close optional DFT access before routing freeze;
5. calculate/configure USB differential-pair geometry and review its continuous GND reference;
6. complete full-board PCB review;
7. run a **fresh DRC** and close it;
8. regenerate production outputs from the final revision and reconcile them with the purchasing BOM;
9. perform first-board electrical, thermal, switching-stress and AutoEN validation.

The existing `hardware/DRC.rpt` is historical and must not be used as proof that the current PCB passes DRC.