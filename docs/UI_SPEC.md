# Front-panel UI specification

Status: **V1 interaction model accepted 2026-10-08**.  
Applies to the current first-article hardware on branch `main`.  
A first SAFE BRING-UP implementation now exists; actuator control and final safety integration remain pending.

This document defines the intended user interaction for the 1.54-inch SSD1309 128×64 OLED, the four front-panel buttons, the passive buzzer and the internal diagnostic LED. It does not define final heater-control gains, safety thresholds, temperature/time ranges or material-profile values; those remain gated by thermal/NTC validation.

## 1. Design goals

1. The dryer must be fully usable from the front panel without Wi-Fi, a phone or the web UI.
2. The normal workflow must remain simple enough to operate without nested menus.
3. Safety logic must remain independent of display/menu state.
4. Faults override normal UI presentation and cannot be cleared merely by silencing the buzzer.
5. The web UI is an additional interface for logging, history, diagnostics and advanced configuration; it is not required for ordinary drying cycles.
6. The front panel should expose the most useful live measurements at a glance: chamber temperature and relative humidity.

## 2. Hardware mapping

Current released hardware mapping:

| Front-panel function | Reference / signal | ESP32 GPIO |
|---|---|---:|
| ON/OFF button | SW3 / BTN_ONOFF | IO35 |
| M button | SW4 / BTN_M | IO32 |
| UP button | SW5 / BTN_UP | IO14 |
| DOWN button | SW6 / BTN_DOWN | IO27 |
| OLED | J4 / SSD1309, I²C address 0x3C | SDA IO21 / SCL IO22 |
| SHT45 | J3, I²C address 0x44 | SDA IO21 / SCL IO22 |
| Passive buzzer | BZ1 / BUZ_DRV | IO33 |
| Internal diagnostic LED | D3 / LED_DRV | IO26 |

All four front-panel buttons are active-low and have pull-ups. Short press, long press and UP/DOWN autorepeat are therefore firmware behaviors rather than hardware constraints.

Physical layout places ON/OFF and M on the left side of the display and UP/DOWN on the right.

## 3. Meaning of ON/OFF

ON/OFF is a **cycle-control button**, not a hardware power switch.

When the dryer has 24 V input power, the controller remains powered. In V1:

- after boot or reset, the controller enters safe `STANDBY`;
- heater and fan default OFF until application logic explicitly enables them;
- a drying cycle is never automatically resumed after power loss, reset or watchdog restart;
- restarting a previous cycle may be considered only as a future deliberate feature with appropriate safety handling.

## 4. Application state model

The V1 user-facing state machine is:

```text
BOOT
  |
  v
STANDBY <---- COMPLETE
  |
  v
SETUP
  |
  v
DRYING
  |
  +------> COMPLETE

BOOT / STANDBY / SETUP / DRYING / COMPLETE
                    |
                    +------> FAULT
```

`FAULT` has display and buzzer priority over all normal states. A future `COOLDOWN` state may be inserted between `DRYING` and `COMPLETE` if final fan-control policy requires it.

The UI state machine must not own heater safety. Control/safety logic independently decides whether heater and fan commands are permitted.

## 5. Button semantics

Default long-press threshold: approximately **1.5 s**, implemented as a firmware constant so it can be tuned after use testing.

### STANDBY

| Button | Short press | Long press |
|---|---|---|
| ON/OFF | Start the currently configured cycle | Same as short press or no extra action |
| M | Enter setup | Enter settings menu |
| UP | No direct action | No direct action |
| DOWN | No direct action | No direct action |

If the OLED is asleep, the first key press wakes the display and is consumed; it must not also start or modify a cycle.

### SETTINGS

From STANDBY, a long press on M opens SETTINGS. Initial persisted options:

- buzzer volume: OFF / LOW / MED / HIGH;
- key click: ON / OFF.

M short selects the next setting; UP/DOWN modify it; M long saves and returns to STANDBY. Settings are stored in ESP32 Preferences/NVS. Active-cycle state is never persisted for automatic restart.

### SETUP

V1 editable fields:

1. target chamber temperature;
2. cycle duration;
3. fan mode, initially `AUTO`.

Material presets are intentionally deferred until their actual values and safety/control implications are agreed. A later `PROFILE` field may provide PLA/PETG/TPU/etc. plus `MANUAL`.

| Button | Short press | Long press |
|---|---|---|
| ON/OFF | Start cycle using displayed settings | No additional action |
| M | Select next editable field | Cancel/return to standby |
| UP | Increase selected value | Autorepeat increase |
| DOWN | Decrease selected value | Autorepeat decrease |

The selected value is visually highlighted/inverted.

