# STEP10B — prova fisica: prima AIR25 raggiunta, regolazione piastra troppo conservativa

**Ricevuto:** 29 ottobre 2026, ora del messaggio circa 19:14 Europe/Rome. Orario civile di avvio/fine del test **NON COMUNICATO**; il CSV usa uptime ESP32 `ms`, non ora civile. **Stato: prova fisica ESEGUITA, acquisizione CSV originale verificata, AIR25 raggiunta, OFF al target e riaccensione termostatica sotto24C NON ancora completamente provati.** NESSUN firmware modificato durante l'analisi. Testo originale operativo utente e analisi vengono conservati assieme; non reinterpretare un comando GPIO come misura di corrente reale.

## Firmware, sorgenti e strumenti

- Build **confermata via `/api/status` fornito dall'operatore**: `HOT-261029-S10B25`. Branch attivo `experiment/step10-heater-diagnostics-air25-2026-10-28`, PR#18 draft. Codice HEATER STEP10B presente nelle sorgenti `firmware/HeaterStep09AirFeedback.h`, `firmware/SafetyController.h`, `firmware/firmware.ino` e `firmware/ThermalLog.h`. SHA esatto del binario installato e build CI del nuovo source **NON verificati**; identificazione runtime è confermata ma il checksum non è quello del dispositivo.
- SHT45 misura **aria camera** (°C, RH); NTC incorporato nella struttura heater misura **resistenza** (Ω). **Sanwa RD701/K sulla PIASTRA heater vicino all'NTC** misura una temperatura locale di contatto; per le precedenti prove il montaggio è noto e NON deve essere richiesto nuovamente salvo necessaria verifica fisica per sicurezza. Posizione Sanwa non è una misura della temperatura interna bead NTC.
- Configurazione firmware STEP10B: SHT45 AIR target25°C e riarmo24°C *solo dopo primo reale AIR25*; ON max20s/ISR20.5s; OFF min12s; R_NTC SOFT OFF <=45kΩ / restart >=60kΩ / hard trip <=40kΩ; sessione max720s; calore comandato cumulativo <=300s; PWM fan100%, limite aria40°C, cooling90s **dall'ultimo HEATER ON**, TCO105 serie segnalato installato e non bypassato, veto Sanwa piastra50°C. Non sono limiti termici di produzione e non autorizzano modifica TCO/protezioni.

## Originale CSV, salvato byte-identico

- Allegato: `filament-last-test (6).csv`.
- Percorso repository: **[`docs/evidence/thermal/2026-10-29-STEP10B-raw.csv`](2026-10-29-STEP10B-raw.csv)** in main e branch sperimentale.
- Lunghezza esatta: **55.054 byte**, intestazione + **976** righe dati, **15** colonne.
- **SHA256: `f587c40055cdac6103ff0b422f6244453a55ad1221a89f6d4813a8d989adf953`**.
- **Git blob SHA1 verificato rispetto ai byte originali: `8a75a50d3ddd031d974004f3134db0b736b20d55`** su entrambi i branch; nessuna formattazione, ricalcolo o rigenerazione del CSV.
- Uptime campioni: prima `775378` ms, ultima `1495378` ms = **720,000s tra i due**. Ultima riga `phase=4`, pulses21, HEATER0, fan comando100%. Stato utente successivo `ms=1708621`, `phase=5`, cycle=false, fan=0, `session_csv_saved=true`, `session_csv_final=true`. Quindi la fase finale5 è **testimoniata dallo status post-run, non nel file**. In questo specifico caso l'ultimo calore termina a +551,196s; il cooldown90s dall'ultimo ON è già scaduto quando entra phase4 al limite 720s: nessuna lunga coda phase4 da aspettarsi nel CSV. La dicitura `session_csv_final=true` indica che il checkpoint disponibile è stato accettato dal firmware come completo, non che contenga una nuova riga phase5.
- Tutti 976 campioni `air_valid=1` e `ntc_valid=1`, `pulse_fault=0`, `ntc_fault=0`, fan comandato100%; loop_gap massimo163ms (non i multi-secondi del S9B), air_age max2010ms, ntc_age max487ms. Questi campi sono dati software, non una prova diretta del flusso/uscita elettrica.

