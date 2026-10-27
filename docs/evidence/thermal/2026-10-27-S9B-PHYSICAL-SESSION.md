# STEP09-B — Verbale della nuova prova fisica HEATER-ON target ARIA25°C (precompilato)

> **27/10/2026 — S9B180 autorizzato dall'utente, file OTA applicativo costruito e verificato, prova fisica NON ESEGUITA.** La dichiarazione storica precedente «approvazione da ottenere» è superata. Software: [CI run 40577584938](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40577584938), artefatto `S9B180-HEAT-CAPABLE-OTA-APP`; .bin applicativo originale SHA256 `2e4cbdf3265f66bb6028ae04b7bff8441d7860700a901f5eb6c1a9ea88bdfedc`. Setup fisico NON verificato da remoto e nessuna lettura S9B da inventare. Non chiedere nuova autorizzazione per lo stesso profilo.

> **Identity correction 2026-10-27:** the sole active draft S9B 180-second firmware is **`HOT-261027-S9B180` / OLED `H27-B180`** in [PR #17](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17). Superseded [200s PR #15](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/15) and duplicate [PR #16](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/16) are **CLOSED WITHOUT MERGE**. Older images labeled `H27-S9B` are **AMBIGUOUS** and MUST NOT be flashed. Real physical S9B still UNRUN; new operator approval for 180s budget and physical setup review still REQUIRED; NO flashing until latest firmware identity CI PASS.

> **CI S9B SOFTWARE ESEGUITA E PASSATA (non prova fisica):** [Actions run #40562778719](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40562778719), job `122395977499`: test C++ sul `SafetyController` reale con GPIO simulato **PASS** (incluso precedente120s NON termina e nuovo180s termina con cooldown, target aria25°C/isteresi, NTC soft50/resume65/hard40, fault/STOP/stale); compilazione Arduino ESP32 **PASS** con 1.126.854 byte FLASH (85%) e 92.096 byte RAM statica (28%), senza overflow. Commit testato `ca5c0e5bf6ea3f9cccd531456649e384c19eda6e`; SHA blob di tutti i file firmware e workflow verificati identici fra tale commit e il branch dopo le aggiunte dei soli documenti. **Non abilitato né installato su prototipo e nessuna qualifica di sicurezza fisica:** `H27-B180` è solo l'identità che l'utente dovrà verificare dopo un eventuale flash, previo consenso specifico alla prova. [PR #17 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17), base branch S9A PR14.

**STATO: NON ESEGUITO, NESSUNA MISURA FISICA S9B ANCORA FORNITA.** Preparato27 ottobre2026 per test proposto che aumenta solamente il budget HEATER ON cumulativo da120s S9A a180s; tutti gli altri limiti/soglie restano invariati. **Non confondere con S9A già eseguita e archiviata**.

**Protocollo indispensabile:** [STEP09B_REAL_AIR25_FEEDBACK_PROTOCOL_2026-10-27.md](../../STEP09B_REAL_AIR25_FEEDBACK_PROTOCOL_2026-10-27.md).
**Repo branch:** `experiment/step09b-air25-budget180-2026-10-27`; PR #17 DRAFT. Identità attesa OLED `H27-B180`, firmware `HOT-261027-S9B180` (**non ancora letto su prototipo**).
**CI compilazione e GPIO-mock:** PASS su commit firmware esatto (vedi nota CI sopra); NON verifica la temperatura del prototipo.
**Autorizzazione utente specifica di 180s cumulativi HEAT:** **ACQUISITA in questa conversazione il 27/10/2026**, come risposta imperativa «fallo» alla domanda esplicita sul test S9B180 (aria25°C, Sanwa STOP50°C, protezioni immutate). La conferma copre **un solo test**, non modifiche hardware/parametri o ulteriori sessioni. **Montaggio fisico e funzionalità ventola attuali non verificati da remoto**: controllare prima dell'uso.
**Data/ora fisica:** NON FORNITA.
**Setup:** Sanwa tipo K fissata stessa posizione S9A; SHT45 aria camera; box chiuso, fan libera realmente funzionante, TCO105 in serie e cablaggio/alimentazione integri: **NON CONFERMATI IN S9B**. Non desumere dal passato una verifica attuale.

## Parametri pre-registrati (NON misure)

Target aria25°C, ON possibile ad aria<=24°C, OFF al>=25°C; NTC SOFT OFF<=50kΩ/riprende>=65kΩ e gap>=12s; FAULT hard<=40kΩ o >=250kΩ, invalid/stale sensori; ON per finestra<=20s con ISR20,5s; **ON totale<=180s (NUOVO S9B)** e runtime<=600s, fan PWM comandata100% e cooldown90s; Sanwa veto MANUALE50°C, STOP lungo e24V disconnettibili.

## Registro osservazioni reale — completare SOLTANTO dopo test

| Quando leggere | Sanwa °C | SHT45 aria °C | HEATER ON/OFF, altri dettagli |
| --- | --- | --- | --- |
| Prima di avviare, dryer freddo | NON FORNITO | NON FORNITO | — |
| Primo HEATER ON | NON FORNITO | NON FORNITO | — |
| Primo HEATER OFF | NON FORNITO | NON FORNITO | — |
| Ciclo 2 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 3 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 4 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 5 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 6 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 7 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 8 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 9 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 10 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 11 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 12 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 13 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 14 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 15 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 16 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 17 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 18 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 19 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Ciclo 20 — HEATER ON / OFF (se avviene) | NON FORNITO / NON FORNITO | NON FORNITO | — |
| Primo raggiungimento aria25°C (se presente) | NON FORNITO | NON FORNITO | — |
| Inizio COOLING/STOP/FAULT | NON FORNITO | NON FORNITO | — |
| Fine cooling, ventola ferma | NON FORNITO | NON FORNITO | — |

- Sanwa massimo osservato durante intera sessione: NON FORNITO.
- Ventola realmente ruotava? NON FORNITO; CSV PWM100% non prova velocità.
- Cause precise STOP/FAULT/cooling e messaggi OLED/serial: NON FORNITO.
- Andamento temperature dopo OFF e post-cooling: NON FORNITO.
- SHT45 arrivata25°C? NON OSSERVATO/TEST NON ESEGUITO.
- Ripartenza spontanea dopo discesa naturale <=24°C? NON OSSERVATO.
- Note odore, cablaggio, TCO, posizione sonde: NON FORNITO.

## CSV originale NUOVO S9B, non quello precedente

**NON DISPONIBILE — PROVA NON ANCORA ESEGUITA.** Non riutilizzare il file S9A `docs/evidence/thermal/2026-10-27-S9A-raw.csv` (33.096 byte,778 righe). Al ricevimento salvare come `docs/evidence/thermal/2026-10-27-S9B-raw.csv` byte-identico, calcolare SHA256 e blob Git, contare campioni, annotare righe missing, analisi per eventi/NTC/aria/RH e tempo totale ON **senza assumere che il setpoint25 sia stato raggiunto**. Non inventare valori da script o simulazione.

## Decisione al momento della preregistrazione

Consenso S9B180 ricevuto e firmware OTA .bin pronto ([CI run40577584938](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40577584938), app `HOT-261027-S9B180-OTA-app.bin`, SHA256 `2e4cbdf3265f66bb6028ae04b7bff8441d7860700a901f5eb6c1a9ea88bdfedc`). **Il flash e il test fisico NON sono ancora avvenuti.** L'operatore verifica setup/fan/Sanwa/TCO/cavi, carica da STANDBY e svolge un solo test sotto osservazione; poi registra temperatura Sanwa e CSV qui. Nessun codice esperimento unito a `main`.
