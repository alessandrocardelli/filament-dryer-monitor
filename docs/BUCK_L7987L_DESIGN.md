# L7987L buck redesign — complete design record

Status: **work in progress**. This file is the authoritative handoff for the L7987L redesign work performed in chat up to the current checkpoint on branch `redesign/buck-sourcing`.

This is intentionally not a short summary. It records requirements, ST source references, calculations, component decisions, rejected alternatives, simulation setup, simulation history, invalidated reasoning, current values, open items, and the exact next step.

> **Repository-state warning**
>
> This commit changes **only this documentation file**. The repository branch still documents the AP66200 as the implemented buck and the L7987L as a redesign under evaluation. Do not infer that the KiCad source has already been updated to all values listed here. The actual KiCad schematic/netlist remains authoritative for implemented connectivity.
>
> During the redesign discussion, a working KiCad block using references `U101`, `L101`, `R105`… was developed locally/in chat. Those references are documented below because they are needed to continue the work, but they must be checked against the actual branch source before the KiCad implementation is committed.

---

## 0. Repository/bootstrap notes

Repository: `alessandrocardelli/filament-dryer-monitor`

Working branch: `redesign/buck-sourcing`

At the time of this documentation update:

- `AGENTS.md` is **not present** in the repository.
- `docs/PROJECT_STATE.md` is **not present**.
- `docs/DECISIONS.md` is **not present**.
- `docs/TODO.md` is **not present**.
- `README.md` still states that the implemented buck is the AP66200 and that a sourcing-risk replacement is under evaluation.
- `hardware/docs/PROCUREMENT.md` still states that `AP66200FVBW-13` is the actual design part and that L7987L is only a candidate until explicitly approved/implemented.

Procurement source used during this work:

- Google Sheet: **Filament Dryer Monitor — BOM finale Mouser**
- tab: **BOM TME**
- URL: `https://docs.google.com/spreadsheets/d/14pd4d5PjS7goH_W73SR9czaZ9BhpDt5480ON451nyR0`

TME is the preferred supplier, but availability/stock is transient and must be rechecked before ordering.

---

# 1. Why the buck is being redesigned

The current/previous design uses a Diodes Incorporated AP66200 24 V -> 3.3 V synchronous buck. The AP66200 became a sourcing risk: TME had no stock and availability at other distributors was inconsistent.

The redesign candidate selected for detailed work is:

- **STMicroelectronics L7987L**
- 4.5–61 V input operating range
- 2 A DC output capability
- asynchronous architecture
- adjustable switching frequency
- programmable current limit
- HTSSOP16 package with exposed pad

The goal is not simply to make the L7987L operate, but to produce a robust 24 V -> 3.3 V rail for the actual dryer electronics with components that are realistically procurable, preferably from TME.

---

# 2. Electrical requirements and project assumptions

## 2.1 Actual load topology

A major correction made during the redesign was to separate the 24 V loads from the 3.3 V buck load.

The heater and fan are **not powered by the 3.3 V buck**.

Actual topology:

- Heater: `24V_PROT`, switched low-side by Q4, approximately **1.6 A at 24 V**.
- Fan: `24V_PROT`, switched low-side by Q5, approximately **0.2 A at 24 V**.
- Buck output: logic 3.3 V rail.
- Existing architecture: `POWER/3V3_BUCK` -> ferrite bead FB1 -> `3V3_MCU`.

3.3 V loads include ESP32-WROOM-32E-N4, CP2102N, OLED/interface circuitry, SHT45 remote sensor rail, buzzer/logic, pull-ups, etc.

## 2.2 3.3 V load budget

**Project calculation, not an ST value.**

The conservative estimates established during the redesign were:

- normal 3.3 V load estimate: about **0.66 A**;
- with SHT45 internal heater case included: about **0.76 A**;
- design target adopted for buck sizing: **1.0 A**.

Therefore all ripple and normal peak-current sizing calculations use:

`IOUT,design = 1.0 A`

This is intentionally conservative.

## 2.3 Original PSU power budget

Known original supply:

- **24 V / 48 W**.

Project calculation:

- heater: `24 V × 1.6 A = 38.4 W`;
- fan: `24 V × 0.2 A = 4.8 W`;
- buck output design target: `3.3 V × 1 A = 3.3 W`;
- ideal subtotal before buck losses: `38.4 + 4.8 + 3.3 = 46.5 W`.

Conclusion: the original 48 W supply has essentially no useful margin before regulator losses, PSU tolerance, startup/transient behavior, etc.

A future **24 V / 3 A / 72 W minimum** regulated supply was proposed as a sensible project target. This is a project recommendation, **not** a specification from the original dryer manufacturer and no exact replacement PSU has yet been selected.

## 2.4 Input-voltage design envelope

Because the final PSU is not yet fixed, the following was adopted as a **project design assumption**:

- nominal VIN: **24.0 V**;
- continuous design range: **21.6–26.4 V** (`24 V ±10%`).

Important: this ±10% range is **not** a verified tolerance of the original eSUN supply. It is an engineering envelope chosen for the redesign.

For inductor maximum-ripple calculation:

`VIN,max = 26.4 V`

## 2.5 Output and frequency targets

- `VOUT = 3.3 V`
- `IOUT,design = 1.0 A`
- nominal switching target: approximately **500 kHz**

---

# 3. ST documentation used

## 3.1 Primary datasheet

STMicroelectronics:

**L7987L — 61 V, 2 A asynchronous step-down switching regulator with adjustable current limitation**

- DocID026362
- **Rev. 4**
- **May 2020**
- **42 pages**
- official PDF: `https://www.st.com/resource/en/datasheet/l7987l.pdf`

Exact datasheet references used during the work:

