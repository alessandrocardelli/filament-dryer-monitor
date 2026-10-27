# STEP08 — Analisi G2/G3 dei dati reali e prossimo test di integrazione

**Stato aggiornato 27 ottobre 2026:** **G2 confronto S6C/S7U ESEGUITO; G3 review effettuata ma QUALIFICA SICUREZZA OPEN; G4 NUOVO REPLAY HOST C++ ESEGUITO / CI PASS LIMITATO.** Test effettivo su [PR #13 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/13), [Actions run 40476716231](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40476716231), branch `test/step08-ntc-air-replay-2026-10-27` (commit sorgente `1057a633aa1cc75b1c1dac3643a04a94d342e69a`). Nessuna nuova prova fisica, nessun cambiamento di firmware di produzione, limiti o installazione. **PASS della simulazione integrata non significa che il prodotto sia qualificato né che il termostato fisico sia attivo.**

**Protocollo preregistrato:** [STEP08_NEXT_TEST_2026-10-27.md](../../STEP08_NEXT_TEST_2026-10-27.md).

## Introduzione obbligatoria

- **Cosa:** confrontare in dettaglio S6C e S7U, identificare con dati reali se il sistema si è fermato per la protezione locale o per il termostato aria, delimitare il test software nuovo.
- **Perché:** progettare una V1 in cui l'SHT45 regola l'ARIA, mentre l'NTC limita il riscaldatore, due temperature che non devono coincidere. Non è appropriato alzare arbitrariamente il limite NTC sperimentale.
- **Precedente:** S6C con 6 finestre di calore e S7U con guardia sperimentale a40kΩ, entrambe fan PWM100; S7U termina al terzo impulso per `NTC R LIMIT`, Sanwa43°C, aria19,52°C.
- **Futuro:** verificare via replay software la precedenza di NTC/FAULT sulla richiesta del termostato, già provato autonomamente nello STEP03; solo dopo discutere una futura prova fisica differente con limiti approvati.

## A. Sorgenti reali e riproducibilità

1. [S6C CSV grezzo verificato](2026-10-25-S6C-fan100.csv), **652 campioni**, **26.859 byte**, blob Git **`7df98ea139fdc73d83b0ee73b003e75b4b19cef0`**. Questa copia sul branch `main` è identica a quella precedente sul branch `experiment/step07-ntc-guard-verified-history-2026-10-25`; confronto SHA blob dopo salvataggio. Non alterare i CSV originali.
2. [S7U CSV grezzo](2026-10-26-S7U-retest-raw.csv), **717 campioni**, **29.142 byte**, SHA256 originale **`c417313414eede75f2269dbfb8717ac6d8246625172e315a007b8056def834ae`**, blob Git **`891ea18eb00efdb6690e7b7e3c0c6466e6a39674`**.
3. [S7U verbale originale con tutte le misure Sanwa](2026-10-26-S7U-RETEST-SESSION.md); [analisi S7U](2026-10-26-S7U-retest-analysis.md). Data resoconto e allegato chat: 27 ottobre; data e ora fisica del test non attestate.
4. Versione sorgente HEAT sperimentale: `fix/step07-clear-test-feedback-2026-10-26`, `HeaterStep07GuardTrial.h`, `HeaterPulseCommissioning.h`, `SafetyController.h`, `HeaterControlPolicy.h`, `AppConfig.h`, `FanController.h`. STEP03, termostato SHT45 standalone, era già PASS in `docs/STEP03_THERMOSTAT_SIMULATION_2026-10-25.md` sul branch operativo; **non ripetere**.
5. Metodo calcolo effettivamente applicato al contenuto integrale dei due file recuperati da GitHub: parsing righe CSV (header 10 campi), valori numerici, comparazione `heater_command` riga per riga; un fronte 0→1 indica prima riga di comando ON, 1→0 prima riga OFF; durata = `(ms_OFF-ms_ON)/1000`; minima NTC e massima SHT45/RH su tutte le righe; delta aria post-OFF = aria ultima riga meno aria prima riga del definitivo OFF. Gli istanti hanno risoluzione di campionamento, NON sono misure indipendenti dell'uscita fisica.

## B. Confronto sintetico

| Metrica misurata/loggata | S6C | S7U |
| --- | ---: | ---: |
| Campioni | **652** | **717** |
| Durata tra prima/ultima riga | **275,664 s** | **179,021 s** |
| ON effettivamente comandati | **6** | **3**, il terzo interrotto |
| Durate ON da transizioni CSV | **20,026 + 20 + 20 + 20 + 20 + 20 s** (120,026 s) | **20 + 20 + 19,250 s** (59,250 s) |
| Gap ON/OFF tra impulsi | **12,298; 12,479; 12,009; 12,844; 12,577 s** | **12,000; 12,000 s** |
| Aria SHT45 a inizio CSV | **18,64 °C** | **18,20 °C** |
| Aria SHT45 all'ultimo HEATER OFF | **22,57 °C** | **19,52 °C** |
| Aria SHT45 a fine CSV | **23,98 °C** | **20,89 °C** |
| Salita SHT45 post-OFF nell'intervallo campionato | **+1,41 °C** | **+1,37 °C** |
| NTC a inizio CSV | **131.945,3 Ω** | **136.951,7 Ω** |
| NTC minimo nel CSV | **30.317,9 Ω** | **39.593,4 Ω** |
| NTC all'ultimo OFF | **30.317,9 Ω** | **39.789,3 Ω** |
| RH iniziale/finale | **64,50% → 53,03%** | **64,12% → 58,65%** |
| `fan_percent` | **100 in 652/652** | **100 in 717/717** |
| Validità aria / NTC | **652/652 entrambi validi** | **717/717 entrambi validi** |
| Sanwa esterno e causa finale | **18 °C iniziale, 53 °C massimo riferito durante calore, 30 °C alla fine del raffreddamento**; sei impulsi completi | **17°C iniziale e primo ON, 36°C prima terzo, 43°C a `NTC R LIMIT`, 25°C a ventola ferma** |

**Nota sui dati:** i 120,026 e 59,250 s sono durate **inferite dai comandi CSV**; non potenza o corrente elettrica misurata. Le durate gap S6C sono leggermente maggiori di12s per risoluzione/tempi di schedulazione. La ventola al100% è un comando PWM, NON prova che giri o abbia una portata nota.

## C. Confronto diretto a energia/tempo simili — prime tre finestre

| Evento | NTC S6C Ω | NTC S7U Ω | Aria S6C °C | Aria S7U °C |
| --- | ---: | ---: | ---: | ---: |
| Avvio ON1 | 132.072,5 | 135.683,9 | 18,67 | 18,21 |
| Fine ON1 | 60.437,6 | 62.287,4 | 18,79 | 18,34 |
| Avvio ON2 | 76.374,2 | 78.696,9 | 18,97 | 18,47 |
| Fine ON2 | 45.723,6 | 46.401,5 | 19,30 | 18,79 |
| Avvio ON3 | 60.841,2 | 61.271,0 | 19,57 | 19,07 |
| **Fine ON3** | **39.157,2** (gap previsto) | **39.789,3** (FAULT a 19,25s) | **20,03** | **19,52** |

- Se si corregge solo la differenza fra aria iniziale misurata, alla fine dei primi tre ON S6C è **+1,39 °C**, S7U **+1,32 °C**: molto simile, ma **due prove non validano un modello di trasferimento** e non danno una portata aria.
- Il punto particolarmente istruttivo è `R_NTC`: alla fine ON3 S6C **39,16 kΩ** e S7U **39,79 kΩ**, in un intervallo di circa 20s. La nuova soglia **40 kΩ** conduce allo stop di S7U proprio in questo intorno. La diversa soglia S6C più bassa consentiva di proseguire, non è prova che fosse sicura per uso continuo.
- S7U: ultimo campione ancora ON `ms=95806, ntc=40.159,9 Ω`; primo OFF `ms=96056, ntc=39.789,3 Ω`, `phase=6` (ABORTED); il guasto display è `NTC R LIMIT`, coerente con trip in quell'intervallo da250ms. Minimo 39.593,4Ω **0,5s dopo** il primo OFF. Nessuna prova indipendente corrente-heater.
- **Entrambe** le sessioni sono transitori molto brevi e lontani dai livelli nominali OEM eBOX Lite **piastra 105/116/118°C, aria40/50/55°C**; **NON** concludere che la camera non possa raggiungere40°C in un ciclo più lungo né che basti far arrivare la piastra a40°C. Raggiungimento del target 40°C richiederà regolazione aria con protezione heater adeguata e caratteristiche termiche verificate.

## D. Qualità e limiti dei campioni

- **S7U:** campionamento pressoché uniforme ogni250ms; 717 righe, nessun intervallo oltre271ms, nessuna riga invalida.
- **S6C:** campionamento NON uniforme in cooldown. Confrontando i tempi consecutivi: **59 intervalli oltre500ms**, dei quali **34 oltre2000ms**, massimo **4281ms**. La sequenza principale di ON/GAP ha transizioni valide, ma non interpolare le zone rade di cooling fingendo di avere 4 campioni al secondo: gli ultimi55 campioni coprono circa87s anziché14s. L'aria finale23,98°C è vera lettura, non una curva continua pienamente osservata.
- Le variazioni RH sono percentuali **relative** influenzate dalla temperatura; NON dimostrano perdita di acqua dal filamento.
- Il Sanwa è un termometro esterno a contatto presso la zona heater, **non** appare nei CSV; SHT45 misura aria; NTC registra ohm del bead. Non convertirli fra loro senza curva locale qualificata né inferire che Sanwa43°C e aria19,52°C debbano essere uguali.
- S6C NTC minimo30,318kΩ e Sanwa massimo53°C **non necessariamente simultanei**. Non estrarre da queste due estreme una calibrazione R→T: le letture Sanwa non sono sincronizzate al campione CSV.

## E. G2 — valutazione finale

**G2: PASS per confronto dati e identificazione del problema, NON per modello termico o safety qualification.** Le prime tre finestre sono abbastanza simili da sostenere che la guardia sperimentale40kΩ interviene nell'intorno già anticipato dal log precedente. S7U dimostra stop per soglia NTC mentre la camera è ancora sotto il setpoint aria, il che è **normale e voluto in una protezione**; NON dimostra un errore nel termostato che non era in esecuzione. I test **non** quantificano la temperatura massima ammessa della piastra, la conversione NTC→temperatura in geometria chiusa, l'effetto di fan PWM diverso da100%, né la raggiungibilità in regime dei40°C aria.

## F. G3 — valutazione dei rischi per la V1 (OPEN)

| Argomento | Evidenza disponibile | Rischio / requisito ancora aperto |
| --- | --- | --- |
| Supervisione heater | Trip S7U40kΩ comandato e messaggio FAULT, Sanwa43°C | Validare significato termico del punto NTC; nessuna promozione40kΩ a limite finale né aumento automatico della soglia |
| Protezione termica indipendente | TCO monouso105°C riferito in serie alla piastra, aggiunto al progetto | Compatibilità col regime OEM105–118°C non verificata; dati di corpo TCO, Th/Tf/Tm e DC rating/montaggio effettivi aperti. D029: aggiunta opzionale, componente **attualmente presente da non bypassare implicitamente** |
| Ventola | Firmware fan PWM100% per entrambi i profili; fan stop fisico S7U riferito | PWM non misura RPM/portata; assenza tachimetro, fault «fanRunning» può non rilevare pale bloccate; studiare adeguata portata/ricircolo, sicurezza in guasto e condizione minima prima della regolazione autonoma |
| Elettronica heater | Comando GPIO19 va LOW per FAULT | Nessuna prova indipendente corrente realmente OFF in guasto MOSFET stuck ON; architettura di protezione appropriata da riesaminare |
| Sicurezza materiale | SHT45 misura aria in zona alta, Sanwa punto singolo su piastra | Possibili hotspot di piastra, pareti, filamento e cablaggio non mappati; posizione e fissaggio sonde da documentare senza inventare |
| Controllo termostatico | `HeaterControlPolicy.h` usa SHT45 come feedback aria e NTC come guard (una volta termicamente qualificato) | Su S7U il normale heater control è disabilitato; serve validare integrazione realistica senza impostare temp piastra=aria; distinta logica safety prima di HEAT V1 |
| Fan AUTO | UI prevede AUTO; S6C e S7U al100%; storico safe bringup al40% | Algoritmo e velocità finale non approvati. Una prima baseline costante100% è **opzione**, non decisione definitiva né prova che OEM usi100% fisso |

**G3 review effettuata, gate di sicurezza ancora OPEN.** I dati di due prove con 100% PWM non permettono scegliere fan AUTO o limite heater definitivo. Non cambiare circuito/TCO, flag enable, PWM o trip senza ulteriore review e decisione esplicita. Nessuna prova fisica autorizzata implicitamente.

## G. STEP08-C — TEST INTEGRATO HOST REALMENTE ESEGUITO / CI PASS LIMITATO

**NOVITÀ rispetto a STEP03:** test sorgente reale C++ `firmware/tests/host/test_step08_real_trace.cpp`, runner `run_step08_real_trace.sh`, workflow `.github/workflows/step08-real-trace-replay.yml`; [PR #13 DRAFT e NON MERGIATA](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/13), branch `test/step08-ntc-air-replay-2026-10-27`. Basato sul branch S7U; **non** sovrascrive il regolatore, non cambia la logica del firmware, nessun GPIO o alimentatore coinvolto.

**CI realmente verificata:** [run #40476716231](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40476716231), job ID **122096639812**, Ubuntu24.04; PASS di compilazione nativa `g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic`, controllo dei quattro flag di sicurezza originali (commissioning true; tre approvazioni/regolatore false), verifica SHA-256 e Git blob originali del CSV, test C++ e upload del log. Tutti gli step della job riportati **success**, log letto in chiaro. Nessun flash ESP32; nessuna pretesa di build firmware V1.

**Output significativi letti dal vero job log:**

```text
PASS: original 717-sample S7U CSV verified
PASS: 717 REAL samples; sequence and mock-AND commands match 237 heater_command=1 rows; raw NTC guard trips at ms=96056 despite AIR request still ON below40C. NO GPIO.
PASS: 8 NEW injected abort cases (latched + cooldown)
PASS: STEP08 HOST ONLY; no firmware flags/thresholds changed, no heater temperature calibration claimed.
PASS: STEP08 host replay completed; thermal safety G3 still OPEN
```

**Scenari e limiti esatti:**
1. Il test legge **tutte le 717 righe originali S7U** ed esegue il vero `HeaterCommissioning::Sequence` (`HeaterPulseCommissioning.h`, profilo `HeaterStep07GuardTrial.h`). Riproduce `phase`, `pulses` e `heater_command` per tutte le righe; risultano **237 campioni con comando ON** e latch `NtcResistanceLimit` a `ms=96056`. Questo verifica coerenza di replay del software con il CSV, non la corrente fisica.
2. La richiesta `HeaterControl::Policy` usa la **temperatura aria SHT45 autentica** e target di prova 40°C, ma una **temperatura locale heater sintetica di 35°C e approvazione modello fittizia** esclusivamente nell'eseguibile host. Esiste perché il NTC reale non ha ancora curva termica validata: **non attribuire 35°C all'heater, non interpretare l'host `approved=true` come firmware approvato**. Combinazione mock `pulseRequest && airWantsHeat && noFault`: a 19,52°C aria il termostato richiederebbe calore, ma la guardia RAW40kΩ ferma tutto. È **un harness di integrazione simulata**, non il percorso termostatico già attivo in `SafetyController`.
3. **Otto NUOVI test di fault-injection** sul vero `HeaterCommissioning::Sequence`: `NtcResistanceLimit`, `NtcInvalid`, `NtcStale`, `AirInvalid`, `AirStale`, `AirLimit`, `FanLost` e `ManualStop`. In ciascun caso abort latching, richiesta OFF e coolingNeeded ancora vero dopo calore. Verifica meccanismo host, non lo stato elettrico o la ventola fisica.
4. **Limitazioni esplicite dell'adapter:** lo stato `fanRunning` è simulato da un prerun di circa1s perché il CSV ha soltanto PWM comando; `ntcAgeMs` e `airAgeMs` reali non sono nel CSV e nel replay sono assunti0; nessuna misura RPM, materiale o temperatura corpo TCO. Le soglie artificiali e l'approvazione fittizia sono locali al test C++ e non scritte in `AppConfig.h`. La PR resta draft separata dal firmware reale.

**G4 / replay host = PASS limitato**: una volta verificata la precedenza tra controllo aria e veto NTC nel mock, rimane da progettare e qualificare il **vero percorso attivo** con NTC/protezione, componenti termici, fan e TCO; non abilitarlo semplicemente perché la simulazione ha PASS. Il termostato standalone STEP03 non è stato rieseguito inutilmente.

## H. Riepilogo checkpoint

- Acquisizione S7U G1: **PASS limitato** e CSV originale salvato.
- Analisi G2 confronto con S6C: **ESEGUITA / PASS come analisi**, senza qualificazione sicurezza.
- G3 review: **ESEGUITA**, salvaguardie reali ancora **OPEN**.
- G4 replay integrato host con codice C++ reale e CI: **PASS LIMITATO**; implementazione della regolazione fisica e validazione sicurezza ancora **OPEN**.
- Decisioni permanenti `docs/DECISIONS.md`: **NESSUNA NUOVA DECISIONE approvata**; non alterata.
- **Prossimo lavoro NUOVO: review G3 dei requisiti termici/protezioni e progettazione del percorso di controllo aria attuabile con interlock qualificati**, senza flash né nuova prova heater prima di decisione esplicita. La PR #13 è DRAFT e non si fonde automaticamente.