### Contextual button hints

Because the four physical buttons flank the OLED (ON/OFF upper-left, M lower-left, UP upper-right, DOWN lower-right), V1 should show very small context-sensitive action labels at the corresponding screen edges when useful. Initial first-article trial:

- STANDBY: `START` / `SET` on the left;
- SETUP: `START` / `NEXT` on the left and `+` / `-` on the right;
- DRYING / SYSTEM: `STOP` / `PAGE` on the left;
- COMPLETE: `OK` at upper-left;
- FAULT: `MUTE` at lower-left.

The hints use a very small font and must remain secondary to measurements/status. If first-article readability is poor or the display feels crowded, remove or abbreviate them rather than shrinking primary data.

### DRYING

Cycle parameters are **locked in V1 once the cycle starts**. This avoids accidental setpoint/time changes during initial application development.

| Button | Short press | Long press |
|---|---|---|
| ON/OFF | No stop action | Stop cycle and return to standby after confirmation/hold completes |
| M | Cycle information pages | Entering setup/settings is not allowed |
| UP | Reserved / no action in V1 | Reserved |
| DOWN | Reserved / no action in V1 | Reserved |

A later firmware revision may permit controlled in-cycle edits, but that is not part of V1.

### COMPLETE

- Heater is OFF.
- Fan behavior after cycle completion is defined by the final control policy; if a cooldown is required, the display must show it explicitly.
- ON/OFF short press acknowledges completion and returns to `STANDBY`.
- M may cycle the final measurement/summary pages until completion is acknowledged.

### FAULT

- Fault presentation overrides the current page.
- Heater is inhibited according to the safety controller.
- M short press silences/acknowledges the audible alarm only.
- Silencing the buzzer does **not** clear the fault and does not re-enable the heater.
- ON/OFF long press may request fault reset only after the underlying fault condition has cleared and the safety logic permits reset.
- If the fault remains present, the system stays in `FAULT`.

## 6. OLED content

The display is 128×64 monochrome. Layouts below are functional wireframes, not final pixel-perfect artwork.

### 6.1 BOOT

On power-up/reset, show the selected **Slewform logo with wordmark** centered on the OLED. The current branding asset is `firmware/assets/slewform/slewform_logo_128x64.h` (90×60 px).

The splash is accompanied by a **short original two-step rising startup chime**, intentionally giving a classic handheld-console startup feel without reproducing the Game Boy sound note-for-note. After first-article use testing, the splash duration is **3.5 s**; 1.5 s was judged too brief to read the branding comfortably.

Safety-critical initialization and safe output defaults happen before/under the splash; the branding sequence must not delay putting heater/fan outputs into their safe state. OLED initialization should blank the display before the first visible frame to suppress random power-up RAM artifacts. Initialization should remain non-blocking where practical.

### 6.2 STANDBY home screen

Primary information is live chamber temperature and relative humidity:

```text
 FILAMENT DRYER

 22.4 C      63 %RH

 Ready
```

The exact typography may use larger digits and unit glyphs than shown in the ASCII mock-up.

After an inactivity timeout (initial target about 60 s), the OLED may blank/sleep to reduce unnecessary OLED aging. The first subsequent button press only wakes it.

### 6.3 SETUP screen

```text
 SET CYCLE

 TEMP       [55 C]
 TIME        04:00
 FAN         AUTO
```

M moves the highlight. UP/DOWN modify the highlighted field.

Final allowed ranges, increments and defaults are **TBD** and must be set only after heater/NTC/control validation.

### 6.4 DRYING main page

```text
 DRYING        02:37
 54.8 C      18.6 %RH

 SET 55 C     FAN 65%
 HEAT ON
```

Priorities:

1. current chamber temperature;
2. current relative humidity;
3. remaining time;
4. target temperature;
5. actuator/control status in compact form.

If space is tight, temperature/RH and remaining time take precedence over secondary status text.

### 6.5 Technical/system page

Accessible with M while drying or from standby:

```text
 SYSTEM
 Chamber    54.8 C
 Heater     72.3 C
 H 63%       F 65%
```

The heater NTC temperature is intentionally a secondary/service value rather than the largest number on the normal screen.

### 6.6 Diagnostics page

Example:

```text
 STATUS
 SHT45       OK
 NTC         OK
 LOG         ON
 WiFi        AP
```

Only show states the firmware can actually determine reliably. Do not invent an `OK` state for unvalidated functions.

### 6.7 COMPLETE

```text
    COMPLETE

    54.1 C
    17.8 %RH

 Cycle finished
```

Completion remains visible until acknowledged or until an agreed timeout is added later.