- **p. 1/42** — headline operating range/features: 4.5–61 V input, 2 A DC, 250 kHz–1.5 MHz adjustable switching frequency.
- **p. 6/42, §2.2, Table 1** — pin descriptions and required external connections.
- **p. 7/42, Table 2** — absolute maximum ratings.
- **p. 8/42, Table 5** — switching/current-limit data; `ISKIP = 0.5 A typ.`.
- **p. 9/42, Table 5** — feedback reference `VFB = 0.800 V typ.`; 0.792–0.808 V at 25 °C and 0.788–0.812 V over the stated operating range.
- **p. 11/42, §4 Functional description** — VOUT is sensed at FB and compared to the internal 0.8 V reference.
- **p. 12/42, §4.1, Figure 5, Eq. 1** — switching-frequency programming with `RFSW`.
- **p. 15/42, §4.2, Eq. 2–3** — soft-start sizing; `ISS ≈ 5 µA typ.`; practical CSS limit discussed.
- **p. 16/42, §4.3** — light-load / pulse-skipping behavior.
- **pp. 17–19/42, §4.5, Figure 8, Eq. 6** — overcurrent protection / programmable peak current limit.
- **pp. 20–21/42, §5.1, Eq. 7–9** — input-capacitor selection.
- **pp. 21–22/42, §5.2, Eq. 10–12** — output capacitor and transient-energy behavior.
- **p. 22/42, §5.3, Eq. 13** — inductor sizing; ST recommends current ripple around **20–40% of output current**, evaluated at maximum VIN; increasing L reduces ripple but strongly slows dynamic load response.
- **pp. 23–24/42, §5.4, Eq. 14–19** — plant / loop model, including `kFF = 30`; exact LC-filter formulation.
- **pp. 27–28/42, §5.4.2, Figure 13, Eq. 24–28** — Type III compensation procedure.
- **p. 28/42, §5.4.2** — bandwidth recommendation `FBW < 0.2 × FSW`.
- **p. 30/42, §5.6** — PCB layout guidance.
- **pp. 31–32/42, §6, Figure 15, Table 7** — official demonstration-board schematic/BOM.

## 3.2 Official demonstration-board values used as reference

The datasheet §6 demonstration board is a 5 V / 2 A example at approximately 500 kHz.

Relevant values from Figure 15 / Table 7:

- `U1 = L7987L`
- `D1 = STPS2L60UF`, 60 V / 2 A Schottky
- `L1 = 15 µH`, Coilcraft MSS1038-153
- `C5 = 47 µF`, 16 V, 1210 output MLCC
- `C7 = 100 nF` bootstrap
- `C10 = 33 nF` soft start
- `R8 = 47 kΩ` frequency programming
- `R9 = 27 kΩ` ILIM programming
- `R7 = 68 kΩ`, `R11 = 13 kΩ` feedback for 5 V
- compensation values around `R5 = 910 Ω`, `R6 = 10 kΩ`, `C8 = 680 pF`, `C9 = 6.8 nF`, `C11 = 68 pF`.

Important distinction:

- the **datasheet demonstration board uses 15 µH**;
- the specific **ST eDSim LoadTransient testbench used in this session initially used 10 µH**.

Do not conflate those two artifacts.

---

# 4. L7987L pin-level design decisions

Working redesign reference: `U101 = L7987L`.

## 4.1 VBIAS

**ST data — p. 6/42, §2.2, Table 1:** VBIAS can be connected to a regulated output above 3 V and should be locally bypassed with 1 µF ceramic.

Decision:

- `VBIAS -> VOUT` (3.3 V rail)
- `C106 = 1 µF` VBIAS-to-GND

Candidate reused part:

- Murata `GRJ21BC72A105KE11L`
- 1 µF
- 100 V
- X7S
- 0805

The 100 V rating is far above what VBIAS requires; it is acceptable because the same part can be reused for VCC bypass.

## 4.2 VCC

**ST data — p. 6/42, Table 1:** VCC requires local 1 µF ceramic bypass.

Decision:

- `R104 = 0 Ω` VIN-to-VCC link
- `C104 = 1 µF` VCC-to-GND

Candidate:

- Murata `GRJ21BC72A105KE11L`, 1 µF 100 V X7S 0805.

## 4.3 EN

**ST data — p. 6/42, Table 1:** if enable control is unused, EN may be connected to VCC.

Current design concept:

- `EN -> VCC` directly.

No external enable sequencing has been adopted at this checkpoint.

## 4.4 SYNCH

No external synchronization is required.

Decision:

- SYNCH left **NC**.

Earlier experimental `C101/C102` from SYNCH to GND were recognized as unnecessary/incorrect for the intended design and should be removed/not populated.

## 4.5 PGOOD

PGOOD is not needed by the project.

Decision:

- PGOOD left **NC**.

Earlier optional `R101/R102` PGOOD-related parts are to be removed/not populated.

## 4.6 Bootstrap

**ST data — p. 6/42, Table 1:** typical bootstrap capacitor is 100 nF between BOOT and LX.

Decision:

- `C107 = 100 nF` BOOT-to-LX.

Value selected; final MPN still pending.

## 4.7 Soft start

**ST data — p. 15/42, §4.2:**

`CSS = ISS × TSS / VREF`

with approximately:

- `ISS = 5 µA typ.`
- `VREF = 0.8 V`.

The ST reference design uses 33 nF for roughly 5.3 ms.

Decision:

- `C110 = 33 nF` SS-to-GND.

Value selected; final MPN still pending.

---

# 5. Switching frequency

Working redesign reference:

- `R108 = RFSW`.

**ST data — Rev.4 p. 12/42, §4.1, Eq. 1:**

`FSW [kHz] = 250 + 12500 / RFSW [kΩ]`

For `RFSW = 47 kΩ`:

`FSW ≈ 250 + 12500/47 ≈ 516 kHz`.

The official demonstration board nevertheless uses 47 kΩ and labels the design as 500 kHz. Therefore 47 kΩ is retained as the practical ST reference value for a nominal ~500 kHz design.

Decision:

- `R108 = 47 kΩ`, 1%, 0603
- design/simulation target: approximately **500 kHz**.

Note on the Bode macro: in the ST `L7987L_for_Bode` testbench the exposed FSW pin is left unconnected. The model is a dedicated Bode macro, so do **not** add an external RFSW to that testbench merely because the real circuit uses one.

---

# 6. Feedback divider

Working redesign references:

- `R107 = RU` = VOUT-to-FB
- `R110 = RD` = FB-to-GND

`R111` was removed from the final divider concept.

**ST data:** `VFB = 0.800 V typ.` from p. 9/42 Table 5.

Project calculation:

`VOUT = VFB × (1 + RU/RD)`

Using:

- `RU = 47.5 kΩ`
- `RD = 15.2 kΩ`

then:

`VOUT = 0.8 × (1 + 47.5/15.2) ≈ 3.300 V`.

Decision:

