# L7987L buck redesign — complete design record

Status: work in progress, checkpoint from the redesign session on branch `redesign/buck-sourcing`.

This document is intentionally more detailed than a normal design summary. It records the electrical requirements, source material, calculations, component decisions, simulation history, invalidated evidence, current checkpoint, and exact next step so that a new chat can resume without reading the original conversation.

> Important repository note: this commit intentionally changes **only this documentation file**. Do not infer that the KiCad source has already been updated to every value documented here. The actual KiCad schematic/netlist remains authoritative for implemented connectivity. The references `U101`, `R105`, etc. below are the references used in the working redesign discussed in chat and must be checked against the current source before editing.

---

## 1. Why the buck is being redesigned

The previous design used an AP66200 24 V to 3.3 V buck regulator. The immediate reason for the redesign is sourcing risk / unavailability of the AP66200 in the preferred procurement channel. The STMicroelectronics L7987L was selected as the replacement candidate because it is designed for high-voltage industrial buses, supports the required 24 V input, provides 2 A DC capability, has adjustable switching frequency and current limit, and has broader sourcing prospects.

The redesign is being carried out on branch:

`redesign/buck-sourcing`

Preferred procurement source for the project is TME. The live purchasing sheet used during the work is:

`Filament Dryer Monitor — BOM finale Mouser`, tab `BOM TME`

The Google Sheet is a procurement aid; schematic/netlist source is authoritative for the implemented circuit.

---

## 2. Electrical requirements and design conditions

### 2.1 Actual 24 V and 3.3 V load topology

The heater and fan are **not** powered from the 3.3 V buck.

- Heater: 24 V rail (`24V_PROT`), switched low-side by Q4, approximately 1.6 A.
- Fan: 24 V rail (`24V_PROT`), switched low-side by Q5, approximately 0.2 A.
- The buck supplies the 3.3 V logic rail.
- The buck output goes to `POWER/3V3_BUCK`, then through ferrite bead FB1 to `3V3_MCU`.

3.3 V loads include the ESP32-WROOM-32E-N4, CP2102N, OLED connector, remote sensor, buzzer/logic, pull-ups and other low-voltage circuitry.

### 2.2 3.3 V load budget

**Our calculation, not an ST value.**

From the actual project loads discussed in the session:

- conservative normal 3.3 V load estimate: approximately 0.66 A;
- with SHT45 internal heater case included: approximately 0.76 A;
- design target selected for buck sizing: **1.0 A**.

The 1 A target is intentionally conservative and is the reference current used for ripple and peak-current calculations.

### 2.3 Power-supply budget

The original dryer supply is 24 V / 48 W.

**Our calculation:**

- heater: `24 V × 1.6 A = 38.4 W`;
- fan: `24 V × 0.2 A = 4.8 W`;
- buck output design target: `3.3 V × 1 A = 3.3 W`;
- ideal subtotal before buck losses: `38.4 + 4.8 + 3.3 = 46.5 W`.

Therefore the original 48 W supply has almost no margin even before regulator losses and tolerances. A stronger regulated 24 V supply will probably be required.

A **24 V / 3 A / 72 W minimum** replacement was proposed as a sensible future supply target. This is a project recommendation, not a requirement from the original dryer manufacturer.

### 2.4 Input-voltage design range

Because the final replacement PSU has not yet been selected, the redesign uses this **project design assumption**:

- nominal input: **24.0 V**;
- continuous design range: **21.6 V to 26.4 V** (`24 V ±10%`).

This ±10% range is **not** a measured or manufacturer-specified tolerance of the original 48 W PSU. It is a chosen design envelope used to avoid sizing the buck only at exactly 24.0 V.

For maximum ripple calculations, `VIN,max = 26.4 V` is used.

### 2.5 Output and switching targets

- `VOUT = 3.3 V`
- design output current: `IOUT = 1.0 A`
- nominal switching target: approximately `500 kHz`

---

## 3. ST source material used

### 3.1 Primary datasheet

STMicroelectronics:

**L7987L — 61 V, 2 A asynchronous step-down switching regulator with adjustable current limitation**  
DocID026362, **Rev. 4, May 2020, 42 pages**  
Official PDF: https://www.st.com/resource/en/datasheet/l7987l.pdf

Important exact references used in this redesign:

- p. 1/42: device headline limits/features; 4.5–61 V operating range, 2 A DC output, 250 kHz–1.5 MHz adjustable switching frequency.
- p. 6/42, §2.2, Table 1: pin descriptions.
- p. 7/42, Table 2: absolute maximum ratings.
- p. 8/42, Table 5: switching-frequency data, peak-current-limit characterization, `ISKIP = 0.5 A typ.`.
- p. 9/42, Table 5: `VFB = 0.800 V typ.`, 0.792–0.808 V at 25 °C and 0.788–0.812 V over the specified condition range.
- p. 12/42, §4.1, Figure 5, Equation 1: switching-frequency programming versus `RFSW`.
- p. 15/42, §4.2, Equations 2–3: soft-start capacitor sizing; internal soft-start current 5 µA typ.; maximum CSS about 270 nF for complete discharge behavior.
- p. 16/42, §4.3: light-load/pulse-skipping behavior.
- pp. 17–19/42, §4.5, Figure 8, Equation 6: overcurrent protection and programmable current limit.
- pp. 20–21/42, §5.1, Equations 7–9: input capacitor selection.
- pp. 21–22/42, §5.2, Equations 10–12: output capacitor and load-transient behavior.
- p. 22/42, §5.3, Equation 13: inductor sizing; ST rule of 20–40% output-current ripple; evaluate at maximum input voltage; larger L slows dynamic response.
- pp. 23–24/42, §5.4, Equations 14–19: loop model / plant model; `kFF = 30`; exact LC-filter transfer-function treatment.
- pp. 27–28/42, §5.4.2, Figure 13, Equations 24–28: Type III compensation method for MLCC output filters.
- p. 28/42, §5.4.2: choose regulator bandwidth typically `FBW < 0.2 × FSW`.
- p. 30/42, §5.6: layout considerations; short high-current input loop; FB divider close to device and away from noisy high-current paths; local VIN/VCC bypass.
- pp. 31–32/42, §6, Figure 15, Table 7: official L7987L demonstration board.

