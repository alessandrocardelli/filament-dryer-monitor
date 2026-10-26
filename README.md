# Filament Dryer Monitor

> **INDICE UNICO DELLE PROVE TERMICHE:** [registro S6A → S6C → S7G → S7U, dati esistenti/mancanti e gate V1](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/THERMAL_EXPERIMENT_LEDGER.md). Verificato il 26 ottobre 2026; usare insieme al protocollo S7U sopra, non in sostituzione dei CSV/verbali.

> **26 OTTOBRE 2026 — NUOVA PRIORITÀ CONFERMATA DALL'UTENTE / CONTINUITÀ OBBLIGATORIA:** l'utente ha scelto di **ripetere UNA nuova acquisizione termica supervisionata S7U**, senza recuperare il CSV della vecchia sessione S7G e senza ripetere i test a heater spento/OTA/menù/ventola già chiusi. Protocollo completo (posizione Sanwa, limiti, manovre, letture al FAULT e picco post-OFF, salvataggio dati, gate fino alla V1): [STEP07 S7U runbook](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/STEP07_S7U_PHYSICAL_RETEST_PROTOCOL_2026-10-26.md). Verbale pre-creato con tutti i campi e stato NON ESEGUITO: [STEP07 S7U sessione](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md). **La nuova prova non è ancora stata svolta, nessun CSV nuovo esiste**. S7U `HOT-261026-S7U` CI PASS ma non collaudato fisicamente; protezioni termiche identiche S7G, uso solo in supervisione. **Veto umano Sanwa 50°C** comunicato nell'ultima procedura S7G, non confondere con 55°C della più vecchia S6C. Il vecchio obiettivo «recuperare CSV S7G» è SUPERATO dalla nuova scelta esplicita; mantenere quel dato mancante nello storico, non richiederlo. Ogni sessione futura richiede dossier e commit secondo `AGENTS.md`.

