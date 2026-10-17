# Heater NTC versus nearby Sanwa reference: diagnostic study

**Date:** 2026-10-17  
**Scope:** First-article open-bench measurements; diagnostics only. **No production heater-control conversion or independent thermal protection decision is changed.**

## Measurement context and meaning

The embedded heater NTC was read on J7/ESP32 GPIO34 through the calibrated 47 kOhm nominal divider. The separately mounted Sanwa RD701/K-type probe read a **nearby heater contact point**, not the thermistor bead itself. The original eSUN eBOX Lite **bottom closure and top lid were both open and the fan OFF** for the series. Hold duration was 120 s at each nominal NTC test target.

The numbers below map **measured NTC resistance to *nearby Sanwa-probe temperature*** in this setup. They **do not** establish the NTC's true internal temperature, its R25 part rating, or performance in the final assembled/ventilated enclosure. The Sanwa instrument/probe also has absolute accuracy and thermal-contact/placement limitations. These measurements must not define heater trip limits or be extrapolated to the OEM plate reference of 105/116/118 C.

| Sanwa near-heater contact (C) | NTC observed (kOhm) | Prior heating-sketch indication (C, approximate) |
| ---: | ---: | ---: |
| 30 | 64.62 | 30.89 |
| 39 | 42.27 | 40.69 |
| 47 | 28.72 | 50.07 |
| 56 | 19.68 | 60.00 |
| 64 | 13.73 | 70.04 |
| 74 | 9.81 | 79.97 |
| 85 | 7.12 | 90.00 |

At 90 C indicated, the full serial hold ended normally at ~531.2 s, R=7.12 kOhm, with independent Sanwa reading 85 C. The heater ON command was present in ~110 of 121 one-second HOLD log samples (rough duty proxy), with enclosure fully open and fan OFF.

## Model comparison

**A. Single beta fitted to Sanwa contact.** Least-squares regress ln(R[Ohm]) against inverse **Sanwa** T[K] at all seven points. Result: **beta = 4421.571 K**, **R25 = 80690.93 Ohm**. Fitted contact temperatures for the seven resistances:

| Sanwa (C) | Single-beta predicted (C) | Predicted minus observed (C) |
| ---: | ---: | ---: |
| 30 | 29.53 | -0.47 |
| 39 | 38.59 | -0.41 |
| 47 | 47.32 | +0.32 |
| 56 | 56.35 | +0.35 |
| 64 | 65.43 | +1.43 |
| 74 | 74.38 | +0.38 |
| 85 | 83.36 | -1.64 |

Internal in-sample max absolute residual **1.64 C**, RMSE **0.89 C**. This is **not** temperature measurement accuracy; the reference probe uncertainty is excluded and the fit intentionally treats two physically distinct thermal sites as one.

**B. Piecewise log-resistance lookup.** At measured bounding resistances R_hi and R_lo, define

```
f = ln(R_hi / R) / ln(R_hi / R_lo)
T_Sanwa_equivalent = T_lo + f * (T_hi - T_lo)
```

It exactly returns each of the seven Sanwa observations *by construction* (zero training residual, not independent validation). A simple leave-one-out interpolation of the five *internal* reference points using the next remaining immediate neighbors gives errors about **-0.10, +0.59, -0.29, +1.31, +0.75 C**, respectively, RMSE **0.74 C**, maximum **1.31 C**. This is a sparse-data smoothness check, not a measurement-accuracy claim; two endpoints cannot be interpolated after their removal without extrapolation.

**Selection:** For purely bench-side presentation of a *Sanwa-equivalent* contact temperature, use **B**, only within the observed **7.12–64.62 kOhm** range (contact readings 30–85 C). No extrapolation beyond either endpoint. For research comparison, expose model A separately, also constrained to the measured range. Do **not** replace `firmware/NtcSensor.h`'s current NTC temperature field or use B for closed-loop heater control: it is a translation between two distinct sensor locations whose offset can change under different airflow, enclosure and power conditions.