## Letture SANWA consegnate in chat (manuali, non nel CSV)

L'utente ha trasmesso la tabella seguente **senza titolo per le due colonne**: NON assegnare in modo definitivo ai due valori significati di «all'accensione»/«allo spegnimento» in assenza dell'etichetta. L'ordine qui conserva quello trasmesso.

```
17 | 28
28 | 35
35 | 37
36 | 38
36 | 38
36 | 38
36 | 38
36 | 38
36 | 38
36 | 38
37 | 38
36 | 38
37 | 38
36 | 38
37 | 38
37 | 38
37 | 38
37 | 38
37 | 39
37 | 39
37 | 38
37 | wait air 24c
Complete Sanwa 26
```

La registrazione indica contatto Sanwa PIASTRA a **38–39°C nel tratto ripetuto** e **26°C a 'Complete'**, ma senza timestamp sincronizzati ai fronti ON/OFF. `WAIT AIR 24C` riportato dall'OLED e compatibile con il controller dopo il setpoint25°C. I numeri nella tabella non sono sensore aria SHT45. Non inventare temperature/picchi intermedi o video non forniti.

## Stato completo /api/status riportato dall'utente, ORIGINALE

```json
{"firmware":"HOT-261029-S10B25","ms":1708621,"air_c":22.89,"rh":49.03,"ntc_ohms":96723.5,"air_valid":true,"ntc_valid":true,"heater_command":false,"cycle":false,"fault":false,"fan_percent":0,"phase":5,"pulses":21,"pulse_goal":0,"pulse_fault":0,"wait_reason":0,"cumulative_heat_ms":213939,"session_csv_saved":true,"session_csv_final":true,"saved_log":true,"ram_samples":976,"ota_idle":false,"ide_ota_ready":false}
```

Questo status è **successivo** al CSV di almeno213,243s di uptime. L'aria `22.89°C` in status **NON** è il valore alla fine dell'HEATER o alla fine del CSV, ma uno stato post-completamento a ventola OFF.

## Analisi derivata (registrare separatamente dall'originale)

| Evento | t relativo alla prima riga CSV | Aria SHT45 | R_NTC | Fase/HEATER |
| --- | ---: | ---: | ---: | --- |
| prima riga | 0,000s | 17,28°C | 135.870Ω | phase1, OFF prerun, fan100% comando |
| primo HEATER ON | +6,000s | 17,25°C | 135.485Ω | phase2 ON |
| primo HEATER OFF | +26,000s | 17,38°C | 62.392Ω | phase3 OFF finestra20s |
| secondo HEATER OFF | +58,007s | 17,87°C | 46.343Ω | OFF per20s finestra |
| terzo HEATER OFF | +82,485s | 18,40°C | 44.959Ω | OFF su SOFT NTC |
| ventesimo HEATER OFF | +522,655s | 24,75°C | 44.351Ω | OFF su SOFT NTC |
| **ultimo (21º) HEATER ON** | +543,196s | 24,89°C | 60.073Ω | phase2 ON |
| **ultimo (21º) HEATER OFF** | **+551,196s** | **24,91°C** | **44.952Ω** | phase3 OFF, 8s ON |
| prima aria >=25°C | **+563,196s** | **25,02°C** | 54.508Ω | phase3, HEATER già OFF |
| picco aria | +587,222s | **25,10°C** | 67.812Ω | phase3, OFF |
| ultimo campione, fine max session720s | +720,000s | **24,47°C** | 91.331Ω | phase4, OFF, fan comando100% |
| status post-test (uptime ms1708621) | +933,243s | **22,89°C** | 96.724Ω | phase5 complete, cyclefalse, fan0, faultfalse |

