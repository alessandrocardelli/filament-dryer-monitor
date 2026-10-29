# Step 06 — dati fisici reali e difetti dashboard (2026-10-25)

> Historical protocol/technical record. Its dated NEXT and pre-test status are not the current work queue; see [PROJECT_STATE](PROJECT_STATE.md) and [TODO](TODO.md). Original checkpoint preserved in archive tags.

**Stato fisico:** una sessione Step06 del firmware `HOT-251025-S6A` è stata effettivamente svolta dall'utente, con **COMPLETE** riferito, 4 finestre di heater COMMAND=ON registrate, ventola comandata 40% durante riscaldamento, sensori diagnostici validi. Non è una qualifica di essiccazione a 40°C aria o del termostato V1.

**Fonte primaria:** file CSV allegato in chat dall'utente, nome `filament-last-test(1).csv` (non ancora committato come CSV grezzo), 411 campioni, intestazione `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent`. Tempo MCU da 125968 ms a 267510 ms, **141.542s** di osservazione. Annotazioni dell'utente: **Sanwa iniziale 18°C, massimo durante heater-on 40°C, lettura dopo spegnimento riferita 27°C, COMPLETE, nessuna anomalia**. Il «max dopo spegnimento 27°C» non equivale alla dimostrazione con curva che non vi sia overshoot immediato; manca serie temporale Sanwa.

## Risultati numerici verificati nel CSV

| Valore | Misura |
|---|---:|
| Quattro fasi `heater_command=1` | inizi MCU 132060, 154185, 176641, 198705 ms |
| Quattro transizioni `heater_command=0` successive | 142103, 164738, 186641, 211157 ms |
| Durata comandata **campionata** | ~10,04s, 10,55s, 10,00s, 12,45s **limite superiore osservato per quarta fase tra due campioni** |
| Nota su ultimo intervallo | Lo scarto ~12,45s nell'ultima transizione NON prova output fisico HIGH per 12,45s. La misura CSV è comando logico e un gap campioni di 2,70s copre fine quarto impulso; ISR hardware è progettata per portare GPIO LOW a 10,5s anche con loop bloccato, ma **non misurata indipendentemente**. L'interfaccia web sincrona può ritardare loop. Serve non confondere `heater_command` con misura elettrica di corrente o GPIO |
| Aria camera SHT45 primo → ultimo | **18,51 → 19,52 °C**, +**1,01°C** nella finestra CSV |
| Aria alla prima riga cooldown → ultima | **18,95 → 19,52°C**, +**0,57°C** con riscaldatore comandato OFF: chiaro ritardo dinamico del flusso piastra→camera |
| Resistenza NTC prima → minimo | **135498,5 → 49043,9 Ω** (135,50 → 49,04 kΩ) |
| Resistenza NTC ultimo campione | **87804 Ω** (87,80 kΩ), non ancora riportata a freddo |
| Minima NTC per le 4 finestre ON | **83,87 / 63,09 / 55,24 / 49,04 kΩ** |
| Umidità relativa al primo / ultimo campione | 63,20 / 62,97 % (variazioni non qualificabili con questa brevità) |
| Validità | Tutti i 411 campioni: `air_valid=1` e `ntc_valid=1`; nessun guasto sensore rilevato nel campionamento |
| Fasi CSV | 1 prerun, 2 pulse, 3 gap, 4 cooldown. Ultima riga ancora phase4: COMPLETE è dato riferito dall'utente, non una riga log `phase=5` |
| Ventola | comando percentuale 100% nei primi 4 campioni (kickstart ~1s) poi 40%. Il CSV registra PWM/command, non un tachimetro o flussometro |

La differenza netta (Sanwa 18→40°C, SHT45 18,51→19,52°C) **non è un errore di calibrazione**: Sanwa = temperatura **superficiale locale vicino alla piastra**, NTC = resistenza del sensore **interno alla piastra**, SHT45 = temperatura **aria nella camera**. Non devono coincidere e variano con tempi differenti. Il CSV non fornisce `T_NTC_C` perché la curva termica del bead non è qualificata; non assegnare `B3950/R25=100k` o calibrare direttamente contro l'aria. Dati vecchi di 7 punti box aperto fan OFF restano di contesto, non soglie prodotto.

