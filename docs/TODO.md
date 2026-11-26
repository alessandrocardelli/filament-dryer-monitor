# One priority — verify J7 resistance measurement with known 1% resistors (UI4 source now prepared)

**26 November source update:** OLED UI4 + nominal Vishay converter now coded, all four thermal thresholds editable and persisted with zero/unconfigured defaults, heater modes OFF, historical UI3 preserved. Product thermal limits are fed through saved settings but never enabled without separate qualification. **CI/build are software evidence only, no replacement installed or physical heating authorized.** Avoid reflashing the old OEM NTC and interpreting its reading as Vishay °C. D033 records the agreement.

**Operator update 23 Nov 2026:** The original unmarked OEM NTC is replaceable. User reports an order of the selected Vishay SMD thermistor (previously chosen **`NTCS0603E3104FXT`**, 0603 1.6x0.8mm, nominal 100kΩ R25, B25/85=4100K, ±1% on each parameter) and an MG Chemicals desoldering braid. Actual new part number/packaging and physical replacement are **not yet verified**. The existing Sanwa contact probe is fixed in place; **do not request to move it, take more photos or repeat old 60-minute trials**.

**Primary data acquired and saved:** [Vishay current official product datasheet](https://www.vishay.com/docs/29056/ntcs0603e3t.pdf) (doc29056) identifies the model/specifications. Older full [Vishay/TME datasheet rev25-Feb-2013 p.5](https://www.tme.eu/Document/63b44cc9baac41e748fd1c8b8eb951ac/ntcs0603.pdf) includes the actual **manufacturer nominal R(T) table for `NTCS0603E3104*XT` at 5°C intervals**. The derived [1°C CSV with source classification](evidence/thermal/NTCS0603E3104FXT-Vishay-RT-1C.csv) contains 39 direct table anchors + 152 log-R interpolations from −40..+150°C, **not original physical measurements**. See [NTC_CALIBRATION_ANALYSIS](NTC_CALIBRATION_ANALYSIS.md) for assumptions, tolerances, and distinction from old Sanwa contact data.

## One pending physical test — ESP32/J7 resistance-readout check using known resistors (23 Nov 2026)

**What / why:** Before interpreting the manufacturer's new Vishay R(T) table in firmware, independently check the complete ESP32 GPIO34 analog NTC readout with a few known resistors. Prior first-article measurements verified ~33–100kΩ but NOT the ~7–12kΩ values involved in the 22 November latched NTC fault. This test isolates the ADC/divider conversion from NTC thermal/contact uncertainty. It cannot identify the cause of the two Sanwa-versus-original-NTC hot-trace disagreements by itself.

**Firmware and instrument prerequisites:** The board was last user-confirmed running `HOT-261031-N8580`, sourced from `1cd96dd76500dfb4d161e1b445d8df6c9d8e524b`. In STANDBY, its existing `/api/status` returns `ntc_ohms`; Arduino USB Serial Monitor at 115200 baud already prints `NTC adc=...mV corr=...mV R=...kOhm` every 2s. Thus NO new firmware/flash required. DMM is used first to read actual resistance of each resistor **before installation** (never use only its nominal ±1% rating). Confirm basic electrical setup and access to a secure 2-pin JST-GH mating test lead for J7; board mating connector is JST `BM02B-GHS-TBT`. Do not jam exposed loose probes/wires into tiny connector contacts.

**Suggested 1%-resistor stock selection** (substitutions near these values acceptable; exact DMM readings control the analysis):

| Label R | Nominal Vishay-NTC-equivalent temperature | Expected corrected J7 ADC voltage using first-article 3.311V/46.572kΩ divider |
| ---: | ---: | ---: |
| 100kΩ | 25.0°C | 2259mV |
| 47kΩ | 42.6°C | 1663mV |
| 22kΩ | 62.2°C | 1062mV |
| **10kΩ** | **85.0°C** | **585mV** |
| **8.2kΩ** | **91.2°C** | **496mV** |
| **6.8kΩ** | **97.2°C** | **422mV** |
| 4.7kΩ | 109.6°C | 304mV |

Resistor "temperature" equivalents are **nominal values from the new Vishay published R(T)** for explanatory purposes only; the physical bench is room temperature and the original NTC remains installed on the heater. The voltage expectations are analytical approximations, not independent observations. Do not confuse displayed `ntc.temperatureC` on the existing firmware with the future Vishay conversion.

**Procedure proposed, to be performed only when operator ready, not yet executed:**
1. With equipment fully powered DOWN and cooled, record nominal ±1%-resistor labels and their DMM-measured ohms. Current Sanwa probe stays fixed, untouched.
2. With power isolated, disconnect ONLY original two-wire NTC from J7. Connect each known resistor **between J7 pin1 GND and pin2 NTC** using a properly insulated 2-pin JST-GH mating adapter. To physically prevent unintended heater energization while substituting the sensor, also disconnect heater harness J5 with power OFF for the duration, without changing the series 105°C TCO or any thermal safety component. This is a non-heating ADC verification, not an autonomous or thermal-cycle test.
3. Power electronics, keep UI in **STANDBY; DO NOT PRESS START**. Read `ntc_ohms` on `http://filament-dryer.local/api/status` (or previously established IP) or record Serial `NTC ... R=...kOhm` via USB at 115200 baud. Once stable, take three readings roughly 2s apart and note spread. On Serial also save `adc`/ `corr` mV if practical. The initial boot could display a sensor fault if there was no resistor: this is NOT evidence of a valid measurement. For every resistor change, power DOWN before swapping and then power back on.
4. For every point, calculate `error_%=100*(ESP32_ohms-DMM_ohms)/DMM_ohms`; a working acceptance *target*, not an established measured result, is |error| <= 2% with stable readings across the 6.8–100kΩ region (and preferably <=1% around 6.8–10kΩ). If violated, do NOT tweak temperature lookup coefficients; diagnose the **electrical** divider/ADC/supply and correct against measured values. Check **actual node mV against the DMM** under power only if necessary and if safe to probe.
5. Power DOWN, remove resistor and test lead, reconnect **original NTC J7** and **heater J5** as originally configured; do not resume heating without a separately reviewed firmware/safety gate for the replacement NTC. Record readings (including missing ones), dates, connection, firmware ID, instrument, any unexpected FAULT, and calculated percent errors in existing thermal diagnostics/evidence and then update this TODO.

**Expected output / next gate:** a small nominal-versus-DMM-versus-ESP32 table and statement whether the analog resistance readout is accurate enough from ~4.7 to100kΩ to support the *new Vishay* published R(T) conversion. This does not validate NTC thermal contact, TCO, fan airflow or material temperatures. It addresses the critical electrical uncertainty without another AIR55 heating cycle. Retain original 22-Nov failure CSV, no changes to 7120Ω fault or other thermostat/protection code and no automatic physical authorization. The 60min heater experiment remains ABORTED.

## Next gate after the readout check — actual Vishay replacement and thermal qualification

1. Confirm delivered package MPN `NTCS0603E3104FXT`. Install it replacing original SMD NTC, avoiding changes to fixed Sanwa probe.
2. Finish J7 resistor test with actual DMM values (especially 6.8/8.2/10kΩ) and correct ADC parameters *only if evidence warrants* before trusting nominal Vishay °C.
3. Verify the OLED six SETTINGS pages, NVS retention and :8081/port80 live `ntc_c`, raw `ntc_ohms` and saved thermal values with heater electrically disconnected. The new curve is **not valid on the old OEM sensor**.
4. All four menu temperatures are user editable; `0` is unconfigured, and incoherent `NTC rearm >= pause`, `pause >= hard fault`, or `AIR fault <= AIR target` cannot pass product controller validity checks. No numerical temperatures were approved as safe.
5. Confirm component mounting, material/TCO105/duct thermal limits and independent faults, then explicitly review a later HEATER-enabled test image and single supervised Noctua AIR55 run. Retain old raw evidence, old firmware and independent protections. A successful CI does not authorize physical heating.

## Historical evidence and deferred work

- **22 Nov Noctua+ASA AIR55 attempt ABORTED** at13m39.8s, NTC7089.7Ω, AIR45.22°C, firmware `HOT-261031-N8580` on device; [original 185-row CSV](evidence/thermal/2026-11-22-AIR55-NOCTUA-ASA-FAULT-raw.csv). Previous 31-Oct OEM run at Sanwa78°C/12min had NTC~11155Ω vs new 22-Nov Sanwa78°C/12min with NTC~7550Ω; no cause proven. Do not reinterpret this as intrinsic R(T) of either component.
- After the new NTC and firmware are appropriately qualified, complete a single supervised useful AIR55 temperature/hold trial and software/fault validation, not arbitrary new thermal checkpoint chains.
- Existing optional [fan-only diagnostic](../firmware/diagnostics/ntc_fan_only/ntc_fan_only.ino) is not the current priority.
