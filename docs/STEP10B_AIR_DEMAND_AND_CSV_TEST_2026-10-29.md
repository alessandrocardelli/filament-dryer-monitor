# STEP10B — Correggere il blocco a HEATER OFF, salvare il CSV e verificare il vero termostato

**29 ottobre 2026. Stato: firmware candidato, NON ancora caricato né provato fisicamente.** Unico branch sperimentale: `experiment/step10-heater-diagnostics-air25-2026-10-28`, **PR #18 DRAFT** contro main. **NON MERGIARE su main**. Firmware identificabile con `HOT-261029-S10B25` / OLED `H29-S10B`. Prima di qualunque HEATER ON serve PASS della CI sull'ultimo **source commit** e verifica/accettazione della nuova logica di riarmo. Il consenso o il collaudo di S9B/S10A non equivalgono automaticamente a questo collaudo.

## COSA / PERCHÉ / PRIMA / DOPO

**COSA:** completare il vero controllo a due variabili: SHT45 richiede calore FINO a quando la camera raggiunge 25°C; il limite locale NTC può mettere il riscaldatore in pausa senza cancellare la domanda di calore. **Solo dopo aver realmente raggiunto 25°C**, il successivo riarmo termostatico richiede aria<=24°C. Correggere la schermata di attesa e il salvataggio dell'intera sessione.

**PERCHÉ:** l'operatore ha osservato **18 cicli con pause HEATER OFF brevi, poi dopo il19° HEATER OFF persistente**, ventola ancora fisicamente rotante, e ha spento. Ha ricordato un massimo Sanwa di **circa 38°C SULLA PIASTRA presso NTC** e aria SHT45 **intorno23,7°C** allo spegnimento; dati indicativi, non sincronizzati, NON un CSV nuovo. Il `filament-last-test (5).csv` scaricato dopo è stato verificato **identico byte per byte al vecchio S9B180** (44.461 byte, 1.067 dati, 10 colonne, SHA256 `1e17e6e131ce7cf9074bfe8d23d31f5e8ea1a605833ff36c0a21fe6568314beb`). Non esistono dati crudi persistenti STEP10A della nuova prova; la causa istantanea del 19° OFF rimane ignota. **La stima AIR~23,7°C allo stop, se accurata, era gia' sotto24°C**: il solo bug di riarmo aria non spiega necessariamente quello specifico arresto. Potrebbe non essersi recuperato NTC>=60kΩ o esserci stato un altro stato/fault non registrato. La correzione risolve un difetto reale di codice, NON una causa fisica certa. **Nel codice STEP10A è stato trovato un errore vero:** `in.airC <= targetAirC - 1` era richiesto per TUTTE le ripartenze, anche dopo una pausa NTC senza aver mai raggiunto25°C.

**PRIMA:** S9A budget120s con aria picco22,97°C/Sanwa piastra<=38°C; S9B180 aria picco23,77°C, guasto AirStale4, soft50k; STEP10A soft45k/resume60k, HARD40k invariato, nessun nuovo CSV, osservazione HEATER OFF dopo19 cicli e non risolta.

**DOPO:** il primo risultato utile è determinare se il firmware raggiunge **25°C aria e interrompe realmente il riscaldamento**, senza perdere traccia in caso di STOP. Solo con una nuova evidenza di piastra, SHT45 e NTC si definisce il successivo envelope per mirare ai livelli aria OEM 40/50/55°C. Non aumentare a caso temperature o tempo.

## Correzioni software STEP10B

1. **Termostato aria a domanda latched:** sotto25°C aria mantiene domanda di calore attraverso OFF NTC, con pausa minima12s + recupero NTC60kΩ; a25°C cancella domanda, poi riabilita solo sotto/uguale24°C. NTC HARD40kΩ e tutte le protezioni indipendenti prevalgono SEMPRE. Questa correzione puo' produrre **più accensioni rispetto a STEP10A vicino ai 25°C**, quindi e' una nuova esposizione da sottoporre a gate fisico.
2. OLED `H29-S10B` mostra `WAIT 12s OFF`, `WAIT NTC 60k`, `WAIT AIR 24C`, `HEATER ON`, `COOLING`, `TEST COMPLETE`, `FAULT` secondo il vero stato. Corregge le vecchie scritte STEP09B/tempo10min sul profilo STEP10B.
3. `/api/status` mantiene tutti i campi vecchi e aggiunge `wait_reason` (0 nessuna, 1 OFF min, 2 NTC recupero, 3 aria isteresi), `cumulative_heat_ms`, `session_csv_saved`, `session_csv_final`. `saved_log` da solo indica solo che esiste **qualche** file storico, non prova il salvataggio dell'ultima prova.
4. File `/last-test.csv`: dopo che il controllo passa irreversibilmente a `Cooldown=4` o `Aborted=6` con heater OFF e NON più riarmabile nella sessione, il firmware scrive un **primo checkpoint** della traccia; durante il raffreddamento continua ad acquisire RAM e salva la **versione finale** dopo cooldown. Questa procedura evita flash durante qualsiasi HEATER ON o HEATER OFF da cui possa ripartire. Può comunque perdere i dati dell'ultimo tratto di raffreddamento se la corrente viene tolta subito, ma protegge la parte di riscaldamento quando la fase terminale è raggiunta. **Non esiste una garanzia di salvataggio se si interrompe l'alimentazione DURANTE una pausa attiva (Phase3)**. Viene tentato rollback del vecchio CSV in caso di errore di rename.
5. Il CSV resta a15 colonne: `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent,pulse_fault,ntc_fault,air_age_ms,ntc_age_ms,loop_gap_ms`. Il browser non chiede l'intera cronologia durante il ciclo.

