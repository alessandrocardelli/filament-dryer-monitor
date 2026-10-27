# STEP09-B — Registro ORIGINALE della futura prova fisica SHT45 target25°C (budget200s)

**STATO: NON ESEGUITO, NON AUTORIZZATO AL 27/10/2026.** Questo file è un modulo preregistrato, NON evidenza che S9B sia stato caricato, acceso, misurato o abbia PASS.

**Prima lettura:** [protocollo S9B](../../STEP09_S9B_NEXT_PHYSICAL_TEST_2026-10-27.md), [review numerica del budget e rischi](2026-10-27-S9B-budget-feasibility-and-safety-review.md), [S9A real dataset già archiviato su main](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md).

## Profilo proposto, non ancora autorizzato

- Branch sorgente `experiment/step09-s9b-25c-200s-2026-10-27`, PR dedicata DRAFT, compilazione/CI da registrare.
- Firmware proposto `HOT-261027-S9B`, OLED `H27-S9B`.
- Cambio rispetto S9A: **HEATER cumulativo 120→200s**, entro massimo di600s sessione. SHT45 target25°C con ON<=24/OFF>=25. NTC soft pause<=50kΩ e ripresa>=65kΩ, hard FAULT<=40kΩ, altri fault invariati. Singolo ON<=20s con ISR20,5s; gap OFF>=12s; fan PWM100%, cooling90s; Sanwa50°C veto manuale; TCO105 esistente invariato.
- Data/ora effettive, set completo letture Sanwa, conferma fisica ventola/TCO, CSV e firmware effettivamente caricati: **NON FORNITI**. Nessuna approvazione al nuovo budget è per ora registrata.

## Gate prima della sessione
- [ ] CI pass S9B sull'ultimo commit codice; job/log reali, link e SHA registrati.
- [ ] Review delle modifiche e del rischio di esposizione aggiuntiva.
- [ ] L'utente acconsente ESPRESSAMENTE a **una sola prova S9B HEATER-ON target25°C con limite cumulativo200s** e veto Sanwa50°C; il consenso S9A non vale.
- [ ] Verifica assemblaggio attuale (Sanwa punto contatto, ventola libera e funzionante, cablaggi integri, TCO105 in serie, alimentazione24V isolabile, box chiuso). Stato attuale NON RIFERITO.
- [ ] Sul dispositivo si legge **`H27-S9B`** prima di ogni HEATER ON.

## Letture durante la prova — ancora ASSENTI

| Finestra o evento | Sanwa °C (reale) | SHT45 aria °C (reale) | Ora / stato HEATER / note |
| --- | --- | --- | --- |
| Avvio da freddo | NON FORNITO | NON FORNITO | — |
| HEATER ON1 | NON FORNITO | NON FORNITO | — |
| HEATER OFF1 | NON FORNITO | NON FORNITO | — |
| Ogni ONn/OFFn successivo | NON FORNITO | NON FORNITO | Trascrivere OGNI evento, anche video |
| Primo attraversamento SHT45 25°C (se avviene) | NON FORNITO | NON FORNITO | Era HEATER già OFF o ON→OFF? |
| Spegnimento definitivo / STOP / FAULT | NON FORNITO | NON FORNITO | Causa, eventuale errore OLED |
| Ventola fermata dopo cooling | NON FORNITO | NON FORNITO | Sanwa finale |

**Il Sanwa va sorvegliato continuamente; STOP manuale immediato se arriva50°C o se fan/materiali si comportano in modo anomalo.** Non inventare OFF Sanwa dove non letto. Non confondere display, CSV ed eventuali letture termocoppia.

## Evidenza file
- `docs/evidence/thermal/2026-10-27-S9B-raw.csv`: **NON ESISTE ANCORA** (corretto finché non arriva l'originale utente).
- Nome originale file, byte, SHA256, campioni, transizioni, cause OFF, RH, NTC, trend aria/cooling: **DA ACQUISIRE**; non copiare dati S9A nel futuro CSV.
- Esito tecnico cutoff target25°C / cap200s / fault / STOP: **NON DETERMINABILE**.

## A test avvenuto
Archiviare CSV byte-identico, hash, verbale originale, report derivato e link del commit sul branch/main (solo evidenza, mai integrare automaticamente firmware HEAT). Aggiornare AGENTS, README, PROJECT_STATE, TODO, ledger, PR; mantenere `docs/DECISIONS.md` invariato salvo una vera approvazione ingegneristica.

**Nessuna prova S9B va avviata sulla base di questo file senza review e nuova approvazione dell'utente.**
