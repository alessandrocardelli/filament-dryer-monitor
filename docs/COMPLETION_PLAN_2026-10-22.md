# Piano maestro di completamento — Filament Dryer Monitor

**Data:** 2026-10-22  
**Stato:** piano operativo; NON autorizza a scaldare, cambiare protezioni, firmware attivo o KiCad.  
**Lavoro corrente:** branch `plan/closed-heater-qualification-2026-10-20`, 8 commit di preparazione oltre `main` al checkpoint. Verificare sempre HEAD/branch prima di riprendere.  
**Fonti vincolanti:** `AGENTS.md`, `docs/DECISIONS.md`, `docs/PROJECT_STATE.md`, `docs/TODO.md` e **sorgenti KiCad/firmware effettivi**; in caso di discrepanza prevalgono KiCad per l'hardware e datasheet/standard per i limiti. Non sostituisce `docs/PROJECT_ROADMAP.md`; ne costituisce la vista operativa aggiornata.

## Che cosa significa "finito"

1. **V1 essiccatore autonomo:** eBOX Lite chiuso e assemblato; ventola e riscaldatore lavorano insieme con regolazione della temperatura **dell'aria in camera**, sorveglianza indipendente del riscaldatore e protezione termica hardware comprovata; pulsanti/OLED sufficienti senza Wi-Fi; arresto, fault, reset e riavvio gestiti senza riattivazione inaspettata.
2. **Progetto funzionale completo:** oltre alla V1 autonoma, registrazione/storico CSV su flash, interfaccia web locale per visualizzazione/gestione e diagnostica secondo le funzioni approvate; operatività autonoma preservata durante guasti Wi-Fi.
3. **Consegna riproducibile:** firmware sorgente versionato e riproducibile, parametri e collaudi archiviati, disegni/schemi e modifiche fisiche documentati, istruzioni per assemblaggio/uso/manutenzione, decisione esplicita se produrre una V2 PCB. Questo non equivale a certificazione di prodotto commerciale.

La chiusura di un gate richiede **evidenza concreta** (foto, dati, log o controllo del sorgente), un risultato PASS/FAIL/OPEN e un commit di aggiornamento documentale. Un documento con una procedura non è un test passato.

## Stato consolidato al 22 ottobre 2026

### Già completato — NON ripetere senza motivo
- PCB first article alimentata, ESP32/USB/UART, OLED, quattro pulsanti, buzzer, SHT45, NTC ADC, driver ventola e driver Q4 testati nei limiti registrati.
- Risolto giunto U4 pin fisico 27/GPIO16; ventola 100% per 1 s poi 40%, run sorvegliato 2–3 min, stop, STANDBY e riaccensione senza auto-resume; SETUP/SETTINGS e persistenza riportati PASS. Foto display con SHT45 19.4 °C/69.7% RH e NTC indicato 15.5 °C **non provano calibrazione**.
- Q4/J5 a vuoto 2026-10-08 e sotto carico reale ~24 V/1.59 A con impulsi 1/5/10 s il 2026-10-14. Non ripetere test elettrici già superati come prerequisito generico.
- Sette punti NTC-resistenza / termocoppia Sanwa presso piastra da 30 a 85 °C (temperatura Sanwa) con **coperchi aperti e fan OFF**; analisi in `docs/NTC_CALIBRATION_ANALYSIS.md`. Non costituiscono curva interna certa del bead né collaudo termico a box chiuso.
- Pannello frontale stampato e fit riferito positivo. Non riaprire cosmetica/nuova stampa per procedere alle fasi A/B.

### Ancora aperto
- Geometria realmente chiusa: posizione SHT45, termocoppia sulla piastra, instradamento sonda e cablaggi, TCO, ventola e assenza di schiacciamenti; nessun PASS di fase A.
- TCO monouso 105 °C 15 A 250 V riportato in serie a HEATER+: MPN/datasheet, Tf/Th/Tm, rating DC, posizione/coupling e compatibilità con i riferimenti eBOX Lite **piastra 105/116/118 °C vs aria 40/50/55 °C** non qualificati. Nessun PASS di fase B.
- Discrepanza NTC a freddo ~20 °C / 122.3 kΩ rispetto ai fit dei punti caldi; conversione e limiti termici definitivi non approvati.
- `main/firmware/SafetyController.h` forza GPIO19 LOW e `heaterPermitted() == false`: riscaldamento nell'app **non implementato**, anche se vecchi diagnostici potevano accenderlo. Identità del firmware effettivamente flashato non definitivamente archiviata.
- Logging LittleFS/CSV e web non implementati; validazione prolungata/fault finale e possibile V2 non completate.

