# STEP08 — Prossimo test: fattibilità della regolazione aria e verifica delle protezioni

**STATO: PREPARATO, NON ESEGUITO.** Data piano: 27 ottobre 2026. **Tipo: analisi riproducibile di due CSV reali + simulazione automatizzata del futuro controllo, senza prova fisica e senza richiedere all'utente di rifare test heater-OFF.** Non abilita alcun riscaldamento né cambia la soglia NTC.

## Introduzione obbligatoria: cosa, perché, collegamento al progetto

**Cosa facciamo:** verifichiamo dai log delle prove S6C e S7U se il profilo termico e la guardia NTC permettono di avvicinarsi al futuro obiettivo di **40 °C dell'aria**; separatamente proviamo in simulazione la logica di controllo aria e le reazioni ai guasti, senza comandare l'heater reale.

**Perché:** S7U ha funzionato come test di cutoff: il firmware si è fermato durante il terzo impulso per `NTC R LIMIT`, con Sanwa **43 °C** e camera SHT45 a soli **19,52 °C**. La camera ha raggiunto solo **20,89 °C** alla fine del CSV. Non avrebbe senso ripetere identici impulsi o innalzare alla cieca il limite 40 kΩ.

**Collegamento precedente:** S6C aveva sei impulsi completi con ventola al 100%, NTC min **30,32 kΩ**, Sanwa massimo **53 °C** e SHT45 aria **22,57 °C** all'ultimo OFF, **23,98 °C** a fine cooling. S7U ha ridotto l'esposizione termica e verificato il cutoff anticipato, ma non ha avvicinato la camera a 40 °C.

**Collegamento futuro:** il risultato STEP08 deve indicare quali condizioni mancano per progettare la regolazione aria della V1, quali protezioni devono essere indipendenti dal normale controllo e quali domande richiederebbero un **successivo test fisico nuovo e giustificato**, non una ripetizione di S7U.

## Fonti obbligatorie e versioni

1. [S7U verbale della prova reale](evidence/thermal/2026-10-26-S7U-RETEST-SESSION.md).
2. [S7U CSV originale](evidence/thermal/2026-10-26-S7U-retest-raw.csv), 717 campioni, SHA256 `c417313414eede75f2269dbfb8717ac6d8246625172e315a007b8056def834ae`.
3. [S7U analisi numerica](evidence/thermal/2026-10-26-S7U-retest-analysis.md).
4. [S6C CSV originale](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step07-ntc-guard-verified-history-2026-10-25/docs/evidence/thermal/2026-10-25-S6C-fan100.csv), 652 campioni; 6 impulsi ON da circa 20 s.
5. Codice reale dei due profili, `firmware/HeaterStep07GuardTrial.h`, `HeaterPulseCommissioning.h`, `SafetyController.h`, `HeaterControlPolicy.h` (o reale nome del modulo da verificare), `AppConfig.h`, `ThermalLog.h`. Branch S7U: `fix/step07-clear-test-feedback-2026-10-26`. Non assumere che `main` contenga firmware HEAT approvato.
6. `docs/DECISIONS.md`, specialmente D029: TCO aggiunto opzionale come requisito progettuale, **ma TCO 105 °C attualmente in serie e da non bypassare implicitamente**.

Le sorgenti reali prevalgono su una descrizione imprecisa del documento.

## Sequenza del TEST08 (nessuna azione manuale sull'hardware)

**A — confronto dati:** analizzare i due CSV senza modificarli. Calcolare durata effettiva dei comandi ON, NTC(Ω) inizio/fine e minimo, SHT45 aria/RH, incremento aria durante heater ON e nei 90 s di cooling, recupero NTC nei gap. Confrontare la relazione *nel singolo setup* con le letture Sanwa storiche. Tracciare separatamente `heater_command`, `phase`, NTC, aria; non inferire correnti o RPM dai comandi.

**B — rischi e limiti:** distinguere `R_NTC <=40kΩ` sperimentale da temperatura massima fisica della piastra e da regolazione aria a 40 °C. Il fatto che la NTC sia salita/scenda rapidamente e che l'aria cresca lentamente NON dimostra che il limite vada abbassato; richiede un criterio tecnico sui possibili hotspot, il contatto Sanwa, TCO montato e materiali. Analizzare fan senza tachimetro, sensori invalid/stale, STOP/FAULT, MOSFET stuck-ON, cooldown e protezioni indipendenti. **Nessuna selezione di nuovo cutoff o bypass del FAULT senza revisione esplicita.**

**C — simulazione funzionale controllore (sviluppo software successivo, senza hardware):** su branch separato derivato dal firmware COLD operativo, costruire scenari host che simulino SHT45 aria che sale verso 40 °C, guardia piastra che interviene per prima, NTC e SHT45 invalid/stale, richiesta heater con fan non disponibile, STOP durante ON, timer massimo, no-restart dopo FAULT e cool-down. Le sole richieste heater sono segnali **simulati**, non output sulla PCB. Dimostrare il comportamento normale della regolazione (richiesta ON/OFF per target aria) e il comportamento di sicurezza (latch FAULT) come concetti distinti. Non ridefinire le decisioni di progetto in silenzio.

**D — evidenze e commit:** salvare codice/test/risultati reali dello STEP08 e un rapporto `docs/evidence/thermal/2026-10-27-STEP08-analysis-and-simulation.md` con comandi di esecuzione, versioni toolchain, risultati PASS/FAIL, CSV sorgenti, scenario per scenario, limiti e commit. Aggiornare README, PROJECT_STATE, TODO e ledger prima del passaggio. Se i test non sono stati realmente eseguiti, scrivere `NON ESEGUITO`, non PASS.

## Gate misurabili

- **G2 PASS** solo dopo confronto numerico riproducibile S6C/S7U e spiegazione esplicita del perché 40 kΩ è solo guardia sperimentale, non setpoint; criteri di protezione rimasti aperti elencati.
- **G3 REVIEW** non passa con un CSV: richiede documentare margini della superficie/NTC, fan reale non misurato, comportamento bloccato-ON, TCO e materiali. Può restare OPEN motivatamente.
- **G4 SOFTWARE PASS** solo se i test automatici documentano risposta a target 40°C, limitazioni di piastra, sensori fault, STOP/cooldown, assenza di heater abilitato sulla board, e CI/build PASS. **Il test software non prova che il dryer possa fisicamente raggiungere 40°C**.
- **GATE verso una NUOVA prova fisica:** da proporre soltanto quando siano chiari obiettivo misurabile, profilazione/limiti rivisti e motivati, condizioni materiali/sensori, intervento manuale e archivio prove. Richiede decisione esplicita dell'utente. Niente ripetizione S7U, niente prove heater-OFF già concluse.

## Istruzioni immediate per l'utente

**Per ora nessuna manovra sul dryer. Non serve caricare firmware, premere ON/OFF o produrre un nuovo CSV.** Lo STEP08 inizia riutilizzando i dati REALI già archiviati. Terminata l'analisi e la simulazione documentata, verrà presentato un protocollo fisico separato SOLO se fornisce un'informazione nuova e giustifica il rischio di un'altra accensione.

**Regola di introduzione per ogni successivo test:** aprire sempre con 4 punti in linguaggio semplice: **COSA facciamo — PERCHÉ — LEGAME con la prova precedente — QUALE prova futura sblocca**. Poi elencare manovre concrete, parametri, limiti, letture, arresto, file raw e gate di chiusura. Questo è un requisito di handoff, non un formato facoltativo.