### 3.2 ST demonstration board data relevant to the redesign

The datasheet §6 demonstration board default settings are:

- `VOUT = 5 V`;
- max `IOUT = 2 A`;
- `FSW = 500 kHz`;
- `VBIAS = VOUT`;
- soft-start 5.3 ms.

Table 7 includes:

- `C1, C2 = 4.7 µF`, 1210, X7S, 100 V;
- `C3, C4 = 1 µF`, 0805, X7S, 100 V;
- `C5 = 47 µF`, 1210, 16 V, 20%;
- `C7 = 100 nF`;
- `C8 = 680 pF`;
- `C9 = 6.8 nF`;
- `C10 = 33 nF`;
- `C11 = 68 pF`;
- `R4 = 0 Ω`;
- `R5 = 910 Ω`;
- `R6 = 10 kΩ`;
- `R7 = 68 kΩ`;
- `R8 = 47 kΩ`;
- `R9 = 27 kΩ`;
- `R11 = 13 kΩ`;
- `L1 = 15 µH`, Coilcraft MSS1038-153, stated 3.86 A saturation / 50 mΩ;
- `D1 = STPS2L60UF`, 60 V / 2 A Schottky;
- `U1 = L7987L`.

**Important discrepancy resolved:** the ST datasheet demonstration board uses **15 µH**, while the specific ST eDSim/SIMPLIS `LoadTransient` example used during this session initially showed **10 µH**. These are not the same artifact and must not be conflated.

### 3.3 ST eDSim/SIMPLIS examples used

ST eDSim testbench family available in the local L7987L folder:

- `TB_L7987L_STEVAL-ISA198V1_Bode.wxsch`
- `TB_L7987L_LineTransient_Neg_BB.wxsch`
- `TB_L7987L_LoadTransient_Neg_BB.wxsch`
- `TB_L7987L_STEVAL-ISA198V1_LineTransient.wxsch`
- `TB_L7987L_STEVAL-ISA198V1_LoadTransient.wxsch`

There is **no dedicated CurrentLimit/OCP testbench** in this folder. Current-limit testing was therefore done by copying and modifying the load-transient bench.

---

## 4. L7987L pin-level design decisions

Working KiCad redesign reference is `U101 = L7987L`.

### 4.1 VBIAS

**ST data:** p. 6/42, §2.2, Table 1: VBIAS may be connected to regulated output above 3 V and should be bypassed with 1 µF ceramic when supplied by output/auxiliary rail.

**Decision:**

- `VBIAS -> VOUT`;
- `C106 = 1 µF` from VBIAS to GND.

Chosen/reused candidate for C106:

- Murata `GRJ21BC72A105KE11L`, 1 µF, 100 V, X7S, 0805.

The 100 V rating is electrically unnecessary at VBIAS = 3.3 V but allows reuse of the same robust part chosen for VCC bypass.

### 4.2 VCC

**ST data:** p. 6/42 Table 1: VCC should be bypassed to signal GND by 1 µF ceramic.

**Decision:**

- VIN to VCC through `R104 = 0 Ω`, 0603;
- `C104 = 1 µF` VCC to GND.

Candidate:

- Murata `GRJ21BC72A105KE11L`, 1 µF, 100 V, X7S, 0805.

### 4.3 EN

**ST data:** p. 6/42 Table 1: connect EN to VCC if enable control is not used.

**Decision:** `EN` connected directly to `VCC`.

### 4.4 SYNCH

No external synchronization is required.

**Decision:** leave SYNCH unconnected in the real circuit.

Earlier experimental capacitors `C101/C102` from SYNCH to GND were recognized as erroneous/unnecessary and should be removed.

### 4.5 PGOOD

PGOOD is not required by the project.

**Decision:** leave PGOOD unconnected.

Earlier PGOOD option resistors `R101/R102` are to be removed/not populated.

### 4.6 Bootstrap

**ST data:** p. 6/42, Table 1, BOOT pin description: 100 nF typical between BOOT and LX.

**Decision:**

- `C107 = 100 nF` between BOOT and LX.

Value is fixed; final MPN is not yet selected.

### 4.7 Soft start

**ST data:** p. 15/42, §4.2, Equation 2:

`CSS = ISS × TSS / VREF`

with approximately `ISS = 5 µA` and `VREF = 0.8 V`.

The ST demonstration board uses 33 nF for approximately 5.3 ms.

**Decision:**

- `C110 = 33 nF` SS to GND.

Value is fixed; final MPN is not yet selected.

### 4.8 Switching frequency

Working KiCad reference: `R108 = RFSW`.

**ST data:** p. 12/42, §4.1, Equation 1:

`FSW [kHz] = 250 + 12500 / RFSW [kΩ]`

For `RFSW = 47 kΩ`:

`FSW ≈ 250 + 12500/47 ≈ 516 kHz`.

The ST demonstration board nevertheless uses `R8 = 47 kΩ` and identifies the board as 500 kHz. Therefore 47 kΩ is treated as the practical ST reference value for the nominal 500 kHz design point.

**Decision:**

- `R108 = 47 kΩ`, 1%, 0603;
- nominal design/simulation frequency target: approximately 500 kHz.

### 4.9 Feedback divider

Working KiCad references:

- `R107 = RU`, VOUT to FB;
- `R110 = RD`, FB to GND.

`R111` was removed / no longer used as part of the divider.

ST feedback reference is 0.8 V.

**Our calculation:**

`VOUT = VFB × (1 + RU/RD)`

with:

- `VFB = 0.8 V`;
- `RU = 47.5 kΩ`;
- `RD = 15.2 kΩ`.

Then:

`VOUT = 0.8 × (1 + 47.5/15.2) = 3.300 V` approximately.

**Decision:**

- `R107 = 47.5 kΩ`, 1%, 0603;
- `R110 = 15.2 kΩ`, 1%, 0603.

Existing 47.5 kΩ Yageo `RC0603FR-0747K5L` can be reused for R107.

### 4.10 Current limit / ILIM

Working KiCad reference: `R109 = RILIM`.

ST eDSim LoadTransient reference: `R4`.

**ST data:** pp. 17–19/42, §4.5. Equation 6:

`RILIM = 27 kΩ × 3 A / ILIM`

or equivalently:

`ILIM = 3 A × 27 kΩ / RILIM`.

