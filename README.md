# Filament Dryer Monitor

Custom ESP32-based controller board that retrofits an **eSUN eBox** filament dryer with
closed-loop humidity/temperature control, a local OLED interface, data logging and a web UI.

The PCB physically replaces the original front panel: display and buttons sit on the front
face of the board, all the electronics on the back.

> **Status: work in progress.** On branch `redesign/buck-sourcing` the schematic and exported
> netlist now implement the L7987L 24 V -> 3.3 V buck redesign, including the external AutoEN
> fault-recovery circuit. The PCB is **not yet synchronized** with this schematic and still
> contains the legacy AP66200 power stage, so the current PCB/production outputs must not be
> treated as manufacturing-ready. Most firmware work is also still in development.

---

## Features

- **SHT45** humidity/temperature sensor (I²C) for accurate chamber readings
- **Characterized NTC thermistor** integrated into the heating element for the safety layer
- **1.54" SSD1309 OLED** (128×64, I²C) + 4 front-panel buttons for local control
- **PWM heater control** with hardware and firmware safety cutoffs
- **PWM fan control** (25 kHz, inaudible)
- **CSV logging** to internal flash (LittleFS)
- **Web UI** over Wi-Fi as a secondary interface
- Powered directly from the dryer's **24 V** supply, USB only for programming

## Safety

Heating is a hazard, and this design treats it as one. Three independent layers:

1. **Hardware TCO** — a thermal cutoff (~100–110 °C) wired in series on the `HEATER+` line
   at the heating element. Off-board, purely mechanical, works even if the MCU is dead.

2. **Firmware safety loop** — runs every cycle, independent of the UI state machine.
   Fail-safe by design: if the NTC is disconnected, the ADC node is pulled toward 3V3.
   An out-of-range/open-sensor condition is treated as a fault and immediately disables
   the heater.

3. **Compile-time gate** — `NTC_CALIBRATED` must be defined before the heater can be driven
   at all, preventing operation with placeholder thermistor coefficients.

**Use at your own risk.** This is a personal project, not a certified product.

---

## Hardware

4-layer PCB. Designed in **KiCad 10** and intended for JLCPCB fabrication/assembly. Component
procurement for the prototype is being migrated to **TME as the preferred supplier**. The
current KiCad schematic and freshly exported netlist on the active hardware branch are the
authoritative electrical state; the PCB is currently one step behind the schematic because the
new L7987L power stage has not yet been placed/routed.

The board itself forms the dryer's replacement front panel, so its outline and component
placement are constrained by the original enclosure mechanics.

### Main blocks

