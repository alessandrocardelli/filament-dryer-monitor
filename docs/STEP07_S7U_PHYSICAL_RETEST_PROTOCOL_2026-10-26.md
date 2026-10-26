# STEP07-S7U — Protocollo eseguibile, tracciabilità e handoff della NUOVA prova fisica

> **POST-SESSIONE 26 OTTOBRE 2026 — ESEGUITA:** questo documento conserva il protocollo *pre-registrato* e quindi usa espressioni future nelle sezioni storiche, ma la prova descritta **È STATA ESEGUITA**, con `NTC R LIMIT` e dati originali archiviati su **main**: [registro misure S7U](evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md), [CSV raw 717 campioni](evidence/thermal/2026-10-26-S7U-retest-raw.csv), [analisi](evidence/thermal/2026-10-26-S7U-retest-analysis.md). Sanwa17/17/36/43°C, cooling in discesa fino25°C a fermata ventola; NTC39789,3Ω a FAULT, aria18,20→19,52°C a OFF→20,89°C a fine acquisizione, 2×20s +19,25s di HEATER comando ON. **NON RIPETERE la sessione**. NEXT G2/G3, review della guardia termica e protezioni senza nuova accensione. Il testo sotto è conservato immutato come piano iniziale, non come istruzione di prova attuale.

**Data preparazione:** 2026-10-26. **Stato:** protocollo scritto; nuova prova **NON ANCORA ESEGUITA**; nessun nuovo risultato fisico disponibile.
**Decisione utente:** ripetere **una nuova sessione** dalla configurazione termica immediatamente precedente al test S7G, usando le correzioni di interfaccia S7U; **NON** recuperare il CSV S7G, **NON** ripetere prove heater-OFF, menù, PWM, OTA o altre verifiche già superate. Prima della nuova sessione vanno verificati soltanto i requisiti immediatamente pertinenti alla sicurezza fisica.
**Riferimenti sorgenti**: `firmware/AppConfig.h`, `firmware/HeaterStep07GuardTrial.h`, `firmware/HeaterPulseCommissioning.h`, `firmware/SafetyController.h`, `firmware/firmware.ino`, `firmware/Ui.h`, `firmware/ThermalLog.h`, `firmware/NetworkService.h`. Branch `fix/step07-clear-test-feedback-2026-10-26`, HEAD originario S7U `8f804f5ef08c9ec91eeadd576fc35fc976250638`, PR #12. I futuri commit di documentazione non alterano necessariamente il binario: **la versione firmware dell'esperimento è `HOT-261026-S7U` / `H26-S7U`**; verificare sempre sul dispositivo.

## 0. Quale problema risolviamo — non fare altre prove scollegate dall'obiettivo

Obiettivo immediato: **verificare in un'unica sessione limitata come il cutoff NTC RAW 40 kOhm interrompa gli impulsi e osservare il Sanwa sul punto riscaldante al taglio e durante il raffreddamento**; correlare al CSV e alla temperatura dell'aria SHT45. NON è una prova di raggiungimento/mantenimento dei 40 °C in camera, NON è validazione della temperatura del bead, NON è una qualifica di sicurezza del dryer V1. Se interviene FAULT, **quello è un risultato utile, non un motivo per riprovare**.

Motivo del rifacimento dichiarato dall'utente: precedente sessione S7G fisicamente eseguita ma con letture manuali soltanto 17 °C (inizio), 36 °C (prima impulso 3), 43 °C (al FAULT); UI non indicava chiaramente heater OFF/cooling, buzzer ripetuto; mancavano l'osservazione completa dell'arresto/raffreddamento Sanwa e un dossier integrato. L'utente ha deciso di **NON recuperare** il vecchio CSV. Questo è **un reset della sessione di acquisizione**, NON del percorso di collaudo del progetto.

## 1. Setup FISICO storicamente usato — conservare il metodo