ST states the programmable peak-current-limit range is approximately 0.85–3.0 A. Table 5 characterizes the tolerance: for example with 27 kΩ, typical peak-current limit is 3.05 A with a broad min/max range, so the simple equation is a nominal design relation, not a precision threshold.

Chronology:

1. Initial candidate: `R109 = 39 kΩ`.
   - nominal `ILIM = 3 × 27/39 = 2.08 A`.
2. After selecting a smaller 15 µH Bourns inductor with `Isat = 2.3 A`, this was judged too close to saturation for comfort.
3. New candidate: `R109 = 47.5 kΩ`.
   - nominal `ILIM = 3 × 27/47.5 = 1.705 A`.

**Current status:** `47.5 kΩ` is the latest candidate but is **not yet fully approved**. It has been exercised in a current-limit simulation; the simulation confirmed current limiting, but the recovery overshoot was huge because that OCP test still used the original ST compensation network rather than the newly designed Type III network. The OCP test must be rerun after final compensation is installed.

---

## 5. Input capacitor design

Working KiCad input capacitor: `C103`.

**ST data:** §5.1, pp. 20–21/42. The input capacitor must be rated for maximum operating voltage and RMS input current. ST also requires local high-frequency bypassing close to VIN/VCC; §5.6 p. 30/42 explicitly calls for at least 1 µF close to VIN and VCC.

### 5.1 Selected main ceramic input capacitor

**Decision:**

- `C103 = Murata GRM32EC72A106KE05L`
- 10 µF
- 100 V
- X7S
- 1210

This part was already present in the live procurement sheet and had TME availability during the session.

### 5.2 Existing bulk input capacitor

The existing project also has a bulk electrolytic input capacitor:

- Panasonic `EEEFK1H101P`
- 100 µF
- 50 V.

The L7987L redesign did not remove the need for upstream bulk energy storage; the exact final interaction between the existing input protection/bulk network and the new local L7987L layout must be checked when the KiCad schematic and PCB are updated.

---

## 6. Output capacitor design

Working KiCad main output capacitor: `C105`.

### 6.1 Main COUT

**ST data:** §5.2 pp. 21–22/42; demonstration board Table 7 uses one 47 µF / 16 V / 1210 MLCC as C5.

**Decision, explicitly frozen during the session:**

- `C105 = Taiyo Yuden EMK325BJ476MM-P`
- 47 µF nominal
- 16 V
- X5R
- ±20%
- 1210

This is the sole main COUT in the current redesign concept.

A previous tendency to insist on a 25 V output MLCC was rejected because the rail is only 3.3 V and ST itself demonstrates a 16 V 47 µF MLCC. The chosen 16 V part is retained unless later DC-bias/availability evidence requires a change.

### 6.2 Optional extra polarized capacitor

`C112` exists as an optional VOUT-to-GND polarized footprint and is currently **DNP / optional**.

An earlier arbitrary suggestion to populate it with 100 µF was explicitly retracted. There is currently **no approved value or requirement** for C112.

### 6.3 DC-bias / ESR modeling status

The nominal 47 µF value matches the ST example well enough for the current control-loop work, but the actual effective capacitance under DC bias and precise impedance model of the Taiyo Yuden part have not yet been incorporated into the final simulations.

Do not invent an ESR for the final part. Use the manufacturer model or impedance/DC-bias data if a more exact corner analysis is required.

---

## 7. Freewheel diode

The L7987L is asynchronous and therefore requires an external Schottky freewheel diode.

### 7.1 ST reference

ST demonstration board:

- `D1 = STPS2L60UF`
- 60 V / 2 A Schottky.

### 7.2 Project choice

The exact `STPS2L60UF` package was not convenient in TME sourcing, so a same-family 60 V / 2 A SMA version was chosen:

- `D101 = STPS2L60A`
- 60 V
- 2 A
- SMA.

This remains the selected diode unless a sourcing or PCB-layout reason requires change.

---

## 8. Inductor design — complete chronology

Working KiCad reference: `L101`.

ST calculation variable/reference: `L` / `L1`.

### 8.1 ST sizing rule

**ST data:** p. 22/42, §5.3, Equation 13.

In CCM:

`L = VOUT × (1 - VOUT/VIN) / (ΔIL × FSW)`

or equivalently:

`ΔIL = VOUT × (1 - VOUT/VIN) / (L × FSW)`.

ST explicitly states:

- current ripple should normally be 20–40% of output current;
- Equation 13 should be evaluated at maximum VIN to guarantee maximum ripple;
- increasing L reduces ripple but strongly slows the response to dynamic load changes.

For project sizing:

- `VOUT = 3.3 V`;
- `VIN,max = 26.4 V`;
- `FSW = 500 kHz`;
- design `IOUT = 1 A`.

### 8.2 Inductance range implied by the ST 20–40% rule

For 40% ripple (`ΔIL = 0.4 A`):

`L = 3.3 × (1 - 3.3/26.4) / (0.4 × 500 k) ≈ 14.44 µH`.

For 20% ripple (`ΔIL = 0.2 A`):

`L ≈ 28.88 µH`.

Therefore the nominal ST-guideline window for the 1 A design point is approximately:

**14.4 µH to 28.9 µH**.

This is a design guideline range, not an absolute operating-limit range.

### 8.3 Original 8.2 µH Eaton

Previous AP66200 design used:

- Eaton `HCM0703-8R2-R`
- 8.2 µH.

At `VIN,max = 26.4 V`:

`ΔIL = 3.3 × (1 - 3.3/26.4) / (8.2 µH × 500 kHz) ≈ 0.704 A`.

Relative ripple at 1 A:

`0.704/1.0 ≈ 70.4%`.

Normal ideal peak at 1 A:

`IL,PK ≈ 1 + 0.704/2 ≈ 1.35 A`.

**Decision:** 8.2 µH is outside ST's 20–40% recommended ripple range and is **not preferred as the final L7987L design value**.

Important correction to earlier reasoning: 8.2 µH was initially described too categorically as “not usable”. Controlled eDSim tests later showed that its load-transient response was actually good. Therefore the correct conclusion is:

- 8.2 µH is not rejected because it produces an obviously bad transient;
- it is rejected primarily because its ripple is much higher than ST's stated design rule for the chosen 1 A design point.

