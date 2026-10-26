# Handoff definitivo 26 ottobre 2026 — prove termiche, FAULT S7G e firmware successivo

> **ADDENDUM FINALE 26 OTTOBRE 2026 — S7U TEST FISICO ESEGUITO, SUPERSEDE OGNI INSTRUZIONE PRE-TEST SOTTO:** Resoconto utente **PROVA S7U**: Sanwa17°C all'inizio/primo HEATER ON,36°C prima del terzo impulso,43°C al FAULT OLED `NTC R LIMIT`; raffreddamento Sanwa regolarmente in discesa, **25°C quando ventola si ferma**. Utente ha allegato `filament-last-test (2).csv`: [ORIGINALE byte-identico 717 campioni su branch S7U](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/evidence/thermal/2026-10-26-S7U-retest-raw.csv), [VERBALE COMPLETO incl. mancanze](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md), [ANALISI NUMERICA](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/evidence/thermal/2026-10-26-S7U-retest-analysis.md). Risultati: 2×20s ON +terzo19,25s troncato da cutoff `NTC=39789,3Ω` (<40kΩ), phase6=ABORTED, 717 campioni /179,021s; SHT45 aria **18,20→19,52°C a OFF→20,89°C fine**, RH64,12→58,65%; PWM fan100% in ogni riga, fermata ventola riferita dall'utente (non campionata). Il Sanwa a contatto piastra **scende**, non ipotizzare picco dopo OFF; la **temperatura ARIA** invece cresce dopo taglio. Il file CSV non incorpora l'ID firmware, 'S7U' è dichiarazione utente. **G1 ACQUISIZIONE PASS LIMITATA** (safety e regolazione V1 non qualificate). **NUOVO NEXT P0 = G2/G3 review dei dati e protezioni**; NON ripetere prova, NON recuperare CSV S7G, NON rifare cold/OTA/fan test. Conservare cronologia precedente come storico pre-prova, non operativo. Firmware caldo PR #12 resta separato da COLD/main, nessun merge.

> **ADDENDUM SUCCESSIVO DEL 26 OTTOBRE 2026 — QUESTO PREVALE SUI «PROSSIMI PASSI» QUI SOTTO.** Dopo il passaggio consegne S7G→S7U, l'utente ha espresso una nuova decisione: **non recuperare più il CSV S7G mancante; ripetere UNA nuova sessione fisica S7U con lettura Sanwa al taglio e andamento di raffreddamento; nessun picco post-OFF presunto o obbligatorio, senza rifare test COLD/OTA/menù/ventola già superati.** Il CSV S7G resta storicamente NON CONSEGNATO, non inventare misure. Protocollo pre-registrato sul ramo S7U: https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/STEP07_S7U_PHYSICAL_RETEST_PROTOCOL_2026-10-26.md . Registro pre-creato (non ancora eseguito): https://github.com/alessandrocardelli/filament-dryer-monitor/blob/fix/step07-clear-test-feedback-2026-10-26/docs/evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md . L'ultima istruzione operatore S7G in chat indicava **veto Sanwa 50°C**, contro **55°C delle precedenti prove S6A/S6C**: documento nuovo esplicita la differenza e usa limite manuale conservativo 50°C. **S7U non fisicamente testato**; non confondere preparazione e risultati. Qualunque nuova lettura, fault, picco post-OFF, CSV raw e analisi devono entrare nel registro e in commit GitHub prima del prossimo gate, secondo AGENTS.md. Le sezioni storiche successive sono conservate come cronologia, non come nuovi ordini di recupero CSV.

Questo è il punto d'ingresso per la prossima chat. Fonte primaria = repository GitHub; applicare sempre `AGENTS.md` prima di ogni attività. Non trattare resoconti della chat come sostituti del firmware/CSV originali.

## Branch, versioni, stato comprovato