### Dashboard: difetti confermati e probabile causa del 4/3

Il codice `firmware.ino` chiama `network.tick(nowMs, !safety.heaterOn(), canOta)`. `NetworkService::tick` esegue `server_.handleClient()` **solo quando `permitHttp` o `uploadActive` sono veri**; durante gli impulsi ON, la gestione HTTP viene sospesa per non allungare il controllo. Perciò il browser vede risposte aggiornate **quando l'heater è OFF**, e può mostrare sempre OFF nonostante il CSV riporti ON. Questo è un **limite di osservabilità della dashboard**, non prova che GPIO19 sia rimasto spento. Non sbloccare semplicemente il server sincrono durante ON: può introdurre latenze e peggiorare sicurezza/temporizzazione. Il percorso asincrono/schedulato eventuale richiederebbe una review dedicata; per ora CSV completo + diagnostica di dato STALE.

La web page riferita mostra durante cooldown `Test 4/3`. Il firmware `HOT-251025-S6A` lato `NetworkService.h` in GitHub già espone `pulse_goal=4` e JavaScript usa `d.pulses+'/'+d.pulse_goal`, quindi quella stringa **non è riproducibile dallo stesso HTML attuale**; possibile cache pagina vecchia (la root HTML precedente non aveva `Cache-Control:no-store`), o differenza fra pagina effettivamente aperta e codice, da non dichiarare certo. Inoltre il conteggio `N/M` **non ha senso nella fase di raffreddamento**: mostrare `Raffreddamento` e basta.

**Correzione predisposta senza modificare le protezioni HEAT:** [PR #9](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/9) partendo dal branch sperimentale, con nuovo ID `HOT-251025-S6B`, differenziato dal binario S6A effettivamente testato; aggiunge cache-control HTML no-store, gestione timeout/staleness del browser con `HEATER NON AGGIORNATO` se assenza risposta HTTP, testo chiaro su pause HTTP durante riscaldamento e fasi in parole (`Raffreddamento`, `Test completato` anziché `4/3`). **Non modifica GPIO o safety**, non va confuso col firmware già sulla PCB e non richiede ripetere il test fisico. La PR resta separata e attende esito verifica software.

## Prossimo risultato necessario

Step06 **prova fisica eseguita / dati raccolti**, ma ancora **analisi di qualificazione parziale**: ora sappiamo che quattro impulsi comandati e il Sanwa/piastra crescono, l'aria segue più lentamente. La prova NON dimostra ancora la capacità del dryer di raggiungere 40°C aria, mantenere il setpoint o gestire calore continuo. Prima di allungare i tempi o abilitare la regolazione continua: confrontare risposta NTC e Sanwa con distribuzione fisica e definire limite termico conservativo, protezione del ventilatore (nessun tachometro), isolamento dei percorsi web sincroni durante heater ON, watchdog per riscaldamento autonomo e cooldown misurato. Evitare nuove prove ripetitive non informative.

Conservare la distinzione: report dei dati dell'utente = osservazioni; codice hardware = modelli/comandi, non conferma corrente di carico.

## 2026-10-25 — correzione dashboard testata e integrata SOLO nell'esperimento

[PR #9](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/9) **MERGED** nel branch `experiment/step06-supervised-heat-2026-10-25` al commit [`5d03c08`](https://github.com/alessandrocardelli/filament-dryer-monitor/commit/5d03c08fe910c40bf95e67a2bfc796eaf9ee3f76); `main` e il branch operativo freddo invariati. [CI #40216224251 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40216224251): test JavaScript inline reali, etichette e stale timeout, regressioni con GPIO simulato, compilazione firmware ESP32. Sono cambiati **solo** `firmware/NetworkService.h` (presentazione/staleness e HTML cache), `firmware/FirmwareBuild.h` (identificativo nuovo `HOT-251025-S6B`), test JavaScript e CI: `SafetyController`, NTC, SHT45, GPIO, fan, impulsi, logger, sketch eseguibile sono rimasti byte-identici alla build fisicamente provata S6A. Il firmware S6B **non è stato caricato sulla PCB e non è stato provato fisicamente**. Non rifare riscaldamento per testare unicamente le scritte della dashboard.