### 8.4 Initial move to 22 µH

Using the 20–40% rule, 22 µH was initially selected as a comfortable mid-range value.

At 22 µH nominal:

`ΔIL = 3.3 × (1 - 3.3/26.4) / (22 µH × 500 kHz) = 0.2625 A`.

Relative ripple:

`26.25%`.

Normal ideal peak:

`IL,PK ≈ 1.131 A`.

With assumed −20% inductance tolerance:

- `Lmin = 17.6 µH`;
- `ΔIL ≈ 0.328 A`;
- `IL,PK ≈ 1.164 A`.

This is comfortably inside ST's ripple recommendation.

### 8.5 22 µH sourcing candidates evaluated

#### Eaton DR125-220-R

Official Eaton DR-series data used in the session:

- 22 µH;
- ±20%;
- Irms about 3.70 A;
- Isat about 4.71 A;
- DCR typical about 39.6 mΩ;
- 12.5 × 12.5 × 6 mm;
- shielded drum-core, single-coil/two-terminal part.

This was the preferred 22 µH candidate if 22 µH were retained.

#### Eaton DRQ125-220-R / DRQ127-220-R

Rejected as unnecessary for this application because DRQ is a coupled/two-coil, four-pad part. It can be configured series/parallel, but this adds topology/footprint complexity for no benefit in a simple single-inductor buck.

#### 22 µH conclusion

Electrically valid, but the approximately 12.5 mm footprint was much larger than desired. More importantly, after reading ST's warning that excess inductance slows dynamic response, the design was reopened rather than treating 22 µH as automatically optimal.

### 8.6 Why 15 µH was reconsidered

At 15 µH nominal and `VIN,max = 26.4 V`:

`ΔIL = 3.3 × (1 - 3.3/26.4) / (15 µH × 500 kHz) ≈ 0.385 A`.

Relative ripple at 1 A:

`38.5%`.

This is just inside ST's 20–40% recommendation and is therefore the smallest conventional value examined that cleanly satisfies the guideline at nominal inductance.

For a ±20% part:

- `Lmin = 12 µH`;
- `ΔIL ≈ 0.481 A`;
- `IL,PK ≈ 1 + 0.481/2 ≈ 1.24 A`.

Thus normal peak current remains modest even at the low-inductance tolerance corner.

### 8.7 Eaton DR127-150-R

TME search showed that among Eaton single-coil 15 µH parts filtered for >3 A operating current, `DR127-150-R` was one of the few available options.

Official data discussed:

- 15 µH;
- Irms about 5.03 A;
- Isat about 9.66 A;
- DCR about 24.7 mΩ;
- 12.5 × 12.5 × 8 mm.

It is electrically excellent but physically very oversized for the actual 1 A rail. The >3 A Irms filter was recognized as unnecessarily strict.

### 8.8 Revised inductor current criteria

The design does **not** need >3 A Irms just because the regulator is capable of 2 A.

For the project 1 A output target, a more rational selection criterion is:

- 15 µH nominal;
- shielded power inductor;
- single coil / two terminal;
- ±20% or better;
- Irms approximately ≥1.5–2 A;
- saturation margin high enough relative to the programmed current limit;
- low/moderate DCR;
- compact footprint and real TME availability.

### 8.9 Current preferred inductor: Bourns SRN6045-150M

Current candidate:

**Bourns `SRN6045-150M`**

Official Bourns data used:

- 15 µH;
- ±20%;
- DCR max 95.8 mΩ;
- Irms 1.9 A;
- Isat 2.3 A;
- approximately 6 × 6 × 4.5 mm;
- shielded power inductor.

Bourns defines Irms around a 40 °C temperature rise and Isat at about 30% inductance roll-off.

Official datasheet: https://www.bourns.com/docs/product-datasheets/srn6045.pdf

This part is far smaller than the Eaton DR127/DR125 family and its normal-current margins are adequate for the 1 A rail.

**Current status:** Bourns `SRN6045-150M` is the preferred L101 candidate, but final approval still depends on final ILIM/OCP validation and final loop/transient verification.

---

## 9. Compensation network — reference mapping

The compensation is Type III because the output filter is MLCC based and its ESR zero is above the desired bandwidth, matching ST §5.4.2.

### 9.1 KiCad references

Current working redesign mapping:

- `R106 = RF`
- `C109 = CF`
- `C111 = CP`
- `R105 = RS`
- `C108 = CS`

### 9.2 eDSim Bode references

In `TB_L7987L_STEVAL-ISA198V1_Bode.wxsch`:

- `RC = RF`
- `CC = CF`
- `CP = CP`
- `RC2 = RS`
- `CC2 = CS`
- `RH = RU` (upper feedback resistor)
- `RL = RD` (lower feedback resistor)
- `L1 = output inductor`
- `COUT = output capacitor`
- `RLOAD = output load`.

This reference mapping is important. During calculations, ST's `RF/CF/CP/RS/CS/RU` terminology should be used; map back to KiCad only at the beginning/end of the calculation.

---

## 10. Compensation evolution — chronological record

### 10.1 Earlier network designed around 8.2 µH

Before the inductance was reopened, the working Type III network was:

- `R105 / RS = 1.62 kΩ`
- `C108 / CS = 390 pF`
- `R106 / RF = 11.8 kΩ`
- `C109 / CF = 18 nF`
- `C111 / CP = 56 pF`

with:

- `RU = 47.5 kΩ`
- `RD = 15.2 kΩ`
- `COUT = 47 µF`
- initially `L = 8.2 µH`.

These values were generated from ST Rev. 4 §5.4.2, Figure 13, Equations 25–28, targeting approximately 60 kHz bandwidth.

### 10.2 Historical Bode simulations with 8.2 µH

These runs used the above network.

#### 8.2 µH, COUT 47 µF

- crossover approximately 47.6 kHz;
- phase margin approximately 70°;
- gain margin approximately 24 dB.

#### 8.2 µH, COUT 30 µF

- crossover approximately 71.5 kHz;
- phase margin approximately 61°;
- gain margin approximately 18 dB.

#### 8.2 µH, COUT 20 µF

- crossover approximately 101.5 kHz;
- phase margin approximately 49.3°;
- gain margin approximately 12.4 dB.