- Dispositivo: prototipo Filament Dryer Monitor dentro l'involucro chiuso eSUN eBOX Lite, **24 V**, con resistenza flessibile arancione sulla piastra metallica e ventola assiale a fianco. **Niente cavo USB durante box chiuso**: l'utente aveva già chiarito che non può restare collegato. Arduino IDE OTA già funzionante tramite rete; niente ripetizioni di collaudo upload e periferiche.
- Riferimento **Sanwa RD701** e termocoppia tipo K: **punta a contatto con il punto metallico vicino all'NTC integrato / zona piastra riscaldante**, non nell'aria. La sonda inizialmente si distaccava; utente ha dichiarato il fissaggio risolto il 25 ottobre. Tenere invariati punto e geometria di contatto, se possibile, per comparabilità. Posizione precisa al millimetro e qualità del contatto **non strumentalmente documentate**: registrare foto/testo del setup della nuova sessione; non inventare posizione.
- **NTC originale incorporato nella pellicola riscaldante**: J7 → GPIO34, misura ESP32 **R_NTC in ohm**; non equiparare alla temperatura Sanwa e non presentare valori termici derivati come calibrati. Nella prova S6C il Sanwa aveva risposta ritardata rispetto alla NTC.
- **SHT45**: nella parte alta della camera del filamento, rivolto all'aria. Misura temperatura e RH **dell'aria**, non piastra. Il limite 40 °C riguarda SOLO aria SHT45.
- **Ventola**: comando PWM 100% durante la prova (nessun tachimetro, giri/portata non misurati). Confermare direttamente la rotazione/flusso; PWM=100% non ne è prova.
- **TCO monouso 105 °C** già installato in serie al riscaldatore, sotto nastro metallico a sinistra secondo utente; NON bypassare. Questo componente non equivale a qualifica termica.
- Cavi Sanwa e 24 V in uscita dal foro normalmente destinato al jack DC: osservare che siano protetti da spigoli, trazione, schiacciamenti, calore/pale. Dato riferito, non certificato fotograficamente dopo chiusura.

## 2. Identità firmware / profilo implementato (sorgenti, NON dedotto dalla chat)

| Parametro | Valore S7U in sorgente | Significato |
| --- | --- | --- |
| Branch | `fix/step07-clear-test-feedback-2026-10-26` | Basato su S7G, PR #12 DRAFT |
| Build | `HOT-261026-S7U` (`H26-S7U`) | Verifica su OLED / `/api/status` |
| Modalità | `kHeaterCommissioningMode=true` | **UNA sola sessione per boot**; termostato normale disabilitato |
| Fan | kickstart 100% 1 s; poi comando 100% | Nessun tach |
| Prerun | 5 s dopo `FanPhase::Running` | Ventola avviata prima di heater |
| Heater massimo | 6 finestre **max 20 s ON**, gap 12 s OFF | Può fermarsi prima per protezione |
| Timer ISR | limite 20,5 s per finestra | Non sostituisce vigilanza umana o verifica corrente |
| Cutoff NTC | `R_NTC <= 40000 Ω` (o >=250000 Ω; fault invalid/stale) | **Stop sperimentale, NON 40 °C della piastra** |
| Cutoff aria | `SHT45 >= 40 °C` o invalid/stale | Stop di prova, NON setpoint termostato |
| Durata massima | overall timeout 330 s | Il percorso può abortire |
| Raffreddamento | ~90 s dall'ultimo HEATER comandato ON | Continuare a osservare Sanwa dopo OFF |
| Logger | fino a 1400 campioni RAM ogni ~250 ms, max 350 s | Su LittleFS **solo dopo** cooling |

**Soglia stop MANUALE da non confondere:** nel protocollo utente dell'ultima prova S7G della chat «Riprendere progetto finale» fu comunicato **Sanwa >=50 °C** come intervento operatore. Alcuni protocolli *precedenti* S6A/S6C della repo riportano **55 °C**. Sono fonti discordanti relative a sessioni diverse: **in questa ripresa si adotta cautelativamente 50 °C** come veto umano; il valore è un vincolo operativo del collaudo, NON un rating termico dichiarato sicuro. Non cambiare retroattivamente i numeri delle vecchie prove. Qualunque cambiamento del veto necessita decisione esplicita/documentata, non un aggiornamento silenzioso. **Interrompere prima di 50 °C** per salita anomala, sonda distaccata, ventola ferma o qualunque altra anomalia. Il ritardo Sanwa può far sottostimare il picco.

