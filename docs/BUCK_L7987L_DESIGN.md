# L7987L buck redesign — current design record

Status: **schematic/netlist implementation complete; schematic review open; PCB integration pending**.

Repository: `alessandrocardelli/filament-dryer-monitor`  
Working branch: `redesign/buck-sourcing`  
TME sourcing, KiCad MPN/manufacturer synchronization and footprint audit closed on **2026-09-02**.  
First Hardware Design Manual-based schematic review completed on **2026-09-04**.

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

> **Current schematic-review status**
>
> The 2026-09-04 review found **no blocking electrical omission** in the implemented L7987L +
> AutoEN topology. This is not yet final schematic sign-off. Four closure items remain:
>
> 1. run a fresh ERC on the current revision;
> 2. close worst-case programmed current-limit versus L1 saturation margin;
> 3. verify effective input/output capacitance and regulator stability/startup margin using the
>    actual sourced capacitor curves at DC bias and temperature;
> 4. verify AutoEN threshold and recovery across the full VIN range, component tolerance and
>    temperature.
>
> DFT/debug access should also be reviewed before PCB synchronization, especially whether to
> expose `3V3_BUCK`, COMP and EN/AutoEN with convenient probe/test pads.

---

## 1. Design objective

Replace the previous AP66200 24 V -> 3.3 V buck with a more readily sourceable high-voltage
regulator while preserving the existing project architecture.

Selected regulator architecture:

- **STMicroelectronics L7987L**
- asynchronous buck
- 4.5–61 V operating input range
- 2 A DC capability
- adjustable switching frequency
- programmable peak current limit
- external compensation

Primary manufacturer source used for the electrical design:

- STMicroelectronics, **L7987L — 61 V, 2 A asynchronous step-down switching regulator with adjustable current limitation**
- DocID026362, Rev. 4, May 2020
- https://www.st.com/resource/en/datasheet/l7987l.pdf

The AutoEN comparator selected in the implemented circuit is the **TI TLV1701**:

- 2.2–36 V supply range
- open-collector output
- rail-to-rail input common-mode capability
- https://www.ti.com/lit/ds/symlink/tlv1701.pdf

The KiCad metadata for the selected TLV1701 has been synchronized to the final sourced part and datasheet.

---

## 2. Electrical requirements

### 2.1 Actual load topology

The heater and fan are **not powered by the 3.3 V buck**.

- Heater: `24V_PROT`, low-side switched, approximately **1.6 A at 24 V**.
- Fan: `24V_PROT`, low-side switched, approximately **0.2 A at 24 V**.
- Buck: supplies the low-voltage electronics only.
- Buck output feeds the existing ferrite bead `FB1`, then the project rail `3V3_MCU`.

The 3.3 V loads include the ESP32-WROOM-32E, CP2102-GM USB-UART bridge, sensor/display interfaces, pull-ups,
buzzer/logic and related circuitry.

### 2.2 3.3 V design current

Project load-budget calculations established during the redesign:

- conservative normal 3.3 V load: approximately **0.66 A**;
- including the SHT45 internal-heater case: approximately **0.76 A**;
- adopted buck design target: **1.0 A**.

Therefore:

`IOUT,design = 1.0 A`

### 2.3 Input-voltage design envelope

The final replacement PSU is not yet fixed. The redesign therefore uses the engineering range:

- nominal input: **24.0 V**;
- continuous design range: **21.6–26.4 V** (`24 V ±10%`).

This ±10% range is a project design assumption, not a verified tolerance specification of the
original eSUN supply.

### 2.4 Original PSU budget

Original supply: **24 V / 48 W**.

Project calculation at the adopted design point:

- heater: `24 V × 1.6 A = 38.4 W`;
- fan: `24 V × 0.2 A = 4.8 W`;
- buck output target: `3.3 V × 1 A = 3.3 W`;
- ideal subtotal before buck losses: **46.5 W**.

The original 48 W supply therefore has very little practical margin. A future regulated
**24 V / 3 A / 72 W minimum** supply remains a sensible project target, but no exact replacement
PSU is selected in this document.

---

## 3. Current implemented topology

The current branch no longer has a separate active `Buck redesign` sheet. The new power stage
is integrated directly into `Power.kicad_sch`.

### 3.1 Input rail and VCC

The real project input node is **`24V_PROT`**.

Current implementation:

```text
24V_PROT ───── U5 VIN1
    │          U5 VIN2
    │          U5 VCC
    │
    ├── C1  10 µF ── GND     local buck input ceramic
    ├── C2 100 nF ── GND     TLV1701 local bypass
    └── C3   1 µF ── GND     L7987L VCC local bypass
```