## Sequenza obbligata — una sola cosa nuova alla volta

| Gate | Lavoro | Chi può produrre l'evidenza | Prova di completamento | Dipende da |
|---|---|---|---|---|
| **A** — sensori e chiusura, NO HEAT | Esame fisico a 24 V/USB rimossi: SHT45 nell'aria della bobina, Sanwa K fissato alla piastra, sonda in uscita senza schiacciamento, J3/J5/J6/J7/TCO/ventola sicuri, coperchi realmente chiudibili. Poi **una sola** verifica ventilazione 2–3 min in geometria chiusa **con J5 fisicamente isolato** se si vuole dare PASS al gate formale no-heater. | Utente: foto, posizione/percorsi, osservazioni T/RH, ventola; assistente: confronto con criteri. | Foto/descrizione di posizioni; due coperchi chiusi, nessun filo pizzicato, airflow libero, SHT45 legge in camera, heater isolato per il collaudo formale. Se J5 resta collegato, annotare limitazione e NON dare PASS all'isolamento. | Presente. Seguire `docs/PHASE_A_B_CLOSED_DRYER_PREP_2026-10-22.md` e `docs/CLOSED_CASE_STAGE1_SENSOR_PROBE_FAN_NO_HEAT.md`. |
| **B** — protezione indipendente | Identificare esatto TCO (foto sigla e punto di contatto), reperire datasheet e rating pertinenti anche 24 V DC, verificare compatibilità Tf/Th/Tm e temperature ammissibili di cavi, plastiche, crimps/superfici; valutare protezione originale. Se occorre cambiare protezione o posizione: opzioni con evidenza, consenso esplicito, nuova decisione `DECISIONS.md`. | Utente: foto/marcature/materiali; assistente: verifica dati produttore/valutazione. | Architettura di cutoff fisico idonea e motivata, installazione/continuità e criteri di arresto indipendenti approvati. **Non** bypassare o alzare soglia automaticamente. | A per posizione reale; nessun riscaldamento finché B non passa. |
| **C** — conversione/limiti NTC e requisiti di controllo | Distinguere SHT45 aria, NTC interno riscaldatore, Sanwa contatto piastra. Riesaminare incertezza a freddo con **un'osservazione co-localizzata senza scaldare** se utile; fissare range, limiti e fault per sensori open/short/stale/outlier; definire stop per fan non confermata (non esiste tachimetro). | Assistente: analisi e criteri; utente: sola misura addizionale mirata se necessaria. | Modello/limiti esplicitamente provvisori o qualificati, test e criteri documentati; nessun uso dei 7 punti open-case come datasheet termistore. | In parallelo a B come analisi; limiti finali devono essere coerenti con B/D. |
| **D** — firmware sperimentale sicuro | Branch separato: programmazione fonte esatta, heater LOW in boot/reset/fault, ventola preavviata, consenso esplicito test, bounded time/energia/temperatura, NTC+SHT45 con guasti e scadenze, log seriale dettagliato, nessun autoresume. **Prima** test a freddo e fault simulati; review e consenso distinto prima di flash che può scaldare. | Assistente: implementazione, review e fonti; utente: solo collaudo fisico controllato. | Commit sorgente, firmware identificato, compilazione e test di stati sicuri documentati; nessuna attivazione termica implicita dalla UI. | A+B e criteri preliminari C. |
| **E** — qualifica termica chiusa | Con approvazione separata: dryer **chiuso**, fan realmente funzionante, Sanwa piastra + SHT45 aria + R_NTC + corrente/TCO/materiali osservati; iniziare da condizione d'aria moderata **inferiore a 40 °C** secondo limiti B/C; incrementi successivi solo dopo PASS. Confrontare eventualmente aria 40/50/55 °C e piastra OEM 105/116/118 °C come **risultati da misurare, non setpoint NTC**. | Utente: prove fisiche supervisionate e log; assistente: analisi, abort criteria. | Tracce di riscaldamento, stabilità, spegnimento, raffreddamento, senza hot spot/fault e con margini rispetto alla protezione. Nessuna corsa automatica a 118 °C. | A+B+C+D e autorizzazione specifica. |
| **F** — firmware V1 autonomo | Trasformare i risultati di E in controllo chiuso temperatura aria, limite protezione heater, FAN AUTO e cooldown, timer, fault latched, watchdog/brownout, UI/SETUP ranges e NVS; usare il firmware attuale come base senza rimpiazzare funzionalità collaudate. | Assistente: codice e test; utente: prova hardware accettazione. | Ciclo reale start/hold/stop/complete/cooldown, valori stabili e fault rilevati; reset e power-cycle non riavviano il riscaldamento. | E per taratura e limiti definitivi. |
| **G** — dati e rete | Implementare LittleFS/storico CSV, gestione errori storage, esportazione, web locale e diagnostica senza dipendenza dal Wi-Fi per l'uso dei pulsanti. Definire retention e comandi remoti che non bypassano la sicurezza. | Assistente: sviluppo/documentazione; utente: prove interfaccia/uso. | Dati persistenti ed esportabili, UI web funzionante, perdita Wi-Fi o memoria piena non compromette protezioni/ciclo locale. | Si può prototipare prima; validazione finale su F. |
| **H** — collaudo finale / rilascio | Prove ripetute a regime entro limiti approvati, fault injection sicuro, power cut e recovery, integrazione termica della scocca, cavi e sensori, backup schemi/BOM/firmware/CAD, decisione se è necessaria PCB V2 (es. D020 bodge) e revisione degli output. In caso di PCB edit, ERC/DRC **nuovi** e release source/output; niente fiducia esclusiva nei report datati. | Utente: prove fisiche; assistente: audit evidenze/versioni. | Checklist di accettazione conclusiva e `PROJECT_STATE/TODO/DECISIONS/README` allineati, commit e versione finale etichettata. | F+G (e V2 solo se motivata). |

