# Filament Dryer Monitor firmware

Current application scaffold: **SAFE BRING-UP UI**.

## What this build does

- boots with the selected Slewform logo on the SSD1309 OLED;
- plays a short original rising startup chime on the passive buzzer;
- reads the SHT45 directly over I2C using command `0xFD` and CRC validation;
- implements debounced ON/OFF, M, UP and DOWN buttons, including long press and UP/DOWN autorepeat;
- implements BOOT, STANDBY, SETUP, DRYING-demo and COMPLETE UI states;
- implements OLED standby sleep/wake behavior;
- implements D021 diagnostic LED patterns;
- keeps the normal loop cooperative/non-blocking.

## Safety status

This is **not heater-control firmware**. GPIO19 (heater) and GPIO16 (fan) are configured as outputs and forced LOW on every loop iteration. The DRYING state is therefore only an interaction/countdown demonstration.

Do not remove the output lock until the real NTC/J7 path, NTC conversion/fault handling, independent TCO and heater-safety gates have been validated and the project documentation updated.

The target-temperature and duration bounds in `AppConfig.h` are deliberately labelled **UI prototype values**. They are not accepted heater limits or product specifications.

## Arduino environment

- ESP32 Arduino core 3.x
- U8g2 library
- Board target: classic ESP32 / ESP32-WROOM-32E
- Serial monitor: 115200 baud

Open `firmware/firmware.ino` as the sketch. The sketch intentionally lives at the root of the `firmware` directory so it can include the canonical Slewform bitmap under `assets/slewform/` without duplicating the generated logo data.