**21 veri fronti HEATER ON e21 fronti HEATER OFF** nel CSV; ON cumulativo dai fronti `213,939s`, IDENTICO al campo `cumulative_heat_ms=213939` in status. Prime due finestre20,000s, 3ª12,439s, successive progressivamente circa8–10s, ultima8,000s. Motivo dominante OFF dal 3º impulso: `R_NTC <=45kΩ`; il comando torna ON da fase3 quando R>=60kΩ e AIR domanda attiva. L'NTC minimo globale misurato è **43.738,2Ω** a +303,361s **dopo un OFF**, circa**3.738Ω sopra40k fault**. NON equiparare questo delta a gradi Celsius né inferire che il limite possa essere abbassato in sicurezza.

**Raggiungimento aria target:** ultimo calore termina a24,91°C a +551,196s; l'aria tocca25,02°C **12s dopo** per inerzia e raggiunge25,10°C; non è stata osservata una commutazione HEATER ON→OFF direttamente causata dal raggiungimento AIR25, solo la corretta inibizione di nuovi ON dopo il setpoint nel contesto dell'attesa. L'aria resta >24°C per tutto il restante CSV e arriva24,47°C al timeout720s. `WAIT AIR 24C` è atteso; **nessun riarmo a24°C è stato fisicamente verificato**.

**Motivo fine sessione:** timeout massimo `maximumRunMs=720000`, NON budget ON300s (effettivo213,939s), NON fault. Il controller passa a phase4/complete senza riavviare il riscaldamento quando scade la sessione. Anche scendendo più tardi sotto24°C in phase5, **questa build one-shot non riaccende**; soltanto un ipotetico termostato V1 di durata maggiore dovrebbe farlo, a condizioni NTC e sicurezze soddisfatte.

**Performance:** impiego ON ~29,7% dei720s totali (non un duty PWM durante un impulso), piastra Sanwa prevalentemente38–39°C e aria salita di circa7,82°C dal primo sample al max. Dimostra che l'envelope corrente è molto conservativo. La piastra NON è controllata direttamente a38°C: è effetto del guard R_NTC45k e ripresa60k con ventola100%. Il precedente S7U ha già osservato NTC40k FAULT vicino al Sanwa43°C; S6C ha raggiunto Sanwa53°C con NTC min30.32k in condizioni diverse. **Qualsiasi nuovo limite locale superiore deve essere deciso con un modello/supervisione che rispetti il margine hard fault e la sicurezza hardware, non modificando casualmente 45→40k o bypassando TCO105**.

## Limitazioni e prossima decisione

- **PASS limitato**: AIR25 raggiunta nel programma 12min, no fault, buona telemetria senza stall plurisecondo, nuovo CSV originale persistito e confrontato. Non chiamare PASS regolazione a due soglie, perché AIR24 non è stata raggiunta mentre la sessione era attiva. Non chiamare PASS prodotto55°C autonomo.
- Le letture Sanwa sono manuali non time-synced, e due colonne non etichettate; il sensore NTC non misura direttamente la temperatura della piastra, e non è calibrato per convertirne R in°C in condizioni chiuse con fan.
- Non eseguire un'altra prova HEATER ON semplicemente per arrivare a24°C; il limite one-shot12min era il confine del test. Nessun nuovo ciclo autorizzato da questo record.
- **Unica prossima attività tecnica**: progettare/verificare a freddo il prossimo **envelope termico locale NTC/Sanwa e strategia di controllo aria/durata V1**, partendo dalle prove già concluse (S6C, S7U, S9A/B, STEP10B). Documentare la distanza dalla soglia NTC40k e caratteristiche fisiche TCO105 senza sostituire/bypassare protezioni automaticamente; eventuale nuova prova riscaldata deve essere specifica, autorizzata e limitata, con software CI e Sanwa sempre sulla piastra.
