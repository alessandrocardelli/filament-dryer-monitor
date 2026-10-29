# STEP09-B — Primo tentativo mirato di raggiungere 25°C ARIA con feedback SHT45 (HEATER ON)

> Historical protocol/technical record. Its dated NEXT and pre-test status are not the current work queue; see [PROJECT_STATE](PROJECT_STATE.md) and [TODO](TODO.md). Original checkpoint preserved in archive tags.

> **AGGIORNAMENTO 27/10/2026 — CONSENSO ACQUISITO.** Dopo la domanda specifica «Autorizzi il test fisico S9B con massimo **180 secondi cumulativi di riscaldamento**, target aria **25°C** e arresto manuale immediato se il Sanwa raggiunge **50°C**, mantenendo montaggio e protezioni S9A?», l'utente ha risposto **ordinando espressamente di procedere («fallo»)**, dopo richieste ripetute di conferma. Questo è interpretato come **autorizzazione esplicita a UNA SOLA prova S9B180 con il profilo già presentato**, non a180s continui, non a cambiare soglie o TCO e non a test futuri. **La conferma del montaggio fisico attuale non è stata fornita e deve essere verificata dall'operatore sul posto**. **Stato fisico S9B: NON ESEGUITO**. La [CI definitiva del codice con ID univoco H27-B180 è PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40572680240); non confondere CI con prova fisica. Non richiedere nuovamente all'utente di approvare i180s.