### 6.8 FAULT

Use plain-language fault identification as the primary message rather than only an error number.

Example:

```text
 !! FAULT !!

 HEATER NTC
 invalid / open

 HEATER OFF
 M = mute
```

Another example:

```text
 !! OVER TEMP !!

 Heater 92.4 C

 HEATER OFF
```

A compact error code may also be logged/displayed for service diagnostics, but it should not replace the readable description.

## 7. Relative humidity in V1

SHT45 RH is always useful for live monitoring, logging and judging drying trend.

For V1, **relative humidity is not by itself an automatic cycle-termination criterion**. Heating changes RH strongly through temperature, so a robust humidity-based completion algorithm must be designed and validated deliberately before it can stop a cycle.

V1 completion is therefore time/control based. Humidity-driven or trend-driven completion is a future feature.

## 8. Fan behavior exposed to the UI

V1 exposes the fan as `AUTO` to the normal user.

The already measured first-article 25 kHz fan thresholds are 25% as the first tested start-from-rest duty and 15% as the lowest tested sustained-running duty. The current SAFE BRING-UP policy deliberately retains margin: 100% for 1 s at cycle start, then 40% for the remainder of the DRYING test cycle. This is a validation policy, not the final FAN AUTO algorithm. The fan stops immediately on cycle stop/completion for now; post-cycle cooldown remains open.

Manual fan percentage can remain available only on a technical/service page if needed for development. It is not a normal V1 cycle-setting requirement.

## 9. Buzzer behavior

BZ1 is passive and can vary frequency and approximate loudness using PWM duty cycle. First-article tests confirmed a tone around 2.7 kHz and a three-note melody; 50% duty was subjectively too loud.

Recommended V1 event mapping:

| Event | Audible behavior |
|---|---|
| Power-up / reset | short original two-step rising Slewform startup chime |
| Button/key feedback | optional very short tick; user-configurable |
| Cycle start | one short confirmation beep |
| Cycle complete | three-note ascending melody, played once |
| Recoverable warning | two short beeps |
| Safety fault | distinctive repeating alarm until muted/acknowledged |
| Fault reset accepted | one short confirmation beep |

Software loudness setting: `OFF / LOW / MED / HIGH`, with final PWM duty values tuned empirically. These are approximate acoustic levels, not calibrated sound-pressure levels.

Normal running must not generate repetitive beeps.

## 10. Internal diagnostic LED D3

D3 is **not** a user-facing indicator because it is hidden inside the installed dryer. It follows D021 unchanged:

| Diagnostic state | D3 pattern |
|---|---|
| Early boot / initialization | solid ON until initialization completes |
| Normal application running | heartbeat: ~100 ms ON every 2 s |
| Recoverable peripheral/sensor warning | double pulse every 2 s |
| Safety/fault state inhibiting controlled outputs | 250 ms ON / 250 ms OFF repeating |
| Board unpowered / firmware halted before LED initialization / GPIO unavailable | OFF |

The LED is useful during development and service after opening the dryer. Safety behavior must never depend on D3.

## 11. Firmware architecture requirements

The application should separate at least these concerns:

- **input manager**: debounce, short/long press, autorepeat;
- **UI state/presentation**: page selection and rendering;
- **cycle controller**: timer and high-level drying state;
- **sensor layer**: SHT45 and NTC acquisition/conversion;
- **safety controller**: independent permission/inhibition for heater operation;
- **fan controller**;
- **buzzer/event manager**;
- **logging layer**;
- **web interface**.

No display page, menu operation or buzzer state may bypass the safety controller.

The normal main loop must remain cooperative/non-blocking. Button holds, melodies, screen timeouts and sensor conversions should be timestamp/state driven rather than implemented with long blocking delays.

## 12. Persistence

V1 should persist user configuration such as:

- last target temperature;
- last cycle duration;
- buzzer loudness;
- key-feedback preference;
- future Wi-Fi/UI preferences as required.

Do **not** persist an active-cycle state in a way that causes automatic heater restart after reboot.

## 13. Items intentionally still open

The following are not decided by this UI specification:

- allowed target-temperature range;
- temperature adjustment step;
- minimum/maximum cycle duration and adjustment step;
- final heater control algorithm and gains;
- NTC conversion/calibration and all safety thresholds;
- fan AUTO policy and post-cycle cooldown duration;
- material profile list and profile values;
- humidity/trend-based automatic completion;
- exact fonts, pixel coordinates and iconography;
- exact buzzer PWM duty values for LOW/MED/HIGH;
- web UI structure;
- CSV/log format.

These must be resolved from hardware/safety validation and later application requirements rather than guessed from the panel design.