- `R107 = 47.5 kΩ`, 1%, 0603
- `R110 = 15.2 kΩ`, 1%, 0603

Existing part available/reusable for 47.5 kΩ:

- Yageo `RC0603FR-0747K5L`.

### ST eDSim reference mapping

In the ST LoadTransient bench the feedback references are:

- `R7` = upper resistor VOUT->FB
- `R11` = lower resistor FB->GND.

The first controlled modification to the clean testbench was therefore:

- `R7 = 47.5 kΩ`
- `R11 = 15.2 kΩ`.

---

# 7. Input capacitor

Working redesign reference: `C103`.

**ST data:** pp. 20–21/42 §5.1 and p. 30/42 §5.6 require a suitably rated input capacitor and local high-frequency bypass close to VIN/VCC.

Selected main local ceramic:

- Murata `GRM32EC72A106KE05L`
- **10 µF**
- **100 V**
- X7S
- 1210

This part was already present in the live procurement sheet and had useful TME availability during the sourcing work.

Existing upstream bulk capacitor in the project:

- Panasonic `EEEFK1H101P`
- 100 µF
- 50 V electrolytic.

No new arbitrary input-bulk value was introduced by the L7987L redesign. Final local placement must follow ST §5.6 during PCB work.

---

# 8. Output capacitor

Working redesign reference: `C105`.

## 8.1 Main COUT

**ST data:** §5.2 pp. 21–22/42; official demonstration board Table 7 uses one 47 µF / 16 V / 1210 MLCC as the main output capacitor.

Selected/frozen project part:

- Taiyo Yuden `EMK325BJ476MM-P`
- **47 µF nominal**
- **16 V**
- X5R
- ±20%
- 1210

This is the sole main COUT in the current redesign concept.

The earlier idea that a 25 V output MLCC was inherently preferable was dropped: the rail is only 3.3 V and the ST reference design itself uses a 16 V / 47 µF MLCC.

## 8.2 Optional extra capacitor

Working optional reference: `C112`.

Status:

- **DNP / optional**.

An earlier arbitrary proposal to populate 100 µF was explicitly retracted. No extra output capacitor is approved at this checkpoint.

## 8.3 DC-bias / exact ESR modeling

The current simulations use a nominal 47 µF output-capacitor model close to the ST testbench. Exact effective capacitance under DC bias and full impedance model of the Taiyo Yuden part have **not yet been corner-validated**.

Do not invent a final ESR/ESL. If final corner analysis requires precision, use manufacturer DC-bias/impedance/SPICE data.

---

# 9. Freewheel diode

The L7987L is asynchronous and needs an external Schottky freewheel diode.

ST reference:

- demonstration board `D1 = STPS2L60UF`, 60 V / 2 A.

Project choice:

- `D101 = STPS2L60A`
- 60 V
- 2 A
- SMA.

Reason: same ST family and required ratings, in a package/sourcing variant more practical for this project.

Status: selected unless later sourcing/layout work gives a reason to change it.

---

# 10. Inductor design — complete chronology

Working redesign reference: `L101`.

For calculations use ST variable/reference `L` / `L1`.

## 10.1 ST rule used

**ST Rev.4 p. 22/42, §5.3, Eq. 13:**

`L = VOUT × (1 - VOUT/VIN) / (ΔIL × FSW)`

or:

`ΔIL = VOUT × (1 - VOUT/VIN) / (L × FSW)`.

ST explicitly says:

- a common design rule is **20–40% ripple of output current**;
- evaluate Eq.13 at **maximum VIN** for maximum ripple;
- increasing L reduces ripple but **strongly impacts transient response time**.

Project values for sizing:

- `VOUT = 3.3 V`
- `VIN,max = 26.4 V`
- `FSW = 500 kHz`
- `IOUT = 1 A`.

## 10.2 Inductance interval implied by the ST guideline

For 40% ripple:

`ΔIL = 0.4 A`

`L ≈ 3.3 × (1 - 3.3/26.4) / (0.4 × 500 kHz) ≈ 14.44 µH`.

For 20% ripple:

`ΔIL = 0.2 A`

`L ≈ 28.88 µH`.

Therefore the nominal ST-guideline window at the chosen 1 A design point is approximately:

**14.4 µH to 28.9 µH**.

This is a design guideline, not an absolute functional limit.

## 10.3 Original 8.2 µH value

Previous design/procurement candidate:

- Eaton `HCM0703-8R2-R`
- 8.2 µH.

At `VIN,max = 26.4 V`:

`ΔIL ≈ 3.3 × (1 - 3.3/26.4) / (8.2 µH × 500 kHz) ≈ 0.704 A`.

At 1 A:

- ripple ratio ≈ **70.4%**;
- ideal normal peak current ≈ `1 + 0.704/2 = 1.35 A`.

### Correct final interpretation of 8.2 µH

The 8.2 µH value was initially rejected too categorically.

What is actually supported:

- It is **well outside ST's 20–40% preferred ripple rule** for this 1 A design point.
- Normal peak current is not inherently excessive.
- Controlled later eDSim load-transient testing showed that 8.2 µH can give a perfectly reasonable transient in this setup.

Therefore the correct reason for rejecting 8.2 µH as the preferred final value is **primarily the explicit ST ripple-design guideline**, not a demonstrated catastrophic transient problem.

## 10.4 Initial move to 22 µH

22 µH was initially chosen because it sits comfortably inside the ST 20–40% window.

At nominal 22 µH:

`ΔIL = 3.3 × (1 - 3.3/26.4) / (22 µH × 500 kHz) = 0.2625 A`.

At 1 A:

- ripple ≈ **26.25%**;
- ideal peak ≈ **1.131 A**.

For assumed −20% L tolerance:

- `Lmin = 17.6 µH`;
- `ΔIL ≈ 0.328 A`;
- `IL,PK ≈ 1.164 A`.

This is electrically comfortable.

### Eaton DR125-220-R

22 µH candidate evaluated in detail:

- Eaton `DR125-220-R`
- 22 µH
- ±20%
- Irms ≈ 3.70 A
- Isat ≈ 4.71 A
- DCR typ ≈ 39.6 mΩ
- 12.5 × 12.5 × 6 mm
- shielded, single-coil / two-terminal.

It was the preferred 22 µH Eaton choice if 22 µH were retained.