## Valori di prova INVARIATI da STEP10A (nessuna nuova qualifica termica)

| Guardia | STEP10B |
| --- | --- |
| SHT45 setpoint aria / riarmo dopo target | 25 / 24°C |
| NTC R locale PAUSA / RIARMO | <=45kΩ / >=60kΩ |
| NTC R HARD FAULT | <=40kΩ (non cambiato), open >=250kΩ |
| ON singolo / hardware ISR | max20s / 20,5s |
| OFF minimo | >=12s |
| Totale HEATER ON / sessione | max300s / 720s (12min) |
| Aria limite guasto | 40°C |
| NTC age / SHT45 age | 1,5s / 5s (non allentati) |
| Ventola | PWM100% comandato; nessun tachometro |
| Cooldown | 90s (controllato, non prematuramente interrotto) |
| Veto operatore | Sanwa **SULLA PIASTRA VICINO ALL'NTC**: stop immediato >=50°C |
| TCO esistente | 105°C segnalato in serie, **non rimuovere né bypassare** |

Il termometro Sanwa misura davvero la PIASTRA. Il riferimento OEM eSUN eBOX Lite riporta temperature nominali piastra105/116/118°C e aria40/50/55°C, ma il punto di misura OEM e i limiti del nostro cablaggio/materiali/TCO modificato non sono stati qualificati; **non autorizza a impostare118°C né a sostituire le protezioni**. La lettura manuale di38°C suggerisce che il controllo attuale sia ancora troppo conservativo per un dryer utile, non fornisce una soglia locale nuova sicura. Per spostare SOFT più in basso dovremmo valutare anche il margine con HARD40k e l'inerzia, non soltanto aumentare il tempo.

## Gate software obbligatorio — fonte GitHub

