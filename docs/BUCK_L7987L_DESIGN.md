# L7987L buck redesign — current design record

Status: **schematic/netlist implementation complete; engineering schematic review PASS; PCB integration pending**.

Repository: `alessandrocardelli/filament-dryer-monitor`  
Working branch: `redesign/buck-sourcing`  
TME sourcing, KiCad MPN/manufacturer synchronization and footprint audit closed on **2026-09-02**.  
Hardware Design Manual-based schematic review and the four electrical closure gates completed on **2026-09-04**.

The actual KiCad schematic and freshly exported netlist override this document if they ever disagree.
The current implementation is in `hardware/Power.kicad_sch`; the temporary
`hardware/buck_redesign_sch.kicad_sch` file is no longer part of the active hierarchy.

> **Important manufacturing-state warning**
>
> The schematic/netlist implement the L7987L redesign, but
> `hardware/Filament_Dryer_Monitor.kicad_pcb` still contains the legacy AP66200 power stage and
> `/Power/VCC_AP66200` routing. The PCB and production exports are therefore **stale** for the
> power stage and must not be used for fabrication until the board has been synchronized,
> rerouted and checked.

> **Schematic-review result — PASS**
>
> The implemented L7987L + AutoEN block has passed the project engineering schematic review.
> The four previously open gates are closed:
>
> 1. current KiCad 10 ERC reviewed and enforced in CI;
> 2. programmed current-limit versus L1 engineering worst-case reviewed;
> 3. effective-capacitance / compensation sensitivity reviewed using the actual sourced parts,
>    with manufacturer curves/models treated as characterization rather than guaranteed minima;
> 4. AutoEN trip and enable levels reviewed across VIN/component corners.
>
> This is **schematic sign-off**, not final hardware validation. PCB-level ringing, thermal
> behavior, real load transients, effective capacitance in-circuit and AutoEN fault/recovery
> timing remain first-board/layout verification items.

---

## 1. Design objective

Replace the previous AP66200 24 V -> 3.3 V buck with a more readily sourceable high-voltage
regulator while preserving the existing project architecture.

Selected regulator architecture:

- **STMicroelectronics L7987L**;
- asynchronous buck;
- 4.5–61 V operating input range;
- 2 A DC capability;
- adjustable switching frequency;
- programmable peak current limit;
- external compensation.

Primary manufacturer source:

- STMicroelectronics, **L7987L — 61 V, 2 A asynchronous step-down switching regulator with adjustable current limitation**;
- DocID026362, Rev. 4;
- https://www.st.com/resource/en/datasheet/l7987l.pdf

AutoEN comparator:

- **TI TLV1701**;
- 2.2–36 V supply range;
- rail-to-rail input common-mode range;
- open-collector output;
- industrial temperature range -40 °C to +125 °C;
- https://www.ti.com/lit/ds/symlink/tlv1701.pdf

---

## 2. Electrical requirements

### 2.1 Load topology

The heater and fan are **not powered by the 3.3 V buck**.

- Heater: `24V_PROT`, low-side switched, approximately **1.6 A at 24 V**.
- Fan: `24V_PROT`, low-side switched, approximately **0.2 A at 24 V**.
- Buck: supplies the low-voltage electronics only.
- Buck output feeds the existing ferrite bead `FB1`, then `3V3_MCU`.

The 3.3 V loads include ESP32-WROOM-32E, CP2102-GM, sensor/display interfaces, pull-ups,
buzzer/logic and related circuitry.

### 2.2 3.3 V current target

Project load-budget values:

- conservative normal load: approximately **0.66 A**;
- including the SHT45 internal-heater case: approximately **0.76 A**;
- adopted buck design target: **1.0 A**.

`IOUT,design = 1.0 A`

### 2.3 Input-voltage design envelope

- nominal input: **24.0 V**;
- continuous engineering range: **21.6–26.4 V** (`24 V ±10%`).

This ±10% range is a **project engineering assumption**, not a verified tolerance specification
of the original eSUN supply.

### 2.4 Original PSU budget

Original supply: **24 V / 48 W**.

Approximate design-point power:

- heater: 38.4 W;
- fan: 4.8 W;
- 3.3 V buck output target: 3.3 W;
- ideal subtotal before buck loss: **46.5 W**.

The original 48 W source therefore has little practical margin. A future regulated **24 V / 3 A /
72 W minimum** source remains a sensible project target, but no replacement PSU is selected here.

---

## 3. Current implemented topology

The redesigned stage is integrated directly into `hardware/Power.kicad_sch`.

### 3.1 Input, VCC and local bypass

The actual buck input rail is `24V_PROT`.

```text
24V_PROT ───── U5 VIN1
    │          U5 VIN2
    │          U5 VCC
    │
    ├── C1  10 µF ── GND     local buck input ceramic
    ├── C2 100 nF ── GND     TLV1701 local bypass
    └── C3   1 µF ── GND     L7987L VCC local bypass
```

The earlier VIN-to-VCC 0 Ω jumper is removed. The existing **C5 = 100 µF / 50 V** bulk
capacitor and the upstream 24 V protection network remain.

### 3.2 Output and VBIAS

- `C10 = 47 µF` is the main pre-bead output capacitor;
- `C21 = 1 µF` bypasses VBIAS on the regulated output;
- the pre-FB1 node is explicitly `3V3_BUCK`;
- `FB1` then feeds `3V3_MCU`.

### 3.3 Unused pins

- U5 pin 12 `PGOOD`: intentionally NC;
- U5 pin 7 `SYNCH`: intentionally NC.

---

## 4. Current reference/value map

### 4.1 Active devices / magnetics

| Ref | Current device | Role |
|---|---|---|
| U5 | L7987L | Buck regulator |
| U1 | TLV1701 | AutoEN comparator |
| Q7 | MMBT3904 | AutoEN EN pull-down transistor |
| L1 | SRN6045-150M, 15 µH | Buck inductor |
| D7 | STPS2L60A | Catch Schottky diode |

### 4.2 Buck capacitors

| Ref | Value | Role |
|---|---:|---|
| C1 | 10 µF | Local `24V_PROT` input capacitor |
| C2 | 100 nF | TLV1701 bypass |
| C3 | 1 µF | L7987L VCC bypass |
| C4 | 33 nF | Soft start |
| C6 | 100 nF | BOOT-to-LX bootstrap |
| C7 | 330 nF | AutoEN EN timing |
| C8 | 39 pF | Type-III `CP` |
| C9 | 18 nF | Type-III `CF` |
| C10 | 47 µF | Main buck output capacitor |
| C20 | 560 pF | Type-III `CS` |
| C21 | 1 µF | VBIAS/output bypass |

### 4.3 Buck control resistors

| Ref | Value | Role |
|---|---:|---|
| R1 | 270 kΩ | AutoEN threshold top |
| R2 | 22 kΩ | AutoEN threshold bottom |
| R4 | 47 kΩ | FSW programming |
| R5 | 100 kΩ | TLV1701 open-collector pull-up |
| R6 | 100 kΩ | Comparator output to Q7 base |
| R29 | 47.5 kΩ | ILIM programming |
| R30 | 47 kΩ | Q7 base-emitter pull-down |
| R31 | 100 kΩ | EN pull-up |
| R32 | 15 kΩ | EN pull-down |
| R33 | 16 kΩ | Type-III `RF` |
| R34 | 16 kΩ | Feedback lower resistor |
| R35 | 1.13 kΩ | Type-III `RS` |
| R36 | 49.9 kΩ | Feedback upper resistor |

---

## 5. Switching frequency

ST relation:

`FSW [kHz] = 250 + 12500 / RFSW [kΩ]`

With `R4 = 47 kΩ`:

`FSW ≈ 516 kHz`

The project therefore treats the stage as a practical **~500 kHz** converter.

---

## 6. Feedback divider

Current values:

- upper `R36 = 49.9 kΩ`;
- lower `R34 = 16 kΩ`;
- L7987L nominal `VFB = 0.800 V`.

`VOUT = VFB × (1 + R36/R34) ≈ 3.295 V`