### DRQ variants rejected

`DRQ125-220-R` and `DRQ127-220-R` were rejected because they are coupled/two-coil four-pad inductors. They can be wired series/parallel, but that is unnecessary complexity for this simple single-inductor buck.

### Why 22 µH was reopened

Two reasons:

1. the physical part was much larger than desired;
2. ST §5.3 explicitly warns that higher L slows dynamic response.

Therefore 22 µH remained electrically valid but was no longer treated as automatically optimal.

## 10.5 15 µH reconsidered

At nominal 15 µH and `VIN,max = 26.4 V`:

`ΔIL ≈ 0.385 A`.

At 1 A:

- ripple ≈ **38.5%**.

This is just inside ST's 20–40% guideline and is the smallest common value evaluated that satisfies it nominally.

For a ±20% part:

- `Lmin = 12 µH`;
- `ΔIL ≈ 0.481 A`;
- normal ideal peak current ≈ `1 + 0.481/2 = 1.24 A`.

## 10.6 Eaton DR127-150-R

15 µH Eaton option found on TME after filtering for >3 A operating current:

- `DR127-150-R`
- 15 µH
- Irms ≈ 5.03 A
- Isat ≈ 9.66 A
- DCR ≈ 24.7 mΩ
- 12.5 × 12.5 × 8 mm.

It is electrically excellent but physically very oversized for a 1 A rail.

This led to an important criterion correction: **the inductor does not need >3 A Irms simply because the IC is rated for 2 A**.

## 10.7 Revised practical inductor criteria

More appropriate selection criteria:

- 15 µH nominal
- shielded power inductor
- single coil / two terminal
- ±20% or better
- Irms roughly ≥1.5–2 A
- saturation margin consistent with programmed ILIM
- acceptable DCR
- compact footprint
- actual TME availability.

## 10.8 Current preferred candidate: Bourns SRN6045-150M

Current candidate:

- **Bourns `SRN6045-150M`**
- 15 µH
- ±20%
- DCR max **95.8 mΩ**
- Irms **1.9 A**
- Isat **2.3 A**
- about **6 × 6 × 4.5 mm**
- shielded power inductor.

Bourns defines:

- Irms around a ~40 °C temperature rise;
- Isat at approximately 30% inductance roll-off.

Official datasheet used:

`https://www.bourns.com/docs/product-datasheets/srn6045.pdf`

This part is much smaller than the Eaton DR125/DR127 alternatives and its normal-current margins are adequate for the 1 A design.

Status at current checkpoint:

- **preferred L101 candidate**;
- normal load-transient behavior has been tested with its 95.8 mΩ DCR model;
- final approval still depends on final ILIM tolerance/margin review and remaining control-loop/corner work.

---

# 11. ILIM / current-limit design

Working redesign reference:

- `R109 = RILIM`.

ST LoadTransient testbench reference:

- `R4`.

**ST Rev.4 pp. 17–19/42, §4.5, Eq. 6:**

`RILIM = 27 kΩ × 3 A / ILIM`

or equivalently:

`ILIM = 3 A × 27 kΩ / RILIM`.

ST's Table 5 shows substantial tolerance around the programmed peak-current threshold, so Eq.6 is a nominal design relation, not a precision comparator threshold.

## 11.1 Initial candidate: 39 kΩ

`RILIM = 39 kΩ`

Nominal:

`ILIM = 3 × 27 / 39 ≈ 2.08 A`.

After the Bourns inductor was selected, this was considered too close to its `Isat = 2.3 A` to be a comfortable protection target once tolerance is considered.

## 11.2 Latest candidate: 47.5 kΩ

`RILIM = 47.5 kΩ`

Nominal:

`ILIM = 3 × 27 / 47.5 ≈ 1.705 A`.

Current status:

- **47.5 kΩ is the latest R109 candidate**;
- current-limit operation was exercised in SIMPLIS and did engage;
- however the exact min/max current-limit tolerance versus Bourns Isat has **not yet been fully corner-validated**.

Do not treat 1.705 A as an exact guaranteed threshold.

---

# 12. Compensation — reference mapping

The chosen topology is Type III, following ST Rev.4 §5.4.2 for an MLCC-based output filter.

## 12.1 Working KiCad references

- `R106 = RF`
- `C109 = CF`
- `C111 = CP`
- `R105 = RS`
- `C108 = CS`

## 12.2 ST eDSim Bode references

In `TB_L7987L_STEVAL-ISA198V1_Bode.wxsch`:

- `RC = RF`
- `CC = CF`
- `CP = CP`
- `RC2 = RS`
- `CC2 = CS`
- `RH = RU`
- `RL = RD`
- `L1 = output inductor`
- `COUT = output capacitor`
- `RLOAD = output load`.

For calculations, use ST notation (`RU`, `RF`, `CF`, `CP`, `RS`, `CS`) and map to KiCad only at the beginning/end.

---

# 13. Compensation evolution — complete chronology

## 13.1 Earlier Type III network for the 8.2 µH stage

Earlier working values:

- `R105 / RS = 1.62 kΩ`
- `C108 / CS = 390 pF`
- `R106 / RF = 11.8 kΩ`
- `C109 / CF = 18 nF`
- `C111 / CP = 56 pF`

with:

- `RU = 47.5 kΩ`
- `RD = 15.2 kΩ`
- `COUT = 47 µF`
- originally `L = 8.2 µH`
- target bandwidth approximately 60 kHz.

These values were derived from ST Rev.4 p. 27–28 §5.4.2 Figure 13 / Eq. 25–28.

## 13.2 Historical Bode runs with 8.2 µH

Using the above network:

### 8.2 µH / 47 µF

Result:

- crossover ≈ **47.6 kHz**
- phase margin ≈ **70°**
- gain margin ≈ **24 dB**.

### 8.2 µH / 30 µF

Result:

- crossover ≈ **71.5 kHz**
- phase margin ≈ **61°**
- gain margin ≈ **18 dB**.

### 8.2 µH / 20 µF

Result:

- crossover ≈ **101.5 kHz**
- phase margin ≈ **49.3°**
- gain margin ≈ **12.4 dB**.

This last case was considered borderline because ST recommends `FBW < 0.2×FSW`; at 500 kHz that is 100 kHz, and the phase margin had also fallen substantially.