- Entrare in [PR#18](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18) e aprire **Checks / Actions** per `STEP10B - chamber-demand latch and CSV checkpoint`. Usare **solo un PASS verificato** sul commit effettivamente compilato (GPIO-mock, STEP08 replay, ESP32 core 3.3.12, U8g2 2.36.19). Una CI precedentemente superata per `HOT-261028-S10A25` **NON** valida STEP10B.
- Richiedere l'esatta identità `HOT-261029-S10B25` / OLED `H29-S10B`. Non installare vecchi ZIP/bin S9B/S10A.
- Passare il gate compilazione/identità PRIMA di pensare a un test fisico. Software PASS non qualifica riscaldamento o RPM.

## Arduino IDE, caricamento da sorgente — PARTE A SENZA TEST HEATER

1. Nel branch `experiment/step10-heater-diagnostics-air25-2026-10-28` selezionare **Code → Download ZIP** ed estrarre. Aprire `firmware/firmware.ino` in Arduino IDE 2.x, mantenendo tutti gli altri file dello sketch nella stessa cartella `firmware`. **Non** usare `main` (ha il heater locked e non include questa versione sperimentale).
2. Verificare board **ESP32 Dev Module** (`esp32:esp32:esp32`), core **Espressif ESP32 3.3.12**, libreria **U8g2 2.36.19**, stesso schema partizioni OTA del prototipo. Compilare prima con **Verify**.
3. Per upload usare la procedura USB/seriale o ArduinoOTA **già verificata**, con riscaldatore elettricamente non alimentato e prototipo in condizioni idonee. Se il firmware precedente è in FAULT, l'OTA applicativo può essere rifiutato: non forzare upload durante FAULT o cooldown; un riavvio controllato a freddo è diverso da riavviare un ciclo HEATER. Non improvvisare cablaggi.
4. A fine upload verificare OLED **H29-S10B**, e dalla propria rete `http://filament-dryer.local/api/status` verificare `"firmware":"HOT-261029-S10B25"`. Il pulsante ON/OFF **NON va premuto per avviare un ciclo in questa parte**. Se identità diversa, STOP. Un upload da solo non certifica la prova.

## Unico successivo test fisico HEATER ON — PARTE B CON NUOVO CONSENSO E PREFLIGHT

**NON avviare** senza consenso informato al nuovo comportamento (domanda aria mantenuta tra pause, quindi più heat possibile), verifica CI dell'immagine caricata e check fisico attuale. L'utente aveva riferito Sanwa fissato sulla piastra presso NTC in precedenza: la sua posizione è ACQUISITA, **non richiederla di nuovo**; verificare solo se e' ancora fissato per motivi di sicurezza.

1. A freddo, accertare fisicamente: Sanwa fissato sulla piastra, cavi integri/non schiacciati, TCO105 ancora in serie, ventola libera e fisicamente funzionante, box correttamente chiuso, alimentazione24V immediatamente isolabile. Nessun filamento/materiale infiammabile durante prova. Se una condizione di sicurezza non è soddisfatta **non iniziare**.
2. Leggere `/api/status` in standby: firmware esatto e sensori aria/NTC validi. Se aria iniziale>=25°C il controllo non dovrà fare HEATER ON; non forzare una prova a temperatura troppo alta.
3. Restare davanti al dispositivo per tutta la sessione. Premere ON/OFF una sola volta. Osservare prerun ventola; registrare Sanwa **PIASTRA** e display a INIZIO, primi tre ON/OFF, poi ogni ON/OFF (video continuo ammesso), **temperatura massima**, andamento dopo OFF; annotare ogni indicazione `WAIT...`, in particolare quando si avvicina a25°C aria. Non aprire ripetutamente la pagina CSV, non scambiare il comando PWM con misura RPM.
4. L'obiettivo principale è verificare **il passaggio da una pausa NTC a riaccensione quando aria e' tra24–25°C, senza aver ancora raggiunto25°C**, e successivamente il vero cutoff SHT45 a25°C con riarmo a24°C **solo se avviene naturalmente nella stessa sessione**. Non prolungare o ripetere per ottenere per forza il risultato.
5. **STOP IMMEDIATO** a Sanwa>=50°C sulla piastra, ventola fisicamente ferma, odore, cavi/anomalie calore, comportamento inatteso, guasto, o se non è possibile monitorare. STOP manuale è pressione **lunga ON/OFF in Drying**, ma per emergenza elettrica isolare24V. Nessuna ripartenza automatica.
6. Dopo STOP/fine/FAULT **mantenere alimentazione per il cooldown**, se elettricamente sicuro; la ventola deve continuare90s, CSV checkpoint deve essere persistito non appena risulta impossibile una nuova accensione, CSV finale dopo cooldown. `/api/status` espone `session_csv_saved` (checkpoint o finale) e `session_csv_final` (completo); `/api/events` mostra `NEW CSV CHECKPOINT` e poi `NEW CSV FINAL READY`.
7. A dispositivo fermo scaricare `http://filament-dryer.local/api/last-test.csv`, `/api/status`, `/api/events`. **Verificare intestazione 15 colonne e ID firmware nella risposta status**, non affidarsi al nome generico `filament-last-test.csv`. Allegare CSV originale e Sanwa in chat; l'assistente salva byte-identico CSV con SHA256, registra letture e aggiorna PROJECT_STATE/TODO e ledger. Non riaccendere HEATER per acquisire dati persi.

**FUTURO:** al termine avremo o il primo controllo aria25°C realmente verificato, o dati espliciti sulle condizioni che lo hanno impedito. Solo allora decidere un envelope piastra più realistico attraverso correlazione NTC/Sanwa/materiali; non anticipare numeri non approvati.

## Esiti che questo firmware NON garantisce

- Non promette25°C aria, ne' test40/50/55°C, ne' 118°C piastra; nessuna qualifica termica di produzione V1.
- Se l'alimentazione viene tagliata durante HEATER ON/Gap con futuro HEATER possibile, non si scrive flash in anticipo; e' un compromesso **voluto** di sicurezza e potrebbe non esserci nessun CSV nuovo. **STOP controllato** seguito da cooldown è il percorso da usare, non power-off ordinario immediato.
- La correzione della domanda aria elimina un difetto logico certo in sorgente, ma **non prova** che fosse l'unica causa dei 19 cicli della sessione senza CSV.
