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

## Precisazione dell'operatore: NON normale pausa ciclica

**Osservazione successiva, da conservare:** «ho visto 18 cicli, heater off dura qualche secondo, qua non finiva più!». Il dettaglio nuovo è la **differenza qualitativa di durata** fra le prime18 pause OFF brevi e la fase OFF dopo il19° ciclo, percepita come persistente/anomala e causa dello spegnimento volontario. **Non caratterizzare l'episodio come una breve pausa ordinaria né affermare che il fault non esistesse.** Tempo OFF finale non quantificato; nessun nuovo CSV STEP10A è stato salvato. La ventola continuava fisicamente a girare.

**Deducibile dal codice, NON osservato:** il ramo `Phase::Gap` non ha un timeout dedicato che produca un'indicazione di attesa esplicita, solo il timeout complessivo di720s; ricomincia a scaldare soltanto con OFF>=12s, R_NTC>=60kΩ e AIR<=24°C. Un'aria che resti sopra24°C (per esempio dopo il primo raggiungimento del target25°C) può prolungare fortemente OFF; non sappiamo se fosse davvero questa la causa al19° ciclo. L'OLED emette semplicemente `HEATER OFF` durante il gap, senza distinguere NTC non recuperato, aria sopra riarmo, o timer. **Priorità software: rendere leggibile la ragione di OFF e la progressione del timeout/uscita e completare il salvataggio sicuro prima di un ulteriore test.**

## Lettura del codice reale STEP10A — possibilità, non diagnosi

La fonte è `firmware/HeaterStep09AirFeedback.h` sul branch sperimentale STEP10A. Dopo HEATER OFF in `Phase::Gap` (fase3) la ripresa richiede **tutte**: almeno12s di OFF, R_NTC>=60kΩ, aria<=24°C. Qualora l'aria avesse raggiunto25°C oppure l'NTC non fosse risalito a60kΩ, il comando può legittimamente rimanere OFF anche a lungo senza fault, fino al timeout della sessione.

Se è stato raggiunto `maximumTotalHeatMs=300000` (300s di comando ON cumulativo) **oppure** `maximumRunMs=720000` (12min), il controller usa `Phase::Cooldown=4`, non riaccende e poi completa. Un fault sensore/fan/guardia può invece portare a `Phase::Aborted=6` e `pulse_fault>0`. L'utente non ha fornito `phase`/dati per discriminare tali casi; **19 cicli non identificano una condizione precisa**. Il contatore `pulses` aumenta alla fine di ciascuna finestra, non equivale alla durata cumulativa ON.

`firmware/firmware.ino` salva `/last-test.csv` con `thermalLog.saveWhenSafe()` **solo dopo** il ciclo e il raffreddamento, quando `loggingSession` viene chiusa e `safety.coolingRequired` diventa falso. Se l'alimentazione viene rimossa prima, i nuovi campioni in RAM si perdono; il precedente `/last-test.csv` può rimanere su LittleFS e non prova che la nuova sessione sia stata salvata. Da non confondere con il vecchio CSV S9B180 di10 colonne.

### Unico recupero non termico, alla prossima accensione sicura

Non ripetere né riavviare il test HEATER. Dopo raffreddamento e solo se si può riaccendere **senza iniziare un ciclo**, aprire `http://filament-dryer.local/api/status` per identificare firmware e `saved_log`, e `http://filament-dryer.local/api/last-test.csv` per vedere se il file persistente è quello nuovo. Un CSV STEP10A ha **15 colonne**, con `pulse_fault,ntc_fault,air_age_ms,ntc_age_ms,loop_gap_ms` in coda; l'originale S9B ne ha10. Verificare anche `ms` iniziale/finale e non scambiare `saved_log=true` per prova che sia stato salvato STEP10A. `/api/events` dopo power-cycle contiene eventi DEL NUOVO AVVIO, non i vecchi eventi volatili.

Se il file non è nuovo, segnare dati persi, **non replicare la sessione per recuperarli**. Analizzare l'architettura di salvataggio/stato del termostato e progettare una modifica con test software prima di proporre un'altra sessione fisica. Nessuna soglia o cablaggio modificato in questo record. TCO105 invariato, mai bypass.

## Supplemento probatorio — 29/10/2026, file recuperato e ventola

**Dati forniti dall'utente dopo lo spegnimento:**

