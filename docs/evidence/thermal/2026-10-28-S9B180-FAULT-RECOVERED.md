# S9B180 — Causa del FAULT recuperata dal dispositivo, 28 ottobre 2026

**STATO EVIDENZA ORIGINALE:** l'utente ha letto direttamente `/api/status` e `/api/events` prima di riavviare, evitando una nuova prova. Il CSV originario S9B180 resta byte-identico `2026-10-28-S9B180-raw.csv`. Le osservazioni seguenti **SUPERSedono** i vecchi passaggi «motivo abort sconosciuto».

## /api/status: valori testuali forniti dall'utente

```json
{"firmware":"HOT-261027-S9B180","ms":1956040,"air_c":18.52,"rh":60.95,"ntc_ohms":124104.4,"air_valid":true,"ntc_valid":true,"heater_command":false,"cycle":false,"fault":true,"fan_percent":0,"phase":6,"pulses":19,"pulse_goal":0,"pulse_fault":4,"saved_log":true,"ram_samples":1067,"ota_idle":false,"ide_ota_ready":false}
```

## /api/events: cronologia integrale fornita dall'utente

```text
[238906 ms] AIR HEAT ON
[247405 ms] NTC/AIR PAUSE
[261406 ms] AIR HEAT ON
[269905 ms] NTC/AIR PAUSE
[284831 ms] AIR HEAT ON
[292830 ms] NTC/AIR PAUSE
[307831 ms] AIR HEAT ON
[315830 ms] NTC/AIR PAUSE
[331331 ms] AIR HEAT ON
[339330 ms] NTC/AIR PAUSE
[356612 ms] AIR HEAT ON
[364611 ms] NTC/AIR PAUSE
[381112 ms] AIR HEAT ON
[388611 ms] NTC/AIR PAUSE
[405568 ms] AIR HEAT ON
[413067 ms] NTC/AIR PAUSE
[430568 ms] AIR HEAT ON
[438067 ms] NTC/AIR PAUSE
[455995 ms] AIR HEAT ON
[463494 ms] NTC/AIR PAUSE
[481994 ms] AIR HEAT ON
[489493 ms] NTC/AIR PAUSE
[508494 ms] AIR HEAT ON
[515993 ms] NTC/AIR PAUSE
[535287 ms] AIR HEAT ON
[542786 ms] NTC/AIR PAUSE
[562287 ms] AIR HEAT ON
[569286 ms] NTC/AIR PAUSE
[575852 ms] FAULT ntc=0 pulse=4
[575852 ms] STATE FAULT
[575864 ms] FEEDBACK ABORTED
[659532 ms] NEW CSV READY
```

`HeaterCommissioning::Fault` enum ordinato: 4=**AirStale**, cioe' `airAgeMs>5000`. Questa è **la causa immediata certa** del test S9B. Il guasto NTC elettrico è0, la fase6 e' ABORTED e la procedura e' terminata in FAULT. I valori live letti a ms1956040 sono successivi alla fine del riscaldamento; **non** sono temperatura/NTC al momento del fault.

## Analisi gap CSV originale (solo misure osservate)

| Dal ms | Al ms | Gap |
| ---: | ---: | ---: |
| 211697 | 213404 | 1707 ms |
| 271904 | 273330 | 1426 ms |
| 339330 | 341610 | 2280 ms |
| 392128 | 395066 | 2938 ms |
| 451566 | 454993 | 3427 ms |
| 515993 | 519785 | 3792 ms |
| **571785** | **575851** | **4066 ms** |
| 631851 | 636521 | 4670 ms |

Questi gap mostrano ritardi della continuita' di logging, progressivamente crescenti. S9B aveva pagina dashboard che periodicamente richiedeva `/api/history.csv` (ogni4s) e firmware che rispondeva sincronicamente **durante i gap HEATER OFF**, trasferendo un CSV sempre maggiore. **Ipotesi causale tecnica forte, non dimostrata con profilazione**. Un singolo gap4,066s non prova che il sensore sia guasto: SHT45 legge normalmente ogni2s e timeout5s; il rallentamento del ciclo può superare il budget di freschezza. I nuovi dati `loop_gap_ms` e `air_age_ms` STEP10 sono disegnati per verificarlo.

## Decisione per il seguito

- Non ripetere S9B e non ri-chiedere i log; **causa AirStale recuperata**.
- Nessuna modifica arbitraria al timeout SHT45 da5s.
- STEP10A proposto: rifiuto dello stream CSV durante ciclo/cooling, diagnostica CSV estesa, acquisizione transizioni, nuovo profilo locale controllato. Vedere `docs/STEP10A_HEATER_AND_LOGGING_TEST_2026-10-28.md`.
- Nessuna decisione produzione presa e `docs/DECISIONS.md` non alterato.
