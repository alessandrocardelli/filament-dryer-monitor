# S9B — Analisi quantitativa del budget termico e proposta di test distinto

**Stato al 27 ottobre 2026: valutazione di progetto su dati reali S9A; S9B FISICAMENTE NON ESEGUITO.** Proposta contenuta in un ramo HEAT-capable **non destinato a flash automatico**. Nessun dato nuovo misurato e nessun nuovo limite termico di prodotto approvato.

## Introduzione obbligatoria

- **COSA:** testare la possibilità di arrivare al **setpoint aria25°C** allungando solo il budget cumulativo HEATER ON, mantenendo il feedback SHT45 e tutti gli interblocchi piastra, ventola, timeout e cooling invariati.
- **PERCHÉ:** nella S9A il raffreddamento è iniziato **per esaurimento di 120s di HEATER comandato**, prima di arrivare a25°C, non per intervento del termostato. Continuare gli stessi cicli in condizioni limitate potrebbe verificare lo spegnimento a25°C, ma il raggiungimento resta una **ipotesi**, non un risultato.
- **PRECEDENTE:** S9A fisica con 13 comandi HEATER ON, Sanwa cicli6–12 ON35/OFF38°C, SHT45 aria iniziale17,64°C, aria22,59°C al cooldown, massimo22,97°C in cooling, NTC minimo48,7167kΩ. Tutti i dati originali, Sanwa e CSV sono su `main` e PR#14 branch; non rifare S9A.
- **FUTURO:** se S9B dovesse raggiungere25°C, osserveremo l'intervento SHT45 HEATER OFF in hardware reale e il comportamento termico successivo. Se non li raggiungesse nei limiti, misureremo il vincolo senza alterare la sicurezza; ulteriori setpoint aria o FAN AUTO non sono autorizzati implicitamente.

## Fonti autoritative

- [S9A verbale e tutte le letture Sanwa](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md)
- [S9A CSV originale inalterato: 33.096 byte/778 campioni, Git blob 9312a724743632e62e28368e27cba7241e55401f](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-raw.csv)
- [S9A analisi dei 13 eventi HEAT e del budget](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-analysis.md)
- Firmware sorgente `firmware/HeaterStep09AirFeedback.h`, `firmware/SafetyController.h`, `firmware/AppConfig.h`, `firmware/FanController.h`, `firmware/ThermalLog.h`.
- `docs/NTC_CALIBRATION_ANALYSIS.md`: le sette coppie vecchie R_NTC/Sanwa sono a box APERTO e fan OFF, **non** calibrano NTC in°C per la versione chiusa/fan ON.
- `docs/DECISIONS.md` D029: il TCO aggiuntivo è opzionale come scelta, **ma l'attuale TCO105 è fisicamente in serie** e non viene bypassato; nessun acquisto/sostituzione impliciti.

## Vincolo misurato e ragionamento dell'estensione

| Indicatore S9A, misurato o da log | Valore | Fonte/limite |
| --- | ---: | --- |
| Aria SHT45 t=0 | **17,64°C** | Prima riga CSV (non temperatura piastra) |
| Aria SHT45 a t≈240s | **21,70°C** | CSV, condizioni transitorie |
| Aria SHT45 a t≈300s | **22,50°C** | CSV, condizioni transitorie |
| Aria all'ultimo HEATER OFF / inizio COOLING t≈307,6s | **22,59°C** | CSV; utente ha letto circa22,7°C sull'OLED |
| Picco durante cooling | **22,97°C** | CSV, dopo HEATER OFF |
| Budget HEATER ON consumato al cooling | **120s software** (~120,64s fra fronti campionati) | `maximumTotalHeatMs=120000`; no misura diretta energia elettrica |
| ON sostenuto negli ultimi cicli | **~8s ogni 24–27s** | Pausa quando NTC scende intorno50kΩ; re-arm>=65kΩ |
| Sanwa negli ultimi 7 cicli completi | **ON35°C / OFF38°C** | Lettura manuale utente al punto di contatto, non massima temperatura bead |
| Minimo NTC nel file | **48,7167kΩ** | Soft limite50kΩ; hard limite40kΩ non raggiunto |
| Tempo residuo al timeout globale S9A | **~292s** | `maximumRunMs=600000` |

**Calcolo, NON previsione validata:** negli ultimi60s di S9A l'aria cresce ~0,80°C, cioè ~0,013°C/s di tempo trascorso. Dal COOLING22,59°C ai25°C mancano2,41°C: prolungare idealmente quel tasso richiederebbe ~181s aggiuntivi di tempo, ma perdite e differenze ambientali ne cambiano la pendenza. Con finestre ultime da ~8s ogni24–27s, 181–250s in più di tempo corrisponderebbero indicativamente a ~54–83s addizionali di HEATER ON comandato. Questo motiva esplorare **+80s** sul tetto S9A, ma NON promette che l'aria raggiungerà25°C. Altra partenza più calda/fredda cambierebbe ulteriormente l'esito. L'esperimento è limitato anche a **600s** totali, che può prevalere sul tetto200s.

