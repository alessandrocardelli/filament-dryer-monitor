# STEP09-B — Firmware HEATER ON a 25°C aria, budget180s — checkpoint software finale

**STATO 27 OTTOBRE 2026: VERSIONE CANDIDATA UNICA PR#17; CI DEL CODICE EFFETTIVO PASS; PROVA FISICA NON ANCORA AUTORIZZATA/NON ESEGUITA.** Nessun flash via questi strumenti, nessuna lettura fisica S9B o CSV S9B.

## COSA / PERCHÉ / PRECEDENTE / FUTURO

**COSA:** firmware sperimentale HEATER-capable con GPIO19 realmente azionabile, SHT45 ARIA target25°C, massimo HEATER ON cumulativo **180 secondi** e protezione locale NTC in RAW ohm.

**PERCHÉ:** nella S9A, 13 ON e ~120s cumulativi si sono conclusi quando SHT45 aria era a22,59°C. L'aria non ha raggiunto25°C; serve una prova diversa per osservare se e quando il controller HEAT spegne il MOSFET in risposta al setpoint aria.

**PRECEDENTE:** [S9A dati reali](2026-10-27-S9A-PHYSICAL-SESSION.md), [CSV originale completo](2026-10-27-S9A-raw.csv), [analisi](2026-10-27-S9A-analysis.md). Sanwa17°C iniziale, ON/OFF stabilizzati35/38°C, fine cooling26°C, aria SHT45 min iniziale17,64°C e picco22,97°C. Nessun FAULT.

**FUTURO:** una e una sola prova fisica supervisionata **solo dopo nuova approvazione dell'utente per l'estensione 120→180s e preflight dell'hardware**; acquisire letture Sanwa + CSV nuovo. Non ripetere S9A, non partire da un altro test fisico a heater OFF e non aumentare target o soglie in automatico.

## Repository e identificativi senza ambiguità