Using the ST feedback-reference limits alone (`0.788–0.812 V`) gives approximately
**3.246–3.344 V** before adding resistor tolerance and bias/leakage terms. The nominal programmed
rail remains correctly centered on 3.3 V.

---

## 7. Inductor, ripple and catch diode

Current inductor: **Bourns SRN6045-150M**.

Reviewed manufacturer ratings:

- nominal L: 15 µH;
- tolerance: ±20%;
- DCR max: approximately 95.8 mΩ;
- Irms: approximately 1.9 A;
- Isat: approximately 2.3 A;
- Bourns defines Isat at 30% inductance reduction.

At ~516 kHz and the 21.6–26.4 V design range:

- nominal `ΔIL ≈ 0.36–0.37 A p-p`;
- normal peak at 1.0 A load: approximately **1.18–1.19 A**;
- with L at -20%, ripple increases by about 25%, giving a normal peak of approximately
  **1.23 A** at the high-line stress point.

Normal operation therefore has large margin to both Irms and Isat.

Catch diode: **STPS2L60A**, 60 V / 2 A Schottky. Its DC reverse-voltage rating has ample margin
over the 26.4 V continuous input engineering maximum. PCB review must still validate switching
ringing, diode dissipation and temperature.

---

## 8. Current-limit programming — CLOSED

Current value: `R29 = 47.5 kΩ`.

ST provides the programming relation:

`RILIM = 27 kΩ × 3 A / ILIM`

which gives a nominal project value of approximately:

`ILIM,nom ≈ 1.705 A`

ST does **not** publish a guaranteed min/max table entry specifically for 47.5 kΩ. The datasheet
instead gives characterized/guaranteed examples at other programming resistances. Therefore the
project does not claim an exact guaranteed `ILIM,max` for R29 = 47.5 kΩ.

For schematic component-selection sign-off, a deliberately conservative engineering envelope was
formed from the worst relative high/low spread of the published ST current-limit points together
with the ±1% R29 tolerance:

- conservative upper stress estimate: approximately **2.15 A**;
- conservative lower estimate: approximately **1.42 A**.

Interpretation:

- the lower engineering estimate remains above the normal worst-case inductor peak (~1.23 A);
- the upper engineering estimate remains below the Bourns 2.3 A Isat rating by about 0.15 A;
- ST also implements pulse-by-pulse current limiting and peak-current foldback in heavy short
  circuit.

**Gate result: CLOSED for engineering schematic sign-off.** No component change is justified from
this review. The 2.15 A figure is an engineering stress envelope, **not a manufacturer-guaranteed
47.5 kΩ maximum**. Fault current and inductor waveform must still be observed during prototype
fault testing.

---

## 9. Input/output capacitance — CLOSED FOR SCHEMATIC SELECTION

Actual sourced parts:

- **C1 = Murata GRM32EC72A106KE05L**, 10 µF, 100 V, X7S, 1210;
- **C10 = Taiyo Yuden LMK325B7476KM-PR**, previous part number for current
  `MSASL32MSB7476KPNB25`, 47 µF ±10%, 10 V, X7R, 1210;
- **C3 = 1 µF / 100 V** local L7987L VCC bypass;
- **C21 = 1 µF / 25 V** VBIAS bypass;
- upstream **C5 = 100 µF / 50 V** remains a separate low-frequency bulk reservoir.

Manufacturer primary documentation confirms the nominal part ratings. Taiyo Yuden also exposes
part-specific temperature/DC-bias simulation data for C10. As is normal for class-II MLCCs, the
characterization curves/models are not treated here as a simple guaranteed minimum capacitance.
No invented `Ceff,min` is therefore recorded.

ST requires local ceramic input/VCC bypassing and treats output capacitance as part of the LC/loop
stability design. The implemented topology provides those local capacitors and uses Type-III
compensation appropriate to a low-ESR ceramic output network.

The design review also checked reduced-capacitance stress cases rather than assuming the printed
47 µF value remains unchanged under bias. For example, using **23.5 µF** as a deliberately severe
C10 stress case moves the ideal LC pole upward but does not invalidate the adopted compensation
architecture or the ~59 kHz recorded crossover relative to the ~500 kHz switching frequency.