**Proposta S9B: tetto HEATER cumulativo 200s (+80s rispetto ai 120s S9A, +66,7%).** Scelto per investigare un target mancato del ~2°C quando tutto il resto era lontano dai limiti registrati e il Sanwa si era stabilizzato, non perché200s siano dimostrati sicuri: 200s significa comunque **più esposizione termica** del sistema e un'esperienza non ancora effettuata. La progressione di Sanwa e il comportamento hardware non possono essere garantiti da due log.

## Controlli invariati — DIFFERENZA UNICA di comportamento termico

| Vincolo | S9A (reale) | S9B (proposto) |
| --- | ---: | ---: |
| Feedback ARIA SHT45 | set25°C, HEAT OFF>=25°C, ON<=24°C | **identico** |
| NTC SOFT HEATER OFF | <=50.000Ω | **identico** |
| NTC ripresa HEAT | >=65.000Ω + OFF>=12s | **identico** |
| NTC HARD abort latch | <=40.000Ω (o >=250.000Ω) | **identico** |
| HEATER ON per singola finestra | max20s con timer ISR20,5s | **identico** |
| Tempo totale sessione | max600s | **identico** |
| HEATER ON cumulativo | max120s | **max200s, UNICA variabile termica aumentata** |
| Ventola comandata | 100% con prerun, cooldown90s | **identico** |
| Guardie sensore invalid/stale, FAN command lost, aria>=40°C | attive | **identiche** |
| Sanwa veto manuale | >=50°C | **identico, fermare subito** |
| TCO105 e cablaggi | dichiarati esistenti | **nessuna modifica autorizzata** |
| Frequenza log/dimensione buffer | 500ms / 1.400 campioni | **identici** (600s+90s ~1.380 campioni) |

**Non equiparare TCO105, Sanwa50°C e resistore NTC50kΩ:** hanno sensori, posizioni e significati fisici diversi. SHT45 aria<=25°C NON significa heater piastra<=25°C. Il Sanwa è una lettura manuale e non chiude autonomamente un interlock.

## Review dei rischi, ancora OPEN prima di una seconda prova a caldo

1. **Esposizione prolungata:** 200s ON cumulativi sono ~67% in più dei120s osservati; il Sanwa potrebbe non mantenersi a38°C con aria e parti che si scaldano. La stabilizzazione precedente non basta a dimostrare la massima temperatura di ogni materiale, bead NTC, cablaggio o parete. Sanwa50°C resta VETO umano; non è un sistema di taglio automatico.
2. **Protezione e guasti:** la curva NTC R→temperatura non è qualificata nel chiuso/fan ON; la SOFT50k misura resistenza e rallenta le accensioni, non fissa una temperatura assoluta garantita della piastra. HARD40k è un fault testato in S7U. GPIO OFF non è misura di corrente: un MOSFET guasto ON non sarebbe rilevato direttamente.
3. **Ventilazione:** fan PWM comandato100% non misura giri/portata reali; accertare fisicamente che ruoti prima e durante il riscaldamento e fermare in caso contrario. L'SHT45 non monitora punti caldi locali.
4. **TCO e materiali:** l'attuale fusibile termico monouso105°C rimane in serie. Non è stato qualificato come DC trip del test né come dispositivo capace di assorbire il rischio di un guasto MOSFET. Non bypassare. Non acquistare automaticamente G4 a152°C per questo test.
5. **Condizioni iniziali:** non assumere temperatura partenza17,64°C come garantita. Se SHT45 >=25°C, il riscaldatore resta OFF; se aria è tra24 e25°C a inizio prova, il firmware potrebbe non avviare HEAT da Prerun/Gap. Non raffreddare artificiosamente il sensore per farlo partire.
6. **Persistenza evidenze:** il CSV resta su LittleFS fino al download, buffer 1400/500ms e salvataggio solo a heating/cooling conclusi; non interpretare un errore nel download come ragione per ripetere riscaldamento.

## Gate di chiusura prima di qualunque upload e prova fisica

- [ ] CI **del branch S9B**: vera compilazione ESP32 e host GPIO-mock che verifica 200s, 20s/12s, NTC40/50/65, cooldown, FAULT, richiesta aria e differenze codice limitate; non accontentarsi della vecchia build S9A.
- [ ] Verifica precisa del montaggio fisico da parte dell'utente: Sanwa sulla piastra nella stessa zona NTC di S9A, fan funzionante, cavi integri, TCO105 non modificato, 24V rapidamente disconnettibili, display e controlli STOP leggibili. **Non fingere verifica remota**.
- [ ] Proposta di profilo fisico presentata all'utente con introduzione breve **COSA/PERCHÉ/PRECEDENTE/FUTURO**, istruzioni su quando leggere Sanwa a ogni ON/OFF e durante cooldown, temperature di arresto e recovery. **Nuova approvazione esplicita** per200s totali HEAT, distinta dall'approvazione S9A a120s.
- [ ] Una sola nuova sessione reale S9B, non automaticamente avviata. PR S9B DRAFT, nessun merge nella normale V1; dati raw originali e Sanwa su repository subito dopo.

**Stato oggi: S9A COMPLETATO; S9B candidato SOFTWARE IN REVISIONE, NON ESEGUITO fisicamente.** Nessuna nuova decisione permanente D0xx.