This last case was considered borderline because ST recommends `FBW < 0.2 × FSW`; at 500 kHz that guideline is 100 kHz, so 101.5 kHz is just over it and the phase margin is much lower.

### 10.3 Bode simulations after changing L to 22 µH but keeping the old compensation

The same compensation was tested with the 22 µH design and output-capacitance corners.

#### Nominal

- `L = 22 µH`
- DCR = 39.6 mΩ
- `COUT = 47 µF`
- ESR = 10 mΩ in that Bode setup
- ESL = 500 pH
- `RLOAD = 3.3 Ω`
- `RU = 47.5 kΩ`
- `RD = 15.2 kΩ`
- old Type III network unchanged.

Result:

- crossover approximately 23.4 kHz;
- phase margin approximately 67.5°;
- gain margin was not explicitly preserved in the conversation record, so it must not be invented.

#### Low-L / reduced-C corner

- `L = 17.6 µH`
- `COUT = 30 µF`

Result:

- crossover approximately 42.6 kHz;
- phase margin approximately 66.9°;
- gain margin not recorded.

#### Lower-C corner

- `L = 17.6 µH`
- `COUT = 20 µF`

Result:

- crossover approximately 61.3 kHz;
- phase margin approximately 61.2°;
- gain margin not recorded.

#### High-L corner

- `L = 26.4 µH`
- `COUT = 47 µF`

Result:

- crossover approximately 20.1 kHz;
- phase margin approximately 65.7°;
- gain margin not recorded.

Note: these corners used 22.0 µH ±20%. The exact Eaton DR125-220-R OCL value discussed later was 22.36 µH ±20%, so exact part corners would be about 17.888–26.832 µH. This difference is small but should be remembered if that part is ever reconsidered.

These 22 µH Bode runs were not invalidated by the later load-transient testbench problem because they were performed in the separate ST Bode testbench and showed physically plausible loop behavior.

### 10.4 Recalculation for the 15 µH Bourns power stage

After selecting `SRN6045-150M`, the power-stage values used for the new compensation calculation were:

- `L = 15 µH`
- `RDC = 95.8 mΩ`
- `CO = 47 µF`
- `RES = 5 mΩ` in the current ST nominal COUT model
- `RO = 3.3 Ω` corresponding to 1 A at 3.3 V
- `RU = 47.5 kΩ`
- target `FSW = 500 kHz`
- target `FBW = 60 kHz`.

**ST data:** §5.4 uses `kFF = 30` in Equation 14. §5.4.2 says Type III is appropriate when the MLCC ESR zero is above target bandwidth and recommends `FBW < 0.2 × FSW`.

At 500 kHz:

`0.2 × FSW = 100 kHz`, so 60 kHz is comfortably below the ST guideline.

Using the exact power-stage expression in ST Equation 17 with the above L/C/R values produced:

`fLC ≈ 5.91 kHz`.

Do not substitute the simple `1/(2π√LC)` formula and call it Equation 17. Equation 17 contains the load/parasitic terms; a simple LC resonance expression is only an approximation when parasitics are negligible relative to load.

Using ST §5.4.2 Equations 26–28 with `FBW = 60 kHz` produced the following theoretical compensation values:

- `RF = 16.065 kΩ`
- `CF = 16.753 nF`
- `CP = 39.63 pF`
- `CS = 566.6 pF`
- `RS = 1.124 kΩ`.

The intended pole/zero placements check as follows:

- `FZ1 ≈ 1 / (2π × RF × CF) ≈ 591 Hz ≈ 0.1 × fLC`;
- `FP1 ≈ 1 / (2π × RF × CP) ≈ 250 kHz ≈ 0.5 × FSW`;
- `FZ2 ≈ 1 / (2π × RU × CS) ≈ 5.91 kHz ≈ fLC`;
- `FP2 ≈ 1 / (2π × CS × RS) ≈ 250 kHz ≈ 0.5 × FSW`.

### 10.5 New 15 µH theoretical Bode test

Testbench saved as:

`TB_L7987L_3V3_24V_Bode_Bourns15u.wxsch`

Graph saved as:

`TB_L7987L_3V3_24V_Bode_Bourns15u_graph.svg`

Configuration:

- `L1 = 15 µH`
- `L1 ESR = 95.8 mΩ`
- `COUT = 47 µF`
- `COUT ESR = 5 mΩ`
- `COUT RLK = 1 MΩ`
- `RLOAD = 3.3 Ω`
- `RH = 47.5 kΩ`
- `RL = 15.2 kΩ`
- `RC = 16.065 kΩ`
- `CC = 16.753 nF`
- `CP = 39.63 pF`
- `RC2 = 1.124 kΩ`
- `CC2 = 566.6 pF`.

Note on FSW in this Bode bench: the symbol is `L7987L_for_Bode`; the exposed `FSW` pin is left unconnected in the ST Bode schematic. Do not add the real-circuit RFSW to this Bode macro without understanding the model. The compensation calculation itself is still based on the STEVAL 500 kHz operating point.

**Simulation result from the SVG:**

- crossover `fc ≈ 59.3 kHz`;
- phase margin `PM ≈ 64.5°`;
- gain margin `GM ≈ 19.6 dB`;
- the target of approximately 60 kHz was therefore achieved very closely.

This theoretical compensation network is a successful design point.

### 10.6 Proposed commercial rounding that was NOT accepted yet

A first E-series rounding was proposed:

- `RF / RC: 16.065 kΩ -> 16.2 kΩ`
- `CF / CC: 16.753 nF -> 16 nF`
- `CP: 39.63 pF -> 39 pF`
- `RS / RC2: 1.124 kΩ -> 1.13 kΩ`
- `CS / CC2: 566.6 pF -> 560 pF`.

This set was **not simulated** and must not be treated as final.

The user checked live TME availability and found that **16 nF MLCC is effectively unavailable in the desired form**. Therefore the `16 nF` rounded choice is discarded.

`1.13 kΩ` is an ordinary E96 value; it is not inherently unusual, but final TME MPN/stock still must be checked.

### 10.7 Current commercial-compensation direction

The next obvious standard capacitor near theoretical `CF = 16.753 nF` is 18 nF:

- 15 nF is about −10.5% from theoretical;
- 18 nF is about +7.4% from theoretical.

`18 nF` is therefore the likely anchor value, but **the network has not yet been recalculated around 18 nF and no TME MPN set has been approved**.