## 13.3 22 µH Bode runs with the old compensation retained

The old Type III network was then tested with a 22 µH stage.

### Nominal

Model:

- `L = 22 µH`
- DCR = 39.6 mΩ
- `COUT = 47 µF`
- COUT ESR = 10 mΩ
- ESL = 500 pH
- `RLOAD = 3.3 Ω`
- `RU = 47.5 kΩ`
- `RD = 15.2 kΩ`
- old compensation unchanged.

Result:

- crossover ≈ **23.4 kHz**
- phase margin ≈ **67.5°**
- gain margin was not preserved in the chat record and must not be invented.

### Low-L / reduced-C corner

- `L = 17.6 µH`
- `COUT = 30 µF`

Result:

- crossover ≈ **42.6 kHz**
- PM ≈ **66.9°**.

### Lower-C corner

- `L = 17.6 µH`
- `COUT = 20 µF`

Result:

- crossover ≈ **61.3 kHz**
- PM ≈ **61.2°**.

### High-L corner

- `L = 26.4 µH`
- `COUT = 47 µF`

Result:

- crossover ≈ **20.1 kHz**
- PM ≈ **65.7°**.

These Bode runs came from the dedicated Bode testbench and were not invalidated by the later load-transient setup mistake.

## 13.4 Recalculation for the 15 µH Bourns stage

New nominal plant values:

- `L = 15 µH`
- `RDC = 95.8 mΩ`
- `CO = 47 µF`
- `RES = 5 mΩ` in the current nominal ST COUT model
- `RO = 3.3 Ω` (1 A at 3.3 V)
- `RU = 47.5 kΩ`
- `RD = 15.2 kΩ`
- target `FSW ≈ 500 kHz`
- target `FBW = 60 kHz`.

**ST source:** Rev.4 pp. 23–28 §5.4 / §5.4.2.

At 500 kHz:

`0.2×FSW = 100 kHz`, so 60 kHz is comfortably below ST's recommendation.

Using ST **Eq.17** with the load and parasitic terms gave:

`fLC ≈ 5.91 kHz`.

Important correction recorded during the work:

- Eq.17 is **not** simply `1/(2π√LC)`;
- the simple LC expression is only an approximation under negligible parasitics / appropriate load assumptions.

Using ST §5.4.2 Eq.26–28 yielded the theoretical Type III values:

- `RF = 16.065 kΩ`
- `CF = 16.753 nF`
- `CP = 39.63 pF`
- `RS = 1.124 kΩ`
- `CS = 566.6 pF`.

Intended placements are approximately:

- first zero near `0.1×fLC`;
- second zero near `fLC`;
- high-frequency poles near `0.5×FSW`.

## 13.5 Theoretical 15 µH Bode test

Testbench name:

`TB_L7987L_3V3_24V_Bode_Bourns15u.wxsch`

Graph name:

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

Result from the SVG:

- crossover `fc ≈ 59.3 kHz`
- phase margin `PM ≈ 64.5°`
- gain margin `GM ≈ 19.6 dB`.

Conclusion: the theoretical network is a good nominal design point and lands almost exactly on the 60 kHz target.

## 13.6 First proposed commercial rounding — not accepted

A first E-series rounding was proposed:

- `RF: 16.065 kΩ -> 16.2 kΩ`
- `CF: 16.753 nF -> 16 nF`
- `CP: 39.63 pF -> 39 pF`
- `RS: 1.124 kΩ -> 1.13 kΩ`
- `CS: 566.6 pF -> 560 pF`.

This exact set was **not accepted** because the user checked TME and found that suitable **16 nF MLCCs were effectively unavailable** in the desired form.

The `1.13 kΩ` value itself is not electrically unusual; it is an E96 value. Availability still needs to be checked live, but the immediate procurement blocker identified was 16 nF MLCC.

## 13.7 Latest compensation direction at current checkpoint

For theoretical `CF = 16.753 nF`, nearby common values considered:

- 15 nF: about −10.5%
- 18 nF: about +7.4%.

Therefore **18 nF is currently the preferred commercial candidate for CF** because it is closer to the theoretical value and commonly available.

However, the complete five-component commercial network has **not yet been finalized or simulated**.

The exact current status is:

- theoretical network is validated;
- the first rounded set containing 16 nF was rejected on TME availability;
- next step is to choose a fully procurable TME set around the theoretical values, likely beginning from **CF = 18 nF**, then run the Bode with those exact commercial values.

Do not claim that `16 kΩ / 18 nF / 39 pF / 1.13 kΩ / 560 pF` has already been simulated or validated: at this checkpoint it has **not**.

---

# 14. eDSim / SIMPLIS testbench inventory

Available L7987L testbenches seen in the ST eDSim installation:

- `TB_L7987L_STEVAL-ISA198V1_Bode.wxsch`
- `TB_L7987L_LineTransient_Neg_BB.wxsch`
- `TB_L7987L_LoadTransient_Neg_BB.wxsch`
- `TB_L7987L_STEVAL-ISA198V1_LineTransient.wxsch`
- `TB_L7987L_STEVAL-ISA198V1_LoadTransient.wxsch`.

There is **no dedicated CurrentLimit/OCP testbench** in that folder.

Current-limit testing was therefore made by copying/modifying the ST LoadTransient bench.

---

# 15. Load-transient simulation history — full chronology

## 15.1 Original ST LoadTransient bench

Original file:

`TB_L7987L_STEVAL-ISA198V1_LoadTransient.wxsch`

The ST note describes approximately:

- `VIN = 48 V`
- `VOUT = 5 V`
- load `0.5 A -> 2 A`
- nominal `f = 500 kHz`.

The load uses approximately:

- `ROFF = 10 Ω`
- `RON = 2.5 Ω`.

Relevant references in this bench:

- `R4 = 27 kΩ` on ILIM
- `RFSW = 47 kΩ`
- `CSS = 33 nF`
- `CBOOT = 100 nF`
- feedback `R7 = 68 kΩ`, `R11 = 13 kΩ` originally
- ST compensation around `R6 = 10 kΩ`, `C9 = 6.8 nF`, `C11 = 68 pF` plus the remainder of the Type III network
- transient example initially had `L1 = 10 µH`.

The PWL VIN source `V2` originally stepped to 48 V after startup.

## 15.2 Invalid early modified-transient sequence