## 3. Preparazione immediata (NON rifare i gate già superati)

1. **Documentare prima**: data/ora locale, temperatura ambiente o SHT45 aria iniziale (se visibile), identificatore `S7U`, alimentazione 24 V, test eseguito con box **chiuso**, posizione Sanwa/fissaggio (foto o descrizione), TCO105 presente, ventola libera, materiale eventualmente inserito (**nessun materiale non certificato vicino al lato caldo**), eventuali modifiche dalla prova S7G. Non sono richiesti nuovi test di tasti, OLED, USB, OTA, fan al 40%, sensori con heater OFF.
2. Se dispositivo non ancora aggiornato, da **GitHub Desktop** fare Fetch/Pull, selezionare esattamente il branch S7U, aprire `firmware/firmware.ino` in Arduino IDE 2.x, usare **porta di rete OTA già collaudata**. Flash soltanto con dispositivo **STANDBY**, non in raffreddamento o FAULT. Verificare **`HOT-261026-S7U`** nella versione OTA/status o `H26-S7U` nel display. Non utilizzare firmware dal ramo COLD o `main`.
3. Con dryer freddo e 24 V temporaneamente disconnessi, verificare **solo**: contatto e assenza di trazione sulla punta Sanwa, chiusura senza schiacciare cavi, cablaggio lontano dalla ventola e dal lato caldo, TCO105 mantenuto in serie, ventilazione e alimentazione 24 V rapidamente interrompibile. Non smontare la sonda per ripetere calibrazioni già fatte. Se una condizione manca, non avviare prova.
4. Box chiuso, alimentazione 24 V; display in **STANDBY** dopo boot, firmware corretto, Sanwa leggibile **per tutto il riscaldamento e raffreddamento**, operatore presente. Una ripresa unica (telefono che inquadri **Sanwa + OLED contemporaneamente**) è consigliata per sincronizzare eventi, **non obbligatoria**. Via web non osservare `HEATER ON` in tempo reale: HTTP può essere sospeso mentre il riscaldatore è acceso.

## 4. UNA sessione fisica supervisionata — esecuzione e osservazioni

**NOTA:** questa sezione descrive la sessione richiesta; S7U è CI PASS ma **non fisicamente validato**. È vietato ripetere automaticamente o allungare impulsi/innalzare cutoff. Se non sussistono le condizioni di §3, fermarsi.

