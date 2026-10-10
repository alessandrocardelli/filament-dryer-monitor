# Filament Dryer Monitor firmware

Current application phase: **SAFE BRING-UP with real fan control**.

## Implemented

- Slewform OLED startup branding and original rising startup chime;
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

## Slewform logo — reproducible monochrome export

The source of truth is `assets/slewform/Slewform_logo.svg` (logo **with wordmark**).
The firmware includes `assets/slewform/slewform_logo_128x64.h`: a **90×60 pixel**,
1-bit XBM bitmap drawn by U8g2 at **(19, 2)** on the 128×64 OLED.

Regenerate locally:

```sh
python -m pip install cairosvg pillow
python firmware/assets/slewform/generate_logo.py
```

The generator normalizes all original SVG colors to the same white silhouette
while preserving transparency, renders at 1536×1024, crops to artwork,
downsamples with Lanczos and applies a 1-bit threshold. Three alternatives
are generated: `light` (165), `balanced` (115, **selected**), and `strong`
(72). Default output is `slewform_logo_128x64.h` plus native 128×64
previews and a 5× comparison image. The selected preview is committed as
`slewform_logo_balanced_128x64.png`.

For another variant, run `python firmware/assets/slewform/generate_logo.py --select strong`.
The script self-checks that every packed bitmap round-trips to its original
90×60 1-bit image. The 1px artwork border prevents cutting off antialiased
edges. **Inspect on the physical OLED before declaring the logo final.**

