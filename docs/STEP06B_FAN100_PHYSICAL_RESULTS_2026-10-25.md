# STEP06B — Misure reali nel box chiuso, FAN PWM 100%, Sanwa con ritardo osservato

**Data analisi:** 2026-10-25. **Tipo di evidenza:** CSV reale fornito dall'utente + letture esterne Sanwa dichiarate. **Non è una qualifica V1 a setpoint 40°C aria.** Il firmware del test è `HOT-251025-S6C` su branch separato `experiment/step06b-fan100-extended-2026-10-25` (SHA sorgente `e6a0d782eda690630f9b496c4eea1e5d37f5acaa`, [PR #10](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/10), [CI PASS #40220089554](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40220089554)). Il ramo operativo è COLD, `main` invariato.

**Tracciabilità input:** allegato utente `filament-last-test (1).csv`, **652 righe dati + intestazione**, **26.860 byte**, SHA-256 originale `90f61c153ef0ade036604536ee60d004741f5850a1313cdb95b61ac93d140e70`. File originale disponibile nella chat/progetto, **non impegnato nel repository**. Dati da 104238 a 379902 ms MCU (durata **275,664 s**). Campi `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent`; una riga è telemetria discreta, non corrente fisica o temperatura Sanwa.

## Osservazioni Sanwa riferite dall'utente — INDISPENSABILI

- **Sanwa superficie locale vicino alla piastra: 18°C iniziale, 53°C raggiunti nel riscaldamento, 30°C alla FINE del cooling**. Il valore finale 30°C **NON** dimostra che la massima dopo HEAT OFF fosse 30°C: non c'è un trend registrato Sanwa.
- L'utente sottolinea: **«il sanwa in riscaldamento misurava in ritardo rispetto al ntc»**. Preservare esattamente questa osservazione; un NTC mostra la propria **resistenza elettrica** in tempo quasi reale con campionamento 500ms, mentre Sanwa visualizza temperatura in un **punto fisico differente**, con contatto/inerzia differenti. Non dedurre dal solo ritardo Sanwa che una specifica `T_NTC` fosse nota né che la piastra fosse soltanto 53°C dappertutto. La **durata del ritardo non è misurabile** perché il Sanwa non è nel CSV con timestamp.
- Il massimo Sanwa di **53°C dista solo 2°C dalla soglia manuale operativa provvisoria 55°C**. Con ritardo di lettura e possibile gradiente spaziale, questo piccolo margine non dimostra sicurezza contro hotspot/sovraelongazione. **Non aumentare automaticamente né la durata di ON né le soglie di arresto nella prossima build**. Il veto manuale è supplementare e non sostituisce una guardia automatica NTC validata.

## Risultati CSV riproducibili

| Parametro | Misura reale dal CSV |
|---|---|
| Campioni | 652; nessun campo mancante; `air_valid=1` e `ntc_valid=1` in tutti |
| Intervallo osservato | **275,664 s** (da timestamp MCU 104238 a 379902 ms) |
| PWM fan riportata | **100% in tutti i campioni**, inclusi ON, GAP e cooling; il duty command non prova RPM/portata |
| HEATER_COMMAND | 6 finestre `1` seguite da `0`, **~20 s ciascuna** nei campioni; rappresentano richiesta firmware, NON misura indipendente di corrente o pin nel caso ISR |
| Primo ON (offset dal primo campione) | 6,094 s |
| Ultimo OFF / inizio COOLING | **188,327 s** dall'inizio dei dati |
| Ultima riga | **275,664 s**, fase ancora **4 = COOLING**, 6 impulsi completati; il CSV **non registra una fase COMPLETE (5)**, pertanto non attribuire COMPLETE se non riferito dall'utente |
| Aria camera SHT45 all'avvio | **18,64°C** |
| Aria camera SHT45 all'ultimo OFF | **22,57°C** |
| Aria camera SHT45 ultimo campione | **23,98°C** (massimo nel file) |
| Incremento aria complessivo | **+5,34°C** (18,64→23,98) |
| Incremento aria durante cooling osservato | **+1,41°C** (22,57→23,98), con HEATER_COMMAND=0, per inerzia/trasferimento ritardato |
| Umidità relativa | **64,50→53,03%** (non implica da sola perdita di acqua nel filamento) |
| NTC resistenza iniziale | **131945,3Ω ≈131,95kΩ** |
| NTC resistenza minima | **30317,9Ω ≈30,32kΩ**, nella prima riga COOLING |
| NTC resistenza all'ultimo campione | **77212,1Ω ≈77,21kΩ** (non di nuovo al valore iniziale) |
| Soglia esperimento NTC | `R_NTC <=19680Ω` stop; minima osservata **10,64kΩ superiore**, **non convertibile in margine °C senza calibrazione** |
| Soglia aria SHT45 | **≥40°C** stop; aria max23,98°C, non raggiunta |
| Profilo energetico (non misura di consumo) | 6×20s nominali ON = 120s; usando ~24V ×1,59A precedenti ≈ **4,58kJ** stimati, non misurati in questa prova |

