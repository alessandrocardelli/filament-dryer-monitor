# S9B — Verbale prima della futura prova fisica HEATER ON (DA COMPLETARE)

**STATO 27/10/2026: NON ESEGUITO, NON AUTORIZZATO PER FLASH O HEAT.** Questo è un modello da compilare **solo con letture reali fornite dall'utente** dopo un'eventuale nuova sessione. **Non esiste ancora alcun CSV S9B**. Non riciclare `2026-10-27-S9A-raw.csv` come se appartenesse a questa sessione.

## Identità e prerequisiti

- Branch: `experiment/step09b-chamber25-extended-budget-2026-10-27`, [PR #16 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/16), base S9A.
- Firmware candidato S9B: `HOT-261027-S9B` / OLED `H27-S9B`; **NON FLASHATO**.
- CI software prima della prova: [GitHub Actions run40562607894 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40562607894) con linker reale ESP32 + mock `SafetyController`. Non sostituisce test fisici.
- Protocollo obbligatorio: [`docs/STEP09B_PHYSICAL_PROTOCOL_2026-10-27.md`](../../STEP09B_PHYSICAL_PROTOCOL_2026-10-27.md). Introduzione cosa/perché/precedente/futuro, timing letture Sanwa e safety/stop in quel documento.
- Nuova autorizzazione specifica dell'utente a **max200s HEATER comandato**: **NON ACQUISITA** (il consenso S9A era per120s).
- Preflight TCO105 in serie, Sanwa a contatto piastra, ventilazione reale, cavi, enclosure e 24V isolabile: **NON RIPORTATO PER S9B**.
- Ora reale prova (Europe/Rome): **NON FORNITA**.

## Profilo PREVISTO, non misurato

- SHT45 aria setpoint25°C, ON permesso<=24°C, OFF a>=25°C (se si raggiunge sotto carico), hysteresis1°C.
- Resistenza NTC soft OFF<=50kΩ, ripresa>=65kΩ + gap>=12s; hard FAULT<=40kΩ o>=250kΩ.
- Max HEATER ON per singola finestra20s (hardware ISR20,5s), max cumulativo200s, time cap600s, ventola PWM100 comandata, cooldown90s.
- Sanwa manuale STOP a>=50°C **in ogni momento**. TCO105 esistente invariato.
- SHT45 e Sanwa misurano **luoghi diversi**, non devono coincidere; R_NTC ohm non convertita in temperatura certa.

## Cronologia eventi REALI — ancora ASSENTE

| Evento | Sanwa °C reale | SHT45 aria °C | Osservazione HEATER e note |
| --- | --- | --- | --- |
| Prima di accendere (freddo) | NON FORNITO | NON FORNITO | — |
| Primo HEATER ON | NON FORNITO | NON FORNITO | — |
| Primo HEATER OFF | NON FORNITO | NON FORNITO | — |
| ON/OFF successivi, uno per finestra, fino allo STOP | NON FORNITO | NON FORNITO | — |
| Eventuale attraversamento SHT45 25°C | NON FORNITO | NON FORNITO | — |
| HEATER definitivamente OFF/COOLING/STOP/FAULT | NON FORNITO | NON FORNITO | — |
| Ventola ferma dopo cooldown | NON FORNITO | NON FORNITO | — |

**CSV originale:** NON DISPONIBILE. Nome upload, byte, SHA256, Git blob, campioni, timestamps, integrità file, dati `air_c`/`ntc_ohms`/`phase`/`heater_command`/`fan_percent`: da aggiungere **solo dopo ricezione vera**.

## Criteri da esaminare SOLO dopo una prova autorizzata

- Raggiunti25°C aria? Se sì, `HEATER ON→OFF` causato dal raggiungimento target **o** già OFF per NTC? Interpretare il CSV riga per riga; la seconda eventualità non dimostra un cutoff termostatico sull'uscita durante ON.
- Se mai raggiunti25°C, fine per budget HEAT200s o timer massimo600s? Non inventare.
- NTC SOFT protetta come previsto? HARD non superata? Guasti / tach mancanti?
- Sanwa vicino alla piastra e andamento dopo OFF, incluso il raffreddamento. Nessuna inferenza su temperatura massima del bead o TCO senza misure.
- Non dichiarare la V1 qualificata né aumentare setpoint o limiti senza ulteriore review.

**S9A (fatto) ≠ S9B (non fatto).** I dati S9A stanno su `main`; non serve ripetere S9A né fare test heater-OFF.