## Regole antistallo per ogni sessione

1. **Bootstrap da `AGENTS.md`** e lettura di branch, HEAD, README, PROJECT_STATE, DECISIONS, TODO e sorgenti correlati; la chat precedente è solo indizio.
2. Nel resoconto usare sempre: **ultimo gate PASS**, **gate corrente OPEN**, **un prossimo passo verificabile**, **evidenza necessaria**, **blocco/sicurezza**, **link commit** se è stato salvato lavoro.
3. **Un gate alla volta.** I passi già spuntati non si ripetono per inerzia; una prova nuova deve rispondere a una domanda nuova.
4. Non segnare PASS per codice scritto, testo di piano o foto che mostrano solo una schermata. Distinguere risultati riferiti dall'utente da dati strumentali indipendenti.
5. **No flash di codice heater-enable, nessun test termico chiuso o modifica TCO** senza i rispettivi gate e autorizzazioni. Vietato bypassare TCO. Se geometria, interlock, temperatura materiali o misure sono incerti, fermarsi e lasciare il riscaldatore OFF.
6. In fase di chiusura del gate salvare dati e log verificati su branch, aggiornare `PROJECT_STATE.md` e `TODO.md`, aggiornare `DECISIONS.md` solo per decisioni esplicitamente concordate. Separare il recupero firmware sperimentale dal codice di produzione.

## Prossima azione specifica

**Gate A / sottopasso A1, solo a dispositivo spento:** raccogliere un'unica vista chiara dell'interno/indicazione delle posizioni effettive di **SHT45**, **Sanwa presso piastra**, **TCO**, cavi del **fondo e coperchio superiore**. Verificare che il cavo della termocoppia possa uscire mentre si chiudono completamente entrambi i coperchi e che nessun cavo ostacoli ventola o si appoggi a zona critica. **Nessuna richiesta di ripetere test PWM/Q4/NTC già passati; nessun riscaldamento.** Dall'esame si passa alle sole correzioni meccaniche strettamente necessarie, poi al gate B.
