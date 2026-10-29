# Next work

## Unica priorità: dimensionare correttamente la temperatura della PIASTRA heater, SENZA test fisico ripetuto

**Ultima prova conclusa, 29 ottobre 2026:** STEP10B `HOT-261029-S10B25` (firmware letto da `/api/status`) ha portato aria SHT45 da17.28°C a picco25.10°C in un ciclo sperimentale max12min, 21 comandi ON e OFF, `cumulative_heat_ms=213939`, nessun fault, massimo gap loop163ms. L'operatore ha trascritto la sequenza delle temperature Sanwa sul contatto PIASTRA heater vicino all'NTC: plateau38–39°C, ultimo COMPLETE26°C. **Raw CSV originale byte-identico già archiviato**: [976 samples](evidence/thermal/2026-10-29-STEP10B-raw.csv), 55,054 byte, SHA256 `f587c40055cdac6103ff0b422f6244453a55ad1221a89f6d4813a8d989adf953`, [verbale con status, Sanwa, eventi e incertezze](evidence/thermal/2026-10-29-STEP10B-PHYSICAL-SESSION.md).

**Prova raggiunta ma NON ancora thermostat PASS:** ultimo HEATER OFF a+551.196s con AIR24.91°C e NTC44.952kΩ; AIR arriva25.02°C 12s DOPO lo spegnimento per inerzia, max25.10°C. Dopo AIR25, `WAIT AIR 24C` era previsto; non scende mai sotto24°C prima di720s, ultimo24.47°C. La phase5 successiva blocca comunque qualsiasi riavvio one-shot; riavvio termostatico a24°C e spegnimento diretto di heater ON al target25°C non osservati. `air_c=22.89` dello status è successivo, con fan OFF. Non rifare test soltanto per raggiungere24.

**Limitazione attuale, verificata:** heater OFF per R_NTC<=45kΩ, rearm R>=60kΩ + OFF>=12s. Da terzo impulso in poi ON circa8–10s; minima R_NTC **43.738kΩ** dopo OFF: solo3.738kΩ sopra **HARD FAULT40kΩ**. Sanwa PIASTRA38–39°C è una lettura di contatto indipendente, non il valore in °C dell'NTC. L'utente chiede ragionevolmente una piastra più calda: si tratta del prossimo problema di ingegneria V1. Non cambiare soglie numeriche unilateralmente: il test S7U ha già FAULT a40k/Sanwa43°C; S6C ha ottenuto Sanwa53°C con NTC minimo30.32kΩ in condizioni differenti.

**PROSSIMA SINGOLA AZIONE:** usare **solo prove storiche già archiviate** e STEP10B per stimare in che misura la soglia NTC SOFT45k, i 20s max, l'isteresi60k, le condizioni fan100%, il TCO105 serie e i materiali limitano la temperatura PIASTRA; sviluppare una proposta motivata per un vero limite LOCALE temperatura/resistenza e una regolazione AIR di durata utente che possa sostenere25 e poi40–55°C in modo sicuro. Documentare in repo la compatibilità dei limiti HARD/soft, inerzia e sicurezza e **ottenere accordo esplicito prima di qualsiasi nuovo HEATER ON o modifica protezioni**. La CI/firmware id nuovi non sono provati sullo stesso hash build; preservare PR#18 DRAFT isolata.

## Backlog breve

- Dopo revisione termica, una singola prova nuova che qualifichi margini piastra, eventuale ON→OFF a25°C e re-ON a24°C entro durata appropriata (non ripetere l'identico12min).
- Completa V1: duration, fan AUTO, fault/STOP e cold network/storage/power-cycle, eventuale TCO migliorativo senza bypass automatico, firmware autonomo HEATER.
- OLED boot e PCB revision solo quando richiesto da evidenze nuove.

Vedi [PROJECT_STATE](PROJECT_STATE.md), [DECISIONS](DECISIONS.md) e [THERMAL_EXPERIMENT_LEDGER](THERMAL_EXPERIMENT_LEDGER.md).