| Block | Part | Notes |
|---|---|---|
| MCU | ESP32-WROOM-32E | Wi-Fi, antenna keepout respected on all copper layers |
| Buck converter | **L7987L** | `24V_PROT` -> 3.3 V, ~500 kHz, 1 A design target; external TLV1701 + NPN AutoEN fault recovery |
| USB-UART | CP2102N | USB used for programming/debug only, with DTR/RTS auto-program circuit |
| Humidity/temp | SHT45 (Adafruit #6174) | I²C `0x44` |
| Display | SSD1309 OLED 128×64 | I²C `0x3C`, mounted on the front face |
| Heater driver | IRLR3636TRPBF (DPAK) | 60 V logic-level N-MOSFET, ~1.6 A heater load, LCSC C67279 |
| Fan driver | CJ2310 (SOT-23) | 24 V fan, ~0.2 A |
| Element temp | Integrated ~82 kΩ NTC | ADC1, 47 kΩ divider resistor, 100 nF filtering |
| Buzzer | Passive + NPN driver | |

The heater and fan remain on `24V_PROT`; they are **not** loads of the 3.3 V buck. The buck is
sized for a conservative **1 A** 3.3 V design target. Detailed calculations, compensation,
SIMPLIS history and the AutoEN rationale are recorded in
[`docs/BUCK_L7987L_DESIGN.md`](docs/BUCK_L7987L_DESIGN.md).

### Current buck integration state

The redesign is now part of `hardware/Power.kicad_sch`; the temporary `Buck redesign`
hierarchical sheet has been removed from the project hierarchy. The exported netlist confirms:

- `U5 = L7987L`;
- `U1 = TLV1701` comparator for AutoEN;
- `Q7 = MMBT3904` AutoEN pull-down transistor;
- `L1 = 15 µH` buck inductor;
- `D7 = STPS2L60A` catch diode;
- L7987L `VIN1`, `VIN2` and `VCC` are tied directly to `24V_PROT`;
- `PGOOD` and `SYNCH` are intentionally NC;
- the existing upstream 24 V protection/bulk network and downstream `FB1 -> 3V3_MCU` filter are retained.

The earlier VIN-to-VCC **0 Ω jumper has been removed**; VCC is now directly connected to
`24V_PROT` with its local 1 µF bypass capacitor.

The pre-FB1 buck output is electrically correct but is currently auto-named by KiCad
(`Net-(U5-VBIAS)`); the historical `3V3_BUCK` label has not yet been restored. This is a naming
cleanup item, not an electrical break.

### Procurement status

The working purchasing BOM is the Google Sheet `Filament Dryer Monitor — BOM finale Mouser`,
tab **`BOM TME`**. It remains the working source for supplier codes, stock and purchasing
choices, but it has **not yet been reconciled to the newly integrated L7987L block**.

At the current checkpoint, final MPN/footprint selection for the new buck components and the
Google Sheet update are intentionally deferred until the electrical schematic is frozen and the
PCB update is ready. Existing MPN fields in the new buck block must therefore be treated as
provisional rather than as the final purchasing BOM.

See [`hardware/docs/PROCUREMENT.md`](hardware/docs/PROCUREMENT.md) for the current sourcing
workflow and open procurement/PCB tasks.

### Heater NTC characterization

The original eSUN flexible heater incorporates an NTC thermistor that cannot be removed
independently from the heater assembly.

The thermistor was therefore characterized **in situ**. The original dryer controller was
used to heat the assembly, after which power was removed and the NTC JST connector was
disconnected from the original electronics. Temperature and NTC resistance were then
recorded during natural cooldown.

Measured values:

| Temperature | NTC resistance |
|---:|---:|
| 25 °C | 82.5 kΩ |
| 28 °C | 75.5 kΩ |
| 30 °C | 67.4 kΩ |
| 35 °C | 51.7 kΩ |
| 40 °C | 43.6 kΩ |
| 44 °C | 35.93 kΩ |
| 50 °C | 28.8 kΩ |

The measurements are consistent with an NTC of approximately **82 kΩ at 25 °C**, with a
single-beta approximation of roughly **β ≈ 4100 K** over the measured temperature range.

Because the original thermistor manufacturer and exact part number are unknown, these values
should be considered an **empirical characterization**, not manufacturer specifications.

The PCB uses:

- **R28 = 47 kΩ, 1%** as the fixed divider resistor
- **C19 = 100 nF** for ADC input filtering
- **GPIO34 / ADC1** for the temperature measurement

The 47 kΩ divider value was selected to provide better ADC voltage span across the useful
heater-temperature range than the original 100 kΩ design.

Final temperature conversion and safety thresholds will be calibrated in firmware using the
measured thermistor data.

### Layer stackup

| Layer | Role |
|---|---|
| F.Cu | Signals + front-facing components (display, buttons) |
| In1.Cu | Solid GND plane — never routed on |
| In2.Cu | Power zones (24 V and 3.3 V, separate zones) |
| B.Cu | Signals + back-facing components |

### Net classes

| Class | Track width |
|---|---|
| Default | 0.25 mm |
| Power_3V3 | 0.5 mm |
| Power_24V | 1.0 mm |
| Power_Heat | 1.5 mm |

Power-class vias: 0.8 mm drill / 0.4 mm annular ring.

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
| NTC (ADC1) | IO34 |
| Button ON/OFF | IO35 |
| Button M | IO32 |
| Button UP | IO14 |
| Button DOWN | IO27 |

The four front-panel buttons (ON/OFF, M, UP, DOWN) each use an external 10 kΩ pull-up
to 3V3 with the button to GND, so a press reads LOW.

IO35 is input-only and has no internal pull-up, so its external pull-up is mandatory.

BOOT and RESET are two additional service buttons for the ESP32-WROOM module
(programming and reset), not part of the normal user interface.

The NTC is connected to **ADC1** because ADC2 cannot be used reliably while Wi-Fi is active.

IO12 is deliberately left unloaded because it is an ESP32 strapping pin that affects flash
voltage selection during boot.

### Reference designators

The schematic uses normal KiCad references together with `Function` fields where semantic
names are useful. During the L7987L integration, **only the newly inserted buck block was
selectively re-annotated** so that its references are compact; existing references elsewhere in
the project were preserved.

Do **not** run a project-wide annotation reset. When adding or replacing a block, either keep
existing references or annotate only the selected new symbols, then regenerate the netlist and
check for collisions.

---

## Firmware

The firmware is based on the **ESP32 Arduino core 3.x** and is structured as modular blocks.

The main loop is intended to remain cooperative and `millis()`-based, with no blocking
delays in normal operation.

| Block | Status |
|---|---|
| SHT45 driver (hand-written I²C, CRC-8, cmd `0xFD`) | Done |
| Fan PWM (LEDC, 25 kHz, 10-bit) | Done — kickstart and duty floor provisional |
| Heater + NTC safety | NTC characterized — conversion, calibration and safety implementation pending |
| OLED (U8g2, SSD1309-specific constructor) | Pending |
| Buttons + UI state machine (STANDBY / DRYING / DONE / FAULT) | Pending |
| LittleFS + CSV logging | Pending |
| Web UI | Pending |
| Buzzer / LED | Pending |
| Non-blocking SHT45 conversion | Pending |

---

## Repository layout

```text
docs/
└── BUCK_L7987L_DESIGN.md   L7987L design record and current checkpoint

hardware/
├── 3dmodels/          3D models used by KiCad
├── docs/
│   ├── PROCUREMENT.md Current sourcing/TME procurement status
│   └── datasheets/    Component datasheets
├── libs/              Custom symbols, footprints and imported libraries
├── production/        Production exports — regenerate after PCB synchronization
├── review/            Hardware review files
├── *.kicad_sch        Hierarchical KiCad schematics
├── *.kicad_pcb        PCB layout
└── *.kicad_pro        KiCad project
```

`hardware/buck_redesign_sch.kicad_sch` is retained only as a temporary/scratch redesign file;
it is no longer part of the active schematic hierarchy. The active implementation is in
`hardware/Power.kicad_sch`.

Firmware development is ongoing and will be added to the repository as it is finalized.

---

## Building the hardware

1. Open `hardware/Filament_Dryer_Monitor.kicad_pro` in **KiCad 10** or newer.
2. Custom libraries resolve through `${KIPRJMOD}`, so the project is portable and requires
   no machine-specific absolute library paths.
3. Treat the current PCB and `hardware/production/` exports as **stale for the power stage**
   until the L7987L block is transferred to PCB and routed.
4. After PCB synchronization, rerun ERC/DRC and regenerate BOM, position files, netlist and
   fabrication outputs from that same revision before ordering boards.
5. Reconcile the final generated BOM with the `BOM TME` Google Sheet only after final MPN and
   footprint decisions are complete.

---

## License

To be defined.