| Ruolo | Branch e riferimento | Stato |
|---|---|---|
| Main (separato) | `main` | Non contiene i nuovi esperimenti HEAT |
| Branch operativo COLD | `feature/wifi-ota-web-logging-2026-10-24` | Documenti e handoff, **heater di produzione disabilitato** |
| Prova fisica precedente | `experiment/step06b-fan100-extended-2026-10-25`; `HOT-251025-S6C` | Fan100%, sessione fisica eseguita, CSV originale copiato in `docs/evidence/thermal/2026-10-25-S6C-fan100.csv` |
| Prova fisica **più recente** | `experiment/step07-ntc-guard-verified-history-2026-10-25`; sorgente `ab17acf4e816a32cc58a4e1d6735ad0c1cb0cdf4`; `HOT-251025-S7G` / OLED `H25-S7G`; [PR #11](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/11) | **Caricato fisicamente dall'utente, test a caldo eseguito, FAULT osservato**, CSV **non ancora fornito**. [CI S7G PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40224936507) |
| **Prossime modifiche software già realizzate** | `fix/step07-clear-test-feedback-2026-10-26`; commit `8f804f5ef08c9ec91eeadd576fc35fc976250638`; `HOT-261026-S7U` / OLED `H26-S7U`; [PR #12](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/12) | Correzioni UI/beep già committate, [CI PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40382200144), PR **draft, non mergiata**, firmware **NON fisicamente caricato** |

Non scambiare il codice del branch operativo COLD o del branch S7U con la build S7G usata durante la prova fisica. Preservare i commit originali e la tracciabilità. Workflow di upload dell'utente: **GitHub Desktop (Fetch/Pull + selezione branch) → Arduino IDE `firmware/firmware.ino` → porta di rete OTA già collaudata**. Evitare istruzioni ridondanti su download ZIP o web OTA.

## Evidenze storiche indispensabili (GIÀ note prima di S7G)

- Il 15 ottobre `docs/PROJECT_STATE.md` e `docs/FIRST_ARTICLE_TEST_MATRIX.md` registrano confronto dinamico Sanwa / **vecchia stima firmware NTC**, con offset fino a +6°C durante riscaldamento, inversione nel raffreddamento. Sonda Sanwa vicino alla piastra **può reagire in ritardo** e misura un punto fisico differente dal bead NTC.
- Sette plateau `docs/NTC_CALIBRATION_ANALYSIS.md` erano con **box aperto e fan OFF**. Non assimilare il termistore interno a una temperatura Sanwa calibrata; non imporre calibrazione `100k B3950` non dimostrata.
- Donor **eSUN eBOX Lite**: temperature nominali di piastra/aria di tipo diverso. Sanwa = superficie locale vicino piastra, NTC = R del sensore embedded riscaldatore, SHT45 = aria camera. **NON** confrontare Sanwa e temperatura camera come se coincidessero.
- D025 40% PWM era margine elettrico per avvio della ventola, NON duty termico ottimo. S6C/S7G PWM comandata 100%, **nessun tachimetro o misura di portata**.
- TCO 105°C fisicamente presente in serie nel prototipo, D029 distingue la scelta di add-on opzionale dal cablaggio reale. Non bypassare.
- `docs/REPOSITORY_FULL_REVIEW_2026-10-25.md` è il precedente audit repository globale: inventario di sorgenti, CAD, test, storia e limiti delle verifiche su binari.

## Prove termiche reali da non ripetere senza nuova motivazione

| Test | Condizioni e risultati |
|---|---|
| S6A `HOT-251025-S6A` | 4×10s max, FAN40%; 411 campioni / 141,542s; aria SHT45 18,51→19,52°C, NTC min49,04kΩ, Sanwa 18→40°C; data presente nel report Step06 |
| S6C `HOT-251025-S6C` | 6×20s max, FAN100%; **652 campioni / 275,664s**; aria SHT45 18,64→22,57°C ultimo heater OFF→23,98°C fine cooling; NTC 131,95→min30,32kΩ; Sanwa iniziale18°C, max53°C durante HEAT, 30°C a fine cooling. CSV originale su branch S7G; il Sanwa aveva già noto ritardo |
| S7G `HOT-251025-S7G` | Nuova guardia RAW NTC 40,00kΩ (da 19,68kΩ di S6C), fan100%, stesso MAX di sei impulsi da20s e 90s cooldown; **prova fisica eseguita**, l'utente riferisce Sanwa **17°C prima del 1° impulso, 36°C prima del 3°, 43°C alla comparsa di FAULT**. Nessun valore registrato del Sanwa **successivamente a HEATER OFF**, né temperatura finale, né CSV S7G ancora ricevuto. Non inventarli. |

Il replay del CSV S6C prediceva crossing 40kΩ durante il terzo impulso; il **FAULT fisico S7G** è compatibile con esso, ma finché manca il CSV non attribuire il codice di fault esatto o il timing. Il veto Sanwa manuale di55°C era un limite del test, **non il massimo di funzionamento certificato della piastra OEM**. I risultati non validano ancora controllo aria40°C continuativo.

## Il problema UX realmente riscontrato nel test S7G

L'utente ha osservato FAULT persistente e buzzer ripetuto all'infinito. Verificato sul sorgente S7G:
- `Buzzer::startFaultAlarm()` avvia sequenza `repeat=true`; non finisce da sola.
- In FAULT `MODE` **pressione BREVE** richiama `buzzer.muteFault()`, **non cancella FAULT**. OLED rimane FAULT anche dopo cooling: corretto fail-safe ma prima spiegato male.
- `SafetyController::coolingRequired()` mantiene la ventola se presente calore recente, fino a circa **90s dall'ultimo HEATER ON**; la ventola deve quindi fermarsi senza che scompaia FAULT. Il file `/api/last-test.csv` viene salvato da `ThermalLog::saveWhenSafe()` **solo quando termina il cooling**; scaricare dopo la fermata ventola. Se file mancante, NON fare un'altra prova a caldo per recuperarlo.
- La frase «Sanwa massimo dopo spegnimento» si riferisce al massimo **misurato nei secondi successivi al taglio HEATER**, inclusa eventuale inerzia del Sanwa, e NON al valore finale del cooling. Questa informazione **non è stata misurata** nel test S7G, non chiederla nuovamente come condizione di proseguimento.

## Modifiche al codice GIA' pronte: PR #12 / S7U (NON reimplementare!)

- `firmware/Ui.h`: OLED con comando HEATER ON/OFF chiaramente visibile, FAULT / HEATER OFF, tipo di guasto, indicazione Sanwa Watch Peak, countdown cooling; status CSV READY solo per nuovo file salvato, CSV SAVE FAILED in errore. Correzione label AIR LIMIT 40C.
- `firmware/Buzzer.h` e `firmware/firmware.ino`: in sola modalità test una singola segnalazione acustica del FAULT, poi silenzio, senza togliere latch; comando MODE mute ancora disponibile. In modalità operativa preservato allarme ripetuto D022.
- `firmware/SafetyController.h`: solo getter di countdown per interfaccia, **nessuna modifica** a cutoff heater.
- `firmware/FirmwareBuild.h`: `HOT-261026-S7U`; test host e workflow CI nuovi. La PR #12 verifica sorgenti guardia/sensori/ventola invariati e ha **GitHub Actions PASS**, non è stata caricata sulla scheda. **Non richiedere un nuovo test caldo per verificare le sole scritte**.
- Documento tecnico completo già sul ramo fix: `docs/STEP07_OPERATOR_UI_FIX_2026-10-26.md`.

## Prossimi passi ordinati per priorità (NON ancora eseguiti)

1. **P0: acquisire il CSV S7G già registrato** dal dryer: se ancora alimentato e dopo che la ventola si è fermata, scaricare `http://IP_DEL_DRYER/api/last-test.csv`. Verificare che appartenga a S7G, non al precedente S6C; estrarre codice di FAULT, timestamp/campioni, andamento NTC e SHT45, fan, modalità di cooldown/salvataggio. Il CSV non contiene Sanwa. Se il file è assente o la scheda è stata riavviata/aggiornata, dichiarare il limite e non ripetere il riscaldamento per ricrearlo.
2. **P0: verificare e mantenere PR #12 S7U** come correzione software UX disponibile; eventuali ritocchi di leggibilità a OLED/buzzer si fanno su branch isolato, con test e compilazione, senza toccare limiti piastra o autorizzare nuova prova. Non merge silenzioso nel branch operativo COLD né `main`.
3. **P1: progettare e sviluppare su un nuovo branch un prototipo di controllo aria SHT45 a40°C in modalità HARDWARE HEATER OFF** (test simulati e CI) prima di qualsiasi abilitazione fisica. NTC grezza come guardia separata e fail-safe da qualificare, non temperatura piastra coincidente con l'aria. Separare decisione su stop fault, latenza, timer hardware, cooldown, fan command ≠ airflow; non inventare modello NTC definitivo.
4. **P1: review di sicurezza**: adeguatezza TCO105 presente, materiali locali, connessioni J5 e cavi, differenziale termico/fan a diverse velocità, guasti sensori, MOSFET bloccato ON, variabili lato comunicazione web/OTA. La guardia 40kΩ è SOLO soglia sperimentale, non legge di sicurezza termica qualificata. Qualsiasi nuova prova a caldo richiede profilo esplicito, motivazione e approvazione dell'utente.
5. **P2: esperienza utente V1**: evitare indicazioni tecniche senza significato per l'operatore, visualizzare HEATER ON/OFF reale, motivo allarme, come silenziarlo, cooldown e file pronto, senza che FAULT venga cancellato dal mute; mantenere cronologia log, distinguere RAM e salvataggio. Non modificare D022/D025/D029 implicitamente.

## Flusso di lavoro per prossima chat

1. Leggere `AGENTS.md` e seguire TUTTO il blocco `Before working`: HEAD e branch attuale, README, PROJECT_STATE, DECISIONS, TODO e sorgenti pertinenti. Poi leggere **questa nota**, `docs/REPOSITORY_FULL_REVIEW_2026-10-25.md`, `docs/NTC_CALIBRATION_ANALYSIS.md`, report S6C, report Step07 e PR #12.
2. Verificare PR #11 S7G e PR #12 S7U/CI, senza confondere codice fisicamente testato e build UI non testata.
3. Prima risposta: stato sintetico esatto e **una sola prossima azione** (preferibilmente ottenere/analizzare CSV S7G), non altra guida prolissa. Per software eseguire modifica su branch, CI e commit subito quando opportuno; aggiornare documenti di handoff.
4. Non descrivere promesse di lavori in background, non chiedere di ripetere prove fisiche già superate, non dire «aspetta che sparisca FAULT» o richiedere file ZIP da scaricare per OTA già configurata.
