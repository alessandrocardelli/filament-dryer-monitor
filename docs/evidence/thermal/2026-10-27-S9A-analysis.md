# STEP09-A — Analisi derivata della prima prova fisica HEATER ON in feedback aria

**Stato: ANALISI ESEGUITA, dati acquisiti.** 27 ottobre 2026. Questo file è **derivato** e separato da [CSV grezzo byte-identico](2026-10-27-S9A-raw.csv) e [verbale delle 13 letture Sanwa dell'utente](2026-10-27-S9A-PHYSICAL-SESSION.md). Il test fisico S9A è concluso; **non ripeterlo come se mancassero le misure**.

## Domanda sperimentale, precedente e prossimo gate

- **Cosa si voleva dimostrare:** che il controllore possa accendere realmente l'HEATER, sospenderlo per protezione/limiti della piastra e (idealmente) disattivarlo quando SHT45 raggiunge il target aria25°C.
- **Perché:** passare dagli impulsi prestabiliti S6C/S7U a un vero controllo a feedback aria con guardia termica locale distinta.
- **Precedente:** S6C 120s comandati ON/6 impulsi, SHT45 18,64→23,98°C a fine log, Sanwa max53°C, R_NTC minima30,318kΩ; S7U 59,25s ON con guasto precoce hard40kΩ, SHT45 18,20→20,89°C a fine log, Sanwa43°C allo stop.
- **Futuro:** capire quale vincolo ha impedito di arrivare a25°C e progettare la **prossima diversa prova di controllo aria**, con limiti approvati e specifica introduzione; non cambiare ora limiti permanenti, hardware o fonte di energia.

## Integrità e metodo

Originale utente `filament-last-test (3).csv`: **33.096 byte /778 righe dati**, SHA256 `6632f79b9e815965fe7f78b6ed31ec39f4a4bcb35b3c4f0c17da655ab4b877e0`, Git blob `9312a724743632e62e28368e27cba7241e55401f` **verificato identico su GitHub**. Parsing delle colonne `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent`. Fronti ON=passaggio `heater_command 0→1`; fronti OFF=passaggio `1→0`; tempi relativi al primo `ms=2406435`, non ora assoluta. La serie è a circa500ms con alcuni salti (massimo 3s); durate ON calcolate sui fronti del log con risoluzione limitata.

**Letture Sanwa**: prese manualmente dall'utente ai cambi di stato visualizzati, non presenti nel CSV né sincronizzate al millisecondo. Sanwa17°C all'inizio, finestra1 ON17/OFF30°C, finestra2 ON29/OFF36°C, finestra3 ON34/OFF37°C, finestra4 ON35/OFF37°C, finestra5 ON35/OFF38°C, finestre6–12 ON35/OFF38°C, finestra13 ON35°C senza lettura OFF dedicata, Sanwa26°C a fine cooling.

## Dati per la diagnosi

| Proprietà | STEP09 S9A | STEP06 S6C | Commento |
| --- | ---: | ---: | --- |
| SHT45 inizio | 17,64 °C | 18,64 °C | Sessioni partite da aria differente |
| SHT45 massimo/fine log | **22,97 / 22,93 °C** | **23,98 °C a fine log** | S9A target25 NON raggiunto |
| Massima crescita rispetto alla partenza | **+5,33 °C** | **+5,34 °C a fine log** | Quasi identica, ma le prove non sono termicamente controllate/replicate |
| HEATER ON comandato | **13 finestre, circa120,64s dai fronti CSV** | **6 finestre, circa120,03s** | Stesso ordine di budget energia comandata; non potenza misurata |
| Durata file | 397,13s | 275,66s | S9A distribuisce calore nel tempo con gap NTC |
| NTC minimo CSV | **48,7167kΩ** | **30,3179kΩ** | Guardie molto diverse; NTC in ohm, non °C calibrati |
| Sanwa più alto riportato | **38°C** (OFF cicli5–12) | **53°C** | Valori manuali e geometrie non certificate uguali; non dedurre maximum assoluto |
| Fan duty nel CSV | 100% comandato | 100% comandato | Non rappresenta giri veri né portata |

### A. Perché 13 ON e fine senza raggiungere25°C

Il codice effettivamente eseguito su PR#14 usa `HeaterStep09AirFeedback.h`:
1. HEATER richiesta con SHT45 aria <=24°C, OFF quando aria>=25°C; NTC sotto50kΩ causa SOFT PAUSE e riprende solo sopra65kΩ dopo almeno12s OFF.
2. Una finestra HEATER dura max20s; `totalHeatMs_ >= maximumTotalHeatMs = 120000` forza `finishHeating()` = fase4 `Cooldown` (NON fase6 `Aborted`).
3. Il nuovo ciclo ha 13 ON: **20; 16,643; 10; 9; 8,5; 8,5; 8; 8; 8; 8; 7,5; 8; 0,5 secondi** dal CSV. In particolare a **t=307,095s** inizia l'ON13, a **t=307,595s** scatta `phase=4` e `heater_command=0`, con aria22,59°C e NTC65,3649kΩ. La somma dei fronti ON è circa120,64s per granularità/jitter, nel sistema il criterio di bilancio è 120000ms. Il limite globale600s non è la causa (fino allora sono trascorsi ~307,6s).
4. Il termostato aria25°C non poteva essere la causa di NESSUNO spegnimento ON di questa prova: l'aria massima nel file è22,97°C, ed al momento cooling era22,59°C. **Il percorso di cutoff termostatico in prossimità di25°C non è stato fisicamente verificato**, anche se la policy e la build sono state testate in host.
5. Nei primi12 OFF ordinari l'NTC alla fine della seconda–dodicesima finestra è tipicamente vicino a49–50kΩ: il regolatore di protezione funziona come previsto dalla soglia soft proposta. La prima finestra termina a20s con NTC63,3184kΩ, quindi per timeout per-finestra. Nessuna prova di fault hard: minima R_NTC48,7167kΩ >40kΩ.
6. Il Sanwa arriva a30°C al primo OFF e poi si stabilizza tra35 e38°C nelle osservazioni dell'utente. Questo suggerisce che gli spegnimenti anticipati limitino il riscaldamento nel punto misurato, ma **non dimostra temperatura uniforme**, temperatura massima hotspot né sicurezza dei materiali.

### B. Cosa succede dopo OFF

- SHT45 all'inizio prova17,64°C, al passaggio in cooling22,59°C, massimo in cooling22,97°C (**+0,38°C** dopo OFF definitivo), ultimo22,93°C: **il trasferimento piastra→aria continua mentre HEATER è già OFF**, senza che le temperature piastra e camera debbano coincidere.
- Cooling nel CSV da t=307,595 a397,130s = ~**89,535s**, fase4, fan_percent100, heater_command0; il salvataggio del file può precedere l'apparizione di una riga fase5. L'utente segnala **Sanwa26°C alla fine del cooling**. Non c'è misura tach di ventola o temperatura TCO.
- RH passa da65,33% a51,36%; è umidità **relativa**, quindi la diminuzione da sola NON dimostra rimozione di acqua dal filamento.

### C. Che cosa è effettivamente PASS e che cosa no

**PASS limitato:** dati fisici e CSV ottenuti; 13 richieste HEATER ON osservate, transizioni normali fra ON/pausa, RAW NTC soft vicino50kΩ senza raggiungere fault40kΩ, Sanwa35–38°C riportato sulle finestre stabilizzate e cooling con Sanwa26°C. Nessuna evidenza di `phase=6` in 778 righe. Attenzione: nessuna misura indipendente della corrente MOSFET o giri ventola.

**NON PASS per il requisito del target:** SHT45 non ha mai raggiunto25°C, quindi non è stato provato fisicamente il termostato che stacca per raggiungimento target e riparte per isteresi. La prova **non è un fallimento di sicurezza** né prova che il dryer non possa raggiungere25°C; è stata conclusa dal budget volutamente conservativo120s.

### D. Direzione raccomandata per il prossimo lavoro (NON decisione di nuova soglia)

Il principale collo di bottiglia osservato **non è che NTC e aria hanno temperature diverse**, ma che **il budget cumulativo120s termina prima di raggiungere25°C**, mentre il vincolo soft NTC50kΩ produce finestre ON sempre più brevi (~8s). L'aria continua a salire in cooling e la sua crescita da partenza per120s di HEAT comandato è simile alla S6C, che partiva1°C più calda.

È ragionevole come **ipotesi tecnica** valutare una sessione S9B che permetta un budget cumulativo maggiore per provare finalmente il cutoff per target25°C **mantenendo la protezione locale NTC soft50kΩ/ripresa65kΩ e hard40kΩ, veto Sanwa50°C, limiti singola finestra/ventola/cooldown**, ma occorre rivedere margini di installazione TCO/materiali/fan, esposizione cumulativa e guasti: la somiglianza dei dati non consente proporre un nuovo numero di secondi come "sicuro" senza review. Non cambiare PWM fan o soglie NTC nello stesso esperimento: sarebbe difficile attribuire gli effetti. L'eventuale nuova sessione è separata, con introduzione obbligatoria, codice+CI, protocollo e approvazione esplicita.

**Prossimo gate singolo:** review quantitativa dei limiti termici e del budget cumulativo in S9A, quindi decisione se proporre un nuovo profilo S9B; *nessun nuovo flash/HEAT automatico*.

## Sorgenti

- [CSV originale S9A](2026-10-27-S9A-raw.csv)
- [Verbale misure Sanwa e tutti gli eventi](2026-10-27-S9A-PHYSICAL-SESSION.md)
- [Protocollo precedente](../../STEP09_REAL_HEAT_AIR_FEEDBACK_PROTOCOL_2026-10-27.md)
- [Controller reale (branch sperimentale)](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step09-real-sht45-feedback-2026-10-27/firmware/HeaterStep09AirFeedback.h)
- [SafetyController (branch sperimentale)](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step09-real-sht45-feedback-2026-10-27/firmware/SafetyController.h)
- [S6C dati originali su main](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-25-S6C-fan100.csv)