- Conferma testuale: «La ventola stava ancora girando, ma non usciva da heater off.» Il funzionamento fisico della VENTOLA al momento della decisione di spegnere e' una osservazione dell'operatore; non prova la fase 3/4/6 o che essa girasse durante ogni impulso.
- URL condiviso: `http://filament-dryer.local/api/status#:~:text=...` include solo frammenti di testo (`{"firmware":"HOT-261028` e `"ide_ota_ready":true}`), **NON un JSON completo**. Non ricostruire phase, pulse_fault, età sensori o firmware completo da questo frammento. L'URL .local non e' raggiungibile dall'assistente. Se il dispositivo e' stato power-cycled, `/api/status` attuale non contiene piu' lo stato volatile della prova.
- **File realmente allegato:** `filament-last-test (5).csv`, 44.461 byte, **1.067 campioni**, 10 colonne `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent`. **SHA256 `1e17e6e131ce7cf9074bfe8d23d31f5e8ea1a605833ff36c0a21fe6568314beb`**: IDENTICO al CSV S9B180 originale gia' archivato `docs/evidence/thermal/2026-10-28-S9B180-raw.csv` (Git blob SHA `db944d63d5a97ad4a3d39bdae4753a9d87822c97`). Nessun file raw STEP10A a15 colonne e' stato acquisito. **NON duplicare o rinominare il vecchio CSV come nuova prova.**
- **Conseguenza verificata:** il file `/last-test.csv` letto dopo lo spegnimento corrisponde a S9B180, non STEP10A. Il nuovo CSV **non risulta persistito** con quel percorso; i nuovi campioni erano presumibilmente in RAM ed e' possibile siano andati persi al power-cycle. Non dimostra che nessun campione STEP10A fosse stato acquisito prima dello spegnimento.

**Interpretazione specifica del display (verifica sorgente `firmware/Ui.h` STEP10A):** `HEATER OFF` testuale su OLED DURANTE `Drying` mappa `Phase::Gap=3`; `Phase::Cooldown=4` mostra `COOLING`, mentre `Phase::Aborted=6` non viene etichettata appropriatamente come fault nel ramo Drying. Se l'utente stava effettivamente leggendo OLED non aggiornato e il firmware era quello di STEP10A, la fase 3 e' **piu' compatibile** con l'osservazione; non e' prova definitiva dell'assenza di fault o di fine budget. `Phase::Gap` riavvia HEATER solo con almeno12s OFF, NTC>=60kΩ E aria<=24°C. In particolare con aria nell'intervallo 24–25°C e piastra gia' recuperata, il firmware **resta in HEATER OFF di proposito**, senza spiegazione sull'OLED. `firmware.ino` configura trial `durationMinutes=15` mentre il controller STEP10A usa timeout interno720s; l'OLED test rimane etichettato `STEP09B` e `TIME 00:10` anche sul candidato STEP10A. Sono difetti di feedback/UI indipendenti dalla causa non osservata del fermo.

**Stato finale:** prova fisica STEP10A-context riferita ma non qualificata, ventola riferita in rotazione, stop dall'operatore, **traccia nuova non recuperata**. Non chiedere di ripetere l'esperimento. Unica prossima attivita': correggere in software la chiarezza di OFF/WAIT/COOLDOWN e il percorso di persistenza della sessione *prima* di qualsiasi nuovo HEATER ON; nessun cambio soglie o TCO. 


## Nuove misure ricordate dall'operatore — 29/10/2026

L'utente aggiunge: «secondo me la temperatura sull'heater è ancora troppo bassa, arriva al massimo a 38 gradi e quando ho spento mi pare che la camera fosse a 23,7 o giù di li». **Conservare l'incertezza:** Sanwa **sulla PIASTRA** massimo circa38°C (ricordo), camera SHT45 circa23,7°C quando spento (ricordo, non log). Le due temperature sono da sorgenti e punti diversi e **NON sono righe CSV del nuovo test**; non usarle come prova del comportamento del controller in corrispondenza della 19ª pausa. Il fatto di 18 pause brevi e 19ª persistente resta l'osservazione principale. Il nuovo CSV STEP10A non è stato persistito; non rifare la prova per acquisirlo.

**Difetto software dimostrato sul sorgente STEP10A (non necessariamente unica causa fisica):** l'ultima condizione di riavvio in `HeaterStep09AirFeedback::Controller::update` richiede aria <=24°C dopo **ogni** pausa, anche se la camera non ha mai raggiunto25°C. Una correzione della latch della domanda aria, indipendente dal recupero NTC, è stata implementata nella linea sperimentale STEP10B; non è una nuova calibrazione della piastra né autorizzazione a bypassare il limite hard NTC40kΩ.