> **Identity correction 2026-10-27:** the sole active draft S9B 180-second firmware is **`HOT-261027-S9B180` / OLED `H27-B180`** in [PR #17](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17). Superseded [200s PR #15](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/15) and duplicate [PR #16](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/16) are **CLOSED WITHOUT MERGE**. Older images labeled `H27-S9B` are **AMBIGUOUS** and MUST NOT be flashed. Real physical S9B still UNRUN; new operator approval for 180s budget and physical setup review still REQUIRED; NO flashing until latest firmware identity CI PASS.

> **CI S9B SOFTWARE ESEGUITA E PASSATA (non prova fisica):** [Actions run #40562778719](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40562778719), job `122395977499`: test C++ sul `SafetyController` reale con GPIO simulato **PASS** (incluso precedente120s NON termina e nuovo180s termina con cooldown, target aria25°C/isteresi, NTC soft50/resume65/hard40, fault/STOP/stale); compilazione Arduino ESP32 **PASS** con 1.126.854 byte FLASH (85%) e 92.096 byte RAM statica (28%), senza overflow. Commit testato `ca5c0e5bf6ea3f9cccd531456649e384c19eda6e`; SHA blob di tutti i file firmware e workflow verificati identici fra tale commit e il branch dopo le aggiunte dei soli documenti. **Non abilitato né installato su prototipo e nessuna qualifica di sicurezza fisica:** `H27-B180` è solo l'identità che l'utente dovrà verificare dopo un eventuale flash, previo consenso specifico alla prova. [PR #17 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17), base branch S9A PR14.

**STATO 27/10/2026: PROTOCOLLO PREREGISTRATO, CI PASS, CONSENSO S9B180 ACQUISITO, PROVA FISICA NON ESEGUITA.** Questo è un esperimento fisico con riscaldatore realmente comandato. Non flashare o accendere in base alla sola approvazione concessa per S9A (120s).

## 0. Introduzione obbligatoria — COSA / PERCHÉ / PRECEDENTE / FUTURO

**COSA:** porteremo il dryer chiuso verso **25,0°C di ARIA SHT45**, con HEATER ON/OFF reale e ventola al100% comandata. **Unica variazione sperimentale deliberata rispetto a S9A: aumentare il tempo cumulativo massimo HEATER ON da120s a180s**; il firmware limita comunque **ciascun ON a20s**, con pausa minima OFF12s e arresti NTC indipendenti. 180s non significa 3min consecutivi.

**PERCHÉ:** S9A ha terminato per esaurimento dei120s di riscaldamento **quando l'aria era22,59°C**; ha raggiunto solo22,97°C nel cooling, quindi **non** ha dimostrato il controllo al setpoint25°C. Il nuovo margine dà una possibilità di vedere per la prima volta la risposta del termostato dell'aria, conservando i vincoli locali già osservati.

**PRECEDENTE:** S9A [778 righe di dati autentici](evidence/thermal/2026-10-27-S9A-raw.csv) e [13 letture Sanwa per ON/OFF](evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md): HEATER ON 13 volte, circa120s ON cumulativi nel CSV; Sanwa17°C iniziale, da ciclo5 a12 ON35/OFF38°C, ciclo13 ON35 poi cooling, Sanwa26°C alla fine. L'NTC minima è48,7167kΩ, i blocchi soft avvengono verso50kΩ, nessun FAULT. Aria17,64→22,59°C all'inizio cooling, picco22,97°C e poi22,93°C finale; **non raggiunti i25°C**. Gli 180s sono il **+50% di durata ON totale comandata**, NON un parametro termico già qualificato. S6C aveva raggiunto Sanwa53°C con profilo NTC differente, quindi NON usare S9A per concludere che 50°C è irraggiungibile.

**FUTURO:** se l'aria raggiunge25°C, verificare dal CSV se HEATER è andato OFF qualora fosse ON, oppure è rimasto OFF finché aria>24°C qualora già in pausa NTC; documentare cosa si è realmente osservato. Solo dopo potremo valutare i margini per una prova successiva a temperature dell'aria più alte. Se non arriva25°C, registrare i dati per capire se interviene budget180s, massimo runtime10min o altre protezioni: **non prolungare la prova sul posto**.

## 1. Identificazione senza ambiguità

- GitHub repo `alessandrocardelli/filament-dryer-monitor`.
- **Branch SOLO S9B:** `experiment/step09b-air25-budget180-2026-10-27`. Base: PR#14 S9A (branch `experiment/step09-real-sht45-feedback-2026-10-27`), senza merge di S9A in `main`; nuova PR S9B DRAFT.
- **Identificativo nuovo OLED: `H27-B180`; firmware completo: `HOT-261027-S9B180`.** Non eseguire questo test se l'OLED mostra `H27-S9A`, anche se la temperatura proposta è uguale25°C.
- **Controllo HEATER GPIO19 REALMENTE ATTUANTE nel nuovo branch; firmware non installato né fisicamente provato quando questo protocollo è stato creato.**
- CI: compilazione ESP32 core3.3.12, U8g2 2.36.19, mock del vero `SafetyController` e test nuovo «120s non termina più, 180s termina e cooldown90s». **Non dichiarare PASS prima di aver consultato log/risultato CI del commit esatto.**

## 2. Profilo di prova — diff intenzionale

| Parametro | S9A eseguito | S9B proposto |
| --- | ---: | ---: |
| Target **ARIA** SHT45 | 25,0°C | **25,0°C INVARIATO** |
| Riaccensione HEATER | aria<=24,0°C | INVARIATA |
| Spegnimento HEATER se aria>=25°C | sì | INVARIATO |
| Protezione locale NTC **SOFT** | HEATER OFF a R<=50kΩ | INVARIATA |
| Recupero dopo pausa NTC | R>=65kΩ + OFF>=12s | INVARIATO |
| **GUASTO HARD NTC** | R<=40kΩ, R>=250kΩ, invalid/stale | **INVARIATO** |
| Limite temperatura aria guasto | aria>=40°C | INVARIATO |
| ON consecutivo massimo | 20s + hardware ISR20,5s | INVARIATO |
| **ON cumulativo massimo** | **120s** | **180s: UNICA VARIAZIONE DI CONTROLLO** |
| Max durata trial (escluso cooldown) | 600s =10min | INVARIATO |
| Ventola | PWM100% comandato + prerun ~5s | INVARIATO |
| Raffreddamento post ultimo ON | 90s | INVARIATO |
| Sanwa su piastra — veto umano | fermare a50°C | **INVARIATO** |
| File CSV locale | RAM1400 campioni a500ms, salvato a freddo | INVARIATO |

La modifica a180s aumenta l'esposizione termica cumulativa massima del50%: non è prova di sicurezza per materiali, hot spot, TCO o ventola. Le stesse soglie SOFT/FAULT non certificano un intervallo di temperatura della piastra; R_NTC non è convertita a°C. Il TCO monouso105°C **attualmente presente in serie** non viene modificato né bypassato. La ventola PWM100% non misura RPM. Elettronica fisica (stuck MOSFET ON, cablaggio, materiali) non è qualificata da questa prova. Non attribuire un guasto o stop software alla temperatura Sanwa se il Sanwa non è interfacciato alla scheda.

## 3. Gate prima di caricare il firmware: SOLO una volta, niente prove fisiche heater-OFF aggiuntive

- [x] **CI nuova S9B PASS confermata** [run40572680240](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40572680240), codice testato commit `3ab2d318b292cc035c9ebd4bfa334bba5f882fa9`, identificativi univoci `HOT-261027-S9B180` / `H27-B180`, mock+build ESP32. Dopo il commit di test, solo documenti cambiati; verificare l'identificativo dopo il flash.
- [x] **Autorizzazione utente acquisita** nel messaggio «Ma porco dio fallo cazzo! Quante volte te lo devo chiedere?» in risposta alla richiesta che nominava **S9B180, target aria25°C e veto manuale Sanwa50°C**. Non richiederla di nuovo: vale solo per un ciclo nel setup esistente, senza variazioni NTC/fan/TCO.
- [ ] Verifica fisica rapida dell'assemblaggio **prima di qualunque HEAT**: stesso dryer chiuso, SHT45 in aria bobina, Sanwa termocoppia K RD701 ben fissata alla piastra vicino bead NTC nella stessa posizione S9A, ventola libera realmente funzionante, nessun filo schiacciato/cambiato, TCO105 in serie integro, 24V alimentazione immediatamente scollegabile, niente deformazioni/odori/danni. Se una condizione è diversa o incerta, fermarsi e segnalarla; **non chiedere foto di routine**.
- [ ] A box freddo, prima del test, leggere SHT45 iniziale e Sanwa. Se l'aria è **>24°C**, il firmware potrebbe non accendere HEATER: non raffreddare forzatamente la sonda né modificare profilo sul momento.
- [ ] Firmware caricabile **soltanto** dal branch S9B verificato, durante STANDBY heater OFF via OTA già usata in S9A; controllare display esattamente `H27-B180`. Nessun flash automatico tramite strumenti in questa conversazione.

## 4. Procedura fisica e letture Sanwa — da seguire SOLO dopo i gate

1. **PRIMA DI PREMERE ON/OFF**, dryer freddo: verifica rapidamente setup/cavi/ventola/TCO e leggi **Sanwa iniziale + SHT45 aria iniziale**. Se SHT45>24°C, non cercare di forzare HEAT. L'eventuale flash OTA S9B deve essere stato effettuato in STANDBY, con OLED `H27-B180`.
2. Premi ON/OFF breve **UNA VOLTA**, osserva la ventola girare fisicamente e il prerun circa5s. Al **PRIMO `HEATER ON`** leggi Sanwa, quindi al **PRIMO `HEATER OFF`** leggi di nuovo Sanwa e aria SHT45 (se possibile video con OLED e Sanwa insieme).
3. **Sanwa: annota la temperatura a OGNI passaggio HEATER ON e HEATER OFF**, indicando il numero di ciclo, come nella prova S9A. Se gli eventi sono troppo ravvicinati per scrivere senza distrarti, **filma contemporaneamente Sanwa e OLED** e trascrivi le letture solo DOPO la prova. **Continua comunque a sorvegliare il Sanwa ININTERROTTAMENTE**, anche tra un evento e l'altro: il limite manuale50°C vale SEMPRE, non soltanto quando annoti.
4. **SE SHT45 ARRIVA A25°C**, annota immediatamente **temperatura Sanwa, stato HEATER ON/OFF, temperatura SHT45 25°C**. Se HEATER era ON, deve spegnersi per target. Se era già OFF in pausa NTC, resta OFF: la riaccensione deve rimanere proibita finché SHT45 non torna<=24°C. Non aspettarti che venga avviato automaticamente il COOLING per il solo raggiungimento25°C: il termostato può continuare a sorvegliare entro budget/timeout. Non forzare raffreddamenti per testare isteresi.
5. Se parte il **COOLING** (per budget180s, timeout10min oppure dopo un eventuale STOP), leggi **Sanwa a inizio cooling + SHT45 aria**. Annotare la causa effettiva se display/log mostrano FAULT/STOP. **Se il target non è mai arrivato a25°C, scriverlo: è un risultato valido, non aumentare manualmente limiti.**
6. **STOP IMMEDIATO** (ON/OFF lungo ~1,5s; in emergenza scollegare 24V) se Sanwa **raggiunge50°C** o appare ventola fisicamente ferma, odore, deformazione, fumo, rumore anomalo, cablaggio caldo/danneggiato, riscaldamento non controllabile o messaggio FAULT. Non aspettare la prossima misura pianificata.
7. A **FINE COOLING**, quando la ventola si ferma, leggi **Sanwa finale** e SHT45 finale. Non spegnere forzatamente a metà raffreddamento se il prototipo è stabile. Una volta HEATER OFF e cooling completato, scarica **solo il nuovo CSV** da `http://filament-dryer.local/api/last-test.csv` (o IP del dryer); non rinominare/correggere manualmente righe. Se non disponibile, annota il problema e **non ripetere tutto per recuperarlo**.

## 5. Dati da consegnare e registrare in GitHub

- Firmware OLED realmente visto (`H27-B180`), ora/sessione soltanto se osservata, conferma che sensore Sanwa/ventola/TCO/cablaggi sono invariati rispetto a S9A e regolari. Non inventare valori.
- Sanwa + aria SHT45 iniziali, Sanwa a OGNI HEATER ON/OFF (o video da cui leggere ogni evento), al primo SHT45 25°C se raggiunto, all'inizio cooling e alla fine cooling. Eventuale massimo Sanwa osservato e causa STOP/FAULT. Video opzionale OLED+Sanwa.
- CSV originale della NUOVA sessione `filament-last-test (...) .csv`, hash SHA256, byte-count e campioni, `phase`, `pulses`, transizioni `heater_command`, NTCΩ, SHT45°C e RH, fan% come comando. Non spacciare percentuale fan per portata reale.
- Verbale preregistrato da completare: `docs/evidence/thermal/2026-10-27-S9B-PHYSICAL-SESSION.md`. Conservare S9A invariato `docs/evidence/thermal/2026-10-27-S9A-raw.csv` e verbale S9A; nuova prova va in file NUOVI e **non** sovrascrive l'originale S9A.
- Analisi in file distinto prima di decidere passo successivo. Update README/PROJECT_STATE/TODO/THERMAL_EXPERIMENT_LEDGER. `DECISIONS` solo se scelta di progetto realmente approvata.

## 6. Criteri di giudizio: non confondere fenomeni

- **G1 dati:** CSV nuovo valido, letture Sanwa e stato finale con timestamp e firmware reali; se assenti dichiarare lacuna.
- **G2 piastra:** pause SOFT vicino50kΩ, hard40kΩ non oltrepassato, Sanwa<50°C nel punto di contatto, cooldown visibile; non prova sicurezza certificata.
- **G3 feedback aria:** 25°C **raggiunti** e osservazione chiara dello stato HEATER ON/OFF a e dopo la soglia; distinguere OFF già per NTC da OFF comandato al target; isteresi24°C solo se naturalmente osservabile, non inventare.
- **G4 tempo:** se raggiunto180s prima del target, il limite cap spiega il cooling, ma non è una qualifica termostato. Limite globale600s è separato e non si interpreta come budget HEAT. Dati S9A comparabili perché sono invariati gli altri parametri e il setup dichiarato.
- **Sicurezza:** nessuna elevazione target>25°C, nessun cambio soglie NTC/fan/TCO sulla scorta del solo risultato S9B. Richiesta nuova autorizzazione separata per altro test HEATER.
