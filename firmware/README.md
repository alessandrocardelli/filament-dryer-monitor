# Filament Dryer Monitor firmware

This branch contains **HEATER-capable experimental firmware**. `main` remains the SAFE BRING-UP non-heating reference. See [PROJECT_STATE](../docs/PROJECT_STATE.md) for current source, physical evidence and installed-image uncertainty.

## Implemented

- Slewform root-growth boot animation followed by a separate solo wordmark synchronized with the existing rising startup chime;
- SHT45 non-blocking acquisition with CRC validation;
- debounced ON/OFF, M, UP and DOWN handling, including long press/autorepeat;
- BOOT, STANDBY, SETUP, SETTINGS, DRYING-test, COMPLETE and FAULT presentation;
- `CycleController` for countdown/completion;
- `FanController` for the real first-article fan;
- `SafetyController` with the selected experimental AIR/NTC thermostat and independent fault/STOP/watchdog handling;
- Preferences/NVS persistence for last cycle UI values, buzzer volume and key-click preference;
- buzzer OFF/LOW/MED/HIGH, optional key click, startup/start/complete/warning/fault sequences;
- D021 diagnostic LED patterns;
- cooperative/non-blocking main loop.

## Fan and heater paths

The historical D025 non-heating bring-up policy was 25kHz, 100% for1s then40%, OFF at stop/completion/fault. It is not this branch's selected heating policy. The experimental path commands100%, with5s prerun and90s post-heat cooling; these provisional settings are not qualified FAN AUTO specifications or measured airflow.

`kHeaterStep09TrialMode=true` selects a real GPIO19 heater-control path **before** the production approval checks in `SafetyController`. Therefore `kHeaterOutputEnabled=false` does not make this branch heater-off. The selected D030 90/85 profile controls raw NTC resistance; its90°C Sanwa-equivalent point is estimated. See [D030](../docs/DECISIONS.md) and [current state](../docs/PROJECT_STATE.md), not historical upload instructions.

The separate product controller is disabled, its approvals false and operational limits unconfigured. No physical90/85 qualification or unattended approval follows from CI. Existing series TCO105 remains unchanged. Boot/STANDBY force heater OFF; an explicit cycle can request experimental heating subject to interlocks.

Implemented experimental services include advisory material presets, live telemetry, CSV/LittleFS history, 12h host-tested logging and authenticated LAN STANDBY settings/STOP with no remote START. These have different software/operator/physical evidence levels in PROJECT_STATE.

## Persistence

ESP32 Preferences/NVS stores:

- last target-temperature UI value;
- last duration;
- selected advisory material profile (optional NVS `mat`, existing schema);
- buzzer volume;
- key-click preference.

The store avoids writes when values have not changed. Active/running-cycle state is not persisted and must not auto-resume after reset/power loss.

## Arduino environment

- ESP32 Arduino core 3.x
- U8g2 library
- Preferences library from the ESP32 Arduino core
- Board target: classic ESP32 / ESP32-WROOM-32E
- Serial monitor: 115200 baud

Open `firmware/firmware.ino` as the sketch.

## Slewform boot animation — reproducible export

Source of truth: `assets/slewform/Slewform_logo.svg`, unchanged.
The active asset is `assets/slewform/slewform_animation_compact.h`, regenerated
with `assets/slewform/generate_animation_compact.py`. The old combined
`slewform_logo_128x64.h` and `generate_logo.py` remain as legacy references.

The OLED boot sequence lasts **4.15 s** after initialization:

- **0–1.20 s:** stationary emblem without roots (extended on user request).
- **1.20–2.40 s:** 11 progressive root-growth stages at 120 ms intervals.
- **2.40–2.75 s:** complete emblem held, still silent.
- **2.75–4.15 s:** separate solo SLEWFORM wordmark; the rising chime starts
  immediately **after** the complete wordmark has been sent and revealed.
- **4.15 s:** STANDBY, heater OFF. This does not disable the experimental heating path on a later explicit cycle.

The 1-bit XBM/PROGMEM bitmap masks are: upper symbol 54×60 (420 bytes),
root geometry 54×60 (420 bytes), and wordmark 122×18 (288 bytes).
**Total bitmap data: 1,128 bytes** plus eleven 16-bit growth radii.
The renderer reveals root pixels by radius, non-blockingly, rather than
storing complete animation frames. Boot images are transmitted only when
changing stage; repeated 100 ms redraws have been removed.
The packed radius table now contains **11 explicit values**. Previously the
declared 11-element array included only ten initializers, so C++ supplied a
zero for the last growth stage: that briefly removed all roots just before
the wordmark. It is now unsized with a compile-time count assertion, and the
last radius covers every root-mask pixel. The generator checks this too. On initial power-up
`initDisplay()` keeps the OLED hidden until the first logo frame is uploaded.
At the symbol-to-wordmark transition, the display is temporarily blanked,
the complete wordmark is uploaded, and the OLED is turned back on before
starting the chime. This eliminates intermediate, partially refreshed logo
frames, at the cost of a brief black transition while I2C data is transferred.
Firmware cannot prevent images shown *before* the ESP32 initializes the OLED.
The user reported intermittent horizontal lines or the previous screen at
power-on: they temporarily appeared absent after the previous `initDisplay()`
change but reappeared in testing without a change to that initialization path.
This remains unresolved. To separate controller retained RAM/power sequencing
from firmware effects, compare cold power-on after a long off interval,
immediate power-cycle, and ESP32 RESET while 24 V remains on. Record when
the artifact appears (before the first emblem or during it). Do not infer a
hardware solution without those observations.

Regenerate from the repository root:

```sh
python -m pip install cairosvg pillow numpy
python firmware/assets/slewform/generate_animation_compact.py
```

The generator verifies XBM packing. Visual and chime timing must still be
validated on the physical OLED and buzzer after flashing the firmware.