The earlier VIN-to-VCC **0 Ω jumper has been removed**. VCC is directly connected to
`24V_PROT`, as are VIN1 and VIN2.

The existing upstream **C5 = 100 µF / 50 V** bulk capacitor remains on `24V_PROT`, together
with the existing input protection network.

### 3.2 VBIAS and output

VBIAS is connected to the regulated 3.3 V buck output and locally bypassed with **C21 = 1 µF**.

The main output capacitor is **C10 = 47 µF**.

The output then feeds the existing `FB1` ferrite bead and downstream `3V3_MCU` rail.

The pre-FB1 regulated output node is explicitly labeled **`3V3_BUCK`** and then feeds `FB1` toward `3V3_MCU`.

### 3.3 PGOOD and SYNCH

Not used by this project:

- U5 pin 12 `PGOOD`: **NC**;
- U5 pin 7 `SYNCH`: **NC**.

The regenerated netlist explicitly marks both as no-connect.

---

## 4. Current reference/value map

The new block was selectively re-annotated after integration so that the references are compact.
Only the new/replaced block was re-annotated; the rest of the project references were preserved.

### 4.1 Active devices / magnetics

| Ref | Current value/device | Role |
|---|---|---|
| U5 | L7987L | Buck regulator |
| U1 | TLV1701 | AutoEN comparator |
| Q7 | MMBT3904 | AutoEN EN pull-down transistor |
| L1 | 15 µH | Buck inductor |
| D7 | STPS2L60A | Catch Schottky diode |

### 4.2 Capacitors

| Ref | Value | Role |
|---|---:|---|
| C1 | 10 µF | Local `24V_PROT` buck input capacitor |
| C2 | 100 nF | TLV1701 supply bypass |
| C3 | 1 µF | L7987L VCC bypass |
| C4 | 33 nF | Soft start |
| C6 | 100 nF | BOOT-to-LX bootstrap |
| C7 | 330 nF | AutoEN EN timing capacitor |
| C8 | 39 pF | Type-III compensation `CP` |
| C9 | 18 nF | Type-III compensation `CF` |
| C10 | 47 µF | Main buck output capacitor |
| C20 | 560 pF | Type-III compensation `CS` |
| C21 | 1 µF | VBIAS/output bypass |

Existing project capacitors relevant to the power path:

- `C5 = 100 µF / 50 V` upstream bulk on `24V_PROT`;
- `C11 = 22 µF` downstream of the buck/ferrite-bead power path.

### 4.3 Resistors

| Ref | Value | Role |
|---|---:|---|
| R1 | 270 kΩ | AutoEN threshold divider, top |
| R2 | 22 kΩ | AutoEN threshold divider, bottom |
| R4 | 47 kΩ | FSW programming |
| R5 | 100 kΩ | TLV1701 open-collector output pull-up |
| R6 | 100 kΩ | TLV1701 output to Q7 base |
| R29 | 47.5 kΩ | ILIM programming |
| R30 | 47 kΩ | Q7 base-emitter pull-down |
| R31 | 100 kΩ | EN pull-up |
| R32 | 15 kΩ | EN pull-down |
| R33 | 16 kΩ | Type-III compensation `RF` |
| R34 | 16 kΩ | Feedback divider lower resistor |
| R35 | 1.13 kΩ | Type-III compensation `RS` |
| R36 | 49.9 kΩ | Feedback divider upper resistor |

`R3` belongs to the pre-existing 24 V input-protection network and is not one of the new buck
control resistors.

---

## 5. Switching frequency

ST Rev. 4, p. 12, Eq. 1:

`FSW [kHz] = 250 + 12500 / RFSW [kΩ]`

With current **R4 = 47 kΩ**:

`FSW ≈ 250 + 12500 / 47 ≈ 516 kHz`

ST also uses 47 kΩ in its nominal 500 kHz demonstration design. The project therefore treats
this as a practical **~500 kHz** switching point.

---

## 6. Feedback divider

ST feedback reference:

`VFB ≈ 0.800 V typ.`

Current divider:

- upper: **R36 = 49.9 kΩ**;
- lower: **R34 = 16 kΩ**.

Project calculation:

`VOUT = VFB × (1 + R36/R34)`

`VOUT ≈ 0.8 × (1 + 49.9/16) ≈ 3.295 V`

So the implemented nominal target remains **3.3 V**.

