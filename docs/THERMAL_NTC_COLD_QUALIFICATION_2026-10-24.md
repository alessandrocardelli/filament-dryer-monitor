# Qualificazione termica del dryer — fase NTC a freddo (24 ottobre 2026)

> **RETTIFICA TECNICA — 24 ottobre 2026, dopo prima prova fisica:** la lettura reale `air_c=19.78°C` viene da **SHT45 in camera, fisicamente distante dall'NTC sulla piastra**. La lettura NTC `126460.9 Ω` e `adc_corrected_mv=2419.8` sono dati elettrici validi, ma usare la temperatura SHT45 come temperatura del termistore per confrontare i modelli **è metodologicamente sbagliato senza prova di equilibrio termico locale**. Gli scarti calcolati dalla build R1 (`+19.98%` donor e `-0.12%` ipotesi 100k/B3950) **NON sono evidenza per scegliere R25/Beta** e non qualificano cutoff heater o curva NTC. Il Sanwa **non è facoltativo** per un confronto con temperatura reale: serve un riferimento termometrico indipendente il più vicino possibile al corpo dell'NTC, idealmente in contatto, documentandone il posizionamento. Un riferimento sulla piastra adiacente può ancora differire dalla temperatura del bead, specialmente durante flusso e riscaldamento. Un solo punto termico non identifica in generale entrambi R25 e beta. La build correttiva **THERM-241024-R2** disattiva in API e dashboard qualsiasi confronto matematico con l'SHT45 e mantiene solo letture grezze, con `comparison_available:false` e `qualified:false`. Nessun cambio heater/fan/sicurezze. Non obbligare il caricamento di R2 per misurare: R1 già fornisce le resistenze corrette, basta ignorare gli scarti errati e rilevare con Sanwa una temperatura locale indipendente. **Non eseguire ulteriori prove a caldo finché non si è definito un posizionamento affidabile della sonda.**


## Obiettivo operativo

Portare il filament dryer dalla modalità sperimentale tre impulsi alla **regolazione effettiva della temperatura in camera SHT45** con NTC indipendente di sorveglianza piastra. Questa fase è un passaggio necessario prima della verifica dei limiti e dell'abilitazione dell'uscita heater continua. Non si tratta di storico 12 ore, grafica, nuovo hardware, un test dei tre impulsi già riuscito, né sourcing TCO.

Branch di lavoro: `feature/thermal-qualification-2026-10-24`, derivato dal firmware fisicamente collaudato `feature/wifi-ota-web-logging-2026-10-24`. Identificativo nuovo firmware: **THERM-241024-R1** (non ancora confermato in hardware).

## Evidenza acquisita prima di questo aggiornamento

- SHT45 misurazione temperatura e umidità camera funziona; dashboard e Arduino OTA già riportati PASS dall'utente.
- NTC su GPIO34: correzione ADC/divisore misurata tramite sostituti resistivi il 2026-10-11; modello termico **non** validato.
- NTC 122.3 kΩ misurata col DMM e circa 124.4 kΩ nell'ESP32 a circa 20 °C in precedente prova a freddo.
- I sette punti 30–85 °C sono `R_NTC` vs **Sanwa vicino alla piastra**, con dryer **aperto e ventola OFF**: non sono una calibrazione temperatura del bead o una prova di sicurezza nel dryer chiuso; si vedano [NTC_CALIBRATION_ANALYSIS.md](NTC_CALIBRATION_ANALYSIS.md).
- Con box chiuso e ventola attiva, primo test firmware `PULSE-241024-R1` completato: tre indicazioni ON, Sanwa 19→23 °C, COMPLETE. Nessuna raccolta di un secondo punto di calibrazione NTC.
- Il TCO 105 °C è riferito come montato in serie sul prototipo. Il suo eventuale upgrade è opzionale D029 ma il pezzo presente **non** viene scavalcato.

NaN## Gate prima del termostato V1

Non modificare `kHeaterCommissioningMode=true`, `kHeaterOutputEnabled=false`, `kHeaterProfileApproved=false` o `kHeaterNtcModelApproved=false` finché non vengono definiti e revisionati con evidenze: modello e posizione NTC, limite termico reale di piastra/parti, limite aria, timeout massimo di conduzione e ciclo, risposta sensore mancante/stale, funzionamento ventola durante fault, stop/restart e protezione contro blocco del loop durante heater ON. Il precedente hardware watchdog vale solo per i brevi impulsi sperimentali, **non** qualifica il controllo continuo. Il setpoint 55 °C è soltanto un valore UI salvato, non un criterio di sicurezza.
