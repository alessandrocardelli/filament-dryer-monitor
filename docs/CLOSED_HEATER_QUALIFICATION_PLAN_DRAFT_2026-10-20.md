# Piano PROPOSTO — qualifica termica eBOX Lite chiuso, ventola ON, riscaldatore ON

**Data:** 2026-10-20  
**Stato:** BOZZA PER DISCUSSIONE, NON AUTORIZZAZIONE AD ALIMENTARE IL RISCALDATORE.  
**Branch:** `plan/closed-heater-qualification-2026-10-20`, basato su `main` commit `87d24c5f618822b81496f25b4f16d3d7736e08af`.  
**Non modifica:** `firmware/`, KiCad, `docs/DECISIONS.md`, né i risultati collaudati.

## Scopo esatto

Portare il retrofit eSUN **eBOX Lite** al funzionamento finale rappresentativo in **configurazione chiusa**, ventola operativa e riscaldatore controllato, confrontando la risposta con i riferimenti OEM del manuale eSUN:

| Livello OEM | Temperatura nominale piastra | Temperatura nominale aria |
| --- | ---: | ---: |
| 1 | 105 °C | 40 °C |
| 2 | 116 °C | 50 °C |
| 3 | 118 °C | 55 °C |

I valori OEM sono **riferimenti di confronto**, misurati in punti differenti (condizioni riportate nel repository: ambiente 20 °C, RH 55%). **118 °C della piastra NON è un setpoint da digitare nell'NTC**. Occorre verificare la temperatura di superficie reale con termocoppia Sanwa, la temperatura/umidità dell'aria con SHT45 e la resistenza/temperatura stimata del sensore heater NTC. Può essere necessario fissare limiti finali diversi in funzione della sicurezza reale.

Fonte del manuale annotata in `docs/PROJECT_STATE.md`: https://www.esun3d.com/uploads/eBOX-Lite-Manual.pdf.

## Collaudi precedenti da NON ripetere

- 2026-10-08: Q4/J5 OFF/ON a vuoto; ventola Q5/J6 accensione/PWM dopo sostituzione iniziale Q5.
- 2026-10-14: heater reale 24 V, 1.59 A, prove 1/5/10 s con spegnimento; NTC sensibile.
- 2026-10-15/17: sette punti stabilizzati resistenza NTC vs Sanwa da 30 a 85 °C rilevati in **scatola APERTA su entrambi i lati, ventola SPENTA**. Ultima prova: NTC vecchio sketch 90 °C, Sanwa 85 °C, R = 7.12 kΩ, hold 120 s; preheat circa 411 s. Nessuna prova dimostra comportamento chiuso o piastra a 118 °C.
- 2026-10-20: saldatura U4 pin fisico 27/GPIO16 riparata; integrato fan 100%/1 s poi 40%, run 2–3 min, stop, power cycle in STANDBY, OLED e dati SHT45 (19.4 °C/69.7% RH), NTC diagnostico (15.5 °C) osservati. J5 heater è **fisicamente collegato**, ma firmware **`SafetyController.h` hard-locks GPIO19 OFF**. Sono passati i test funzionali già documentati; le fotografie non certificano la chiusura finale dei coperchi.

## Blocchi tecnici ancora reali

**B1 — Geometria, sensori, cablaggi e materiale.** Verificare che entrambi i coperchi si chiudano senza pizzicare alcun cavo né interferire con ventola/bobina; fissaggio ed esposizione dell'SHT45 in aria di camera, lontano da radiazione diretta della piastra e da getto localizzato; accesso termocoppia Sanwa con passaggio protetto che non alteri sensibilmente la chiusura; cavo NTC/J7, heater/J5, fan/J6, connessioni crimp e distanze dalla piastra. Identificare filamento stampato e rating termico dell'involucro e dei cablaggi. Verificare accessibilità stacco alimentazione 24 V. Le foto della UI non documentano ancora questi criteri.

**B2 — Protezione termica indipendente.** L'attuale TCO monouso marcato **105 °C / 15 A / 250 V**, installato in serie a HEATER+ e aderente al flex heater con Kapton (ultimo ripristino riferito 2026-10-16), **può intervenire prima del valore OEM 118 °C della piastra**. Verificare posizione effettiva, effettiva temperatura del suo corpo e documentazione/rating (non basta la marcatura 105 °C), limiti del sistema e eventuale protezione originale OEM. Proporre, valutare e concordare una scelta di cutoff indipendente idonea all'inviluppo operativo **senza bypass e senza sostituzione arbitraria**. Qualsiasi eventuale modifica della protezione deve essere deliberata e documentata in `docs/DECISIONS.md` prima dell'esecuzione. Non tentare automaticamente 118 °C con TCO attuale.

**B3 — Sensing heater/controllo di sicurezza.** L'identità del termistore e la calibrazione assoluta sopra il campo storico non sono stabilite; `docs/NTC_CALIBRATION_ANALYSIS.md` registra un freddo 122.3 kΩ / ~20 °C incompatibile con parte della curva antica e una **ipotesi**, non prova, di 100k/B3950. La tabella NTC-vs-Sanwa aperta rappresenta due punti fisici differenti. Prima del nuovo controllo definire letture raw R_NTC, soglie prudenti e validazione di errore aperto/corto/stale; SHT45 è feedback camera, NTC è monitor heater, Sanwa è misura indipendente. Non trasferire direttamente la curva di comparazione a limiti di sicurezza.