1. In **STANDBY**, senza riscaldamento, leggere e registrare il **Sanwa iniziale**, e la temperatura aria SHT45 iniziale se a display. Annotare **ora locale** inizio / eventuale video. Non è una nuova prova heater OFF: è il punto zero del test caldo.
2. **Premere ON/OFF brevemente UNA volta** da STANDBY. Il comando è nel sorgente `firmware.ino`; un ON/OFF lungo durante DRYING (> circa 1,5 s) esegue STOP. **Non** premere ON/OFF in continuazione per i singoli impulsi: li comanda il firmware.
3. Osservare partenza ventola e **flusso reale**: kickstart ~1s e prerun ~5s da RUNNING; se non parte, fermare. Durante la prova leggere **OLED**: `HEATER ON n/6` durante il comando ON, `HEATER OFF` nelle pause; la scritta indica **comando GPIO**, non misura indipendente di corrente. Annotare temperatura Sanwa **alla comparsa del primo HEATER ON** (baseline confrontabile con S7G 17 °C).
4. Se la sequenza arriva all'impulso n.3, **immediatamente prima del terzo HEATER ON** annotare Sanwa (S7G: 36 °C). Se il test abortisce prima, scrivere `non raggiunto`, non inventare lettura.
5. **Quando il riscaldatore va definitivamente OFF**, per `FAULT / HEATER OFF`, conclusione impulsi, o STOP manuale: annotare immediatamente Sanwa **al momento del taglio**, ora/minuto/secondo o timestamp/video, messaggio esatto OLED (per esempio `NTC R LIMIT`, **solo se veramente mostrato**), impulso in corso e ventilazione. **Non assumere che S7G/S7U faccia sempre FAULT NTC:** registrare la causa realmente vista. In FAULT restano due beep in modalità test, poi silenzio; MODE breve consente mute ma **NON** cancella FAULT.
6. **Alla comparsa dello spegnimento definitivo**, annotare il Sanwa sul punto heater (lettura al FAULT/STOP/fine impulsi) e osservarlo durante il raffreddamento. **Normalmente ci aspettiamo che stabilizzi o scenda** perché non viene più fornita energia elettrica all'heater. È sufficiente annotare se scende regolarmente: **non è richiesto cercare, aspettare o registrare un secondo «picco post-OFF»**. Se la lettura dovesse eccezionalmente salire ancora, annotare l'anomalia, ma senza presumerla: può dipendere da ritardo della termocoppia o redistribuzione del calore e richiederebbe interpretazione. **Non confondere gli HEATER OFF nelle pause di 12 s con lo spegnimento definitivo** dopo FAULT, STOP o fine sequenza. Il Sanwa è uno strumento esterno e il suo andamento non viene registrato nel CSV MCU.
7. Interrompere subito il ciclo con **ON/OFF lungo ~1,5 s** se ancora in DRYING e se Sanwa >= **50 °C**, se il contatto sonda è dubbio, manca aria, rumori/odori anomali, crescita non controllata o altro guasto. Per emergenza, **togliere immediatamente 24 V** (non aspettare cooldown o salvataggio CSV). Annotare esplicitamente la mancanza di CSV se l'alimentazione d'emergenza lo impedisce. In FAULT l'app resta latched: **non** attendere che sparisca e **non** cercare di riavviare il test nello stesso boot.
8. Se l'operazione rimane sicura, lasciare completare ventilazione residua, annotare Sanwa **alla fermata ventola** e testo OLED `COOL DONE / CSV READY`, `CSV SAVE FAILED` oppure `TEST COMPLETE`. `CSV READY` è riferito a una nuova sessione quando visualizzato in FAULT, non a un vecchio log. Scaricare tramite **`http://filament-dryer.local/api/last-test.csv`** (oppure `http://<IP_DRYER>/api/last-test.csv`) **solo a fine cooling e salvataggio**. Pagina web già testata, nessun USB necessario; se non scarica annotare HTTP/errore e non ripetere calore per la sola acquisizione. Se necessario salvare anche `/api/events` oppure foto del testo di fault; il CSV **non include il codice del fault**.

## 5. Dati da raccogliere — nessun «va bene» o riassunto vago

Per la sessione compilare **`docs/evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md`**, con fonte delle informazioni e `NON OSSERVATO` se manca qualcosa. Campi obbligatori o esplicitamente *non osservati*:
- data, ora e timezone, persona che ha eseguito il test; ID build e branch/commit sorgente; eventuale deviazione dal firmware preparato;
- configurazione fisica / posizione-fissaggio Sanwa / foto (se presenti), box chiuso, TCO, ventola visivamente verificata, cablaggio, 24 V;
- Sanwa iniziale, al primo impulso ON, prima terzo impulso (se raggiunto), alla fine HEATER ON definitiva, osservazione se il Sanwa scende/stabilizza dopo l'OFF definitivo (annotare un'eventuale risalita solo se osservata), valore a fine cooling (quando osservato);
- SHT45 aria/RH iniziali o disponibili, estremi SHT45 e R_NTC **calcolati dal CSV**, non confonderli con Sanwa;
- ogni impulso effettivamente osservato (ordine, ON/OFF, causa interruzione), stato OLED finale e fault testuale, eventuale beep/azione MODE, fan cooldown/fine;
- eventuale azione operatore e motivazione, incluso cutoff 24 V; eccezioni, deviazioni, anomalie e limiti dei dati;
- CSV **raw non modificato** più file eventi/foto/video se forniti; hash SHA-256 byte del file raw, numero righe, intestazione, percorso commit. Se CSV assente scrivere `MANCANTE`, motivazione, senza riesecuzione automatica;
- analisi derivata in **documento separato** dai valori grezzi: timing dalla colonna `ms`, `heater_command`, `phase`, `pulses`, NTC(Ω), SHT45 aria(°C) e RH, `fan_percent` = PWM comando, non RPM. Correlazione Sanwa↔CSV solo quando sincronizzata da tempo/video; altrimenti classificare *approssimativa*.

