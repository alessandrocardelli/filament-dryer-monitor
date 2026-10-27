# STEP08 — Registro di esecuzione e risultati (G2/G3, integrazione post-S7U)

**STATO: NON ESEGUITO — modulo pre-registrato il 27 ottobre 2026.** Questo file serve a rendere il prossimo test riproducibile. **Non contiene risultati simulati, build o CI inventati.**
**Protocollo da seguire:** [STEP08_NEXT_TEST_2026-10-27.md](../../STEP08_NEXT_TEST_2026-10-27.md).

## Introduzione al test in parole semplici
- **Cosa:** confrontare i CSV S6C e S7U e fare un test host **nuovo** del coordinamento tra richiesta di temperatura aria e cutoff NTC reale.
- **Perché:** S7U si è fermato a `NTC R LIMIT` con Sanwa43°C e SHT45 aria19,52°C: il dryer deve imparare a regolare aria senza ignorare la protezione della piastra.
- **Da dove arriviamo:** S6C ha completato sei ON20s e Sanwa max53°C, S7U 2×20s+terzo19,25s e cutoff40kΩ. Il normale test di termostato standalone STEP03 è GIÀ PASS, NON va ripetuto.
- **Verso dove:** gate di sicurezza e integrazione software verificabili, poi eventualmente una prova fisica nuova separata e approvata.

## Fonti / vincoli
- S7U CSV: `2026-10-26-S7U-retest-raw.csv` hash SHA-256 `c417313414eede75f2269dbfb8717ac6d8246625172e315a007b8056def834ae`, 717 campioni; Sanwa letture manuali archiviate nel verbale.
- S6C CSV: branch `experiment/step07-ntc-guard-verified-history-2026-10-25`, `docs/evidence/thermal/2026-10-25-S6C-fan100.csv`, 652 campioni.
- Sorgenti di calcolo e policy: `firmware/HeaterStep07GuardTrial.h`, `HeaterPulseCommissioning.h`, `HeaterControlPolicy.h`, `SafetyController.h`; il modello NTC °C NON è approvato.
- Nessuna azione sul prototipo: no flash, no ON/OFF, no heater OFF retest, no heater ON autorizzato; solo host/repo.

## A. Calcolo dati esistenti (da riempire quando eseguito)

| Metrica | S6C dal CSV originale | S7U dal CSV originale | Metodo/script + commit |
|---|---|---|---|
| SHA originali, numero righe, anomalie | NON ESEGUITO | NON ESEGUITO | — |
| Timestamp ON/OFF e durate | NON ESEGUITO | NON ESEGUITO | — |
| Resistenza NTC per ogni transizione | NON ESEGUITO | NON ESEGUITO | — |
| Temperatura SHT45 aria/RH per transizione | NON ESEGUITO | NON ESEGUITO | — |
| Evoluzione NTC durante pause/cooling | NON ESEGUITO | NON ESEGUITO | — |
| Variazione aria dopo ultima disattivazione | NON ESEGUITO | NON ESEGUITO | — |
| Assunzioni / incertezze / Sanwa eventi esterni | NON ESEGUITO | NON ESEGUITO | — |

## B. Valutazione G2/G3 (da completare)
- Distinzione temperatura locale Sanwa/NTCΩ/aria SHT45: NON ESEGUITO.
- Adeguatezza 40kΩ guard vs requisito aria40°C, limiti inferenza: NON ESEGUITO.
- Guasto ventola non rilevato da PWM, MOSFET stuck ON, TCO105, materiali, misura sonda: NON ESEGUITO.
- Rischi aperti, priorità, criteri tecnici per nuova prova fisica eventuale: NON ESEGUITO.

## C. Nuovo test integrazione host (NON ripetere STEP03)
| Caso | Input e fault injection | Esito atteso | Esito realmente verificato | Link test/CI |
|---|---|---|---|---|
| Replay S7U | 717 campioni reali + richiesta teorica aria 40°C | la guardia NTC prevale sul termostato quando scende a40kΩ | NON ESEGUITO | — |
| No restart | phase abort a R_NTC<=40kΩ | latch FAULT, nessuna riaccensione automatica | NON ESEGUITO | — |
| Sensori invalid/stale | guasti sintetici specifici | HEAT richiesta OFF e fault | NON ESEGUITO | — |
| Ventola indisponibile | segnale simulato fan lost | HEAT richiesta OFF e fault | NON ESEGUITO | — |
| STOP e cooldown | evento STOP durante richiesta ON | heater OFF, cooldown previsto | NON ESEGUITO | — |
| Sicurezza firmware reale | revisione AppConfig + build/CI | nessun heater fisico abilitato | NON ESEGUITO | — |

**Non assimilare una richiesta virtuale ON a energia reale né usare i dati registrati per predire senza validazione il tempo necessario a 40°C aria.**

## D. Chiusura e commit
- Codice / script realmente usati: NON ESEGUITO.
- Versioni toolchain / comandi / output PASS/FAIL: NON ESEGUITO.
- Analisi/decisione G2/G3: NON ESEGUITO.
- Stato G4 replay: NON ESEGUITO.
- Commit evidenze e link: NON ESEGUITO.
- Unico NEXT dopo review: DA DECIDERE A DATI VERIFICATI. Nessuna nuova prova HEAT implicita.