> **26 ottobre 2026 — HANDOFF CANONICO S7G→S7U E PROSSIME MODIFICHE:** [leggere per primo il passaggio consegne completo](docs/HANDOFF_2026-10-26_STEP07_S7G_TO_S7U.md). S7G è **l'ultimo firmware testato fisicamente**: Sanwa 17°C iniziale, 36°C prima dell'impulso3, 43°C al FAULT; **CSV S7G ancora mancante** (codice/timing trip da verificare), massimo Sanwa POST-OFF sconosciuto. FAULT resta latched anche dopo ventola cooling; buzzer S7G si ripete finché MODE breve lo silenzia; CSV si salva soltanto dopo cooling. [PR #12](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/12) prepara già il firmware **S7U** con OLED HEATER ON/OFF chiaro, countdown/CSV READY/CSV FAILED e buzzer test a singolo allarme; [CI S7U PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40382200144), MA S7U non è stato caricato/testato fisicamente. **P0**: recuperare/analizzare CSV S7G senza ripetere riscaldamento. **P1**: nuovo prototipo termostato SHT45 solo simulato HEATER OFF, safety review; no merge involontario esperimenti su branch operativo o main.

> **2026-10-26 — STEP07 S7G PROVA REALE / UX BLOCCANTE:** utente ha osservato Sanwa **17°C prima del primo impulso, 36°C prima del terzo, 43°C alla comparsa di FAULT**. La causa numerica del FAULT e il punto R_NTC devono ancora essere verificati nel CSV che non è stato consegnato. Il **picco Sanwa dopo il taglio heater resta sconosciuto**, non richiedere ripetizione a caldo solo per registrarlo. Il firmware S7G aveva FAULT permanentemente latched (corretto), buzzer FAULT ripetuto all'infinito (inutilizzabile per il collaudo; MODE breve lo silenzia), display senza chiara indicazione di HEATER OFF o countdown e CSV visibile solo dopo cooling. [PR #12](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/12) prepara **solo correzioni di feedback OLED e allarme di prova**, nuovo ID `HOT-261026-S7U`: grande `HEATER ON/OFF`, `FAULT / HEATER OFF`, `SANWA: WATCH PEAK`, tempo residuo cooling, `CSV READY` verificato per nuova sessione, avviso acustico singolo anziché infinito. **FAULT termico, NTC40kΩ, fan100 e impulsi invariati; D022 buzzer ripetuto in modalità normale resta invariata. Non ripetere prove a caldo solo per il display.** [Rapporto/protocollo nel ramo fix](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/STEP07_OPERATOR_UI_FIX_2026-10-26.md).

> **2026-10-25 — AUDIT REPOSITORY COMPLETO SORGENTI/TESTO, GATE STEP07-G:** letto bootstrap AGENTS, README, PROJECT_STATE storico (spec. confronto Sanwa/NTC dinamico 15ott), DECISIONS D001–D029, TODO, tutti i documenti tecnici applicabili, firmware/CI/sketch archiviati e KiCad schema/netlist/PCB; materiali binari STEP/PDF/immagini inventariati, **non** rivisti pagina per pagina. [Audit](docs/REPOSITORY_FULL_REVIEW_2026-10-25.md). Le misure S6C originali da652 righe sono ora conservate come [CSV repository](evidence/thermal/2026-10-25-S6C-fan100.csv); il nuovo test CI le rilegge. Nuovo firmware sperimentale separato [PR #11](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/11), `HOT-251025-S7G`: fan100%, stesso MAX 6×20s S6C e watchdog GPIO 20,5s, ma stop NTC raw **anticipato da19,68kΩ a40kΩ** e latch FAULT. Il replay dei dati S6C attraversa 40kΩ durante il 3° ON t~89,897s, non prova temperatura Sanwa o sicurezza. TCO105 presente, differenza piastra/aria e lag Sanwa preesistente restano vincoli; **non** abilitare il termostato automatico, non aumentare esposizione termica. Compilazione/test CI stage07 necessari prima di considerare codice pronto; un nuovo test fisico richiede sorveglianza e approvazione separata. `main` invariato.

> **2026-10-25 — STEP06B PROVA REALE FAN100 ACQUISITA:** S6C `HOT-251025-S6C` box chiuso, Sanwa utente **18°C iniziale → max53°C durante heater →30°C a fine cooling**. **Osservazione essenziale:** Sanwa in riscaldamento **rispondeva in ritardo rispetto alla variazione NTC**; ritardo in secondi non misurato, non sostituire R_NTC con una falsa T_NTC e NON assumere 53°C massimo assoluto della piastra. CSV utente `filament-last-test (1).csv`, **652 campioni/275,664s**, 6 finestre heater-command ON di~20s (fino a120s nominali), fan PWM100% in tutte le righe (comando, non RPM), SHT45 aria **18,64→22,57°C alla fine HEAT→23,98°C fine cooling** (+5,34°C totale; +1,41°C dopo OFF), NTC resistenza **131,95→min30,32kΩ→77,21kΩ a fine log**. Sensori validi, nessun arresto automatico documentato; CSV finisce in phase4 cooling, NON certifica da solo COMPLETE. Soglia NTC automatica 19,68kΩ non raggiunta mentre Sanwa a53°C è soli2°C sotto veto manuale55°C: **margine Sanwa insufficiente per aumentare ON/temperature senza riqualificare limite automatico piastra e inerzia**. La prova S6C varia **sia duty fan sia energia heater** rispetto S6A; non attribuire aumento aria al solo fan. **NEXT Step07** rivedere protezione piastra in resistenza, Sanwa delayed/non sincronizzato, timer/fan/cooldown prima di autorizzare termostato continuo. Nessun nuovo flash/prova deciso. [Analisi completa](docs/STEP06B_FAN100_PHYSICAL_RESULTS_2026-10-25.md).

> **2026-10-25 — NEXT REALE STEP06B FAN100 / 6×20s: PREPARATO, CI PASS, NON ANCORA ESEGUITO:** dopo la prima caratterizzazione S6A (fan40%, heater 4×10s, Sanwa locale 18→40°C, aria SHT45 18,51→19,52°C), la vecchia **D025 40%** è confermata solo politica di test *avviamento PWM* (25% primo avvio, 15% regime minimo, 40% margine): **non è una scelta aerotermica**. Nuovo firmware sperimentale `HOT-251025-S6C` su branch `experiment/step06b-fan100-extended-2026-10-25`, [PR #10](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/10), **CI esatto PASS [#40220089554](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40220089554)**: FAN **100% sempre**, 6×20s HEAT max nominale120s, gap12s, prerun5s, ISR cutoff20,5s ogni finestra, cooldown90s, timeout330s, logger 350s, NTC raw abort ≤19,68kΩ / SHT45 **aria** ≥40°C immutati, veto manuale Sanwa piastra ≥55°C. Energetica massima nominale stimata **4,58kJ** (3× vecchia S6A), non qualificazione V1, né certezza raggiungimento aria40°C. `main` e branch operativo freddo invariati; il dispositivo fisico riporta ancora S6A finché utente non aggiorna. **NEXT:** GitHub Desktop branch dedicato → Arduino IDE OTA, solo previo preflight box/sonda/cavi/TCO/ventola/24V isolabile, una prova sorvegliata e CSV+Sanwa. NON ripetere S6A. [Protocollo Step06B](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step06b-fan100-extended-2026-10-25/docs/STEP06B_FAN100_EXTENDED_2026-10-25.md).

> **2026-10-25 — STEP06 FISICO COMPLETE / CARATTERIZZAZIONE LIMITATA PASS:** utente ha effettuato prova reale `HOT-251025-S6A`, con ventola in funzione, box chiuso e **4 intervalli HEATER COMMAND ON** nel [CSV allegato in chat]. Dati validi **411 campioni/141,542s**, aria SHT45 **18,51→19,52°C** (+0,57°C DURANTE il cooldown), NTC **135,50→min49,04kΩ**, Sanwa **18→40°C** locale piastra; dopo riferisce 27°C. **Sanwa piastra ≠ SHT45 aria**; NTC è resistenza bead senza conversione termica validata. Non prova mantenimento aria a40°C né corrente fisica indipendente; Step07 termo-regolazione resta OPEN. UI: `heater OFF` apparente dipende da HTTP sospeso durante ON, `Test 4/3` nonsense in cooling (sorgente attuale `pulse_goal=4`, eventuale HTML cache non confermata). [PR #9 MERGED](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/9) solo in ramo sperimentale ([CI PASS #40216224251](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40216224251)); nuova versione S6B distingue dati STALE da vero OFF, mostra RAFFREDDAMENTO senza contatori incoerenti, HTML no-cache; **S6B NON fisicamente caricato**, S6A provato non alterato. **Non ripetere test riscaldato per collaudare soltanto UI.** [Rapporto](docs/STEP06_REAL_HEAT_RESULTS_AND_WEB_2026-10-25.md).

> **2026-10-25 — STEP06 PROVA REALE AUTORIZZATA, FIRMWARE PRONTO, PROVA FISICA ANCORA OPEN:** approvato in chat test supervisionato **4×10s HEATER ON con ventola** e arresti R_NTC<=19,68kΩ / T_aria>=40°C / veto manuale Sanwa>=55°C. Build ESP32 esclusivamente sperimentale `HOT-251025-S6A` su ramo `experiment/step06-supervised-heat-2026-10-25` e [PR #8](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/8), **non mergiata**; [Actions #40213105781 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40213105781), artifact `step06-HOT-251025-S6A-OTA` / firmware.ino.bin SHA256 `13fe97ad2492d52a0fc333fb827e26352f4c4dd53430f488c46e3ada996ec604`. Nessun flash o riscaldamento fisico eseguito. Correzione FAN: 100% **1s kickstart**, poi 40%; **5s prerun dopo Running** (NON 5s al 100%). Primo test richiede preflight sonda/cavi/TCO105/ventola/24V staccabile, ID visualizzato, CSV e Sanwa sempre osservabile. `main` e firmware standard heater-OFF invariati. [Procedura](docs/STEP06_HEAT_EXPERIMENT_APPROVAL_2026-10-25.md).

> **2026-10-25 — STEP 05 MERGED / PASS SOFTWARE, STEP 06 NON AUTORIZZATO:** [PR #6](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/6) integrata **solo** nel branch operativo, merge commit [`ec6ce53`](https://github.com/alessandrocardelli/filament-dryer-monitor/commit/ec6ce53e4710849dadb10477d42e19185183083b). [CI finale PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40195971805): ESP32 core3.3.12/U8g2 2.36.19, 14 policy+12 pulse+41 cycle assertion e mock postheat STOP/FAULT/fan-loss, compilatore rifiuta attivazione a un solo flag. Sorgente `COLD-251025-S5` ancora heater OFF in entrambe le modalità. Candidato 4×1.5s con 12s gap/60s cooldown/abort R_NTC<=42.27kΩ o aria>=35°C **non approvato**; Sanwa 40°C è solo veto operatore proposto per futura prova. Buffer 640×250ms. `main` e PCB reali invariati; NON flash né prova a caldo. [Rapporto](docs/STEP05_HEATER_SAFETY_PREFLIGHT_2026-10-25.md). **NEXT: revisione protocollo e permesso esplicito prima di qualsiasi Step06 heater-enabled; la continuità a caldo resta OPEN.**

> **2026-10-25 — STEP 05 PASS SOFTWARE; STEP 06 TEST FISICO NON AUTORIZZATO:** PR [#6](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/6), ramo separato `feature/step05-heater-safety-preflight-2026-10-25`, firmware sorgente `COLD-251025-S5` con impulsi e termostato entrambi OFF, trial-reviewed flag false e `static_assert` fail-closed. [CI #40195612529 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40195612529): ESP32 compilato, vecchie suite + nuovo mock STOP/FAULT/cooldown e 4-pulse candidate 1,5s simulati PASS. Traccia RAM 160s, OLED/web con conteggio dinamico 4/4, nessun upload sulla PCB, nessun nuovo riscaldamento. 4×1,5s, NTC≥42,27kΩ, aria<35°C, Sanwa umano 40°C sono **proposte per la futura prova**, non limiti V1 validati, né autorizzazione. Restano da valutare comportamento fisico watchdog/fan e TCO105 in serie. **NEXT: review e approvazione separata prima di una eventuale sessione STEP06, oppure proseguire nel software senza HEAT.** [Rapporto](docs/STEP05_HEATER_SAFETY_PREFLIGHT_2026-10-25.md).

> **2026-10-25 — STEP 04 PASS ANALISI (non qualifica termica), STEP 05 NEXT:** [studio e protocollo NTC/piastra](docs/STEP04_NTC_PLATE_PROTECTION_STRATEGY_2026-10-25.md). Verificata topologia reale J7–R28–GPIO34, calibrazione ADC elettrica e guasti, sette misure storiche NTC/Sanwa (aperte/fan OFF), differenza fisica rispetto a SHT45 aria e limiti della conversione non identificata. Le soglie del vecchio test breve (**R_NTC 42,27 kΩ, aria 35°C**) sono soltanto baseline di arresto per commissioning, **non** temperature NTC validate; il firmware corrente `COLD-251025-S3` resta con entrambi i riscaldamenti OFF. Gap per futura V1: continuazione FAN/cooldown su FAULT o STOP da revisionare e testare; TCO 105°C fisico invariato, stato opzionale D029 preservato. **NEXT STEP 05:** safety review + definizione preventiva di un unico nuovo protocollo graduato/limitato, compilazione/test cold, separata approvazione utente prima di firmware heater-enabled o qualunque prova a caldo. Nessun codice/cablaggio modificato in STEP04.

> **2026-10-25 — STEP 03 MERGED, STEP 04 NEXT:** [PR #5](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/5) integrata **soltanto** nel branch operativo `feature/wifi-ota-web-logging-2026-10-24`; commit merge [`3b584a5`](https://github.com/alessandrocardelli/filament-dryer-monitor/commit/3b584a53660dd706bd5bc3d689c214465b28a9d4). [CI finale PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40192935185): ESP32 build, 14 policy scenarios + 12 pulse scenarios + 41 assertion scripted SHT45/air control, comandi heater HIGH=0 nel mock con config reale. Firmware sorgente `COLD-251025-S3` ha entrambi i percorsi HEAT disabilitati; dati e limiti host sono sintetici e non autorizzano calore. **Nessun flash sulla PCB, `main` non modificato. STEP 04 NEXT: analisi NTC elettrica separata da protezione termica piastra, con Sanwa locale e piano prove; nessuna prova calda senza gate successivi.** [Rapporto](docs/STEP03_THERMOSTAT_SIMULATION_2026-10-25.md).

> **2026-10-25 — STEP 03 PASS SOFTWARE, PR [#5](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/5):** variante `COLD-251025-S3` derivata dal branch fisicamente collaudato Wi-Fi R3 / codice R4, con `kHeaterCommissioningMode=false`, `kHeaterOutputEnabled=false`, flag approvazione termica sempre false. Build Arduino ESP32 e suite host PASS su [Actions #40192603027](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40192603027): 14 test policy, 12 test pulse, 41 assertion simulate di termostato SHT45, fault, timer, stop e riavvio, mock SafetyController config di default con **GPIO19 HIGH=0**. Gli ON del log `SIM_ONLY` sono esclusivamente richieste teoriche host con NTC/limiti inventati, **mai attuazione hardware o approvazione NTC**. `SafetyController.h`, `firmware.ino`, ADC, UI, fan, Wi-Fi e OTA invariati. PCB non aggiornata, nessun test a caldo, `main` invariato. **NEXT STEP 04:** strategia protezione NTC resistenza / limiti piastra e protocollo caratterizzazione. [Evidenze](docs/STEP03_THERMOSTAT_SIMULATION_2026-10-25.md).

> **2026-10-25 — STEP 02 CHIUSO (conferma utente), STEP 03 NEXT:** dopo tre foto dell'assemblaggio (posizioni della ventola, flex heater, NTC, TCO riferito sotto nastro, SHT45 in camera), l'utente comunica **«L'ho fissata»** riguardo alla termocoppia Sanwa e approva la chiusura dello step meccanico 02. Evidenza: **dichiarazione dell'utente**, non nuova fotografia o verifica indipendente della tenuta dopo aperture/chiusure. Non è stata riferita una nuova prova termica. Il cablaggio provvisorio di Sanwa e 24 V attraverso il foro jack DC resta da osservare per protezione da spigoli/trazione **prima di future prove a caldo**; non riaprire per questo tutti i test precedenti. **NEXT = step 03: termostato SHT45 simulato, uscita fisica heater DISABILITATA, mock/fault tests e CI, sul ramo derivato dal firmware operativo; nessuna autorizzazione a riscaldamento continuativo**. [Rapporto step 02](docs/STEP02_SENSOR_PROBE_MOUNTING_2026-10-25.md).

> **2026-10-25 — STEP 01 MERGED / STEP 02 NEXT:** [PR #4](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/4) integrata **solo** nel branch operativo `feature/wifi-ota-web-logging-2026-10-24`, merge commit [`612c44b9`](https://github.com/alessandrocardelli/filament-dryer-monitor/commit/612c44b9c9d686d74451bf09432b03749b33aeb3). CI del branch PR PASS due volte: [run 40175557554](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40175557554), ESP32 build, host test e controllo invarianti heater. `main` e dispositivo fisico non toccati. **Step 02 OPEN: sola ispezione a 24 V/USB disconnessi** di posizioni SHT45/NTC (bead interno identificabile via cablaggio), Sanwa, TCO 105°C già montato, ventola, J3/J5/J6/J7, instradamento cavi e chiusura di entrambi i coperchi. Richiedere solo 2–3 foto utili e nota su eventuali ostacoli; non ripetere Q4, PWM, heater pulses o OTA. Nessuna nuova prova a caldo autorizzata. Verifica visiva firmware R4 su OLED/web resta OPEN e separata.

> **2026-10-25 — STEP 01 PASS SOFTWARE ([GitHub Actions PASS #40175231036](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40175231036)):** il firmware `WIFI-241024-R4` della PR [#4](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/4) compila con ESP32 core 3.3.12 e U8g2 2.36.19; eseguiti con successo mock controller, 14 test policy e 12 test pulse. GitHub Actions conferma file `AppConfig.h`, `SafetyController.h`, `HeaterPulseCommissioning.h`, `FanController.h` identici al branch operativo. Unica correzione funzionale visibile: SHT45 temperatura aria distinta da NTC resistenza della piastra, senza pubblicare T NTC non qualificata; l'NTC interno non è stato calibrato. **Resta OPEN il controllo visivo su PCB del nuovo OLED/web**; nessun flash, prova termica o merge. **NEXT: step 02 preparazione fisica a dispositivo spento; step 03 sviluppo termostato OFF simulato possibile in parallelo.**

> **2026-10-24 — piano esecutivo finale:** [docs/EXECUTION_PLAN_2026-10-24.md](docs/EXECUTION_PLAN_2026-10-24.md) definisce i passi 01–11 per completare controllo SHT45 aria, sorveglianza piastra NTC, prove termiche progressive, V1 autonoma, CSV/web e release. Priorità corrente: step 01, senza modificare heater.

> **2026-10-24 — pulizia Git completata:** 12 → **6 branch**. PR [#3](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/3) merged in `main`: piani e policy termica *non attuante*, nessuna modifica al firmware Safe Bring-up di `main`. Sei branch storici già incorporati sono stati cancellati dopo verifica, rimosso il workflow temporaneo e puliti i riferimenti CI. Il branch firmware di prototipo **resta `feature/wifi-ota-web-logging-2026-10-24`** (Arduino OTA collaudato dall'utente; test 3×1,5 s, non controllo V1). Priorità successiva: **completare il funzionamento termico del dryer**, non storico 12h. [Verbale](docs/BRANCH_CLEANUP_2026-10-24.md).

> **2026-10-24 — Arduino IDE OTA preparato, firmware `WIFI-241024-R3`:** l'utente riferisce che l'upload web R2 è riuscito su PCB, dashboard aggiornata; Chrome `/ota` funziona in finestra incognito (problema di autenticazione locale nel profilo ordinario, causa esatta non determinata). Su branch `feature/wifi-ota-web-logging-2026-10-24` integrato `ArduinoOTA` del core ESP32 su UDP **3232**, password OTA **già salvata in NVS**, pubblicazione `_arduino._tcp` sul responder mDNS **esistente** e callback che richiama `safety.stop()`/`fan.stop()` prima del trasferimento. Il servizio riceve OTA solo in STANDBY, senza heater/cooldown/fault; HTTP OTA `/ota` rimane disponibile. **R3 non ancora caricato/testato fisicamente**: serve prima un'installazione R3 via Web OTA, poi test `Arduino IDE → Porta di rete → Carica`; monitoraggio e tre impulsi precedenti non da ripetere. [Guida](docs/WIFI_OTA_WEB_2026-10-24.md).

> **2026-10-24 — OTA su PCB PASS (test utente):** aggiornamento web `WIFI-241024-R1` → `WIFI-241024-R2` completato dalla pagina `/ota` usando Microsoft Edge; dopo il riavvio la dashboard mostra le tre nuove etichette SHT45/NTC. Il browser Chrome mostrava HTTP 401 su `/ota`, mentre Edge ha autenticato/caricato correttamente: causa Chrome non ancora accertata. **IDE compile sì; Arduino IDE upload via rete non ancora implementato:** richiede integrazione aggiuntiva ArduinoOTA, distinta dal Web OTA già presente, con sicurezze heater OFF/idle. Non rifare test riscaldatore. [Guida](docs/WIFI_OTA_WEB_2026-10-24.md).

> **2026-10-24 — Wi-Fi fisico verificato; prima OTA da provare:** l'utente conferma connessione hotspot 2,4 GHz `gina`, dashboard raggiungibile sia tramite IP sia `filament-dryer.local`, letture sensori e API status/events funzionanti su PCB. Modificate le etichette dashboard (Temperatura camera SHT45, Umidità camera SHT45, Resistenza piastra NTC), nuova versione `WIFI-241024-R2` per riconoscere il primo aggiornamento OTA. La CI prepara il binario applicativo `.bin` come artifact, con limite slot OTA 4 MB default. **OTA fisico e persistenza CSV non ancora validati**. Non ripetere il collaudo precedente degli impulsi; non introdotti cambiamenti ai limiti del riscaldatore. [Procedura](docs/WIFI_OTA_WEB_2026-10-24.md).

> **2026-10-24 — WiFi / OTA / dashboard / CSV ramo feature dedicato:** `feature/wifi-ota-web-logging-2026-10-24` aggiunge al firmware termico sperimentale già provato: configurazione WPA2 AP con password casuale leggibile su OLED e credenziali STA salvate soltanto in NVS; dashboard e API locali con NTC ohm/SHT45/fasi/pulse events; storico RAM 250 ms con esportazione e CSV salvato **dopo** cooldown su LittleFS; upload browser OTA protetto da password admin e consentito solo in STANDBY heater OFF. Versione `WIFI-241024-R1`; prima installazione sempre via USB, prove rete/OTA fisiche ancora OPEN. Tre impulsi da 1.5 s e limiti invariati; TCO aggiuntivo opzionale D029. **Precedente test fisico PASS riportato:** `PULSE-241024-R1`, Sanwa 19→23 °C, 3 HEATER ON, COMPLETE, nessuna anomalia. [Manuale](docs/WIFI_OTA_WEB_2026-10-24.md).

> **2026-10-24 — riconoscimento firmware su OLED senza USB:** nel ramo di prova tre impulsi ora il firmware identifica la versione **PULSE-241024-R1** per 3 s dopo l'animazione Slewform e **P24-R1** nella pagina STANDBY. Aggiunte fase PRERUN/PULSE/GAP/COOLDOWN in DRYING e causa del fault leggibile. L'utente ha riferito nessun riscaldamento rilevato dopo flash del precedente ramo: questo non prova né un upload errato né un output HIGH, poiché solo 3 impulsi di 1.5 s sono comandati e i blocchi sensore/fan non erano leggibili senza USB. Modifiche al ramo sperimentale, prove su hardware ancora da ripetere con il nuovo ID; nessun nuovo setpoint o TCO modificato. [Procedura](docs/FIRST_CLOSED_HEATER_PULSE_TEST_2026-10-23.md).

> **2026-10-23 — FIRST CLOSED HEATER PULSE TEST READY IN SOURCE:** new independent branch `test/closed-case-heater-pulses-2026-10-23` with **three 1.5 s live heater pulses**, fan prerun, NTC-resistance and SHT45 cutoffs, ESP32 2.2 s hardware timer, 45 s fan cooldown and 250 ms HOT_CSV. **ESP32 firmware compilation plus all 14+12 host scenarios PASS** [GitHub Actions](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40022340102). No hardware flash/test or case qualification yet. Only this one new experiment is planned; don't rerun passed UI/fan/Q4 tests. TCO optional D029, 105 C installed not bypassed. [Exact procedure](docs/FIRST_CLOSED_HEATER_PULSE_TEST_2026-10-23.md).

> **2026-10-23 — build firmware ESP32 riuscita:** [GitHub Actions PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40009733825) per sketch Arduino ESP32 core 3.x / U8g2 e test C++ del controller. Il codice ha il controllo riscaldatore integrato, ma la build attuale mantiene GPIO19 OFF per impostazione di sicurezza: calibrazione NTC e limiti ancora da validare. Nessun test sulla PCB eseguito. [Dettagli](docs/HEATER_FIRMWARE_INTEGRATION_2026-10-23.md).

> **2026-10-23 — FIRMWARE RISCALDATORE INTEGRATO (branch `feature/integrated-heater-controller-2026-10-23`):** il codice principale ora contiene una reale catena sensori → controllo aria con NTC sorveglianza → consenso ventola → GPIO19/Q4, fault latch e stop, con stato OLED/Serial. **Output heater ancora DISABILITATO in AppConfig**: termistore e limiti non ancora qualificati. Policy 14 test host PASS; ulteriore mock driver su g++ testato OFF (zero HIGH) e con fixture sintetica (HIGH dopo prerun, LOW dopo stop/fault). **Compilazione Arduino/ESP32 e prove PCB NON eseguite.** Il TCO resta opzionale D029. [Rapporto tecnico](docs/HEATER_FIRMWARE_INTEGRATION_2026-10-23.md).

> **2026-10-23 — controllo riscaldamento, lavoro concreto senza attuazione:** sul ramo feature/heater-policy-cold-tests-2026-10-23 è stato introdotto un modello C++ isolato di richiesta calore, **14 test nativi g++ PASS**, con fault per sensori, temperatura, ventola comandata e timeout. Nessuna GPIO attivata, SafetyController di produzione invariato: HEATER OFF. Calibrazione assoluta NTC non risolta e prove calde non autorizzate. Priorità NTC/sensori/cablaggi; TCO opzionale (D029). [Risultati e prossime misure](docs/HEATER_CONTROL_COLD_STAGE_2026-10-23.md).

> **2026-10-23 — rettifica D029:** il TCO è un'**aggiunta opzionale**, non un requisito necessario al funzionamento del dryer; il precedente vincolo D028 che ne rendeva l'installazione obbligatoria per qualsiasi prova a caldo era eccessivo e viene superato. Priorità: sensori/NTC, controllo heater sicuro, verifiche termiche progressive; CSV/web in parallelo. Il prototipo ha tuttora un TCO 105 °C fisicamente in serie: documentarne gli effetti, nessun bypass automatico. Il codice integrato mantiene heater OFF; nessuna prova riscaldamento autorizzata da questa rettifica. [D029](docs/DECISIONS.md), [piano](docs/COMPLETION_PLAN_2026-10-22.md).

> **2026-10-23 — sviluppo sbloccato (D028):** gate TCO B necessario prima delle prove a caldo, **non** per attività senza calore. Proseguire da subito su firmware/CSV 12 h, web locale, fault simulati, UI e verifiche meccaniche a freddo, mantenendo heater hard-OFF. Il modello G4A00152C resta candidato tecnico selezionato (D027); montaggio/qualifica non completati. [Piano aggiornato](docs/COMPLETION_PLAN_2026-10-22.md).

> **2026-10-23 — TCO, acquisto ora identificato:** prima scelta **MICROTEMP G4A00152C da WTT Elettronica**, articolo FT152G, [eBay291602208480](https://www.ebay.it/itm/291602208480). **1 pezzo7,75 EUR nel carrello verificato =1,85+5,90 trasporto**, oggetto **Italia**, stima27–29 ottobre al CAP48018. Il sito dello stesso WTT dichiara esplicitamente **G4A00152C** e lo stesso EAN8029564057789: risolto il precedente dubbio sul codiceTC152. Trasporto5,90 è un compromesso esplicito; nessun acquisto. [Fonti, prove e limiti](docs/TCO_PURCHASE_DECISION_2026-10-23.md). D027 invariata (Tf152/Th137/Tm205;5 A/24 V DC); qualifica termica ancora OPEN, sviluppo heater-OFF prosegue secondo D028.


> **2026-10-22 completion planning (feature branch only):** the single gated plan [docs/COMPLETION_PLAN_2026-10-22.md](docs/COMPLETION_PLAN_2026-10-22.md) covers A–H from closed-case SHT45/Sanwa/cable preparation and independent TCO qualification through controlled heater tests, production-style V1 firmware, CSV/web and final documentation. **Next: gate A1, with power disconnected, verify actual sensor/probe/TCO locations and whether both lids close without pinching wires.** No heater test, TCO change or firmware heater enable was authorized; previously passed fan/UI/Q4 benchmarks remain passed. This plan is currently on `plan/closed-heater-qualification-2026-10-20`, not `main`.


Custom ESP32-based controller board that retrofits an **eSUN eBOX Lite** filament dryer with closed-loop humidity/temperature control, a local OLED interface, data logging and a web UI.

The PCB physically replaces the original front panel: display and buttons sit on the front face of the board, while the main electronics are on the back.

> **2026-10-20 fan regression RESOLVED (user-reported physical repair):** The assembled-board fan failed because of an unreliable solder joint at U4 **physical pin 27 = GPIO16**. Touching that lead with a DMM probe temporarily restored the fan; after addressing its solder joint the user confirms normal kick-start at **100%**, followed by **40%** running duty. Earlier SETUP (5 checks) and SETTINGS (5 checks including power-cycle persistence) were reported passed. **2026-10-20 further user-reported pass:** a supervised approximately 2–3 min DRYING fan run and long ON/OFF stop back to STANDBY/fan OFF completed without reported anomalies; user also reported no apparent heater activation. This is **not electrical proof of heater power isolation**. **2026-10-20 user-reported pass:** full power removal (24 V and USB) and reapplication produced normal Slewform startup, STANDBY, fan OFF, and no cycle auto-resume. 2026-10-20 sensor OLED photo check: STANDBY and DRYING show SHT45 19.4 °C and 69.7% RH; SYSTEM shows Chamber 19.4 °C, Heater NTC 15.5 °C, H 70% rounded and F 40%; software HEATER LOCKED. Sensor data/display confirmed, absolute NTC calibration remains open (cold discrepancy 3.9 °C). Actual electrical heater-OFF, sealed-unit clearances and hot-use safety remain unverified. The separate LEDC channel-change branch is **not demonstrated to be the cause or cure**; do not merge it for this repair. See [docs/ASSEMBLED_UI_SOFTWARE_HEATER_OFF_TEST.md](docs/ASSEMBLED_UI_SOFTWARE_HEATER_OFF_TEST.md) and [docs/PROJECT_STATE.md](docs/PROJECT_STATE.md).

> **2026-10-19 recovered bench firmware:** [firmware/test_archive/TEST_FIRMWARE_ARCHIVE.md](firmware/test_archive/TEST_FIRMWARE_ARCHIVE.md) now tracks the 2026-10-08 full integrated firmware as **exact historical Git blobs** and eight **explicitly reconstructed, not original** fan/I²C/SHT45/OLED/NTC bench sketches based on passing test conversations. Standalone hot-heater sketch originals are NOT recovered. This addition was merged without changing active `firmware/firmware.ino`; an experimental LEDC fix remains separate and **not verified** on hardware.


> **2026-10-19 later firmware/assembly update:** User reports complete assembly of PCB/front panel/eBOX Lite and **heater power J5 left wired in** by choice. The normal Slewform logo/menu application at `firmware/firmware.ino` already commands heater GPIO19 LOW at boot and each loop (`SafetyController.h`: no heater-enable path), while running a fan-only DRYING TEST at 100%/1s then 40% PWM. **This software lockout is not independent electrical heater isolation**, and the previous J5-DISCONNECTED closed-box acceptance was not performed in that specified configuration. Next: replace any standalone 90 C heater diagnostic flash image with the integrated application, then run the **supervised assembled UI/SHT45/fan and no-heater-activity check** in [docs/ASSEMBLED_UI_SOFTWARE_HEATER_OFF_TEST.md](docs/ASSEMBLED_UI_SOFTWARE_HEATER_OFF_TEST.md). The heater protection/TCO/thermal qualification gate is unchanged; no closed heat test authorized and no UI/fan result yet reported.

> **First-article integration checkpoint — 2026-10-17:** the prototype PCB has now been electrically brought up with OLED/buttons, SHT45, buzzer, fan and independent standalone heater diagnostics. Seven heater NTC/Sanwa bench points through **85 °C at the nearby Sanwa** were collected with **both enclosure covers open and the fan OFF**, so they do not establish closed dryer performance or the embedded bead's exact temperature. The integrated application still **hard-disables heater GPIO19** pending safe heater regulation and review of the installed **105 °C one-shot series TCO** against eBOX Lite's nominal 105–118 °C plate temperatures. **2026-10-19 update:** user reports the front panel has been printed and the fit is OK. Next: first **closed-box PCB/UI/SHT45/fan test with heater power J5 disconnected** using the existing SAFE BRING-UP app; see `docs/CLOSED_ENCLOSURE_NO_HEAT_TEST.md`. The previously documented optional NTC diagnostic study and the full gated roadmap are committed. Enclosed heater-power testing and upper eBOX Lite plate-temperature claims remain gated by wiring/sensor placement and independent TCO suitability. This supersedes the earlier front-panel print task. ** `docs/NTC_CALIBRATION_ANALYSIS.md` records the provisional analytical models; `docs/PROJECT_ROADMAP.md` gives the integrated assembly/control/qualification plan. The older checkpoint below is historical.

> **Current status — 2026-10-08:** active hardware branch **main**. The released fabrication-source checkpoint remains **`74a3000127371ac6c3b3b7197f9036dec5b58eef`** (2026-09-17). First-article bring-up has validated MCU/USB/UART after the documented D020 collector-cross bodge, the four front-panel buttons, internal diagnostic LED, passive buzzer, SHT45, SSD1309 graphical output, fan driver/real fan and unloaded heater-switching path. The real heater remains blocked pending the real NTC/J7 path, final NTC conversion/fault handling and independent series TCO. The V1 front-panel interaction model is specified in `docs/UI_SPEC.md` and D022. A **SAFE BRING-UP** application now exists under `firmware/`: Slewform animated root-growth startup (1.2 s stationary intro, corrected 11-stage radius table) followed by a separate wordmark/chime (reproducible SVG-to-XBM export), OLED pages, SHT45 acquisition, button handling, persisted settings, modular cycle/fan/safety controllers and D021 LED patterns are implemented. The heater remains hard-locked LOW; the already-validated fan is now enabled only during DRYING test cycles with a provisional 100%/1 s kick-start followed by 40% at 25 kHz. See `docs/PROJECT_STATE.md` and `hardware/docs/PROCUREMENT.md`.

For a new work session, read AGENTS.md, docs/PROJECT_STATE.md, docs/DECISIONS.md and docs/TODO.md before changing the design.

---

## Features

- **SHT45** humidity/temperature sensor (I²C)
- **Characterized NTC thermistor** integrated into the heating element
- **1.54 in SSD1309 OLED** (128×64, I²C) + four front-panel buttons
- PWM heater control with hardware and firmware safety layers
- PWM fan control
- CSV logging to internal flash planned
- Web UI planned
- Powered from the dryer's 24 V supply; USB is used for programming/debug

## Safety concept

The custom controller's heater-control and safety behavior must be qualified for the modified unit. The mandatory engineering work is to verify actual temperature feedback (heater NTC and chamber SHT45), fault/off behavior, fan/cabling/material safety and bounded heater actuation.

- **Firmware:** the current application still hard-disables heater drive. A future controller must shut down on missing/invalid/stale sensor signals and other identified faults; these protections are **not implemented/validated in the current heater-locked application**.
- **Optional hardware TCO enhancement:** the user reports no separate TCO on the original unit. A 105 °C one-shot TCO is nevertheless currently installed in series with HEATER+ on the modified unit, and may limit the OEM temperature range. Its future retention/replacement is optional as a design requirement, but its current physical presence must be considered; removal/bypass or heater-enable requires separate engineering review/authorization, not merely this documentation change.
- **Cold-test gate:** keep drive disabled until the new heater-control and safeguards are developed/reviewed; supervised hot tests need a specific test plan and authorization.

See D029: TCO procurement is **not** a mandatory prerequisite to designing or testing the heater controller; modified-system safety still needs demonstrated validation.

This is a personal project and not a certified product.

---

## Hardware

The board is a 4-layer KiCad 10 design intended for JLCPCB **bare-board fabrication and manual component assembly**. TME is the preferred prototype component supplier. The current KiCad schematic and PCB on the active branch are the authoritative implementation.

### Main blocks

| Block | Part | Notes |
|---|---|---|
| MCU | ESP32-WROOM-32E | Wi-Fi; final antenna copper/edge review remains a release item |
| Buck converter | **L7987L** | 24V_PROT -> 3.3 V, about 516 kHz, 1 A design target |
| Buck fault recovery | TLV1701AIDBVR + MMBT3904 | External AutoEN based on COMP fault detection |
| USB-UART | CP2102-GM | USB Full-Speed programming/debug |
| USB ESD | AKS1201 / USBLC6-2SC6-compatible topology | Two protected data channels |
| Humidity/temp | SHT45 | I²C 0x44 |
| Display | SSD1309 OLED 128×64 | I²C 0x3C |
| Heater driver | IRLR3636TRPBF | 60 V DPAK N-MOSFET, about 1.6 A heater load |
| Fan driver | IRLML2060TRPBF | 24 V fan low-side switch |
| Heater temperature | Integrated ~82 kΩ NTC | GPIO34 / ADC1, R28 = 47 kΩ, C19 = 100 nF |
| Buzzer | Passive buzzer + NPN | Defined startup state with base-emitter pull-down |

Heater and fan stay on 24V_PROT; they are not powered by the 3.3 V buck.

### L7987L electrical state

The active implementation is in hardware/Power.kicad_sch.

Key implemented parts/values:

| Ref | Value / part | Role |
|---|---|---|
| U5 | L7987L | Buck regulator |
| U1 | TLV1701AIDBVR | AutoEN comparator |
| Q7 | MMBT3904 | AutoEN EN pull-down |
| L1 | SRN6045-150M, 15 µH | Inductor |
| D7 | PMEG6030EP,115 | Catch diode, 60 V / 3 A Schottky |
| C1 | 10 µF / 100 V X7S | Main local input ceramic |
| C3 | 1 µF / 100 V X7S | Current local VIN/VCC bypass present in source |
| C6 | 100 nF | BOOT-LX bootstrap |
| C10 | 47 µF / 10 V X7R | Main pre-bead output capacitor |
| C21 | 1 µF / 25 V | VBIAS bypass |
| R4 | 47 kΩ | FSW, ~516 kHz |
| R29 | 47.5 kΩ | ILIM, ~1.705 A nominal |
| R36 / R34 | 49.9 kΩ / 16 kΩ | Feedback, ~3.295 V nominal |

Compensation: R33 = 16 kΩ, C9 = 18 nF, C8 = 39 pF, R35 = 1.13 kΩ, C20 = 560 pF.

Output architecture remains 3V3_BUCK -> FB1 -> 3V3_MCU. PGOOD and SYNCH are intentionally NC. The upstream C5 = 100 µF / 50 V bulk capacitor remains.

**VIN/VCC ceramic bypass — resolved:** ST requires a ceramic of 1 µF or higher across VIN-to-power-GND and another across VCC-to-IC-GND, close to the device. In the current design C1 = 10 µF / 100 V X7S already satisfies the VIN-side requirement, while C3 = 1 µF / 100 V X7S is the VCC bypass. The extra buck C22 added on 2026-09-09 was therefore redundant and was deliberately removed in commit `24a9170a`. Current C22 is unrelated: it is the CP2102 REGIN 1 µF / 25 V capacitor.

Detailed calculations and AutoEN rationale are in [docs/BUCK_L7987L_DESIGN.md](docs/BUCK_L7987L_DESIGN.md).

### Engineering review

The L7987L + AutoEN engineering review completed on **2026-09-04**. The accepted design basis remains:

- R29 = 47.5 kΩ -> ~1.705 A nominal current limit;
- conservative component-selection envelope ~1.42–2.15 A;
- actual sourced C1/C10 reviewed with class-II MLCC bias sensitivity;
- AutoEN reviewed COMP trip window ~1.56–2.07 V;
- recorded simulation ~59.1 kHz crossover, 64.9° phase margin, 19.6 dB gain margin.

These are engineering/simulation results; physical validation remains required.

The current `hardware/ERC.rpt` (2026-09-17) reports **0 errors and 0 warnings**. The existing GitHub ERC workflow contains the reviewed waiver logic, but its automatic push trigger currently names redesign/buck-sourcing, not the active pcb/l7987l-layout branch.

### Current PCB integration

The current board at hardware checkpoint `74a3000` contains the L7987L stage and AutoEN block on **B.Cu**. The old AP66200 stage and /Power/VCC_AP66200 are absent.

The saved 2026-09-17 DRC reports **0 active errors, 0 active warnings, 0 unconnected pads** and one **excluded** U4 B.Silkscreen-to-board-edge warning.

Local external copper/zones are present for 24V_PROT, /Power/3V3_BUCK, 3V3_MCU, HEATER_SW, LX and GND. Both internal layers remain solid GND.

The fabrication package has progressed to JLCPCB CAM review; this does **not** constitute physical validation of the assembled hardware.

### U5 exposed pad — resolved for planned hand assembly

U5 pad 17 is a 3.2 × 3.2 mm GND exposed pad on B.Cu. The accepted design intentionally uses **no via-in-pad**. Instead, six GND vias, each **0.60 mm diameter / 0.30 mm drill**, sit immediately outside the EP in two rows of three: 3 above and 3 below U5. This arrangement was implemented in commit `c9dc4129` and is present in the current PCB.

The peripheral vias provide a short thermal/GND path into the internal GND planes without placing open vias directly under the solderable exposed pad. For the planned prototype assembly, U5 is fitted by lightly pre-tinning the EP, applying flux and heating with hot air. Because this flow does not use a stencil for U5, the full B.Paste definition on pad 17 is not a prototype release blocker.

If the process later changes to stencil/reflow or external PCBA, paste-windowing and via treatment must be reopened for that process. First-board U5 temperature measurement remains part of validation. See decision D019 and `docs/ASSEMBLY.md`.

### CP2102-GM / USB

The classic CP2102-GM custom QFN footprint currently uses:

- 0.50 mm pitch;
- perimeter pads **0.95 × 0.28 mm**;
- exposed pad **3.25 × 3.25 mm**;
- solder-mask expansion **+0.06 mm**;
- **3 × 3** exposed-pad paste apertures, each 0.9 × 0.9 mm.

REGIN is on 3V3_MCU and now has a local **C22 = 1 µF / 25 V X7R 0603, Murata GCM188R71E105KA64J**, in addition to the local 100 nF decoupling.

USB differential routing:

- target: **90 Ω differential**;
- B.Cu referenced to In2.Cu;
- width: **0.20 mm**;
- gap: **0.25 mm**;
- KiCad tuning profile: **USB_90R**;
- long U2 -> U3 pair stays entirely on B.Cu with no vias;
- J2 A6/B6 = D+, A7/B7 = D−;
- U3 channel 1↔6 carries D+, channel 3↔4 carries D−;
- the short Type-C duplicated D+ fanout uses two signal vias and a short F.Cu bridge, with a nearby GND stitching via; D− stays on B.Cu.

The stored netlist was regenerated on 2026-09-17 after the MCU correction; USB pinout still requires first-board functional verification.

### Ground planes and stackup

| Layer | Role |
|---|---|
| F.Cu | Front components and signals |
| In1.Cu | **Solid GND plane** |
| In2.Cu | **Solid GND plane** |
| B.Cu | Back components/signals + local power copper |

Stackup entered in KiCad: 35 µm copper; 0.10 mm FR4 between F.Cu-In1 and In2-B.Cu; 1.24 mm FR4 core between inner layers; total board thickness 1.6 mm.

**Both inner layers are deliberately full GND.** There are no internal 3V3 or 24 V power planes.

For the L7987L, PGND and SGND are not separate project nets or split inner planes. They are current-return regions on the same GND net; local B.Cu geometry and via placement keep pulsed returns out of quiet control returns.

### Power distribution

Power remains on the external layers.

- 24V_PROT: wide traces/local zones are appropriate where current requires them.
- 3V3_BUCK: compact local copper around the buck output/Cout/FB1 path is preferred.
- 3V3_MCU: may remain mainly track-distributed, with local copper where useful.
- LX and other fast switched nodes: keep copper only as large as needed.

The capacitance from external power copper to the nearby GND plane is not a reason to avoid local power pours; the important restriction is minimizing fast switch-node area.

### Netclasses

| Class | Clearance | Track | DP width / gap | Via dia/drill |
|---|---:|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.20 / 0.25 mm default | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | — | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | — | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | — | 0.80/0.40 mm |
| USB | 0.20 mm | **0.20 mm** | **0.20 / 0.25 mm** | 0.60/0.30 mm |

USB data nets are USB_D+, USB_D−, USB_CONN_D+, USB_CONN_D−.

### Procurement

The working purchasing BOM is the Google Sheet **Filament Dryer Monitor — BOM finale Mouser**, tab **BOM TME**.

The live **BOM TME** was checked against the current production BOM/netlist: **94 mounted component references**, of which **61 items / 43 TME purchase rows** and **33 references marked in-house** for one assembled board. TME order was placed 2026-09-20. **C11 (22 µF, 25 V, X5R, 0805)** is outstanding: TME confirmed a stock-location problem and is checking its warehouse. No replacement/cancellation has been approved. The display, external sensor, heater thermal cutoff and mating cabling are outside this PCB purchasing BOM; their actual inventory remains to be checked.

See [hardware/docs/PROCUREMENT.md](hardware/docs/PROCUREMENT.md).

### Heater NTC characterization

The original eSUN heater includes an NTC whose exact manufacturer/part number is unknown. It was characterized in situ during cooldown after disconnecting it from the original controller.

| Temperature | Resistance |
|---:|---:|
| 25 °C | 82.5 kΩ |
| 28 °C | 75.5 kΩ |
| 30 °C | 67.4 kΩ |
| 35 °C | 51.7 kΩ |
| 40 °C | 43.6 kΩ |
| 44 °C | 35.93 kΩ |
| 50 °C | 28.8 kΩ |

The measurements are consistent with about 82 kΩ at 25 °C and a single-beta approximation near β ≈ 4100 K over the measured range. These are empirical measurements, not manufacturer specifications.

PCB interface: R28 = 47 kΩ 1%, C19 = 100 nF, GPIO34 / ADC1.

### GPIO map

| Function | GPIO |
|---|---|
| SDA / SCL | IO21 / IO22 |
| UART TX0 / RX0 | IO1 / IO3 |
| BOOT / RESET | IO0 / EN |
| Status LED | IO26 |
| Fan PWM | IO16 |
| Buzzer | IO33 |
| Heater PWM | IO19 |
| NTC | IO34 / ADC1 |
| Button ON/OFF | IO35 |
| Button M | IO32 |
| Button UP | IO14 |
| Button DOWN | IO27 |

Do not run a project-wide annotation reset when changing a hardware block; preserve established references or annotate only selected new circuitry.

---

## Firmware

Firmware uses the ESP32 Arduino core 3.x and is intended to remain cooperative/non-blocking in normal operation.

| Block | Status |
|---|---|
| SHT45 driver | Application driver implemented with non-blocking conversion wait and CRC validation |
| Fan PWM | Real DRYING-test control implemented: 25 kHz, 100%/1 s kick-start then provisional 40%; final FAN AUTO/cooldown still open |
| Heater + NTC safety | NTC characterized; conversion/calibration/safety implementation pending |
| OLED | First-article graphical output validated; SAFE BRING-UP renderer implemented |
| Buttons + UI state machine | Live-tested; SETUP and SETTINGS flows implemented, including persisted cycle/buzzer preferences |
| LittleFS + CSV logging | Pending |
| Web UI | Pending |
| Buzzer / LED | OFF/LOW/MED/HIGH volume, optional key click, startup/start/complete/warning/fault patterns and D021 LED semantics implemented; final acoustic tuning still pending |
| Non-blocking SHT45 conversion | Implemented in SAFE BRING-UP scaffold |

---

## Repository handoff files

~~~text
AGENTS.md                       Mandatory workflow before project work
docs/PROJECT_STATE.md           Current authoritative checkpoint
docs/DECISIONS.md               Durable engineering decisions
docs/TODO.md                    Immediate work list
docs/BUCK_L7987L_DESIGN.md      Detailed L7987L design/layout record
docs/ASSEMBLY.md                Assembly/bring-up sequencing
docs/UI_SPEC.md                 Front-panel V1 interaction specification
firmware/README.md               SAFE BRING-UP firmware status and usage
hardware/docs/PROCUREMENT.md    Sourcing/footprint/manufacturing state
hardware/*.kicad_*              Actual KiCad implementation
~~~

## Current next steps

1. Await TME's definitive response about **C11** and confirmation that the remaining available parts will ship; do not change the KiCad MPN or procurement BOM until an actual replacement/cancellation is agreed.
2. Verify the physical delivery, identities, quantities and packages of the TME order against the live `BOM TME` / `TME_IMPORT`; verify the stock of all items marked `In casa`.
3. Check availability and mechanical/electrical fit of off-board items (SSD1309 OLED, SHT45 sensor, independent heater thermal cutoff, mating connectors/cables and mounting hardware).
4. On receipt of the bare PCBs, inspect fabrication/assembly details and follow `docs/ASSEMBLY.md`; **leave R5 unfitted for initial 3.3 V bring-up**.
5. Perform first-board buck, USB, thermal and fault-recovery testing before normal heater operation. See `docs/PROJECT_STATE.md` and `docs/TODO.md`.

**First-article bring-up update (2026-10-04):** Gate 3 MCU/USB/UART is validated. CP2102 enumeration, bidirectional UART, ESP32 flashing, runtime serial output and automatic programming all pass on the prototype after the documented Q2/Q3 collector-cross bodge (D020). Next staged bring-up is Gate 4A I2C support.