**Il registro va committato prima della successiva decisione fisica**, insieme al CSV (se disponibile), con stato chiuso/incompleto dichiarato. Nessuna metrica inventata; non trasformare l'ultima stringa del CSV in COMPLETE senza evidenza; non dedurre portata dai PWM; non attribuire FAULT NTC senza prova nel testo eventi/UI.

## 6. Prossime decisioni e percorso fino alla V1 — gate con esito misurabile

**Gate G1 — acquisizione**: record sessione completo o campi mancanti dichiarati, dati raw/identità, andamento Sanwa dopo OFF, esito FAULT/COMPLETE e raffreddamento; aggiornare `PROJECT_STATE`, `TODO`, `README` e verbale del test con commit. Non richiedere vecchio CSV S7G.
**Gate G2 — analisi indipendente**: sovrapporre serie `R_NTC`, SHT45 aria e finestre `heater_command`; classificare trigger e ritardo al Sanwa senza falsa equivalenza NTC°↔Sanwa°. Determinare se 40kΩ interviene con margine ragionevole **nelle condizioni effettivamente osservate**; evidenziare incertezze della sonda.
**Gate G3 — qualifica interblocchi e modello**: verificare l'adeguatezza di guardia su NTC in ohm, fan senza tach, timeout, eventuale ritardo termico della sonda e raffreddamento, sensori stale/fault, TCO105 reale e materiali; decidere se serve una **nuova** prova *diversa e motivata*, con limite approvato separatamente. Nessuna modifica tacita ai limiti.
**Gate G4 — termostato aria**: implementare **prima in host/simulazione con heater fisicamente disabilitato** controllo SHT45 target 40 °C; compilation/CI e review; solo poi pianificare gate fisico indipendente se motivato. Non continuare semplicemente i 6 impulsi.
**Gate G5 — dryer V1**: protezioni termiche approvate, controllo aria e raffreddamento, durata impostabile, failure handling, UI e dati/web/OTA coerenti. Storico di 12 h esplicitamente rinviato; non blocca i gate termici ora.

## 7. Regola obbligatoria per ogni sessione successiva

Ogni prova nel progetto deve avere **PRIMA**: domanda ingegneristica, setup specifico (sonde e punti), branch/commit/build, limiti, manovra precisa, punto di arresto, misure e criterio di successo/fallimento, posizione del futuro CSV/verbale; **DOPO**: tutti i valori reali osservati con tempo e fonte, anomalie, file grezzi, analisi distinta, decisione e singola prossima azione, commit + link al commit. Se manca un file, dichiararlo come gap e non ripetere test senza una nuova motivazione esplicita. È vietato sostituire questi dettagli con un sommario generico in chat.

**Riferimenti retrospettivi:** `docs/STEP07_OPERATOR_UI_FIX_2026-10-26.md`, `docs/STEP06B_FAN100_EXTENDED_2026-10-25.md` (branch S6C), `docs/STEP02_SENSOR_PROBE_MOUNTING_2026-10-25.md` (branch operativo), `docs/HANDOFF_2026-10-26_STEP07_S7G_TO_S7U.md` (branch operativo). In conflitto con log o codici sorgente, usare sorgenti effettivi e **segnalare** discrepanze.