- Repository: `alessandrocardelli/filament-dryer-monitor`.
- **Unico branch scelto:** `experiment/step09b-air25-budget180-2026-10-27`, [PR #17 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17), su base esperimento S9A PR#14.
- Firmware completo univoco: **`HOT-261027-S9B180`**; stringa breve OLED **`H27-B180`**. Contrassegno diverso dal vecchio `HOT-261027-S9B`/`H27-S9B`, **AMBIGUO** in tre immagini 180s e200s precedenti. **Non usare immagini con il vecchio ID.**
- [PR #15 200s](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/15) e [PR #16 duplicata200s](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/16): **entrambe CHIUSE SENZA MERGE**. I branch restano come cronologia per audit, non candidati da caricare.
- **`main` non ha ricevuto il firmware sperimentale HEAT.** Solo documenti di analisi/protocollo/evidenza possono essere copiati in `main`.

## Unica variabile termica modificata rispetto a S9A

| Parametro | S9A prova già eseguita | S9B180 candidato |
| --- | ---: | ---: |
| Target SHT45 ARIA |25,0°C| **25,0°C**, invariato |
| OFF se aria>=25°C, nuova ON se aria<=24°C | sì | invariato |
| NTC R SOFT stop e riarmo | <=50kΩ e >=65kΩ | invariato |
| NTC R FAULT duro | <=40kΩ (anche aperto>=250kΩ) | invariato |
| Singolo HEATER ON, hardware ISR | max20s / 20,5s | invariato |
| Pausa OFF minima | 12s | invariato |
| **Budget HEATER ON cumulativo** | **120s** | **180s**, solo cambiamento termico (+50%) |
| Durata massima runtime |600s| invariato |
| Ventola PWM comandato, cooling |100%,90s| invariato |
| Veto umano Sanwa |stop>=50°C| invariato |
| TCO105 in serie | riferito montato | invariato, NON bypassato |

Non presentare R_NTC convertita in°C di bead qualificata, PWM100 come tachimetro o il TCO105 come protezione DC certificata. Il funzionamento della versione V1 resta da qualificare.

## Verifica software reale su GitHub

**ESECUZIONE FINALE LINKATA AL CODICE FINALE:** [GitHub Actions run #40572680240](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40572680240), job `122430763656`, conclusione **SUCCESS**. Commit testato `3ab2d318b292cc035c9ebd4bfa334bba5f882fa9` (include identità, posizione OLED e asserzioni CI); il branch al momento del controllo era esattamente su quel commit, nessun cambiamento codice successivo. Ambiente CI Ubuntu24.04, esp32:esp32 core3.3.12 + U8g2 2.36.19.

Estratto dai **log reali**, non da ipotesi:

```
PASS: STEP09B 180s budget, same NTC/fan limits, no V1 heater approvals
PASS STEP09B actual SafetyController GPIO-MOCK: real ON, SHT45 25C feedback/hysteresis, NTC soft50k/hard40k, 20s/12s windows, late 25C SHT45 cutoff after >120s ON, new180s cap, STOP, cooldown, stale/fault/no restart. NO REAL GPIO.
PASS STEP09B native fixture. NO ESP32 connected or heater energized.
Sketch uses 1126854 bytes (85%) of program storage space. Maximum is 1310720 bytes.
Global variables use 92096 bytes (28%) of dynamic memory, leaving 235584 bytes for local variables. Maximum is 327680 bytes.
PASS: ESP32 binary linked with no DRAM overflow
```

Il test C++ usava il vero `SafetyController` con stub GPIO; dopo 7 finestre HEAT da20s (oltre il vecchio limite120s), SHT45 arriva a25°C mentre NTC è90kΩ e il comando heater va OFF; riarmo bloccato fino a24°C. Testate anche fine a180s, collaudo logico FAULT e cooldown. **NESSUNO di questi test alimenta il riscaldatore fisico** o valida l'ISR hardware in tempo reale, l'effettivo flusso FAN, il TCO DC, le temperature materiali o il MOSFET guasto ON.

### Controllo della leggibilità OLED 128x64

Con font a larghezza6 pixel, `FW` a x=2 occupa fino a14px; `HOT-261027-S9B180` (17 caratteri) inizia a x=20 e termina a122px senza sovrapposizioni né clipping. `H27-B180` (8 caratteri) in standby da x=78 termina a126px. Entrambi i limiti di geometria sono ora **asseriti nella pipeline**. La leggibilità reale OLED è comunque un futuro riscontro dell'utente dopo il flash, non una misura eseguita nel mock.

## Remaining gate — non saltarlo

- [x] Codice S9B180 con identificatori univoci e differenza termica limitata al budget.
- [x] GPIO mock, SHT45 target tardivo e fault/STOP, build ESP32 CI reale PASS sui log.
- [x] PR #15 e #16 200s chiuse senza merge, PR #17 unica candidata.
- [ ] L'utente ha approvato esplicitamente **una prova fisica S9B180 HEATER-ON 25°C aria con budget max180s cumulativi** (il «Procedi» riguardava lavoro software, il consenso S9A era solo per120s).
- [ ] Preflight dell'operatore: Sanwa sulla stessa zona piastra accanto NTC di S9A; FAN fisica libera e realmente rotante; TCO105 sempre in serie; cavi integri, enclosure invariata, alimentazione24V immediatamente scollegabile.
- [ ] Solo dopo, upload in STANDBY della versione esatta e riscontro OLED **`H27-B180`**, poi una sola prova fisica con [protocollo chiaro](../../STEP09B_REAL_AIR25_FEEDBACK_PROTOCOL_2026-10-27.md), Sanwa a **OGNI HEATER ON e HEATER OFF** o video OLED+Sanwa continuo, all'eventuale aria25°C, a inizio cooling e fine cooling; **STOP IMMEDIATO SANWA>=50°C** e su anomalie. Scaricare CSV dopo cooling e compilare [registro fisico S9B](2026-10-27-S9B-PHYSICAL-SESSION.md).

**Nessuna modifica a `docs/DECISIONS.md`**: nessuna soglia definitiva V1 è stata approvata, solo un candidato di prova.
