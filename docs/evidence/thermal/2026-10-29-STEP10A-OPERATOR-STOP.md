# STEP10A — osservazione operatore: HEATER OFF persistente dopo circa 19 cicli

**Data di ricezione dell'osservazione:** 2026-10-29 (Europe/Rome). **Ora e data effettive di avvio/fine della prova:** non comunicate. **Stato: SESSIONE INTERROTTA DALL'OPERATORE; DATI INCOMPLETI, NESSUNA CONCLUSIONE SULLA CAUSA**. Non confondere con la prova S9B180 del 28 ottobre, con 19 fronti ON e causa AirStale già accertata.

## Osservazione originale (testuale, non reinterpretata)

> «niente ho spento. perchè arrivati al ciclo 19 va in heater off e ci rimane. Non so neanche se ha fatto un csv...»

La segnalazione arriva dopo il firmware/protocollo STEP10A. **È verosimile che l'utente riferisca il nuovo STEP10A, ma l'identità realmente installata non è stata confermata da OLED o /api/status in questo messaggio**. Non registrare un flash, una temperatura di piastra, un raggiungimento dei25°C o una phase4/6 come fatti. `HEATER OFF` è l'indicazione riferita dall'operatore; l'uscita GPIO19 non è stata misurata indipendentemente.

### Fatti acquisiti e dati mancanti

| Elemento | Evidenza |
| --- | --- |
| Stato dopo circa ciclo19 | l'operatore vede `HEATER OFF` che permane |
| Intervento | l'operatore ha spento il dispositivo; tipo di comando/punto di sezionamento e istante rispetto a cooling non specificati |
| Contatore cicli | «ciclo 19» riferito; nessun evento temporizzato o trace disponibile |
| Firmware ID esatto | NON COMUNICATO per questa sessione; candidato STEP10A `HOT-261028-S10A25` soltanto da contesto |
| `phase`, `pulse_fault`, `ntc_fault`, `ms` e log eventi | NON ACQUISITI |
| SHT45 air/RH al ciclo19 | NON ACQUISITI; nessuna prova che25°C siano stati raggiunti |
| NTC kΩ, temperatura Sanwa sulla PIASTRA, Tmax post-OFF | NON ACQUISITI, non inventare |
| Ventola reale e stato di raffreddamento | NON COMUNICATI |
| CSV nuovo | ESISTENZA NON VERIFICATA; nessun CSV allegato o acquisito |

## Lettura del codice reale STEP10A — possibilità, non diagnosi

La fonte è `firmware/HeaterStep09AirFeedback.h` sul branch sperimentale STEP10A. Dopo HEATER OFF in `Phase::Gap` (fase3) la ripresa richiede **tutte**: almeno12s di OFF, R_NTC>=60kΩ, aria<=24°C. Qualora l'aria avesse raggiunto25°C oppure l'NTC non fosse risalito a60kΩ, il comando può legittimamente rimanere OFF anche a lungo senza fault, fino al timeout della sessione.

Se è stato raggiunto `maximumTotalHeatMs=300000` (300s di comando ON cumulativo) **oppure** `maximumRunMs=720000` (12min), il controller usa `Phase::Cooldown=4`, non riaccende e poi completa. Un fault sensore/fan/guardia può invece portare a `Phase::Aborted=6` e `pulse_fault>0`. L'utente non ha fornito `phase`/dati per discriminare tali casi; **19 cicli non identificano una condizione precisa**. Il contatore `pulses` aumenta alla fine di ciascuna finestra, non equivale alla durata cumulativa ON.

`firmware/firmware.ino` salva `/last-test.csv` con `thermalLog.saveWhenSafe()` **solo dopo** il ciclo e il raffreddamento, quando `loggingSession` viene chiusa e `safety.coolingRequired` diventa falso. Se l'alimentazione viene rimossa prima, i nuovi campioni in RAM si perdono; il precedente `/last-test.csv` può rimanere su LittleFS e non prova che la nuova sessione sia stata salvata. Da non confondere con il vecchio CSV S9B180 di10 colonne.

### Unico recupero non termico, alla prossima accensione sicura

Non ripetere né riavviare il test HEATER. Dopo raffreddamento e solo se si può riaccendere **senza iniziare un ciclo**, aprire `http://filament-dryer.local/api/status` per identificare firmware e `saved_log`, e `http://filament-dryer.local/api/last-test.csv` per vedere se il file persistente è quello nuovo. Un CSV STEP10A ha **15 colonne**, con `pulse_fault,ntc_fault,air_age_ms,ntc_age_ms,loop_gap_ms` in coda; l'originale S9B ne ha10. Verificare anche `ms` iniziale/finale e non scambiare `saved_log=true` per prova che sia stato salvato STEP10A. `/api/events` dopo power-cycle contiene eventi DEL NUOVO AVVIO, non i vecchi eventi volatili.

Se il file non è nuovo, segnare dati persi, **non replicare la sessione per recuperarli**. Analizzare l'architettura di salvataggio/stato del termostato e progettare una modifica con test software prima di proporre un'altra sessione fisica. Nessuna soglia o cablaggio modificato in questo record. TCO105 invariato, mai bypass.