For final rail-tolerance closure, resistor tolerance and the L7987L feedback-reference limits must
be included together; nominal divider arithmetic alone is not a complete worst-case output-accuracy
calculation.

---

## 7. Inductor, ripple and catch diode

Current inductor:

- **L1 = 15 µH**;
- final sourced MPN: Bourns `SRN6045-150M`;
- reviewed ratings: DCR max ~95.8 mΩ, Irms ~1.9 A, Isat ~2.3 A.

Using the implemented nominal output (~3.295 V), ~516 kHz switching frequency and the project
21.6–26.4 V continuous input range, the first-order CCM buck relation gives approximately:

- `ΔIL ≈ 0.36 A p-p` at 21.6 V;
- `ΔIL ≈ 0.37 A p-p` at 26.4 V.

At the 1.0 A design load this corresponds to a normal-operation inductor peak of approximately
**1.18–1.19 A**, comfortably below the reviewed 1.9 A Irms and 2.3 A Isat ratings.

This closes the normal-load ripple/current sanity check. It does **not** close the current-limit
fault case, because the maximum programmed peak current over IC tolerance still has to be
compared explicitly with L1 saturation current in Section 8.

The corresponding L1 footprint has been validated against the Bourns recommended land pattern.
PCB placement/routing remains pending.

Current catch diode:

- **D7 = STPS2L60A**;
- 60 V / 2 A Schottky class used in the design.

Its static reverse-voltage rating has ample margin over the project 26.4 V continuous input design
maximum. Final PCB/fault validation must still consider ringing/transient stress, diode current,
power loss and temperature in the real implementation.

The L7987L is asynchronous, so D7 is part of the high-current switching loop and must be placed
appropriately during PCB redesign.

---

## 8. Current-limit programming

Current value:

- **R29 = 47.5 kΩ**.

Using the ST current-limit relation applied during the redesign, the nominal programmed peak
limit is approximately:

`ILIM ≈ 1.705 A`

This was selected to sit above the normal 1 A design operating peak while still providing a
meaningful hardware current limit.

### Open worst-case gate

The 2026-09-04 schematic review deliberately leaves one item open here: the L7987L current-limit
threshold has device tolerance, so **ILIM,max**, not only the nominal 1.705 A value, must be
compared with the Bourns SRN6045-150M saturation-current behavior and with any relevant diode/
regulator stress.

This is a verification gate, not evidence that the present value is wrong. The nominal design has
clear normal-operation margin; final sign-off requires the worst-case fault-current comparison.

The L7987L itself provides pulse-by-pulse current limiting/foldback behavior, but deep-short
simulation showed a release/recovery problem that motivated the external AutoEN circuit below.

---

## 9. Input/output capacitor verification

The implemented power-stage capacitors are structurally consistent with the L7987L topology:
local VIN, VCC, bootstrap, VBIAS and output capacitance are all present.

However, schematic sign-off must use the **effective** capacitance of the final sourced parts, not
only their printed nominal values. Before closing this gate:

1. check C1 and C10 manufacturer capacitance-versus-DC-bias data at their actual operating
   voltage;
2. include temperature and tolerance where material;
3. confirm the resulting effective values remain compatible with the L7987L input/output
   recommendations, compensation assumptions and startup behavior;
4. retain the upstream C5 bulk capacitor as a separate lower-frequency reservoir rather than
   counting it as a substitute for the local high-frequency ceramic loop.

This check is intentionally listed as pending because it depends on the exact sourced capacitor
curves.

---

## 10. Type-III compensation

Current implemented compensation network:

- **R33 / RF = 16 kΩ**;
- **C9 / CF = 18 nF**;
- **C8 / CP = 39 pF**;
- **R35 / RS = 1.13 kΩ**;
- **C20 / CS = 560 pF**.

This network is the final commercial-value set adopted for the current schematic.

### Validation record

The final-value simulation results recorded during the design session were:

- crossover frequency: **~59.1 kHz**;
- phase margin: **~64.9°**;
- gain margin: **~19.6 dB**.

ST recommends regulator bandwidth below roughly `0.2 × FSW`; with a nominal ~500 kHz
switching frequency, the ~59 kHz crossover is comfortably below that guideline.

A recorded load-transient run using the final design values used a step of approximately
**0.33 A -> 1.32 A** and produced approximately:

- `VOUT,min ≈ 3.254 V`;
- `VOUT,max ≈ 3.361 V`;
- inductor-current peak `≈ 1.59 A`;
- clean recovery.