### Transizioni e andamento aria

| Impulso | t ON (s) | t OFF (s) | T aria a inizio/fine ON (°C) | NTC a fine ON (kΩ) |
|---:|---:|---:|---|---:|
| 1 | 6,094 | 26,120 | 18,67 →18,79 | 60,44 |
| 2 | 38,418 | 58,418 | 18,97 →19,30 | 45,72 |
| 3 | 70,897 | 90,897 | 19,57 →20,03 | 39,16 |
| 4 | 102,906 | 122,906 | 20,27 →20,82 | 34,92 |
| 5 | 135,750 | 155,750 | 21,14 →21,70 | 32,43 |
| 6 | 168,327 | 188,327 | 22,05 →22,57 | 30,32 |

Il Sanwa **non è presente in questa tabella per impulso**: non inventare temperature specifiche ai vari ON senza dati dell'operatore.

### Qualità dei dati e limiti di osservabilità

- I sei ON hanno **80 campioni da ~250ms ciascuno**; la transizione GPIO fisica non è stata misurata. L'ISR dovrebbe tagliare ciascun HIGH dopo20,5s anche se loop in ritardo, ma il log da solo non ne prova la latenza effettiva.
- Nel cooling ci sono **55 campioni su 87,337s**, distanziati irregolarmente (max gap circa4,281s), perché il firmware dà servizio HTTP quando HEATER OFF e il loop può rallentare; non derivare costanti termiche millisecondo-precise da questi campioni.
- Fan 100% di comando non significa portata di progetto misurata o prova di corretto flusso nei canali; il sistema non ha tachimetro.
- La comparazione con S6A (**fan40% e 40s heater ON** contro S6C **fan100% e 120s ON**) **non isola l'effetto della ventola**, essendo cambiata contemporaneamente l'energia del riscaldatore. Non dichiarare che fan100 è più efficiente dal semplice differenziale +5,34 contro +1,01°C.
- **Distinzione fisica tassativa:** NTC embedded heater R(Ω) ≠ T superficie Sanwa(°C) ≠ T aria camera SHT45(°C). Un ritardo Sanwa rispetto alla variazione NTC non giustifica forzare a coincidenza i valori né estrapolare una temperatura interna bead.

## Correzione di tracciabilità: ritardo Sanwa/NTC GIÀ provato il 15 ottobre

**Questa è evidenza preesistente, non una nuova scoperta del 25 ottobre.** Nel repository la sezione [`docs/PROJECT_STATE.md` — `2026-10-15 Independent Sanwa thermocouple / NTC comparison`](PROJECT_STATE.md) e la sezione corrispondente di [`docs/FIRST_ARTICLE_TEST_MATRIX.md`](FIRST_ARTICLE_TEST_MATRIX.md) registravano una prova dinamica con uno sketch standalone, setpoint indicato dall'NTC 50°C, sonda Sanwa presso la piastra. Coppie **[Sanwa °C / stima NTC dello sketch °C]**: a riposo `20/16,5`; in riscaldamento `30/34,7`, `40/45,6`, al cutoff `44/50`; in raffreddamento `40/40,7` e `35/33,97`. La discrepanza dinamica raggiungeva **+6°C** nella direzione NTC stimato meno Sanwa, poi convergeva e invertiva segno durante il raffreddamento. Quella registrazione parlava **già** di ritardo della sonda/contatto **e/o gradiente spaziale** e sconsigliava di calibrare in base a coppie dinamiche. Questo significa che la frase «l'utente osserva per la prima volta Sanwa in ritardo» sarebbe ERRATA: nel test S6C ha soltanto **riconfermato** un comportamento già registrato.