Do not simply replace CC with 18 nF and call the compensation complete. The exact next step is documented at the end of this file.

---

## 11. eDSim/SIMPLIS load-transient work — complete chronology

### 11.1 Original ST LoadTransient bench

ST file:

`TB_L7987L_STEVAL-ISA198V1_LoadTransient.wxsch`

The original note describes:

- `VIN = 48 V`
- `VOUT = 5 V`
- load current switching `500 mA -> 2 A`
- nominal `f = 500 kHz`
- STEVAL-ISA198V1 example.

The load was implemented with approximately:

- `ROFF = 10 Ω`
- `RON = 2.5 Ω`.

Original ST component references relevant to later modifications include:

- `R4 = 27 kΩ` on ILIM;
- `RFSW = 47 kΩ`;
- `CSS = 33 nF`;
- `CBOOT = 100 nF`;
- original compensation around `R6 = 10 kΩ`, `C9 = 6.8 nF`, `C11 = 68 pF`, plus the other ST network elements;
- original feedback around `R7 = 68 kΩ`, `R11 = 13 kΩ` for 5 V;
- eDSim transient example initially showed `L1 = 10 µH`.

The input source `V2` was a PWL source with entries including 48 V from 100 ns onward.

### 11.2 Invalid early modified-transient sequence

Several early simulations were modified too aggressively before validating the untouched ST example. These produced implausible low-load regulation and were incorrectly interpreted as physical pulse-skipping behavior.

Examples included:

- 100 mA -> 1 A;
- 400 mA -> 1 A;
- 100 mA -> 800 mA;
- later 100 mA -> 800 mA and 150 mA -> 800 mA with 22 µH.

Some runs showed VOUT excursions of roughly 2–4 V on a 3.3 V rail even before/around the step. These results are **invalid as design evidence** because the modified testbench setup had not been validated.

Two important corrections came from this failure:

1. The earlier derivation of a fixed “pulse-skipping threshold” as `ISKIP / 2 = 250 mA` was invalid. ST §4.3 states pulse skipping occurs when output current is lower than half the inductor ripple and separately describes charging to `ISKIP`; it does **not** state that the load boundary is `ISKIP/2`.
2. When a modified model gives behavior grossly inconsistent with ST's own demonstrated regulation, the setup must be treated as suspect before inventing a physical explanation.

The correct methodology was restarted from a clean ST testbench and thereafter only one variable was changed at a time.

### 11.3 Clean ST baseline validation

The untouched/restored ST testbench was rerun first.

Observed baseline:

- load approximately 0.5 A -> 2 A at about 6.5 ms;
- return 2 A -> 0.5 A at about 7.5 ms;
- VOUT nominal approximately 4.98–4.99 V;
- `VOUT,min ≈ 4.904 V` at about 6.504 ms;
- `VOUT,max ≈ 5.059 V` at about 7.505 ms;
- `IL,peak ≈ 2.59 A`.

This established that the ST/SIMPLIS model and original bench behaved sensibly.

### 11.4 Divider-only change: 5 V -> 3.3 V

Only the feedback divider was changed:

ST testbench references:

- `R7` upper resistor: `47.5 kΩ`;
- `R11` lower resistor: `15.2 kΩ`.

All other ST values were intentionally left unchanged, including 48 V VIN and 10 Ω / 2.5 Ω load resistances.

Because the output was now 3.3 V, the unchanged resistor load naturally became:

- low load: `3.3/10 = 0.33 A`;
- high load: `3.3/2.5 = 1.32 A`.

This was intentional for isolation of the feedback change and is not an error.

Observed result:

- `VOUT,min ≈ 3.247 V`;
- `VOUT,max ≈ 3.357 V`;
- deviations roughly −1.6% / +1.7%;
- `IL,peak ≈ 1.84 A`.

Conclusion: changing the feedback divider alone did not destabilize the ST example.

### 11.5 VIN-only change: 48 V -> 24 V

Next, only `V2` was changed from 48 V to 24 V.

PWL source entries were changed so the 48 V entries became 24 V while the `t=0, V=0` entry was left unchanged.

With all other values unchanged:

- load remained approximately 0.33 A -> 1.32 A;
- `VOUT,min ≈ 3.2466 V`;
- `VOUT,max ≈ 3.3534 V`;
- deviation about ±1.62%;
- `IL,peak ≈ 1.726 A`.

Conclusion: moving the example to the real nominal 24 V bus caused no problem.

### 11.6 L-only test: 8.2 µH

Next only L1 was changed:

- `L1 = 8.2 µH`;
- ESR was intentionally left unchanged for this comparative run.

Observed:

- `VOUT,min ≈ 3.2490 V`;
- `VOUT,max ≈ 3.3483 V`;
- `IL,peak ≈ 1.749 A`.

This run demonstrated that 8.2 µH gives a perfectly respectable load transient in this particular test. It therefore invalidated the earlier claim that 8.2 µH was intrinsically unacceptable because of transient behavior.

It did **not** eliminate the separate 70% ripple objection from ST §5.3.

### 11.7 L-only test: 15 µH, ESR unchanged

Next:

- `L1 = 15 µH`;
- the previous ESR was left unchanged for one isolated comparison.

From the SVG, approximate values were:

- `VOUT,min ≈ 3.22 V`;
- `VOUT,max ≈ 3.36 V`;
- `IL,peak ≈ 1.61 A`.

This illustrated the tradeoff described by ST: higher inductance lowers peak/ripple current but can slow the transient response slightly.

### 11.8 Bourns-realistic L test

Next the 15 µH inductor ESR was changed to the Bourns datasheet value:

- `L1 = 15 µH`;
- `ESR/DCR = 95.8 mΩ`.

Observed from SVG:

- `VOUT,min ≈ 3.234 V`;
- `VOUT,max ≈ 3.371 V`;
- `IL,peak ≈ 1.62 A`.

Conclusion: adding the Bourns DCR did not produce a concerning change. This became the preferred inductor model for later work.

---

## 12. Current-limit simulation

Because ST provided no dedicated OCP bench, a copy of the LoadTransient testbench was used.

Save name established during the session:

`TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5.wxsch`

Graph name:

`TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5_graph.svg`

### 12.1 Configuration