Before the clean ST baseline was re-established, the testbench was modified too aggressively and several results were wrongly interpreted as physical light-load/pulse-skipping behavior.

Examples included modified load cases such as:

- 100 mA -> 1 A
- 400 mA -> 1 A
- 100 mA -> 800 mA
- 150 mA -> 800 mA
- later similar tests with 22 µH.

Some produced implausibly large 3.3 V rail excursions.

These runs are **invalid as design evidence** because the modified testbench had not first been validated.

Two explicit corrections resulted:

1. `ISKIP = 0.5 A typ.` from Table 5 does **not** imply a fixed load boundary of `ISKIP/2 = 250 mA`.
2. ST §4.3 says pulse skipping occurs when output current is lower than half the **inductor ripple**, while also describing the internal skip-current behavior. Those statements must not be collapsed into the incorrect `ISKIP/2` rule.

From that point onward the methodology was reset: start from a clean ST bench and change **one variable at a time**.

## 15.3 Clean ST baseline validation

The restored untouched ST bench was rerun.

Observed approximately:

- load 0.5 A -> 2 A at about 6.5 ms
- return 2 A -> 0.5 A at about 7.5 ms
- VOUT nominal about 4.98–4.99 V
- `VOUT,min ≈ 4.904 V`
- `VOUT,max ≈ 5.059 V`
- `IL,peak ≈ 2.59 A`.

Conclusion: ST/SIMPLIS model and original bench are sane.

## 15.4 Divider-only change to 3.3 V

Only feedback was changed:

- `R7 = 47.5 kΩ`
- `R11 = 15.2 kΩ`.

Everything else remained original, including VIN = 48 V and 10 Ω / 2.5 Ω load resistors.

Because VOUT changed to 3.3 V, the unchanged load resistors naturally became approximately:

- `3.3/10 = 0.33 A`
- `3.3/2.5 = 1.32 A`.

This was intentional for isolation of the feedback change.

Observed approximately:

- `VOUT,min ≈ 3.247 V`
- `VOUT,max ≈ 3.357 V`
- about −1.6% / +1.7%
- `IL,peak ≈ 1.84 A`.

Conclusion: divider change alone behaves correctly.

## 15.5 VIN-only change 48 V -> 24 V

Next only the PWL source V2 was changed:

- both 48 V entries became **24 V**;
- the `t=0, V=0` entry remained unchanged.

All other values stayed the same.

Observed:

- load remained ~0.33 A -> 1.32 A
- `VOUT,min ≈ 3.2466 V`
- `VOUT,max ≈ 3.3534 V`
- about ±1.62%
- `IL,peak ≈ 1.726 A`.

Conclusion: moving the example to the real nominal 24 V bus caused no problem.

## 15.6 L-only test: 8.2 µH

Next only L1 was changed to 8.2 µH; ESR was intentionally kept unchanged for the controlled comparison.

Observed:

- `VOUT,min ≈ 3.2490 V`
- `VOUT,max ≈ 3.3483 V`
- `IL,peak ≈ 1.749 A`.

Important conclusion:

- 8.2 µH does **not** show a bad load transient in this test;
- its final rejection is therefore based mainly on the ST ripple-design rule, not on this transient.

## 15.7 L-only test: 15 µH, ESR unchanged

Next:

- `L1 = 15 µH`
- prior ESR intentionally left unchanged.

From the SVG, approximate result:

- `VOUT,min ≈ 3.22 V`
- `VOUT,max ≈ 3.36 V`
- `IL,peak ≈ 1.61 A`.

This qualitatively illustrates ST's §5.3 tradeoff: more L reduces peak/ripple current but can slow the dynamic response slightly.

## 15.8 Bourns-realistic 15 µH test

Then the inductor model was changed to the real Bourns candidate:

- `L1 = 15 µH`
- `ESR/DCR = 95.8 mΩ`.

Observed from SVG:

- `VOUT,min ≈ 3.234 V`
- `VOUT,max ≈ 3.371 V`
- `IL,peak ≈ 1.62 A`.

Conclusion: the Bourns DCR does not create a concerning normal transient change.

---

# 16. Current-limit / OCP exploratory test

Because no dedicated ST OCP bench exists, a copy of LoadTransient was used.

Saved testbench name:

`TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5.wxsch`

Recommended graph name:

`TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5_graph.svg`

Configuration for this exploratory test:

- `VIN = 24 V`
- feedback for 3.3 V
- `L1 = 15 µH`
- `L1 ESR = 95.8 mΩ`
- original ST compensation still present
- `R4 / RILIM = 47.5 kΩ`
- `ROFF = 10 Ω`
- `RON = 1.5 Ω`.

At 3.3 V, a 1.5 Ω load would ideally demand about:

`3.3/1.5 ≈ 2.2 A`.

This is intentionally above the nominal ~1.705 A programmed current limit.

Observed qualitatively from the SVG:

- OCP/current limiting clearly engaged;
- inductor current was approximately in the **1.4–1.6 A region** during the limited interval;
- VOUT collapsed to roughly **2.1 V** under overload;
- when the overload was released, the old-ST-compensation test showed a very large VOUT overshoot, visually around **7.2–7.3 V**.

Interpretation at this checkpoint:

- the test confirms that the programmed current limit is active;
- it is **not yet valid to draw a final conclusion about the overshoot**, because this OCP test still used the original ST compensation while the power stage/output voltage had been changed substantially;
- the correct next strategy was to finish the nominal compensation first, then repeat any OCP/fault test with the final compensation before deciding whether protection changes are needed.

No EN-restart hardware, foldback workaround, extra 100 µF solution, or later OCP recovery strategy has been validated at this checkpoint. Do not invent one.

---

# 17. Bode testbench setup details

The dedicated ST Bode testbench is:

`TB_L7987L_STEVAL-ISA198V1_Bode.wxsch`

Working copy saved as:

`TB_L7987L_3V3_24V_Bode_Bourns15u.wxsch`

The Bode symbol is `L7987L_for_Bode`.

Important peculiarity:

- its exposed FSW pin is left unconnected in the ST schematic;
- do not add the real RFSW to this macro merely to mimic hardware;
- compensation calculations are still referenced to the 500 kHz operating point of the ST design.

The user's PDF of the current Bode bench confirmed the main setup:

