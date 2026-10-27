# STEP09-A — Verbale fisico originale HEATER-ON SHT45 feedback (modulo)

**STATO: NON ESEGUITO.** Il 27/10/2026 l'utente ha detto **«Confermo»** al primo test con target aria **25°C** e arresto manuale Sanwa **50°C**. Questa approvazione NON è una lettura strumentale, né prova che sia avvenuto il flash o che il montaggio sia stato controllato.

**Procedura integrale obbligatoria:** [STEP09_REAL_HEAT_AIR_FEEDBACK_PROTOCOL_2026-10-27.md](../../STEP09_REAL_HEAT_AIR_FEEDBACK_PROTOCOL_2026-10-27.md).

## Identità prevista (da verificare nell'esecuzione)
- GitHub PR #14: https://github.com/alessandrocardelli/filament-dryer-monitor/pull/14 (DRAFT).
- Branch: `experiment/step09-real-sht45-feedback-2026-10-27`.
- Firmware: `HOT-261027-S9A` / OLED `H27-S9A` (**NON ANCORA LETTO DAL PROTOTIPO**).
- CI verificata al commit `d643420206af8343b893edc7dfd1eea6138568e0`: [Actions 40486080140 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40486080140), test SafetyController host GPIO-mock + ESP32. Le modifiche successive prima del run sono documentali; confrontare firmware sorgenti sul commit che sarà flashato.
- Data/ora effettiva sessione (Europe/Rome): **NON FORNITA**.
- Operatore: **NON RIPORTATO**; nessun nome dedotto.
- Conferma su OLED build esatta: **NON CONFERMATA**.
- Contatto Sanwa RD701 tipo K sulla piastra presso bead NTC: **PREVISTO, DA VERIFICARE**, non presentare le condizioni S7U come certe oggi.
- SHT45 aria camera, eSUN eBOX Lite chiuso, ventola realmente ruotante, TCO105 in serie, alimentazione24V scollegabile, cavi protetti: **PREVISTO, PREFLIGHT ANCORA NON CONFERMATO**.
- Verifica controllo riscaldatore realmente OFF al boot / prima start: **NON CONFERMATA**.
- Approvazione specifica profilo/50°C Sanwa: **ACQUISITA 2026-10-27** (utente: «Confermo»).

## Parametri firmware candidati (non misure fisiche)
- Target ARIA SHT45 25°C; HEATER ON possibile quando aria<=24°C, OFF se aria>=25°C.
- NTC RAWΩ soft PAUSE<=50kΩ, ripresa>=65kΩ + gap>=12s; hard FAULT<=40kΩ / >=250kΩ.
- Massimo20s consecutivi HEAT (ISR GPIO19 20.5s), massimo120s ON totale e 600s tempo prova; ventola PWM100%, cooling90s.
- Veto manuale Sanwa >=50°C, STOP ON/OFF lungo circa1.5s; emergenza 24V OFF.
- Software stati `Phase=2 Pulse`, `Phase=3 Gap`, `Phase=4 Cooldown`, `Phase=5 Complete`, `Phase=6 Aborted`. `pulses` CSV rappresenta finestre completate, non necessariamente cicli interamente conclusi a temperatura.
- Nessuna conversione NTCΩ→temperatura bead approvata; fan100% è comando PWM, non RPM.

## Preflight eseguito dall'operatore
- Identità esatta S9A su OLED: **NON VERIFICATA**.
- Sanwa stabilmente a contatto e a temperatura ambiente: **NON VERIFICATA**.
- Ventola realmente libera e visibile all'avvio: **NON VERIFICATA**.
- TCO105, alimentazione24V, cablaggi/chiusura immutati e integri: **NON VERIFICATI**.
- Risposta STOP ed eventuale alimentatore estraibile: **NON VERIFICATA**.

## Misure REALI Sanwa — ancora assenti

| Evento | Sanwa °C | Temperatura ARIA SHT45 °C | Intervento/nota |
| --- | --- | --- | --- |
| Prima avvio (raffreddato) | NON FORNITO | NON FORNITO | — |
| Primo HEATER ON | NON FORNITO | NON FORNITO | — |
| Primo HEATER OFF (senza FAULT) | NON FORNITO | NON FORNITO | — |
| Eventuale secondo HEATER ON | NON FORNITO | NON FORNITO | — |
| Primo raggiungimento SHT45 25°C, se avvenuto | NON FORNITO | NON FORNITO | — |
| HEATER OFF definitivo / FAULT / STOP | NON FORNITO | NON FORNITO | — |
| Ventola si arresta alla fine cooling | NON FORNITO | NON FORNITO | — |

- Andamento Sanwa dopo OFF (discesa, stabilità, eventuale salita **solo se realmente vista**): NON FORNITO.
- Ventola realmente girava? NON FORNITO.
- Stato/OLED finale, causa/FAULT o limite cum heat: NON FORNITO.
- Eventuali altri eventi e note: NON FORNITO.

## CSV originale della nuova prova
- Stato: **NON RICEVUTO** — la prova non è stata ancora fatta.
- Nome file originale allegato: —
- Hash SHA256: —
- Numero campioni e intervalli MS: —
- Header e check validità sensori: —
- Tracce di heater ON/OFF, aria/RH/NTC e fan: —
- Se file non disponibile: annotare motivo, NON chiedere test ripetuto solo per recuperarlo.
- Non riutilizzare per errore CSV S7U (717 campioni), che si trova già in GitHub.

## Interpretazione e gate successivo
**NON ELABORABILE** senza dati reali. Distinguere la nuova verifica S9A feedback del riscaldatore da S7U guardia anticipata: non confondere due test. Alla ricezione delle prove effettuare commit raw originale, analisi, e aggiornare ledger/README/PROJECT_STATE/TODO. Nessuna promozione a termostato V1 né aumento a 40°C senza valutazione dei margini fisici.