Based on the Bourns load-transient bench:

- `VIN = 24 V`;
- feedback for 3.3 V;
- `L1 = 15 µH`;
- `L1 ESR = 95.8 mΩ`;
- original ST compensation still present;
- `R4` (ST ILIM resistor) changed from 27 kΩ to **47.5 kΩ**;
- `ROFF = 10 Ω`;
- `RON` changed from 2.5 Ω to **1.5 Ω**.

At 3.3 V, a 1.5 Ω load would demand ideally:

`3.3/1.5 = 2.2 A`,

which is intentionally above the proposed current-limit region.

### 12.2 Expected nominal current limit

From ST Equation 6:

`ILIM = 3 A × 27/47.5 ≈ 1.705 A` nominal.

### 12.3 Simulation result

Observed qualitatively/approximately from the SVG:

- overcurrent protection clearly intervened;
- inductor current during the overload was held roughly around the 1.4–1.6 A region rather than following the ideal 2.2 A load request;
- VOUT collapsed to roughly 2.1 V during the overloaded interval;
- when the heavy load was released, VOUT showed a very large overshoot, visually around 7.2–7.3 V.

### 12.4 Interpretation

The first part is useful evidence that `RILIM = 47.5 kΩ` does invoke current limiting and keeps current comfortably below the Bourns 2.3 A saturation specification in this simulation.

The huge release overshoot must **not** be taken as the final circuit behavior because this OCP test still used the original ST 5 V/STEVAL compensation network, while VOUT, VIN and L had already been changed. The loop had not yet been retuned to the new Type III design.

Therefore:

- `R109/RILIM = 47.5 kΩ` remains a candidate;
- the OCP test is preliminary;
- final OCP/recovery behavior must be retested after final compensation values are installed.

---

## 13. Important simulation/modeling lessons and discarded reasoning

### 13.1 Do not infer a load boundary from ISKIP/2

ST Table 5 gives `ISKIP = 0.5 A typ.` and §4.3 describes DCM pulse-skipping behavior, but the load boundary is stated as output current below half the **inductor ripple**, not `ISKIP/2`.

The earlier `250 mA` “threshold” conclusion was retired.

### 13.2 Do not over-trust arbitrary COUT ESR/ESL assumptions

An earlier suggestion to model COUT as a made-up combination such as 100 µF / 50 mΩ / 1 nH was retracted. Final modeling should use the actual chosen capacitor and manufacturer data when precision matters.

### 13.3 ST 20–40% ripple is a design rule, not an operating hard limit

The controlled 8.2 µH transient shows that operation can look fine even with ripple above 40%. However, the project should not ignore an explicit ST design guideline without a specific reason.

The final design therefore moved toward 15 µH because it satisfies the guideline while avoiding the unnecessary size and slower response of 22 µH.

### 13.4 Distinguish ST demonstration board from eDSim examples

- Datasheet demonstration board: L1 = 15 µH.
- eDSim LoadTransient example used in this session: L1 initially 10 µH.

Statements such as “ST uses 10 µH” must specify that they refer to the eDSim transient example, not the datasheet demonstration board.

### 13.5 Use exact ST reference names while calculating

For compensation and feedback calculations, use ST notation (`RU`, `RF`, `CF`, `CP`, `RS`, `CS`, etc.) and only map to KiCad at the start/end. This avoids repeated confusion between project reference designators and datasheet/testbench names.

---

## 14. Current proposed KiCad values

These are the latest design values/proposals from the session. “Selected” means the design currently intends to use it; “candidate” means it still requires a specific validation step.

| KiCad ref | Function | Latest value / part | Status |
|---|---|---|---|
| U101 | buck IC | ST L7987L, HTSSOP16 | selected |
| C103 | main local VIN ceramic | Murata GRM32EC72A106KE05L, 10 µF 100 V X7S 1210 | selected |
| R104 | VIN->VCC link | 0 Ω 0603 | selected |
| C104 | VCC bypass | Murata GRJ21BC72A105KE11L, 1 µF 100 V X7S 0805 | selected |
| C105 | main COUT | Taiyo Yuden EMK325BJ476MM-P, 47 µF 16 V X5R 1210 ±20% | selected |
| C106 | VBIAS bypass | 1 µF, same Murata candidate as C104; VBIAS tied to VOUT | selected concept |
| C107 | bootstrap | 100 nF BOOT-LX | value selected, MPN pending |
| C110 | soft start | 33 nF SS-GND | value selected, MPN pending |
| R107 | feedback upper / RU | 47.5 kΩ 1% | selected |
| R110 | feedback lower / RD | 15.2 kΩ 1% | selected |
| R108 | RFSW | 47 kΩ 1% | selected |
| R109 | RILIM | 47.5 kΩ 1% | **candidate; final OCP retest pending** |
| D101 | freewheel diode | STPS2L60A, 60 V / 2 A SMA | selected |
| L101 | inductor | Bourns SRN6045-150M, 15 µH ±20%, DCR max 95.8 mΩ, Irms 1.9 A, Isat 2.3 A | **preferred candidate** |
| C112 | optional polarized COUT | DNP | optional/not selected |
| R106 | Type III RF | 16.065 kΩ theoretical | final commercial value pending |
| C109 | Type III CF | 16.753 nF theoretical | final commercial value pending; 16 nF commercial proposal rejected due TME availability |
| C111 | Type III CP | 39.63 pF theoretical | final commercial value pending |
| R105 | Type III RS | 1.124 kΩ theoretical | final commercial value pending |
| C108 | Type III CS | 566.6 pF theoretical | final commercial value pending |

Connections also selected:

- EN -> VCC;
- VBIAS -> VOUT;
- SYNCH NC;
- PGOOD NC;
- BOOT cap directly BOOT-LX;
- exposed pad / GND layout to follow ST signal/power-ground guidance.

---

## 15. What is already validated

### 15.1 Validated from ST documentation

- L7987L is suitable for a 24 V bus and 3.3 V output.
- 47 kΩ RFSW is consistent with the ST 500 kHz demonstration-board design.
- 33 nF soft-start and 100 nF bootstrap follow ST reference values.
- VBIAS may be connected to 3.3 V VOUT with 1 µF bypass.
- Type III compensation is appropriate for the MLCC output filter.
- ST's inductor guideline is 20–40% ripple and should be checked at maximum VIN.

