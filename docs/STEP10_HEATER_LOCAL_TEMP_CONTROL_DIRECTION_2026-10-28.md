# STEP10 — Correggere il controllo della zona riscaldante (NON aumentare i cicli)

> **Precisazione temporale ON20:** nel CSV l'ultimo OFF (ciclo19) avviene al `ms=569285`; `phase=6` compare a `ms=575851`, **6,566s dopo**. Il codice `HeaterStep09AirFeedback.h` richiede `minimumOffMs=12000`, quindi **non può aver legalmente iniziato il20° ON nei6,566s successivi** se era installata la build S9B180 prevista. Il20° ON è una osservazione manuale da preservare, ma **non corroborata** dai comandi registrati; possibilità di diversa interpretazione OLED/tempi, firmware differente o altro comportamento da verificare, non di impulso certo sfuggito. Recuperare `pulse_fault` e `firmware` da status; non inventare un ON non mostrato dal CSV. 

**28 ottobre 2026 — DIREZIONE TECNICA PROPOSTA DOPO S9B180 REALE, NON UN NUOVO TEST AUTORIZZATO.** S9A e S9B sono entrambe prove HEATER ON **ESEGUITE** e i CSV/Sanwa originali sono archiviati. Qui non vengono approvate né nuove temperature di piastra né modifiche ai valori di soglia. Non cambiare lo schema/TCO/cablaggi automaticamente.

## COSA / PERCHÉ / DA DOVE ARRIVIAMO / DOPO

- **COSA:** sostituire la logica sperimentale che interrompe l'HEATER a una resistenza NTC50kΩ e semplicemente accumula più secondi, con un controllo esplicito di **due variabili distinte**: l'**ARIA della camera** (SHT45, setpoint di essiccazione) e la **zona riscaldante** (NTC sulla piastra, obiettivo/limite locale qualificato, guasto indipendente).
- **PERCHÉ:** con la strategia attuale la piastra nel punto Sanwa si stabilizza attorno35–37°C, l'aria cresce sempre più lentamente e non raggiunge il modesto target25°C; ulteriori cicli sono una soluzione inefficiente e insufficiente per progettare un essiccatore con aria40–55°C.
- **PRECEDENTE:** S9A 120s budget, aria17,64→22,59°C al normale COOLING (fase4), Sanwa max38°C. S9B180 20 cicli osservati, 19 fronti ON/OFF registrati (~175,332s comando), aria16,77→23,54°C al passaggio `phase=6 ABORTED`, picco23,77°C in arresto, Sanwa nei cicli stabilizzati35/37°C, finale26°C. Ambedue con NTC verso50kΩ a ogni OFF. S7U a R_NTC~39,79kΩ aveva Sanwa43°C al FAULT; S6C aveva raggiunto Sanwa53°C con R_NTC fino30,32kΩ ma differente storia termica. Questi sono **valori Sanwa al punto di misura**, NON calibrazione del bead NTC o temperatura massima di tutti i materiali.
- **FUTURO:** prima stabilire il motivo della fase6 di S9B; poi disegnare e verificare al banco (host CI/ESP32) un controllo più efficace e guardato della temperatura zona heater, con un unico protocollo fisico HEATER ON distinto, Sanwa/CSV e consenso specifico al nuovo envelope. Nessun altro test fisico heater-OFF richiesto.

## Fatti tecnici che cambiano la strategia

| Elemento | Osservazione / conseguenza |
| --- | --- |
| Aria SHT45 S9B | min16,75°C, massimo23,77°C; nessun raggiungimento di25°C |
| NTC e piastra | il valore soft50kΩ interrompe cicli intorno7–8s, rearm65kΩ dopo raffreddamento |
| Sanwa | c6–c19 ON35°C/OFF37°C; questo limitatore mantiene il punto contatto relativamente freddo |
| Potenza e ventilazione | le prove registrano solo HEATER COMMAND e fan PWM100, non corrente reale né RPM |
| Trend dell'aria S9B | +1,25°C/min circa t100→160s, +0,47°C/min t400→460s: progressivo rallentamento |
| Fine prova S9B | **phase6 ABORTED** su159 campioni; **nessuna phase4**; motivo NON registrato. STOP software e guasto possono causare abort. |
| NTC calibrata? | NO: curva precedente ottenuta OPEN/fan OFF con Sanwa; non certifica T_NTC reale nello stato chiuso/fan100 |
| Interblocco fisico | TCO105 riportato in serie e non modificato; non qualificato come protezione fail-safe DC del sistema |

## Priorità 0 — leggere il motivo del precedente ABORT, SENZA riaccendere