- `L1 = 15 µH, ESR = 95.8 mΩ`
- `COUT = 47 µF, ESR = 5 mΩ`
- `RLOAD = 3.3 Ω`
- `RH = 47.5 kΩ`
- `RL = 15.2 kΩ`
- theoretical compensation values listed in §13.4.

---

# 18. Simulation file naming convention

The user explicitly requested that **every future simulation instruction include the exact save name**.

Established names:

### Current-limit exploratory bench

`TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5.wxsch`

Graph:

`TB_L7987L_3V3_24V_CurrentLimit_Bourns15u_RILIM47k5_graph.svg`

### Theoretical Bode bench

`TB_L7987L_3V3_24V_Bode_Bourns15u.wxsch`

Graph:

`TB_L7987L_3V3_24V_Bode_Bourns15u_graph.svg`

For future work, continue the same style: encode output/input condition, test type, inductor, and any major compensation/current-limit variant in the filename.

---

# 19. Current proposed component values

These are the **latest values/proposals**, not necessarily already committed to KiCad.

| Working KiCad ref | Function | Latest value / part | Status |
|---|---|---|---|
| `U101` | buck IC | ST L7987L, HTSSOP16 | selected redesign IC |
| `C103` | local VIN ceramic | Murata `GRM32EC72A106KE05L`, 10 µF 100 V X7S 1210 | selected |
| `R104` | VIN->VCC link | 0 Ω 0603 | selected |
| `C104` | VCC bypass | Murata `GRJ21BC72A105KE11L`, 1 µF 100 V X7S 0805 | selected |
| `C105` | main COUT | Taiyo Yuden `EMK325BJ476MM-P`, 47 µF 16 V X5R 1210 ±20% | selected/frozen nominal |
| `C106` | VBIAS bypass | 1 µF; same Murata candidate as C104; VBIAS tied to VOUT | selected concept |
| `C107` | bootstrap | 100 nF BOOT-LX | value selected, MPN pending |
| `C110` | soft start | 33 nF SS-GND | value selected, MPN pending |
| `R107` | feedback upper / RU | 47.5 kΩ 1% | selected |
| `R110` | feedback lower / RD | 15.2 kΩ 1% | selected |
| `R108` | RFSW | 47 kΩ 1% | selected |
| `R109` | RILIM | **47.5 kΩ candidate** | nominal ~1.705 A; tolerance review pending |
| `D101` | freewheel diode | STPS2L60A, 60 V / 2 A SMA | selected |
| `L101` | inductor | **Bourns SRN6045-150M**, 15 µH ±20%, DCR max 95.8 mΩ, Irms 1.9 A, Isat 2.3 A | preferred candidate |
| `C112` | optional extra output capacitor | DNP | no value approved |
| `R106` | Type III RF | theoretical **16.065 kΩ** | theoretical design validated; commercial value pending |
| `C109` | Type III CF | theoretical **16.753 nF** | theoretical design validated; 16 nF rounded value rejected for TME availability; 18 nF candidate |
| `C111` | Type III CP | theoretical **39.63 pF** | commercial value pending |
| `R105` | Type III RS | theoretical **1.124 kΩ** | commercial value pending |
| `C108` | Type III CS | theoretical **566.6 pF** | commercial value pending |

Latest first-round commercial proposal, **not yet validated as a set**:

- RF: 16.2 kΩ proposed initially
- CF: 16 nF proposed initially, **rejected because suitable MLCC availability on TME was poor/nonexistent**
- CP: 39 pF proposed
- RS: 1.13 kΩ proposed
- CS: 560 pF proposed

Latest direction: choose a complete TME-procurable set, likely with **CF = 18 nF**, then simulate that exact set.

---

# 20. What is already validated

## 20.1 From ST documentation

- L7987L is suitable for a 24 V bus and a 3.3 V output.
- 47 kΩ RFSW is consistent with the official ~500 kHz demonstration-board design.
- 33 nF soft start follows the official ST reference design.
- 100 nF BOOT-LX follows the ST pin/reference guidance.
- VBIAS may be connected to the 3.3 V output with 1 µF bypass.
- Type III compensation is the appropriate ST method for the chosen MLCC output filter.
- ST's inductor design rule is 20–40% ripple and must be checked at maximum VIN.
- ST explicitly warns that increasing inductance reduces ripple but slows dynamic response.

## 20.2 By project calculation

- 47.5 kΩ / 15.2 kΩ gives approximately 3.300 V nominal output.
- 8.2 µH gives about 70.4% ripple at the 1 A / 26.4 V design point.
- 15 µH gives about 38.5% nominal ripple at the same point.
- 15 µH at −20% gives about 0.481 A ripple and ~1.24 A ideal normal peak current.
- 22 µH gives about 26.25% ripple but is not required to meet ST's guideline.
- 47.5 kΩ RILIM gives nominal `ILIM ≈ 1.705 A` by ST Eq.6.
- theoretical Type III values for the 15 µH / 47 µF stage are `16.065 kΩ / 16.753 nF / 39.63 pF / 1.124 kΩ / 566.6 pF`.

## 20.3 In eDSim/SIMPLIS

- clean ST LoadTransient baseline behaves correctly;
- changing only the divider to 3.3 V behaves correctly;
- changing only VIN to 24 V behaves correctly;
- 8.2 µH does not produce a poor load transient in the controlled test;
- 15 µH behaves reasonably and demonstrates the expected ripple/transient tradeoff;
- Bourns 15 µH with 95.8 mΩ DCR behaves reasonably in normal load transient;
- theoretical 15 µH Type III compensation gives approximately:
  - `fc ≈ 59.3 kHz`
  - `PM ≈ 64.5°`
  - `GM ≈ 19.6 dB`;
- exploratory RILIM = 47.5 kΩ OCP test clearly enters current limiting.

---

# 21. What is not yet validated / still open

1. **Final commercial compensation values and MPNs.**
   - The theoretical network is validated.
   - The first rounded set failed the procurement check because 16 nF MLCC was not available in a useful TME option.
   - Need a fully procurable set, likely using 18 nF for CF, then run Bode with exact commercial values.

2. **Commercial-value compensation Bode and normal load transient.**
   - Not yet run at this checkpoint.

3. **Compensation corners with final commercial values.**
   - Need sensible L/C/tolerance/DC-bias corners after nominal commercial values are chosen.

