# STEP09 — Preparazione tecnica al controllo reale aria SHT45 con protezione heater NTC

**Stato: PROTOCOLLO PROGETTUALE PREPARATO, NON ESEGUITO.** 27 ottobre 2026. Nessuna nuova prova fisica autorizzata; nessun cambio a soglia, firmware, fan, cablaggi o TCO in questo documento. Progettazione deve prendere come stato vero GitHub HEAD prima dell'attività.

## Introduzione obbligatoria (COSA / PERCHÉ / PRECEDENTE / FUTURO)

**COSA:** progettare, in un ramo di sviluppo distinto e inizialmente **non attuante**, il funzionamento normale del dryer: temperatura **dell'ARIA della camera** controllata dall'SHT45, riscaldatore comandato ON/OFF dal regolatore, NTC dell'heater usata come **protezione/limitatore indipendente**, e ventola con flusso previsto prima/durante/dopo il riscaldamento. Il firmware deve rimanere heater-locked mentre si svolge il lavoro software.

**PERCHÉ:** il prodotto finale deve asciugare il filamento a una temperatura aria scelta dall'utente, pur avendo una piastra molto più calda. Il limite sperimentale RAW NTC40kΩ era pensato per il collaudo di shutdown e fermava S7U quando l'aria era appena19,52°C; non è il setpoint dell'aria né un limite validato per l'esercizio OEM.

**COLLEGAMENTO ALLE PROVE PRECEDENTI:** eBOX Lite nominale (manuale OEM) associa **piastra 105/116/118°C** e **aria 40/50/55°C** (due zone e non necessariamente due anelli firmware OEM). **S6C** ha dato 6 impulsi fan100, Sanwa massimo53°C e aria finale23,98°C; **S7U** ha stoppato durante impulso3 con `NTC R LIMIT`, Sanwa43°C, aria19,52°C al cutoff e20,89°C a fine log, fan100 e cooldown riuscito. **STEP03** policy SHT45 da sola PASS; **STEP08** confronto G2 PASS analitico, replay integrato C++ G4 PASS in mock CI [run40476716231](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40476716231), ma safety hardware G3 ancora OPEN. **Non ripetere S3/S7U e non spacciare fixture host per firmware approvato.**

**COLLEGAMENTO AL FUTURO:** una volta verificato il percorso reale `SHT45 feedback→richiesta calore→interblocchi indipendenti→heater`, con limiti termici e hardware giustificati e revisione del controllo, si potrà formulare **una sola nuova prova fisica chiusa** per verificare il raggiungimento/stabilizzazione di un primo target aria e il limite del riscaldatore. Quella prova avrà un protocollo diverso da S7U, soglie approvate, Sanwa e CSV; serve **consenso esplicito** prima di qualunque HEAT reale.

## 1. Confini di architettura (fatti sorgente e opzioni da deliberare)

- **Confermato in codice ma non abilitato nel firmware S7U:** `HeaterControlPolicy.h` valuta `chamberTemperatureC` SHT45 e `targetChamberC` con isteresi per la richiesta HEAT ON/OFF. `heaterTemperatureC` è input di protezione locale, non un secondo setpoint uguale all'aria. L'attuale `SafetyController.h` ha percorso normativo disattivato dai flag `kHeaterOutputEnabled=false`, `kHeaterProfileApproved=false`, `kHeaterNtcModelApproved=false`; con S7U è attivo solo commissioning a impulsi.
- **Punto tecnico da risolvere prima dell'attuazione:** la policy attuale richiede heater `temperatureC` con `ntcThermallyCalibrated=true`; in esercizio S7U l'NTC è usata come resistenza raw. Non si può promuovere un fittizio `heaterTemperatureC=35°C` usato nel test STEP08 a temperatura calibrata reale. La soluzione (calibrazione con incertezze o protezione RAW disaccoppiata da modello termico) va **progettata, esaminata, testata senza HEAT fisico e approvata esplicitamente**.
- **Ventola:** per una prima caratterizzazione, la **proposta** è mantenerla comandata a 100% durante ON e in cooldown, così il confronto con S6C/S7U è più interpretabile. Non è una scelta definitiva della V1, non un fatto provato sul ventilatore OEM, né garanzia di flusso fisico; la UI prevede `FAN AUTO` ma la regolazione della velocità è ancora aperta. Non impostare subito PID sulla ventola solo per elevare l'aria, perché il flusso potrebbe scendere e riscaldare localmente la piastra.
- **Protezione indipendente:** secondo D029 il TCO aggiuntivo è opzionale come scelta di architettura, **ma l'attuale fusibile 105°C è fisicamente installato** e non va bypassato o sostituito solo perché potrebbe ostacolare i livelli OEM. Richiede verifica del suo corpo, del rating DC e del materiale/posizione prima di definire la prova calda. La soglia NTC40kΩ **non** è una soglia di temperatura della piastra.
- **Limiti del sistema:** `heater_command=0` prova solo output logico, senza misura corrente; `fan_percent=100` prova solo PWM, senza tach né misura portata. In assenza tach prevedere mitigazioni e strategia di guasto realistiche prima dell'uso autonomo.

