# Step 06 — approvazione utente e binario di prova reale (25 ottobre 2026)

**Stato:** firmware HEAT compilato/testato su runner GitHub, **NON caricato**, prova **NON eseguita**. L'utente ha esplicitamente approvato la proposta di un test reale a box chiuso con ventola ON, 4×10s, gap12s, arresti NTC/SHT45/Sanwa in chat il 25 ottobre 2026; ha chiesto di accelerare e di spiegare il profilo ventola.

**Fonte:** PR [#7](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/7) = definizione candidata cold-only, **NON mergiata**, protetta; PR [#8](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/8) = **esperimento separato** heat-enabled, bozza non da mergiare nel branch operativo o `main`.

**SHA sorgente reale:** `cf2c0ecfaeabe55611bb2d2aacb6ce6c77790c9a` nel ramo `experiment/step06-supervised-heat-2026-10-25`. Identità firmware `HOT-251025-S6A` / OLED `H25-S6A`.

**CI esatta [40213105781 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40213105781)**:
- compilazione `arduino-cli` ESP32 3.3.12 / U8g2 2.36.19,
- test GPIO mock con **vera** configurazione sperimentale, 4 finestre attive da 10s, fan prerun, blocco NTC/aria/fan, STOP, no-repeat, cooldown; regressioni step01–05 PASS;
- artefatto OTA applicativo `firmware.ino.bin` archiviato come **`step06-HOT-251025-S6A-OTA`**, artifact ID `12606995251`. Solo BIN applicazione OTA, **NON** combined/full-flash image. Pesa 1.125.040 byte; SHA-256 `13fe97ad2492d52a0fc333fb827e26352f4c4dd53430f488c46e3ada996ec604`.
- Il ramo normale `kHeaterOutputEnabled=false`, `kHeaterNtcModelApproved=false`: NON è un termostato V1. La build sperimentale abilita SOLO commissioning una volta per boot e conservazione cooldown. Nessuna prova fisica o convalida ISR hardware realmente compiuta.

## Correzione ventola

La frase «100% per 5s poi 40%» era inesatta. Il codice esistente in `FanController.h` porta PWM 100% per **1s** (`kFanKickstartMs=1000`) per aiutare il rotore a partire, poi **40%** (`kFanBringupRunPercent=40`) verificato in primi test; la sequenza aspetta **5s interi da quando il comando è `FanPhase::Running`** prima del primo impulso heater. I 5s NON sono a 100%. Il 40% è un punto di test PWM, non il flusso ottimale o una garanzia di rotazione.

## Profilo e limiti autorizzati per una sola caratterizzazione

- 4×10s nominali ON, 12s gap, 5s prerun dopo kickstart, timer ISR taglio HIGH a 10,5s per singola finestra anche se il loop blocca (rischi hard fault fisici non qualificati), timeout complessivo 155s, cooldown 60s; un solo ciclo per boot.
- auto-abort su NTC in errore/stale oltre1,5s oppure `R_NTC<=19680Ω` o `>=250000Ω`, SHT45 invalid/stale oltre5s oppure aria≥40°C, fan comandata non in Running o perdita comando, STOP, timeout.
- **veto manuale Sanwa≥55°C vicino piastra**, indipendente dalla MCU e NON letto automaticamente. La soglia NTC 19,68kΩ deriva dalla correlazione Sanwa 56°C con box aperto/fan spenta, **non** è una temperatura interna del bead verificata in box chiuso. I valori non sono safety limits definitivi.
- **TCO 105°C attualmente in serie non viene toccato** (D029 add-on opzionale, ma presente nel dispositivo); nessun bypass.
- A ~24V×1,59A stimati, 40s on corrispondono a ~1.526 J: aumento rilevante vs 3×1,5s già visti. Non usare se le condizioni meccaniche/cablaggio/ventola/protezione non sono confermate.

## Preflight utente minimo, un'unica sessione (ancora OPEN)

1. Dryer SPENTO/freddo: termocoppia Sanwa fissata e leggibile dall'esterno; ventola libera, cavi nel foro jack DC protetti da spigoli/trazione e da pale; entrambi i coperchi chiusi senza schiacciamenti; TCO 105°C in serie non rimosso, 24V interrompibile immediatamente; nessun materiale combustibile vicino, mai incustodito.
2. Caricare il **file `firmware.ino.bin` OTA applicativo esatto verificato**, solo da `/ota` della dashboard locale **in STANDBY** e heater OFF (oppure procedura Arduino IDE equivalente già collaudata); il firmware ESP32 precedentemente sul dryer resta in esecuzione finché la nuova immagine non è installata. NON utilizzare `bootloader.bin`, `partitions.bin` né immagini complete su web OTA. Non avviare il ciclo prima di confermare su OLED/seriale l'ID `HOT-251025-S6A`.
3. Con Sanwa visibile e CSV/Serial registrati a 115200, verificare effettiva rotazione/soffio ventola. Operatore presente, capace di fermare mediante pressione lunga ON/OFF e tagliare 24V se necessario. **Se non si può osservare la sonda o interrompere il 24V, non partire**.
4. Avviare un solo ciclo e monitorare Sanwa+SHT45+NTC. Interrompere manualmente immediatamente se Sanwa raggiunge 55°C, sonda si distacca, aria/piastra salgono anormalmente, ventola manca, cavo/superficie mostra alterazioni o si verificano anomalie; in urgenza togliere alimentazione (prevale su eventuale cooldown della ventola).
5. Lasciare raffreddare, salvare `HOT_CSV` seriale e `/api/last-test.csv` se disponibile, annotare temperatura Sanwa all'avvio, picco durante calore e picco post-OFF; registrare qualunque STOP/FAULT e comportamento ventola. Non riavviare automaticamente per raggiungere 40°C, no incremento limiti.
6. Questa sessione fornirà finalmente informazioni reali su ritardo R_NTC / Sanwa / SHT45 con ventola in funzione e box chiuso per preparare uno **step07 termostato aria 40°C** con margini veri. Finché non abbiamo log, lo step06 non è PASS fisico.

**Prossima azione unica:** l'utente verifica preflight, effettua upload e prova solo se tutte le condizioni sopra sono vere; poi manda CSV + osservazioni Sanwa. Nessuna altra simulazione sostituisce queste misure.