**Gate result: CLOSED for schematic component selection.** This does not turn manufacturer
characterization into a guaranteed `Ceff,min`. First-board load-transient response and real
in-circuit output behavior remain mandatory bring-up checks.

Manufacturer references:

- Murata GRM series / part specification: https://www.murata.com/
- Taiyo Yuden `MSASL32MSB7476KPNB25` product data and DC-bias model:
  https://ds.yuden.co.jp/TYCOMPAS/eu/detail?pn=MSASL32MSB7476KPNB25&u=M

---

## 10. Type-III compensation

Final implemented network:

- `R33 / RF = 16 kΩ`;
- `C9 / CF = 18 nF`;
- `C8 / CP = 39 pF`;
- `R35 / RS = 1.13 kΩ`;
- `C20 / CS = 560 pF`.

Recorded final-value simulation results:

- crossover: **~59.1 kHz**;
- phase margin: **~64.9°**;
- gain margin: **~19.6 dB**.

Recorded load-transient run (~0.33 A -> 1.32 A):

- `VOUT,min ≈ 3.254 V`;
- `VOUT,max ≈ 3.361 V`;
- inductor-current peak `≈ 1.59 A`;
- clean recovery.

These are **session-recorded SIMPLIS/eDSim results**. They are not measurements from physical
hardware and were not re-created from scratch during the 2026-09-04 documentation update.

---

## 11. Why AutoEN was added

Deep-short simulation/testing of the L7987L model showed COMP could remain heavily saturated
during a persistent fault. Releasing the short without clearing the loop state could produce an
undesirable recovery overshoot.

A manual EN reset cleared the state and produced a clean restart. The final schematic therefore
adds an external AutoEN fault detector that forces EN low when COMP rises above a defined threshold.
It complements, rather than replaces, the L7987L internal current limit/foldback.

---

## 12. AutoEN implementation and corner review — CLOSED

Comparator: **U1 = TLV1701**.

Connections:

- `IN+` -> L7987L `COMP`;
- `IN-` -> `VREF_FAULT`;
- `V+` -> `24V_PROT`;
- `V-` -> GND;
- C2 = 100 nF local supply bypass;
- open-collector OUT uses R5 pull-up and R6/Q7 drive network.

Threshold divider:

- R1 = 270 kΩ from `24V_PROT` to `VREF_FAULT`;
- R2 = 22 kΩ from `VREF_FAULT` to GND.

Nominal threshold:

- 21.6 V input -> approximately **1.627 V**;
- 24.0 V input -> approximately **1.808 V**;
- 26.4 V input -> approximately **1.989 V**.

The final corner review included:

- input range 21.6–26.4 V;
- R1/R2 ±1% tolerance;
- 100 ppm/K resistor TCR in opposing directions for a conservative temperature check;
- TLV1701 input offset/bias error at industrial temperature.

The resulting conservative reviewed COMP trip window is approximately **1.56–2.07 V**.
This remains well below the L7987L high-COMP fault condition used by the protection concept,
leaving more than 1 V of separation at the reviewed upper trip corner.

EN network:

```text
24V_PROT ── R31 100k ──┬── EN
                        │
                     R32 15k
                        │
                       GND

EN ── C7 330n ── GND
EN ── Q7 collector
```

Nominal `VEN ≈ 3.13 V` at 24 V. At the reviewed low-line/tolerance corner the EN-high level
remains approximately **2.7 V or higher**, comfortably above the L7987L maximum enable-high
threshold of 0.9 V.

The recorded persistent-fault simulation showed repeated shutdown/retry under a maintained fault
and clean eventual restart after fault removal.

**Gate result: CLOSED for schematic sign-off.** Real comparator propagation, EN timing, COMP
waveform and restart behavior remain prototype measurements.

---

## 13. ERC — CLOSED AND CI-ENFORCED

A new KiCad 10 GitHub Actions workflow (`.github/workflows/kicad-erc.yml`) now runs ERC on the
current schematic.

The fresh 2026-09-04 ERC report contains:

- **1 error**: `power_pin_not_driven` on `#PWR02` GND;
- **11 warnings**: seven `unconnected_wire_endpoint`, two `lib_symbol_mismatch`, two
  `pin_to_pin` warnings.

