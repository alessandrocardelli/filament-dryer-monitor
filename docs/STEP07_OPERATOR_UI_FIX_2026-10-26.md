# Step07: prima prova S7G ed errore di interfaccia operatore — 26 ottobre 2026

## Prova fisica già eseguita (NON ripetere per testare il display)

- Firmware effettivamente provato: `HOT-251025-S7G`, branch `experiment/step07-ntc-guard-verified-history-2026-10-25`, sorgente `ab17acf4e816a32cc58a4e1d6735ad0c1cb0cdf4`.
- Sanwa vicino al riscaldatore: **17°C prima dell'impulso 1, 36°C prima dell'impulso 3, 43°C quando è comparso FAULT**. Non sono state fornite letture sincronizzate dopo l'arresto o il CSV della stessa prova: **massimo Sanwa successivo all'OFF sconosciuto**. Il FAULT è coerente con la nuova soglia NTC R≤40kΩ, ma tipo e timestamp richiedono verifica del CSV; non dichiarare che il trip è stato fisicamente qualificato da sole letture Sanwa.
- L'utente non sapeva come individuare il momento della disattivazione HEATER, perché il display non lo segnalava chiaramente; trovava il FAULT persistente e il buzzer perpetuo; il significato della richiesta di *temperatura massima dopo spegnimento* non era stato spiegato.
- Significato corretto: dal momento in cui **l'heater viene comandato OFF definitivamente** (nel test S7G, al FAULT) osservare il Sanwa per l'inerzia/ritardo e annotare il valore più alto raggiunto **dopo quel momento**, anche se si verifica diversi secondi dopo. Non è la temperatura «alla fine del raffreddamento», e non la stessa cosa del dato di 43°C al FAULT. Il Sanwa è esterno, l'ESP32 **non può memorizzare automaticamente questo picco**.
- Il comportamento delle allarmi S7G era progettato: `Buzzer::startFaultAlarm(nowMs)` con `repeat=true`; nello stato `AppState::Fault` la pressione BREVE MODE chiamava `muteFault()`, ma mancava l'informazione usabile sul display. Il FAULT latched **rimane anche quando la ventola si è fermata**, per sicurezza. L'utente non doveva attendere la sparizione del FAULT o il silenziamento automatico per cercare il CSV.
- Il file `last-test.csv` si salva **dopo il cooldown**; attendere che la ventola si fermi e il salvataggio venga confermato. Non eseguire altre prove solo per questo.

## Revisione software preparata in PR #12

Branch `fix/step07-clear-test-feedback-2026-10-26`, nuova identità **`HOT-261026-S7U`** / OLED `H26-S7U` per evitare confusione con S7G già caricato.

- Sul display durante i segmenti, `HEATER ON n/6` e `HEATER OFF` grandi, senza il generico `DRYING TEST` e senza confondere il SET aria con un limite piastra; viene mostrato `NTC TRIP 40k FAN 100%`.
- Sul display FAULT compare **`FAULT / HEATER OFF`**, causa breve (`NTC R LIMIT` quando pertinente), **`SANWA: WATCH PEAK`**, countdown dell'effettiva finestra di ventilazione residuale (circa90s dal più recente HEATER ON), e in seguito `COOL DONE / CSV READY` **solo dopo il salvataggio di una nuova sessione**. Se fallisce, `CSV SAVE FAILED`, non dichiarare CSV pronto.
- Il beep FAULT in modalità commissioning si riproduce **una volta sola**, poi rimane silenzioso, ma **FAULT NON viene cancellato**. MODE lo può silenziare prima. La decisione D022 della ripetizione nella modalità normale è preservata.
- Scritte SHT45 limite `AIR LIMIT 40C` (precedentemente etichetta erronea35C) allineate al parametro esatto.
- Parametri firmware termici, guardia NTC40k, hardware watchdog20,5s, fan100, SHT45 aria40C, impulsi max6x20s, log RAM e tutti i fault hardware **invariati**: revisione solo UI, breve allarme e funzione **read-only** per il countdown dal recente HEATER ON.
- Test automatizzati: vera `Buzzer.h` su simulatore LEDC; controlli dell'output/test UI; suite completa storica GPIO; replay CSV S6C; compilazione ESP32, con CI che blocca ogni modifica alle guardie.

**Non caricare né lanciare questa versione per ripetere S7G.** Dopo verifica del CSV S7G esistente, decidere il successivo obiettivo del controllo aria; la revisione UI rimarrà disponibile nell'esperimento per una sessione futura **solo se tecnicamente necessaria e autorizzata**.

## Procedura di osservazione semplice per future prove

L'operatore non deve stimare temperature dal web, che non è aggiornato durante HEATER ON. L'OLED deve mostrare lo stato effettivo del comando riscaldatore. Quando passa a `FAULT / HEATER OFF` (oppure `HEATER OFF` definitivo), prendere nota del Sanwa in quel momento, seguirne l'andamento durante la ventilazione residua e registrare **solo il valore maggiore osservato successivamente**. La lettura Sanwa **non è sincronizzata automaticamente** col CSV: registrare un video che inquadri simultaneamente Sanwa e OLED è una possibile semplificazione, non un requisito per ripetere il test.

Il test fisico S7G ha evidenziato un difetto di comunicazione più che una nuova esigenza di riscaldamento: priorità a qualità dei dati acquisiti e controlli automatici, non a un'altra accensione.
