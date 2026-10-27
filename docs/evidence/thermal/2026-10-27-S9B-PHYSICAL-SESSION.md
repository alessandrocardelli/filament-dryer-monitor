# STEP09-B — Verbale della nuova prova fisica HEATER-ON target ARIA25°C (precompilato)

**STATO: NON ESEGUITO, NESSUNA MISURA FISICA S9B ANCORA FORNITA.** Preparato27 ottobre2026 per test proposto che aumenta solamente il budget HEATER ON cumulativo da120s S9A a180s; tutti gli altri limiti/soglie restano invariati. **Non confondere con S9A già eseguita e archiviata**.

**Protocollo indispensabile:** [STEP09B_REAL_AIR25_FEEDBACK_PROTOCOL_2026-10-27.md](../../STEP09B_REAL_AIR25_FEEDBACK_PROTOCOL_2026-10-27.md).
**Repo branch:** `experiment/step09b-air25-budget180-2026-10-27`; PR STEP09B draft (numero da annotare appena creata). Identità attesa OLED `H27-S9B`, firmware `HOT-261027-S9B` (**non ancora letto su prototipo**).
**CI compilazione e GPIO-mock:** non ancora riportata qui, da compilare dopo verifica dell'HEAD codice esatto.
**Approvazione utente specifica di 180s cum HEAT:** **NON ANCORA REGISTRATA** al momento creazione modulo. I 120s di S9A erano stati accettati separatamente; "procediamo col prossimo test" non contiene valori di budget/sicurezza esplicitati.
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
| Quinto HEATER OFF (se presente) | NON FORNITO | NON FORNITO | — |
| Decimo HEATER OFF (se presente) | NON FORNITO | NON FORNITO | — |
| Quindicesimo HEATER OFF (se presente) | NON FORNITO | NON FORNITO | — |
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

Nessun nuovo dato fisico; prossimo vero gate è CI ESP32+mock e review/conferma del profilo180s prima di flash/HEAT. Nessun firmware unito in `main` e nessun test automatico fisico iniziato.