**B4 — Software prova sicura.** `main/firmware/SafetyController.h` tiene heater OFF sempre; la UI `SET 54 °C` è soltanto dimostrativa. Il prossimo programma di prova ON, da sottoporre a review separata, deve conservare boot e reset heater-OFF, comando esplicito di abilitazione sorvegliata, ventola avviata prima di erogare calore, limiti energetici/temporali conservativi per ogni ciclo, spegnimento su dati invalidi/non plausibili/mancanti, controllo termico heater e aria con limiti giustificati, log seriale (tempo, potenza/duty, R_NTC e temperature approssimate, SHT45 T/RH, current se misurabile da banco, cause arresto). L'assenza di **tachimetro ventola** sulla PCB significa che un PWM impostato non dimostra rotazione: non dichiarare un interlock airflow reale sulla sola base del comando PWM. I test di guasto e di reset vanno fatti senza affidarsi al firmware come sola protezione. Archiviare esattamente il sorgente e la revisione flash testata sul repository **prima del collaudo**. Non riutilizzare un vecchio sketch heater temporaneo senza source/limiti verificati.

## Sequenza di lavoro proposta — con decisioni per ogni gate

| Fase | Operazione | Evidenza richiesta e condizione per procedere |
| --- | --- | --- |
| **A — Chiusura meccanica + fan NO HEAT** | Predisporre SHT45, Sanwa e cavi; verificare entrambi i coperchi chiusi, airflow/rumore, temperatura ambiente iniziale, contatto protezione. Il fan-only già superato non va ripetuto a banco: questa è una **diversa geometria chiusa**, da controllare una sola volta. Se occorre una garanzia di zero heater, scollegare J5 fisicamente per questa fase. | Niente cablaggi pizzicati; Sanwa ben fissato; fan stabile; materiali e sensore posizione documentati; no heater energizzato. |
| **B — Decisione TCO e sicurezza** | Analizzare/risolvere B2 e B3; decidere rating e posizionamento protettivo prima di una prova a pieno carico chiusa. | Decisione esplicita con evidenze, no bypass; limiti misurabili e arresto indipendente; fail-safe review. |
| **C — Sviluppo firmware prova ON** | Su branch, creare e verificare codice limitato/sorvegliato, con fan ON, acquisizione NTC R, SHT45, arresti e log, heater OFF per default. Test a freddo e software-fault, poi eventuale autorizzazione distinta alla prova sotto carico. | Sorgente esatto commit, configurazione programmata, stati sicuri (avvio, stop, fault, mancanza sensori). |
| **D — Prima prova termica chiusa + fan ON** | Solo dopo A/B/C: prova supervisionata iniziando da un target **aria moderato sotto 40 °C** determinato dai limiti approvati (non 90/118 NTC); log delle tre temperature: aria, piastra Sanwa, NTC/resistenza, più duty e corrente, temperature critiche cablaggi/materiali/TCO. Controllo rapido di rise/switch-off/cooldown. | Assenza di anomalie o interventi termici, chiusura integra, correlazione dei tre sensori sufficientemente comprensibile; confronto con prove OEM, senza assumere equivalenza di siti. |
| **E — Livelli OEM progressivi** | Solo se fase D passa e protezioni/limiti lo consentono: caratterizzare **40 → 50 → 55 °C aria** per step approvati, registrando la piastra misurata e verificando avvicinamento ai **105 → 116 → 118 °C OEM nominali** come *risultati*, non comandi diretti all'NTC. Raffreddamento/FAULT/stop controllati. | Temperature e stabilità reali, uniformità operativa e sicurezza con fan ON. Non forzare 118 se componenti/protezione o gradienti impongono un limite inferiore. |
| **F — Integrazione normale V1** | Solo dopo qualifica D/E, integrare in UI il vero controllo heater chiuso, setpoint camera/tempo, FAN AUTO/cooldown, fault, persist settings senza autorestart, watchdog, e prove di sicurezza/soak. | Funzione dryer sicura e ripetibile con controlli; successivamente logging/CSV/web/UI e V2 PCB se giustificati. |

### Da chiarire prima della prima prova a caldo chiusa

1. **Materiale del frontale stampato** e vicinanza della zona più calda alla PCB/display; caratteristiche termiche di isolanti/crimp/cavi.
2. **TCO 105 °C:** confermare la configurazione fisica attuale prima di ogni nuova prova ON; raccogliere marcatura completa/scheda tecnica, distanza dalla piastra/NTC e temperatura di lavoro nominale.
3. **SHT45 e Sanwa:** collocazione effettiva nel volume chiuso e passaggi cavo sonde senza pizzicamenti né aperture rilevanti.
4. **Firmware:** exact currently flashed build/branch (a log precedente riporta fan channel0, `main` channel9), ma non interrompere l'attuale funzionamento fan-only per questo controllo finché non iniziamo a preparare un nuovo eseguibile versionato.

## Vincoli di approvazione

Questa bozza **non autorizza a caricare firmware che abiliti GPIO19 né ad alimentare la piastra a dryer chiuso**. Chiedere consenso per B/C e validare l'idoneità della protezione indipendente, poi autorizzazione separata per la prima prova D. Non eliminare il 105 °C TCO per ottenere 118 °C. Nessun test già superato deve essere ripetuto salvo un motivo tecnico specifico (la geometria chiusa è un motivo specifico diverso).