La `T_NTC` del 15 ottobre **non era temperatura misurata internamente al bead**, bensì conversione da una curva empirica storica; `+6°C` non è una misura accurata della costante di tempo Sanwa, del gradiente fisico o dell'errore di calibrazione assoluto. Le letture manuali non hanno timestamp sincronizzati, dunque il **ritardo in secondi rimane ignoto** sia nel vecchio che nel nuovo test.

**Conseguenza progettuale che doveva essere applicata già prima della prova S6C:** la coppia in equilibrio `R_NTC=19,68kΩ / Sanwa vicino piastra=56°C` deriva da box **aperto**, fan OFF, hold 120s. Non può essere presa come protezione automatica certa di un veto manuale **55°C di Sanwa in una salita dinamica nel box chiuso con FAN 100%**; il precedente ritardo/gradiente era già prova sufficiente per non promettere quell'equivalenza. Il fatto nuovo di S6C è che Sanwa ha registrato max53°C a distanza solo2°C dal veto, mentre NTC minima30,32kΩ (molto sopra il trip19,68kΩ). **Mantenere bloccata l'escalation di potenza/tempo prima di qualificare una guardia automatica in Ω**; non attribuire la causa del ritardo a un solo meccanismo e non fissare una conversione precisa in °C da questi dati.

**Errore del processo di review riconosciuto:** nella predisposizione Step04→Step06B si è consultata la tabella stabilizzata dei sette punti, ma non si è recuperata in modo esplicito la sezione storica 2026-10-15 `PROJECT_STATE.md` / `FIRST_ARTICLE_TEST_MATRIX.md` sull'offset dinamico. Leggere solo i sommari recenti, senza ricercare lo storico per *Sanwa + transient/lag*, non soddisfa lo scopo delle regole AGENTS.md. Per il prossimo gate Step07, citare in modo visibile **entrambe** le evidenze (test dinamico del 15 ottobre e test S6C del 25 ottobre) prima di fissare la soglia NTC e il protocollo caldo.

## Valutazione e azione ingegneristica successiva

**STEP06B — prova termica supervisionata acquisita / analisi PASS LIMITATA**: sei impulsi da20s risultano completati nella traccia, sensori validi, FAN100 comandata, SHT45 aria salita di5,34°C, Sanwa punto piastra arrivato a53°C. Non è dimostrato setpoint40°C, loop termostatico, uniformità di camera o adeguatezza termica contro hotspot. L'utente ha riferito il Sanwa **ritardato**: questo è un **vincolo esplicito sul futuro controllo**, non un dettaglio da dimenticare.

**NEXT prioritario:** prima di allungare ulteriormente HEAT o provare regolazione automatica prolungata, revisionare **(1)** limite piastra automatico in Ω con margine giustificato nelle condizioni fan100/box chiuso, **(2)** scelta conservativa del tempo massimo ON e afterheat in presenza di ritardo Sanwa, **(3)** fan running *comandata* ≠ flusso reale e limiti elettronici, **(4)** osservazione Sanwa sincronizzata (timestamp) in eventuale prova futura, e **(5)** gate hardware timer/safe stop. Non usare la sola soglia Sanwa55°C come protezione real-time, soprattutto in un test più lungo. Non abilitare semplicemente `kHeaterOutputEnabled`/`kHeaterNtcModelApproved` o promettere setpoint40°C senza test. Non ripetere S6C per aggiungere un altro campione se il limite Sanwa non è stato riesaminato. Le decisioni progettuali D025 (fan40% bringup non definitivo) e D029 (TCO separato opzionale, TCO105 presente in prototipo non bypassato) restano invariate fino a motivata revisione.

## 2026-10-25 — CSV grezzo conservato nella repository

È stata aggiunta la copia **testuale** dei 652 campioni dell'allegato reale S6C a [`docs/evidence/thermal/2026-10-25-S6C-fan100.csv`](evidence/thermal/2026-10-25-S6C-fan100.csv). L'upload originario (file `filament-last-test (1).csv`) ha SHA256 bytes `90f61c153ef0ade036604536ee60d004741f5850a1313cdb95b61ac93d140e70`; la copia testuale nel repository può differire nella codifica dei fine-riga, ma i **652 campioni, i campi e i valori numerici sono conservati**. Questo impedisce che i prossimi test/analisi si basino soltanto sui resoconti di chat. La nuova suite S7G userà direttamente questo file come input di replay.
