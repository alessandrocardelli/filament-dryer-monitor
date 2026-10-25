# Step 06 — primo riscaldamento reale con ventola e dryer chiuso

**Data:** 25 ottobre 2026.  
**Stato:** **CANDIDATO SOFTWARE**, non ancora approvato, non caricato, nessuna misura termica eseguita.  
**PR:** [#7](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/7) sul branch `feature/step06-fan-on-thermal-characterization-2026-10-25`, da `feature/wifi-ota-web-logging-2026-10-24` HEAD `18706f277a0facc1d45e6dba065b08489d6fd304`.

## Perché cambiamo prova

L'utente chiede un riscaldamento reale con ventola attiva **come nell'eBOX Lite**, non altri cicli minimi senza risultato. Il profilo dello step05 (4×1,5 s, 6 s ON, circa 229 J @24 V/1,59 A), soglia aria 35°C e resistenza NTC ≥42,27 kΩ, **non permette una caratterizzazione significativa della risposta della camera verso il primo livello OEM, 40°C di aria**. Era solo un esercizio di sicurezza/guardie, non una prova del dryer funzionante.

La nuova prova deve ottenere almeno una **traccia utile di salita delle temperature aria e piastra** con **box chiuso e ventola ON**. Non dimostra ancora il mantenimento termostatico a 40°C: quello resta lo step07 dopo lettura delle misure e guardie giustificate. **Non ripetere le prove a box aperto o i tre impulsi storici.**

## Profilo proposto, una sessione supervisionata — NON autorizzato

| Parametro | Candidato offline in `firmware/HeaterStep06Candidate.h` |
| --- | --- |
| Stato sorgente GitHub | `COLD-251025-S6`, `kHeaterCommissioningMode=false`, `kHeaterStage05TrialReviewed=false`, `kHeaterStep06TrialReviewed=false`, `kHeaterOutputEnabled=false` |
| Riscaldamento previsto solo per futura build dedicata | 4 finestre ×10 s massimo nominale ON, gap12 s, una sola sequenza per power-on |
| Ventola | accensione 100% kickstart, quindi PWM 40% come bring-up; prerun 5s dopo comando Running (assenza tachimetro) |
| Timer hardware massimo per ogni intervallo | 10,5s independent timer ISR spegne GPIO19 se loop stall, da qualificare nel flash sperimentale |
| Tempo ON nominale complessivo | 40 s, su 81 s fino all'inizio cooldown |
| Cooldown | 60s nella state machine, con STOP se prova in sicurezza e osservazione Sanwa post-OFF |
| Timeout intera sequenza | 155s |
| Stop NTC elettrico | NTC mancante, non valido, stale oltre1,5s; R_NTC≤19,68kΩ o ≥250kΩ |
| Stop aria | SHT45 non valido, stale oltre5s, T_aria ≥40°C |
| Stop ventola | comando Running non raggiunto in12s o perso durante sequenza; **il comando NON verifica rotazione reale** |
| Veto operatore proposto | Sanwa vicino a piastra **≥55°C**, distacco sonda, trend termico imprevisto, ventola immobile, odore/materiali/cavi sospetti o impossibilità di osservare il dispositivo |
| Stima energia, non misura nuova | 24 V ×1,59 A ≈38,16 W; 40 s ≈ **1526 J** ON nominali; circa 9× l'energia dei 3×1,5s storici (172J), ma test aperto fan OFF nel passato ha avuto fino a ~110s ON, *non* garanzia di risposta equivalente in box chiuso |

**AVVERTENZA:** il punto Sanwa **non è** l'interno del termistore; R_NTC=19,68kΩ era correlato a **Sanwa 56°C** nella prova con entrambe le metà aperte e fan OFF. Non usare l'equivalenza come calibrazione chiusa. Il veto Sanwa 55°C non è automatico (il firmware non legge la termocoppia). I parametri 19,68kΩ/55°C/40°C sono **soglie candidate della prima sessione**, non limiti definitivi per un prodotto. Il test può arrestarsi prima di raggiungere 40°C dell'aria: quel dato sarebbe comunque utile per calcolare la dinamica piastra→aria e la prossima revisione.

## Gate pratico unico prima del flash di una versione heater-enabled

1. **Nessun carico ora:** i file GitHub e il firmware eventualmente sul dryer restano heater-OFF per questa fase. Un binario caricato senza riscontro dei requisiti non deve poter scaldare.
2. **Revisione finale** dell'identificativo build riscaldata, della configurazione e del file binario, con CI ESP32 + test host sullo SHA esatto, compresa verifica del timer hardware in avvio; una build heater ON verrà derivata **soltanto dopo** decisione esplicita sul profilo e approvazione utente. Non mergiare un'eventuale build HEAT in `main` o nel branch operazionale standard.
3. **Fisica a dispositivo OFF:** Sanwa meccanicamente fissata nel punto noto, al contatto ripetibile; cavi Sanwa e due conduttori 24V non schiacciati contro foro jack DC e parti calde, entrambe le metà chiuse, TCO 105°C già installato sempre in serie; materiale vicino al heater non danneggiabile entro i limiti proposti (non dimostrato); ventola libera e verso di flusso corretto.
4. **Misurabilità:** tensione/corrente monitorabile se disponibile, Sanwa leggibile durante tutta la prova, Serial HOT_CSV registrato; annotare con orologio/timestamp valori esterni Sanwa, e `t, ON/OFF, R_NTC, T_aria, RH, fan`. L'ESP32 non legge il Sanwa in automatico. Tenere libero accesso all'interruzione del 24 V: se necessario tagliare subito l'alimentazione anche se ciò ferma la ventola, privilegiando l'interruzione d'emergenza.
5. **Una singola prova autorizzata:** no unattended operation, nessun tentativo di raggiungere l'aria 50–55°C o piastra 105–118°C, nessun bypass TCO, nessun autoresume. Terminata la prova guardare **il massimo Sanwa anche dopo HEAT OFF**, non fermare la misura al quarto impulso. Se non abbiamo dati attendibili, abortire e non alzare le soglie.
6. **Risultato richiesto:** curva/punti sincronizzati camera aria/Sanwa piastra/R_NTC nel dryer chiuso e fan ON. Questa evidenza, non altre simulazioni, permette di sviluppare il primo vero controllo aria 40°C nello step07.

## Stato tecnico

Il percorso standard del termostato resta disabilitato e usa una `ntc.temperatureC` storica **non calibrata**: NON attivare `kHeaterOutputEnabled` o `kHeaterNtcModelApproved` per superare artificialmente il blocco. Qui si usa soltanto il ramo separato di commissioning in Ω, con pulse timeout indipendente dal loop e guardie SHT45/fan/NTC.

**Decisione non ancora presa:** parametri del test fisico e produzione di una build `HEAT` distinta. Non modificare `docs/DECISIONS.md` per una mera proposta. Lo stato fisico remains OPEN.