### 15.2 Validated by calculation

- 47.5 kΩ / 15.2 kΩ feedback gives approximately 3.300 V.
- 8.2 µH gives approximately 70.4% ripple at 1 A / 26.4 V and is outside the preferred ST range.
- 15 µH gives approximately 38.5% nominal ripple and is inside the ST range.
- 15 µH at −20% gives about 0.481 A ripple and about 1.24 A normal peak current.
- 22 µH gives about 26.3% ripple but is not required to satisfy the guideline.
- 47.5 kΩ RILIM corresponds nominally to about 1.705 A peak limit by Equation 6.

### 15.3 Validated in SIMPLIS/eDSim

- Clean ST LoadTransient example works correctly.
- Divider-only change to 3.3 V works.
- VIN-only change to 24 V works.
- 8.2 µH transient is good in the controlled test, proving it was wrong to reject 8.2 µH on transient behavior alone.
- 15 µH Bourns model with 95.8 mΩ DCR gives a reasonable transient.
- New theoretical Type III network for 15 µH / 47 µF produces about 59.3 kHz crossover, 64.5° phase margin and 19.6 dB gain margin.
- 47.5 kΩ current-limit resistor does activate current limiting in the modified OCP test.

---

## 16. What is NOT yet validated / still open

1. **Final commercial Type III components from TME.**
   - The theoretical network is validated.
   - The first rounded network was not validated because 16 nF MLCC was unavailable in the required TME search.
   - Need an actual available set of MPNs and then a Bode run with exactly those values.

2. **Final current-limit behavior.**
   - RILIM 47.5 kΩ is only a candidate.
   - Rerun the overload/OCP test after final compensation is installed.
   - Check recovery overshoot with the real compensation network.

3. **Final Bourns inductor approval.**
   - Normal current/ripple and transient behavior are satisfactory.
   - Final approval depends on final ILIM/OCP and compensation verification.

4. **Actual COUT effective capacitance / DC bias corner.**
   - Current loop work uses nominal 47 µF.
   - Manufacturer DC-bias model/data should be used for final worst-case if needed.

5. **Compensation corners with final TME values.**
   - Need nominal and sensible L/C tolerance corners after commercial parts are fixed.

6. **Line transient.**
   - ST line-transient testbench exists but has not yet been adapted to the final project design in this documented sequence.

7. **Final KiCad implementation and layout.**
   - This documentation commit does not modify the schematic or PCB.
   - Need to apply the approved values/connectivity to the actual KiCad branch and then rerun ERC/DRC and layout review.

8. **Final PSU selection.**
   - 24 V / 3 A is a project recommendation, not yet a chosen part.

---

## 17. Exact next step

Do **not** run the previously proposed `StdComp` values with a 16 nF MLCC.

The next step is:

1. Search the **live TME catalog** for actually available 0603 compensation parts close to the theoretical network:
   - `RF theoretical = 16.065 kΩ`;
   - `CF theoretical = 16.753 nF`;
   - `CP theoretical = 39.63 pF`;
   - `RS theoretical = 1.124 kΩ`;
   - `CS theoretical = 566.6 pF`.
2. Use **18 nF as the first CF candidate** because 16 nF MLCC was found unavailable and 18 nF is closer to 16.753 nF than 15 nF.
3. Select actual TME MPNs/values for all five compensation parts; do not assume 1.13 kΩ or any capacitor value is stocked until checked live.
4. Recalculate/check pole/zero placements with the actual commercial values.
5. Copy the validated theoretical Bode testbench and save the commercial-value version as:

   `TB_L7987L_3V3_24V_Bode_Bourns15u_TMEComp.wxsch`

6. Run Bode with exactly the selected TME values and save the graph as:

   `TB_L7987L_3V3_24V_Bode_Bourns15u_TMEComp_graph.svg`

7. Compare crossover, phase margin and gain margin against the theoretical result (`59.3 kHz`, `64.5°`, `19.6 dB`).
8. Only after the commercial compensation passes, copy those values into the load-transient bench and rerun normal load transient, then rerun the saved OCP/current-limit case.

---

## 18. Simulation file naming convention going forward

The user explicitly requested that every future simulation step include an exact save name.

Established files:

- OCP/current-limit bench:  
  `TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5.wxsch`
- OCP graph:  
  `TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5_graph.svg`
- theoretical 15 µH Bode bench:  
  `TB_L7987L_3V3_24V_Bode_Bourns15u.wxsch`
- theoretical Bode graph:  
  `TB_L7987L_3V3_24V_Bode_Bourns15u_graph.svg`

Next commercial compensation files should use:

- `TB_L7987L_3V3_24V_Bode_Bourns15u_TMEComp.wxsch`
- `TB_L7987L_3V3_24V_Bode_Bourns15u_TMEComp_graph.svg`

Continue this naming style for later line/load/OCP tests so that the operating point and major hardware choices are encoded in the filename.

---

# Current checkpoint

**Resume here:**

- Target: 24 V nominal -> 3.3 V / 1 A, L7987L, ~500 kHz.
- Design VIN envelope: 21.6–26.4 V (project assumption).
- Preferred inductor: **Bourns SRN6045-150M, 15 µH, DCR max 95.8 mΩ, Irms 1.9 A, Isat 2.3 A**.
- Main COUT: **Taiyo Yuden EMK325BJ476MM-P, 47 µF / 16 V**.
- Feedback: **47.5 kΩ / 15.2 kΩ**.
- RFSW: **47 kΩ**.
- RILIM: **47.5 kΩ candidate**, final OCP recovery retest pending.
- Theoretical Type III: **RF 16.065 kΩ, CF 16.753 nF, CP 39.63 pF, RS 1.124 kΩ, CS 566.6 pF**.
- Theoretical Bode is good: **fc ≈ 59.3 kHz, PM ≈ 64.5°, GM ≈ 19.6 dB**.
- First rounded compensation set was **not** accepted because **16 nF MLCC is unavailable on TME**.
- **Exact next action:** choose live-TME commercial MPNs for the five Type III parts, likely starting from **CF = 18 nF**, recalculate/check the pole/zero placements, then run `TB_L7987L_3V3_24V_Bode_Bourns15u_TMEComp.wxsch` and compare against the theoretical Bode.