Se il firmware S9B è ancora alimentato dallo stesso avvio e sulla medesima rete, aprire `http://<IP corrente del dryer>/api/status` e annotare **`phase`, `fault`, `pulse_fault`**, più `http://<IP corrente del dryer>/api/events` (evento `FAULT ntc=... pulse=...` o `FEEDBACK ABORTED`). Anche il display in FAULT mostra il testo di `pulse_fault`. In caso di reboot i dati potrebbero essere persi: dichiararlo e **NON rifare la prova per recuperarli**.

`HeaterCommissioning::Fault` nel codice ha enum `None=0; NtcInvalid=1; NtcStale=2; AirInvalid=3; AirStale=4; AirLimit=5; NtcResistanceLimit=6; FanUnavailable=7; FanLost=8; OverallTimeout=9; PulseWatchdog=10; ManualStop=11`. L'NTC elettrico separato può avere un guasto con `pulse_fault=0` ma `phase=6` dopo STOP; il fault del monitor deve essere letto dai log eventi. **Non inventare la causa** dalla riga `phase=6`. Il gap CSV4,066s prima del cambio fase è un indizio di possibile blocco del loop/sensori/web, NON prova di timeout/stale specifico. Non cambiare soglie termiche in presenza di guasti non diagnosticati.

## Priorità 1 — aumentare il livello termico locale in modo misurabile, non a caso

1. Formulare setpoint/limite **della piastra/zona heater** distinto dall'aria. Serve misurare e correlare NTC ohm con temperatura **nel punto rilevante** in condizioni CHIUSO/FAN ON; Sanwa misura un contatto vicino all'NTC ma non necessariamente il bead o un hotspot. Valutare le serie già disponibili S6C/S7U/S9A/S9B e l'incertezza di posizionamento/ritardo; **nessun ipotetico numero in°C diventa automaticamente una soglia sicura**.
2. Con NTC che diminuisce R all'aumentare T, **consentire un heater più caldo significherebbe ridurre la soglia SOFT di R**, oppure usare un modello T_NTC qualificato. Ma ciò richiede controllare ANCHE il margine del **FAULT indipendente a40kΩ** e l'inerzia dopo OFF, evitando di avvicinare troppo soft/hard e creare FAULT/rischio. Non modificare una soglia senza l'altra review, e non sopprimere il guasto elettrico del sensore.
3. Il **setpoint aria SHT45** continua a comandare la richiesta di calore: spegne all'obiettivo e riaccende sotto isteresi; la protezione locale NTC sovrascrive sempre la richiesta. Mantenere limiti temporali per singola accensione, watchdog hardware, fault latch, prerun e cooldown, max sessione, log integrale, controllo ventola, veto Sanwa, TCO senza bypass.
4. Prima di un ulteriore flash, correggere la telemetria: aggiungere `pulse_fault`/`ntc_fault` e la causa di arresto all'originale CSV, **registrare i fronti HEATER ON/OFF come eventi anche durante blocchi di campionamento** dove possibile, e differenziare chiaramente `FAULT / COOLING` da `COMPLETE / COOLING`. Host mock/regressioni e CI link ESP32 necessari.
5. Definire un successivo **unico esperimento HEATER ON reale** per controllare il nuovo envelope locale con Sanwa in continuo, stop manuale su temperatura reale, SHT45 aria e guardie. Scegliere gradino termico e durata solo dopo la review fisica TCO/cablaggi/materiali e l'analisi della finestra di sicurezza; non utilizzare OEM piastra105–118°C come valori automaticamente permessi.

## Decisioni non prese

- **NO nuovo limite piastra in°C** definito e NO nuova soglia NTC ohm firmata.
- **NO approvazione** a rendere permanente il controllo heater sul ramo `main`.
- **NO modifica** TCO105, ventola, cablaggi o potenza24V.
- **NO S9C fisico autorizzato**. L'approvazione precedente copriva esclusivamente il budget180s di S9B già eseguito.
- `docs/DECISIONS.md` resta invariato finché non sarà selezionata e accettata la soluzione termica corretta.

## Fonti

- [S9B180 verbale vero con 20 letture Sanwa e ABORT](evidence/thermal/2026-10-28-S9B180-PHYSICAL-SESSION.md)
- [S9B180 CSV originale](evidence/thermal/2026-10-28-S9B180-raw.csv)
- [S9A confronto](evidence/thermal/2026-10-27-S9A-analysis.md)
- [NTC analisi e limiti](NTC_CALIBRATION_ANALYSIS.md)
- [Controllo attuale, solo nel branch sperimentale](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step09b-air25-budget180-2026-10-27/firmware/HeaterStep09AirFeedback.h)