## 2. Sequenza di lavoro SENZA accendere il dryer

1. **Review misure già disponibili**, SENZA ripetizione: S6C/S7U CSV raw e letture Sanwa, sette plateau originali aperti/fanOFF da `docs/NTC_CALIBRATION_ANALYSIS.md`, eventuale errore/lag di sensore; dichiarare cosa è valido per chiuso fanON e cosa non lo è. Distinguere grandezze e incertezze. La caratterizzazione non è sufficiente automaticamente a fissare un limite piastra.
2. **Definire la strategia di protezione locale della V1** come proposta non ancora approvata: gestione NTC aperta/corto/invalid/stale, guardia superiore reale (temp qualificata o raw calibrata in un range documentato), watchdog heater, eventuale falla MOSFET stuck ON, cool-down e fan non verificabile. Non copiare il limite 40kΩ da STEP07 come se fosse un requisito finale, né rimuoverlo oggi dai firmware di test.
3. **Review compatibilità termica del setup**: TCO105 in serie, eventuali temperature tipiche vicino al corpo, materiale pareti/film/cavi, geometria SHT45 in aria rappresentativa e minima ventilazione. Documentare cosa è noto dal repository e cosa richiederebbe una nuova misura mirata. Nessuna nuova ricerca di ricambi TCO di routine e nessuna modifica fisica senza decisione.
4. **Implementare su nuovo ramo SAFE/COLD non attuante la catena di controllo reale:** target SHT45 aria (per il primo livello OEM obiettivo finale di confronto **40°C**, non setpoint piastra), richiesta ON/OFF con isteresi, sorveglianza NTC indipendente, fail OFF, tempi massimi, ventola start/cooldown e fault latched. Richiedere revisione prima di abilitare qualunque flag heater; firmware fisicamente scaricato non cambia.
5. **Test software nuovi e CI:** distinguere dagli STEP03/08 già PASS, verificare vero coordinamento nel `SafetyController` e stati `DRYING→STOP/FAULT/COOLING` con guardia documentata, fault invalid/stale, fan fault, timer, boot/restart e nessuna attuazione hardware in compilazione COLD. Non limitarsi a mock `airWantsHeat && pulse`; testare il percorso reale di integrazione (ma uscita hardware LOW).
6. **Gate di progetto e piano primo HEAT:** dopo report G3 e CI, sottoporre **una sola futura prova con target aria controllato** (non altri 6 impulsi fissi), soglie e limite Sanwa approvati per l'assetto realmente presente, responsabile stop, log tempo/SHT45/NTC/HEATER/fan/Sanwa, marker at first ON, opening/stabilization events e cooldown, CSV/hashes. Solo dopo scelta esplicita, e mai senza sicurezza fisica, si abilita un branch test heater e si consegnano istruzioni operative.

## 3. Criteri di chiusura e stato

| Gate | Stato oggi | Pass richiesto |
| --- | --- | --- |
| S7U G1 | PASS limitato | RAW e verbale conservati |
| STEP08 G2 | PASS analitico | Confronto reale S6C/S7U archiviato |
| STEP08 G4 MOCK | PASS software limitato | [PR#13 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/13) e [CI PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40476716231), nessuna approvazione fisica |
| G3 termica/sicurezza | **OPEN** | Protezione NTC e margini di temperatura, fan, TCO/installazione/materiali esaminati |
| STEP09 controllo reale COLD | **NON ESEGUITO** | Codice in ramo isolato + test nuovo pathway + CI e review |
| Nuovo collaudo fisico feedback aria | **NON AUTORIZZATO / NESSUN PROFILO DEFINITO** | G3 + STEP09 software review + protocollo separato approvato |

**Nessuna nuova decisione D0xx è qui presa.** Le scelte finali di fan AUTO, NTC limite, tipo di regolazione e TCO rientrano in G3; `docs/DECISIONS.md` si aggiorna SOLO a scelta effettivamente approvata. Non interpretare questo documento come consenso dell'utente a cambiare componenti o scaldare la piastra.
