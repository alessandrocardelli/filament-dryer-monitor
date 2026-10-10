# Filament Dryer Monitor firmware

Current application phase: **SAFE BRING-UP with real fan control**.

## Implemented

- Slewform root-growth boot animation followed by a separate solo wordmark synchronized with the existing rising startup chime;
- SHT45 non-blocking acquisition with CRC validation;
- debounced ON/OFF, M, UP and DOWN handling, including long press/autorepeat;
- BOOT, STANDBY, SETUP, SETTINGS, DRYING-test, COMPLETE and FAULT presentation;
- `CycleController` for countdown/completion;
- `FanController` for the real first-article fan;
- `SafetyController` that continuously forces the heater output OFF;
- Preferences/NVS persistence for last cycle UI values, buzzer volume and key-click preference;
- buzzer OFF/LOW/MED/HIGH, optional key click, startup/start/complete/warning/fault sequences;
- D021 diagnostic LED patterns;
- cooperative/non-blocking main loop.

## Fan bring-up policy

The current fan policy is intentionally provisional:

- 25 kHz PWM;
- 100% duty for 1 s when a DRYING test cycle starts;
- 40% duty after kick-start;
- 0% on stop, completion or fault.

This retains margin over the measured first-article thresholds (25% first tested start duty, 15% lowest tested sustained duty). It is **not** the final FAN AUTO algorithm. Post-cycle cooldown remains undecided.

## Heater safety status

This is still **not heater-control firmware**.

GPIO19 / HEATER_PWM is configured as an output by `SafetyController` and forced LOW on every main-loop pass. There is no heater-enable path in this phase.

Do not add heater control until the real NTC/J7 path, NTC conversion/fault handling, independent TCO and heater safety gates are validated and recorded.

The target-temperature values visible in SETUP are UI prototype values only; they are not accepted heater limits.

## Persistence

ESP32 Preferences/NVS stores:

- last target-temperature UI value;
- last duration;
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

The OLED boot sequence still lasts **3.5 s**:

- **0–1.5 s:** 11 levels of root growth (150 ms each); upper emblem fixed.
- **1.5–2.1 s:** full emblem held; **silent**.
- **2.1–3.5 s:** separate solo SLEWFORM wordmark; start rising chime on entry.
- **3.5 s:** STANDBY. The heater stays hard-locked OFF.

The 1-bit XBM/PROGMEM bitmap masks are: upper symbol 54×60 (420 bytes),
root geometry 54×60 (420 bytes), and wordmark 122×18 (288 bytes).
**Total bitmap data: 1,128 bytes** plus eleven 16-bit growth radii.
The renderer reveals root pixels by radius, non-blockingly, rather than
storing complete animation frames. `Ui::requestRender` forces the wordmark
refresh during the same main-loop pass as the chime's first note.

Regenerate from the repository root:

```sh
python -m pip install cairosvg pillow numpy
python firmware/assets/slewform/generate_animation_compact.py
```

The generator verifies XBM packing. Visual and chime timing must still be
validated on the physical OLED and buzzer after flashing the firmware.
