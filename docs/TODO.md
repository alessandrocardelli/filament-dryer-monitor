# Next work

## Unica priorità: verificare STEP10B e caricare il firmware SENZA prova HEATER

**Sorgente attivo:** branch unico `experiment/step10-heater-diagnostics-air25-2026-10-28`, PR #18 DRAFT contro `main`. Nuovo firmware `HOT-261029-S10B25`, OLED `H29-S10B`, [protocollo completo](STEP10B_AIR_DEMAND_AND_CSV_TEST_2026-10-29.md). Il codice e' stato modificato dopo l'ultimo GPIO-mock/CI STEP10A: **non attribuire il precedente PASS al nuovo codice**.

**Cosa è cambiato:** domanda AIR a25°C mantenuta attraverso la pausa NTC45k/riarmo60k, isteresi24°C solo dopo25°C realmente raggiunti; messaggi OLED `WAIT 12s OFF / WAIT NTC 60k / WAIT AIR 24C`, API `wait_reason`, `cumulative_heat_ms`, `session_csv_saved` e `session_csv_final`; checkpoint LittleFS quando HEATER è irrevocabilmente spento e salvataggio finale dopo cooldown. HARD NTC40k, heater timer20.5s, Tmax aria40C, fanPWM100, TCO105 e veto Sanwa50°C restano invariati. Non è un nuovo limite termico qualificato; la correzione può permettere più HEAT vicino ad AIR25°C.

**Stato prova precedente:** l'operatore ha riferito18 pause brevi, 19ª HEATER OFF persistente, ventola fisicamente in moto, MAX Sanwa sulla piastra **~38°C** e aria **~23,7°C** allo spegnimento (entrambe stime). Il file allegato era byte-identico al vecchio S9B180; nessun nuovo CSV STEP10A. La condizione esatta del 19º OFF non è provata, ma la prematura richiesta AIR<=24°C dopo ogni pausa NTC è un difetto sorgente dimostrato. [Verbale originale e precisazioni](evidence/thermal/2026-10-29-STEP10A-OPERATOR-STOP.md). Non ripetere S9B o STEP10A.

**PROSSIMO PASSO UNICO:** controllare il risultato del nuovo workflow host GPIO-mock + ESP32 core3.3.12/U8g2 2.36.19 sul commit della PR#18; correggere eventuali fallimenti. Solo dopo PASS reale, caricare via Arduino IDE da ZIP del branch sperimentale con riscaldatore disalimentato, verificare `H29-S10B` su OLED e `HOT-261029-S10B25` da `/api/status`, **senza premere avvio ciclo HEATER**. Fare riferimento alla procedura Parte A del nuovo protocollo.

**Dopo:** un solo vero test termico potrà essere proposto con autorizzazione specifica alla domanda aria latch, preflight TCO105/ventola/cavi/Sanwa sulla piastra/24V isolabile, istruzioni Parte B già archiviate, acquisizione CSV a15 colonne+Sanwa. NO aumento arbitrario temperatura piastra: SOFT45k e HARD40k distano solo5kΩ e vanno riesaminati con evidenze termiche. Nessun merge su main, nessuna modifica decisione D0xx.

## Backlog breve

- Qualificare envelope temperatura PIASTRA NTC/Sanwa/materiali e margine TCO per andare verso target aria OEM40/50/55°C.
- Completare V1 autonoma: mantenimento setpoint, fan AUTO, timer, STOP, latch fault, lunga durata, persistenza robusta anche power loss in HEATER Gap (nessuna scrittura flash mentre un riarmo è possibile).
- Revisionare la PCB solo con evidenze nuove e decisione esplicita.

Vedi [PROJECT_STATE](PROJECT_STATE.md) per stato autorevole e [ledger](THERMAL_EXPERIMENT_LEDGER.md) per prove originali.