The single error is an ERC modeling condition: the board receives power from an external passive
connector, so KiCad sees the GND power symbol as not being driven by a `Power output` pin. The
actual net is connected; this is not an electrical open.

The CI therefore contains an explicit reviewed waiver for **only that exact error** and permits
only the already-reviewed warning classes up to their current counts. The gate fails if:

- another ERC error appears;
- the waived error changes identity;
- a new warning class appears;
- one of the reviewed warning counts increases.

The workflow run after adding the gate completed successfully. `hardware/ERC.rpt` has been
refreshed to the current L7987L schematic state.

**Gate result: CLOSED.** ERC is now a repeatable regression check instead of a stale manual report.

---

## 14. Repository verification

Verified against the actual schematic/netlist on `redesign/buck-sourcing`:

- old AP66200 absent from the active exported netlist;
- active buck `U5 = L7987L` in `/Power/`;
- VIN1, VIN2 and VCC on `24V_PROT`;
- no VIN-to-VCC 0 Ω jumper;
- local input/VCC/comparator bypass capacitors present;
- `L1 = 15 µH`, `D7 = STPS2L60A`, `C10 = 47 µF`;
- feedback `R36 = 49.9 kΩ`, `R34 = 16 kΩ`;
- FSW `R4 = 47 kΩ`;
- ILIM `R29 = 47.5 kΩ`;
- compensation `R33 = 16 kΩ`, `C9 = 18 nF`, `C8 = 39 pF`, `R35 = 1.13 kΩ`,
  `C20 = 560 pF`;
- TLV1701 pin mapping and COMP/reference connections correct;
- R5/R6/Q7 open-collector AutoEN drive present;
- EN contains R31/R32/C7 timing network;
- PGOOD and SYNCH explicitly NC;
- upstream protection/bulk retained;
- downstream `FB1 -> 3V3_MCU` retained;
- pre-FB1 output explicitly `3V3_BUCK`;
- temporary redesign sheet removed from the active hierarchy.

No missing component from the intended L7987L + AutoEN electrical block was found.

---

## 15. Design-for-test / debug access

Existing project test access includes `24V_PROT`, `3V3_MCU` and GND.

Before PCB placement, decide whether to add convenient probe/test pads for:

- `3V3_BUCK` before FB1;
- L7987L COMP;
- L7987L EN / AutoEN control node.

These are **DFT recommendations**, not unresolved schematic electrical gates.

---

## 16. Closed sourcing / footprint state

Closed items include:

- TME sourcing pass;
- manufacturer/MPN synchronization into KiCad;
- final footprint audit;
- L1 footprint against Bourns recommended layout;
- U5 HTSSOP footprint geometry check;
- strict symbol/pinout audit;
- explicit `3V3_BUCK` label;
- TLV1701 metadata/datasheet synchronization.

Sourcing and footprints should only be reopened if later PCB/layout work forces an electrical or
mechanical component change.

---

## 17. PCB state and next implementation sequence

The current PCB still contains the pre-redesign AP66200 implementation. Schematic and PCB are
therefore intentionally out of sync at this checkpoint.

Next hardware steps:

1. decide optional buck DFT/debug access for `3V3_BUCK`, COMP and EN/AutoEN;
2. update PCB from the signed-off schematic and remove the old AP66200 stage;
3. place L7987L, D7, L1 and local capacitors according to ST switching-loop/layout guidance;
4. route the new stage and reconnect the existing `24V_PROT` / FB1 architecture;
5. perform dedicated PCB review: high-current loops, return paths, thermal paths, switching-node
   clearance/ringing risk, USB routing, ESP32 antenna keepout, heater/fan paths and U5 exposed-pad
   paste strategy;
6. run DRC;
7. regenerate BOM, CPL/position data, fabrication outputs and production netlist from the same
   revision;
8. reconcile generated production data with the final `BOM TME` before ordering;
9. on the first board, validate output regulation/transient response, component temperatures,
   switching-node stress and AutoEN fault/recovery behavior.

**Current project gate:** schematic electrical review is complete; PCB/layout integration is the
next design phase.