4. **RILIM tolerance / Bourns saturation margin.**
   - 47.5 kΩ is only a nominal candidate.
   - Need to use ST current-limit min/max/tolerance data to check worst-case protection versus normal peak current and Bourns Isat.

5. **OCP recovery behavior with the final compensation.**
   - The exploratory OCP bench with old ST compensation showed a very large release overshoot.
   - It must be repeated only after the actual commercial compensation is fixed before deciding whether that overshoot is a real design issue.

6. **Actual COUT DC-bias / effective-capacitance corner.**
   - Nominal 47 µF model only so far.

7. **Line-transient adaptation.**
   - ST line-transient bench exists but has not yet been adapted and validated for the final project stage.

8. **Final L101 approval.**
   - Bourns SRN6045-150M is the preferred candidate but should remain a candidate until ILIM/fault/corner checks are complete.

9. **Final PSU selection.**
   - 24 V / 3 A is only a project recommendation.

10. **KiCad implementation.**
    - This documentation commit does not update the schematic/PCB.
    - Final approved connectivity/values must be applied to the actual branch, then ERC/DRC/layout/BOM must be regenerated and reviewed.

---

# 22. Problems/anomalies encountered and corrections

## 22.1 Incorrect early light-load conclusion

An early attempt treated `ISKIP/2 = 250 mA` as a fixed pulse-skipping load threshold. This is **retired**.

Correct reading of ST §4.3:

- pulse skipping occurs when output current is below half the inductor ripple;
- `ISKIP` is a separate internal current quantity involved in the skip behavior;
- ST does not state that the load threshold is `ISKIP/2`.

## 22.2 Invalid early transient setups

Several modified transient benches produced implausible 3.3 V behavior because too many variables were changed before validating the original ST bench.

Those results must not be used as design evidence.

Correct method adopted afterward:

1. validate untouched ST bench;
2. change one variable at a time;
3. check DC operating point and transient after each change.

## 22.3 Eq.17 attribution correction

The simple resonance equation `1/(2π√LC)` was at one point incorrectly attributed directly to ST Eq.17.

Correction:

- ST Eq.17 contains load/parasitic terms;
- the simple expression is only an approximation under suitable assumptions.

## 22.4 ST 10 µH versus 15 µH confusion

- eDSim LoadTransient example: 10 µH.
- datasheet demonstration board: 15 µH.

Always identify which source is being discussed.

## 22.5 Output-capacitor arbitrary-model ideas retracted

An arbitrary 100 µF / large-ESR/ESL modeling suggestion was retracted.

Final detailed COUT modeling must be based on the selected real capacitor and manufacturer data, not invented parasitics.

## 22.6 8.2 µH conclusion corrected

Earlier statement “8.2 µH absolutely does not work” was too strong.

Current technically defensible conclusion:

- it works reasonably in the controlled transient test;
- it is not preferred because its ~70% ripple at the chosen 1 A design point is far outside ST's recommended 20–40% range.

---

# 23. Exact next step

**Do not modify KiCad yet.**

The exact next step is:

1. Search the live TME catalog for a **complete, actually procurable Type III compensation set** close to the theoretical values:
   - `RF = 16.065 kΩ`
   - `CF = 16.753 nF`
   - `CP = 39.63 pF`
   - `RS = 1.124 kΩ`
   - `CS = 566.6 pF`.
2. Because useful 16 nF MLCCs were not available, start from **CF = 18 nF** unless the live TME search reveals a better practical option.
3. Choose exact commercial values/MPNs for all five parts; do not optimize around a value that cannot be bought.
4. Save a new Bode testbench with an explicit name, for example:
   - `TB_L7987L_3V3_24V_Bode_Bourns15u_TMEComp.wxsch`
5. Insert the exact commercial values and run the Bode.
6. Save the graph as:
   - `TB_L7987L_3V3_24V_Bode_Bourns15u_TMEComp_graph.svg`
7. Check crossover, phase margin, gain margin and absence of problematic additional crossings.
8. Only if that passes, apply the same compensation to a copy of the normal LoadTransient bench and re-run the 24 V / 3.3 V Bourns test.
9. After nominal commercial compensation is validated, return to RILIM/OCP and repeat the fault test with the final compensation before drawing any conclusion about the large release overshoot seen with the old ST compensation.

Every future simulation step must include the exact save filename.

---

# 24. Current checkpoint

**Resume here in a new chat:**

- Target: **24 V nominal -> 3.3 V / 1 A**, ST L7987L, ~500 kHz.
- Design VIN envelope: **21.6–26.4 V** (project assumption, not original PSU spec).
- Current preferred inductor: **Bourns SRN6045-150M**, 15 µH ±20%, DCR max 95.8 mΩ, Irms 1.9 A, Isat 2.3 A.
- Main COUT: **Taiyo Yuden EMK325BJ476MM-P**, 47 µF / 16 V X5R 1210; C112 remains DNP.
- Feedback: **47.5 kΩ / 15.2 kΩ**.
- RFSW: **47 kΩ**.
- RILIM: **47.5 kΩ candidate**, nominal ~1.705 A; tolerance/corner review still pending.
- Theoretical Type III for the 15 µH stage: **RF 16.065 kΩ, CF 16.753 nF, CP 39.63 pF, RS 1.124 kΩ, CS 566.6 pF**.
- Theoretical Bode result: **fc ≈ 59.3 kHz, PM ≈ 64.5°, GM ≈ 19.6 dB**.
- First rounded set `16.2 kΩ / 16 nF / 39 pF / 1.13 kΩ / 560 pF` was **not accepted** because suitable 16 nF MLCC availability on TME was poor/nonexistent.
- **18 nF is the current CF commercial candidate**, but the complete commercial network has **not yet been selected or simulated**.
- Normal Bourns load transient with old ST compensation was acceptable: approximately **VOUT min 3.234 V, max 3.371 V, IL peak 1.62 A**.
- Exploratory OCP with `RILIM = 47.5 kΩ`, `RON = 1.5 Ω` engaged current limiting but showed a large release overshoot while still using old ST compensation; do not treat that as final until repeated with final compensation.
- KiCad has **not** been updated by this documentation commit.
- **Exact next action:** choose the five compensation values/MPNs from live TME, likely starting with `CF = 18 nF`, then run the commercial-value Bode using the filename convention above.