## Cold-end discrepancy and plausible alternative NTC identity

Earlier fully cold observations with ambient air (SHT45) and independent Sanwa near 20 C:

- Direct DMM on disconnected NTC **122.3 kOhm**.
- ESP32 divider with NTC plugged into J7 **~124.4 kOhm** (about 1.7% higher than direct reading).
- Seven-hot-point regression (A) extrapolated to 20 C predicts **103.91 kOhm**, and would translate **122.3 kOhm** to **16.87 C**. This discrepancy is **much larger** than the ~1.7% difference between the two resistance readout methods.
- **Alternative hypothesis, NOT a verified component specification:** a conventional **100 kOhm at 25 C / beta=3950 K** thermistor would predict **125.35 kOhm at 20 C**. The direct 122.3 kOhm is **2.44% below this**; J7's ~124.4 kOhm is ~0.76% below. That plausible match is consistent with the embedded thermistor being physically hotter than the externally attached Sanwa when heater power is applied. For this *hypothetical* thermistor model, 64.62 kOhm would correspond to internal NTC ~35.2 C while the Sanwa reads 30 C; 7.12 kOhm would correspond to NTC ~99.3 C while Sanwa reads 85 C. These apparent thermal gradients are not independently confirmed, especially given open-cover fan-off geometry.
- Conversely, the repository also records **82.5 kOhm at nominal 25 C from the donor controller cooldown**. That historical temperature was **not independently measured at the bead** with the present calibrated setup. It is a constraint to revisit, not evidence to discard. The cold reading by itself cannot decide between thermistor type, probe temperature gradient, ambient equilibration history, model nonlinearity and old-reference errors.

**Do not identify a 100k/B3950 part as established.** No thermistor part-number/datasheet or independent two-temperature bead measurement has resolved this.

## Isolated optional diagnostic sketch

`firmware/diagnostics/ntc_reference_viewer/ntc_reference_viewer.ino`

- Passive ADC-only viewer: GPIO19 heater forcibly LOW and GPIO16 fan LOW, no heater-enable/temperature-control function.
- Uses the first-article calibrated ADC parameters exactly as `AppConfig.h`: slope 0.984369, offset -2.322 mV, divider supply 3311 mV, pullup 46572 Ohm, 16 samples, 12-bit 11dB and 150..3000 mV validity window.
- Outputs NTC resistance, current application temperature conversion (empirical old table, outside beta4100/R25 82500), **old standalone heating-test** conversion (50 C anchored above 50 C), piecewise log-R bench Sanwa-equivalent (only measured interval), 7-point-beta contact prediction (only measured interval), and alternative **100k/B3950 thermistor hypothesis**.
- At room-temperature resistance ~122 kOhm, the Sanwa-equivalent field deliberately prints **`OUT_OF_MEASURED_RANGE`**, instead of inventing a room calibration.
- The viewer is *separate* from `firmware/firmware.ino`. Its existence does not turn on the main firmware heater, change `SafetyController.h`, or write any updated curve to production control.
- Not yet uploaded to the PCB/physically validated; the calculations were checked analytically using the seven fixed points.

## Next useful validation without heating

If further accuracy is necessary, wait for the heater assembly to equilibrate to a known ambient temperature, then take **one paired NTC resistance and independently measured temperature at/near the thermistor body**, with heater OFF. Check the original donor 25 C/82.5k entry and the hypothetical 100k/B3950 curve against that observation. A second independent equilibrium temperature, if easily available, could distinguish beta/R25, but **no repeated hot plateau tests are requested at this stage**.

The eventual **closed box + OEM-running fan** qualification separately needs SHT45 chamber-air readings, thermistor/plate location comparison, component/housing thermal checks and an independently appropriate thermal cutoff. The installed **105 C one-shot TCO is not validated for reaching OEM 105–118 C nominal plate temperatures**; never bypass it merely to reach those values.