These are **session-recorded SIMPLIS/eDSim results**. They were not independently re-run as
part of the 2026-09-04 schematic review; the repository review verified that the implemented
schematic values match the recorded final design.

Earlier experimental compensation/inductor combinations in the previous work record are
superseded by the values above.

The final capacitor-effective-value check in Section 9 must be reconciled with these loop-design
assumptions before schematic sign-off.

---

## 11. Why AutoEN was added

Deep-short testing of the L7987L showed that the internal current-limit/foldback mechanism can
leave COMP heavily saturated during a persistent fault. When the short was released, the stored
control-loop state could produce an undesirably large recovery overshoot.

A manual EN reset cleared the control state and produced a much cleaner restart. The final
design therefore implements an external fault detector that automatically forces an EN reset.

This is not a replacement for the L7987L's internal current limit; it is an additional recovery
mechanism built around the observed COMP behavior.

---

## 12. AutoEN implementation and review gate

### 12.1 Comparator

Current comparator: **U1 = TLV1701**, SOT-23-5.

Correct pin mapping used by the current symbol/netlist:

- pin 1 = `IN+`;
- pin 2 = `V-` / GND;
- pin 3 = `IN-`;
- pin 4 = open-collector `OUT`;
- pin 5 = `V+`.

Current connections:

- `IN+` -> L7987L `COMP`;
- `IN-` -> `VREF_FAULT`;
- `V+` -> `24V_PROT`;
- `V-` -> GND;
- C2 = 100 nF local supply bypass.

### 12.2 Fault threshold

Divider:

- R1 = 270 kΩ from `24V_PROT` to `VREF_FAULT`;
- R2 = 22 kΩ from `VREF_FAULT` to GND.

At 24 V nominal:

`VREF_FAULT = 24 × 22 / (270 + 22) ≈ 1.808 V`

Because the reference is derived directly from `24V_PROT`, its nominal threshold is intentionally
line-dependent. Across the current project continuous VIN design envelope:

- at 21.6 V: `VREF_FAULT ≈ 1.63 V`;
- at 26.4 V: `VREF_FAULT ≈ 1.99 V`.

This is not automatically an error, but it means the AutoEN trip/recovery point cannot be signed
off from the 24 V nominal simulation alone. The final check must include VIN, R1/R2 tolerance,
TLV1701 input offset/temperature behavior, COMP behavior and the resulting restart sequence.

### 12.3 Open-collector output and Q7

The TLV1701 output is open collector, therefore it requires a pull-up.

Current network:

```text
24V_PROT ── R5 100k ──┬── U1 OUT
                       │
                       └── R6 100k ──┬── Q7 base
                                     │
                                  R30 47k
                                     │
                                    GND

Q7 emitter   -> GND
Q7 collector -> EN
```

Normal condition (`COMP < VREF_FAULT`):

- U1 output transistor conducts and holds the output low;
- Q7 remains off;
- EN is allowed to rise through its own network.

Fault condition (`COMP > VREF_FAULT`):

- U1 releases its open-collector output;
- R5 pulls the comparator-output node high;
- R6 drives Q7;
- Q7 pulls L7987L EN low.

### 12.4 EN timing network

Current EN network:

```text
24V_PROT ── R31 100k ──┬── EN -> U5 pin 5
                        │
                     R32 15k
                        │
                       GND

EN ── C7 330n ── GND
EN ── Q7 collector
```

With Q7 off, the DC divider gives approximately:

`VEN ≈ 24 × 15 / (100 + 15) ≈ 3.13 V`

which is comfortably above the L7987L enable-high threshold.

The RC network provides a finite reset/retry interval rather than an instantaneous enable
transition.

### 12.5 AutoEN validation record

The persistent-fault simulation recorded during the design session showed repeated shutdown /
retry behavior under a maintained fault and a clean eventual restart when the fault was
removed.

As with the Bode/transient results, this is a **session-recorded simulation result**, not a new
full-corner simulation performed during the 2026-09-04 review. The open review gate is therefore
to repeat/extend the validation across the design VIN envelope and relevant component corners.

---

## 13. Repository verification and 2026-09-04 schematic review

The current branch schematic and exported netlist were checked against the intended design.

Verified from the actual implementation/netlist:

- old AP66200 is absent from the active exported netlist;
- active buck is `U5 = L7987L` in sheet `/Power/`;
- `24V_PROT` is the actual input rail;
- U5 VIN1, VIN2 and VCC are on `24V_PROT`;
- no VIN-to-VCC 0 Ω resistor remains;
- local input, VCC and comparator bypass capacitors are present;
- `L1 = 15 µH`, `D7 = STPS2L60A`, `C10 = 47 µF` are present;
- feedback values are **R36 = 49.9 kΩ** and **R34 = 16 kΩ**;
- FSW value is R4 = 47 kΩ;
- ILIM value is R29 = 47.5 kΩ;
- final compensation values are R33 = 16 kΩ, C9 = 18 nF, C8 = 39 pF,
  R35 = 1.13 kΩ and C20 = 560 pF;
- U1 TLV1701 pin mapping is correct in the exported netlist;
- U1 `IN+` is connected to U5 `COMP`;
- U1 `IN-` is connected to the R1/R2 `VREF_FAULT` divider;
- U1 V+ is on `24V_PROT` and V- is on GND;
- U1 open-collector output has the R5 pull-up and R6/Q7 base network;
- Q7 collector is connected to `/Power/EN`, emitter to GND;
- EN includes R31 = 100 kΩ pull-up, R32 = 15 kΩ pull-down and C7 = 330 nF;
- U5 PGOOD and SYNCH are explicitly no-connect;
- the existing input protection/bulk section is retained;
- the existing downstream FB1 -> `3V3_MCU` path is retained;
- the pre-FB1 output is explicitly labeled `3V3_BUCK`;
- the temporary `Buck redesign` sheet is removed from the active hierarchy.

No missing component from the intended L7987L + AutoEN electrical block was found in the
current netlist.

The review method follows the project Hardware Design Manual hierarchy: start from actual
requirements and the implemented schematic, use generic design heuristics as review prompts,
and close device-specific limits against manufacturer primary documentation before promoting a
check to a verified requirement.

---

## 14. ERC state

The committed `hardware/ERC.rpt` is dated **2026-08-09** and therefore predates the L7987L
redesign. Its errors/warnings refer to the previous power-stage implementation and cannot be used
as evidence that the current schematic passes ERC.

The exported current netlist is newer than that report. A **fresh ERC from the exact current
schematic revision is mandatory before schematic sign-off**. Every remaining ERC item must then
be either corrected or deliberately justified/documented.

---

## 15. Design-for-test / debug access

The existing project already exposes useful board-level test access including `24V_PROT`,
`3V3_MCU` and GND. For the redesigned power stage, the 2026-09-04 schematic review recommends
reviewing whether to add convenient pads/test points for:

- `3V3_BUCK` before FB1, so the regulator output can be separated from downstream filtering/load;
- L7987L COMP, to observe loop/fault saturation behavior;
- L7987L EN / the AutoEN control node, to observe reset/retry timing.

These are not declared electrically mandatory components. They are DFT/debug recommendations to
resolve before placement, while adding access is still cheap and does not require probing small IC
pins during bring-up.

---

## 16. Closed cleanup and sourcing items

Items that were previously listed as open but are now closed:

- explicit `3V3_BUCK` net label is present;
- TLV1701 metadata/datasheet is synchronized;
- final TME sourcing pass is complete;
- manufacturer/MPN fields are synchronized into KiCad;
- final footprint audit is complete, including L1 and U5;
- strict symbol/pinout audit is complete for the selected parts.

The scratch `hardware/buck_redesign_sch.kicad_sch` file remains only as a historical working
artifact and is not part of the active schematic hierarchy.

---

## 17. PCB state and mandatory next implementation sequence

The current PCB is still the pre-redesign board. It contains the AP66200 footprint and legacy
`VCC_AP66200` routing, so schematic and PCB are intentionally out of sync at this checkpoint.

Next hardware steps, in order:

1. close the remaining schematic gates: ILIM/L1 worst case, effective capacitor values and
   AutoEN corner behavior;
2. decide the additional DFT/debug test-point access for `3V3_BUCK`, COMP and EN/AutoEN;
3. run a fresh ERC on that exact schematic revision and resolve/justify every item;
4. only after schematic sign-off, update PCB from schematic and remove the old AP66200 stage;
5. place the L7987L, D7, L1 and local capacitors according to ST high-current-loop/layout
   guidance;
6. route the new power stage and reconnect the existing `24V_PROT` and FB1/3.3 V architecture;
7. perform the dedicated PCB review: switching loops, return current, thermal path, USB,
   ESP32 antenna keepout, heater/fan current paths and manufacturability;
8. run DRC;
9. regenerate BOM/CPL/fabrication/production netlist from that exact revision;
10. reconcile the generated production data against the final `BOM TME` before ordering.

Sourcing and footprint selection are already closed and are **not** a blocker for the remaining
schematic review.