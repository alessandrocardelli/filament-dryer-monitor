# Filament Dryer Monitor

Custom ESP32-based controller board that retrofits an **eSUN eBox** filament dryer with
closed-loop humidity/temperature control, a local OLED interface, data logging and a web UI.

The PCB physically replaces the original front panel: display and buttons sit on the front
face of the board, all the electronics on the back.

> **Status: work in progress.** The schematic is complete and ERC-clean; the PCB layout and
> most of the firmware are still under development. See [`docs/TODO.md`](docs/TODO.md).

---

## Features

- **SHT45** humidity/temperature sensor (I²C) for accurate chamber readings
- **NTC thermistor** on the heating element for the safety layer
- **1.54" SSD1309 OLED** (128×64, I²C) + 4 tactile buttons for local control
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
   Fail-safe by design: if the NTC is disconnected, the ADC node is pulled toward 3V3 and
   reads as "hot", so an open sensor shuts the heater off rather than leaving it on.
3. **Compile-time gate** — `NTC_CALIBRATED` must be defined before the heater can be driven
   at all, preventing operation with placeholder thermistor coefficients.

**Use at your own risk.** This is a personal project, not a certified product.

---

## Hardware

4-layer PCB, 81.5 × 48 mm. Designed in **KiCad 10**, targeted at **JLCPCB/LCSC** fabrication
and assembly.

### Main blocks

| Block | Part | Notes |
|---|---|---|
| MCU | ESP32-WROOM-32E | Wi-Fi, antenna keepout respected on all copper layers |
| Buck converter | AP66200 | 24 V → 3.3 V |
| USB-UART | CP2102N | With auto-program (DTR/RTS) circuit |
| Humidity/temp | SHT45 (Adafruit #6174) | I²C `0x44`, via STEMMA QT cable |
| Display | SSD1309 OLED 128×64 | I²C `0x3C` |
| Heater driver | N-MOSFET, DPAK | ~1.6 A continuous, flyback diode |
| Fan driver | DMN6140L (SOT-23) | 24 V fan, 0.2 A |
| Element temp | NTC thermistor | ADC1, 11 dB attenuation |
| Buzzer | Passive + NPN driver | |

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
| Fan PWM | IO25 |
| Buzzer | IO33 |
| Heater PWM | IO19 |
| NTC (ADC1) | IO34 |
| Button ON/OFF | IO35 |
| Button M | IO32 |
| Button UP | IO39 |
| Button DOWN | IO4 |

All buttons: external 10 k pull-up to 3V3, button to GND, pressed = LOW.
IO12 is deliberately left unloaded (strapping pin).

### Reference designators

This project uses **functional reference designators** where they carry meaning
(`Q_heat`, `Rfb_top`, `J_sens`) and sequential numbering for generic passives.
When re-annotating in KiCad, always choose **"Keep existing annotations"** — a full
re-annotation will destroy these names.

---

## Firmware

Written with the **Arduino IDE** (ESP32 Arduino core 3.x), structured as modular blocks.
The main loop is cooperative and `millis()`-based — no blocking `delay()` calls outside
the SHT45 driver.

| Block | Status |
|---|---|
| SHT45 driver (hand-written I²C, CRC-8, cmd `0xFD`) | Done |
| Fan PWM (LEDC, 25 kHz, 10-bit) | Done — kickstart and duty floor provisional |
| Heater + NTC safety | Blocked on NTC characterization |
| OLED (U8g2, SSD1309-specific constructor) | Pending |
| Buttons + UI state machine (STANDBY / DRYING / DONE / FAULT) | Pending |
| LittleFS + CSV logging | Pending |
| Web UI | Pending |
| Buzzer / LED | Pending |
| Non-blocking SHT45 conversion | Pending |

---

## Repository layout

```
hardware/     KiCad project, custom symbol and footprint libraries,
              3D models, generated gerbers
firmware/     Arduino sketch and modular headers
docs/         Datasheets, circuit explanation, TODO list
```

## Building the hardware

1. Open `hardware/Filament_Dryer_Monitor.kicad_pro` in KiCad 10 or newer.
2. Custom libraries resolve through `${KIPRJMOD}`, so the project is portable —
   no library setup needed after cloning.
3. Gerbers for the current revision are in `hardware/gerbers/`.

## Building the firmware

1. Install the ESP32 board support package (Arduino core 3.x) in the Arduino IDE.
2. Install the `U8g2` library.
3. Select **ESP32 Dev Module** as the board.
4. Open `firmware/firmware.ino` and flash over USB.

---

## License

To be defined.
