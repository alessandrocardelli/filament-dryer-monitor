# Step 05 — safety review e candidato limitato: SOLO software finché non approvato

Data: 25 ottobre 2026.
Base sorgenti: `feature/wifi-ota-web-logging-2026-10-24` @ `3f1fcd60250e43579b2d009cadd4859c362c4755`.
Branch isolato di implementazione: `feature/step05-heater-safety-preflight-2026-10-25`.
PR [#6](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/6).
**NON equivale a test termico, autorizzazione al flash o validazione V1.** Il dryer fisico continua con l'ultimo firmware effettivamente flashato in precedenza (R3 riportato dall'utente).

## 1 — Configurazione attuale: uscita HEATER e commissioning fisicamente OFF

`firmware/AppConfig.h` conserva:
- `kHeaterCommissioningMode=false`;
- `kHeaterStage05TrialReviewed=false`, nuova condizione `static_assert(!kHeaterCommissioningMode || kHeaterStage05TrialReviewed)` che blocca il semplice cambio di un flag;
- `kHeaterOutputEnabled=false`, `kHeaterProfileApproved=false`, `kHeaterNtcModelApproved=false`, limiti del normale controllo termostatico non configurati;
- `kPostHeatCooldownMs=45000` solo **guardia candidata temporale** per il comportamento del firmware; non prova che il sistema si raffreddi fisicamente in 45 s.

Identificativo compilazione `COLD-251025-S5`. Non produrre né distribuire un'immagine che comanda HIGH su GPIO19 senza review separata, identificativo heater-enabled inequivocabile, protocollo fisico approvato e consenso espresso dell'utente. `main` escluso.

## 2 — Review degli interlock effettivi

| Percorso | Evidenza nel codice / azione | Limite residuo |
|---|---|---|
| BOOT | `SafetyController::begin()` imposta OUTPUT GPIO19 e forza LOW prima dell'inizializzazione periferiche; nessuna ripartenza ciclo autonoma da settings/NVS | Non equivale a fail-safe esterno in caso di cortocircuito MOSFET |
| STOP/COMPLETE/FAULT | `enterState()` usa `safety.stop()` e OFF immediato; la nuova memoria `lastHeaterOnMs_` conserva l'obbligo di cooldown della ventola anche dopo l'arresto/fault; al termine la ventola viene fermata in stato non-DRYING | Prove solo host/mock; rischio di ventola realmente ferma resta se PWM comandato ma rotore bloccato |
| Commissioning futuro | `HeaterPulseCommissioning::Sequence` limita prerun, impulsi, intervalli, tempo totale, NTC raw min/max, aria massima, guasti/stale e latch single run; su ESP32 `pulseTimer_` interrompe il GPIO con ceiling 2,2s se loop bloccato | Timer ISR/hardware non qualificato sotto reale guasto; necessaria verifica specifica prima di heater-enabled |
| Normale V1 | `HeaterControlPolicy.h` ha isteresi SHT45, limiti temporali, fan commanded, guasti latched | In questa build disabilitato. **Non** esiste un timeout hardware indipendente come quello dei brevi impulsi per l'eventuale regolazione continua; da risolvere prima di Step07 heater-on |
| OTA/network | Telemetria web in sola lettura, OTA richiesta in standby senza heater/cooldown/fault; callback OTA taglia heater e fan; i pulsanti e il controllo locale non dipendono da rete | Non è stato ripetuto il test fisico OTA per questa build, né richiesto |
| Logging | `ThermalLog` resta RAM-only durante test e scrive CSV LittleFS dopo cooldown. Capacity aumentata da 400 a **640 campioni da 250ms (=160s)** per non perdere l'inizio di una sessione candidata di circa 107s, con timeout complessivo 140s | Memoria e compilazione da verificare via CI; export post-run e conservazione reali ancora da validare Step10 |
| TCO aggiuntivo | Corpo monouso 105°C in serie al riscaldatore riferito dall'utente, marcatura/dati tecnici incompleti | D029: TCO opzionale come retrofit, **non** si può bypassare/rimuovere implicitamente il componente presente. Rilevare temperatura della zona e valutare caratteristiche reali prima di regime alto |
| Sanwa e cablaggi | Termocoppia Sanwa fissata secondo utente, SHT45 camera, NTC embedded heater. Sono siti di misura differenti | Contatto Sanwa e cavi 24V/sonda nel foro del jack DC da osservare nel preflight della futura prova autorizzata, senza riaprire collaudi superati |

**Nota tecnica:** il controllo fan `fan.phase()==Running` attesta esclusivamente un comando PWM; **non** attesta giri/flusso. Se la ventola fisica si blocca ma continua a ricevere comando, i limiti su aria/NTC sono l'unico feedback termico disponibile. Non qualificarne l'uso autonomo prima di valutare tale rischio.

## 3 — Primo nuovo candidato per Step06: QUATTRO impulsi, non approvato

`firmware/HeaterStage05Candidate.h` è predisposto ma non può scaldare con i flag correnti. Il profilo aggiunge **un solo impulso** ai tre già superati nel test precedente; non introduce una sequenza di riscaldamento continuativo.

| Voce | Proposta software, ancora da approvare |
|---|---:|
| Prerun ventola comandata | 5 s dopo riconoscimento fase running |
| Impulsi heater | **4 × 1,5 s**, senza autorestart |
| Gap fra impulsi | 12 s |
| Tempo massimo ON complessivo | 6 s, escludendo interruzioni anticipate |
| Cutoff hardware per singolo impulso ESP32 | 2,2 s, da rivedere fisicamente |
| Cooldown dopo ultimo impulso | 60 s nella state machine, con ulteriori osservazioni Sanwa dopo OFF |
| Timeout dell'intero profilo | 140 s |
| Abort elettrico NTC | `R_NTC <= 42,27kΩ` oppure `>=250kΩ`, invalid o stale >1,5 s |
| Abort SHT45 aria | `T_aria >=35°C`, invalid o stale >5 s |
| Abort ventola | startup non raggiunto entro 12s, comando fan perso, STOP, FAULT, timer |
| Guard nominale fan post-heat per altre modalità future | 45 s provvisori, **non garanzia termica** |

**Confronto energia storica (stima elettrica):** alla potenza precedentemente misurata di circa `24 V × 1,59 A = 38,16 W`, i 3 impulsi precedenti (~4,5 s ON) implicavano ~**172 J**; 4 impulsi (~6s ON) ~**229 J**, ossia **+57 J/+33%**. I valori sono **stima**, non misura del nuovo test né garanzia termica. Dopo la prima prova chiusa le risposte di piastra/Sanwa/NTC e postheat potrebbero essere diverse dal setup aperto/fan OFF; 42,27kΩ storici non equivalgono necessariamente a 39°C piastra reale chiusa.

**Veto manuale candidato:** se nella futura prova il Sanwa raggiunge **40°C** o mostra aumento anomalo, l'operatore **ferma subito** la prova, senza aspettare che la logica automatica riconosca il limite. Questo valore è una **soglia operativa cautelativa proposta per la sola prova iniziale**, non una temperatura massima nominale di prodotto né un limite completo di protezione. Se temperatura/sonda/contatto incerti, stop immediato. L'operatore deve poter tagliare 24 V immediatamente; l'interruzione d'emergenza prevale sul mantenimento del cooldown.

## 4 — Unica sessione fisica futura (SOLO DOPO APPROVAZIONE ESPLICITA)

**Non eseguire ora.** Prima di qualsiasi flash/test riscaldato:
1. Review del diff, CI PASS su commit immutabile e identificativo **heat-enabled distinto** da `COLD-251025-S5`; non cambiare un flag in un binario di provenienza incerta. Verificare la possibilità reale di arrestare 24 V, l'applicazione del timeout ESP32 in condizioni bloccate, i sensori e le condizioni di abilitazione; esplicitare prerequisiti residui o lasciare test NON APPROVATO.
2. Dispositivo spento/freddo: entrambi i coperchi chiudibili, cavi Sanwa/24 V dal foro jack protetti da spigoli/trazione, nessun cavo nelle pale, TCO 105°C **ancora in serie**, termocoppia ferma e a contatto presso NTC, SHT45 in camera. Ventola realmente libera, osservata durante test; no tachometro.
3. Serie a temperatura ambiente, nessun materiale sensibile esposto indebitamente, alimentazione 24 V verificata, presenza dell'operatore per l'intera prova, Sanwa leggibile e acquisizione di output seriale/CSV possibile. Predisporre STOP e stacco rapido 24 V; senza visibilità Sanwa non effettuare test.
4. Solo dopo esplicito consenso per **quel** profilo: avviare **un solo** ciclo. Non toccare/tentare misura su conduttori esposti durante l'alimentazione, non lasciarlo incustodito, non riavviare il ciclo nella stessa sessione. Interrompere alla prima anomalia di sensori/ventola/cablaggi, calore inaspettato, odore, scocca che si deforma o Sanwa >= 40°C.
5. Annotare tempi di HEAT ON/OFF; trend R_NTC, T_SHT45, RH, Sanwa sulla piastra e temperatura massima **anche dopo OFF**, non soltanto a fine impulso. Il log MCU non legge il Sanwa: annotarlo con riferimento temporale. Confermare ventola durante cooldown e assenza di riavvio heater. Se emergono discrepanze o il TCO interrompe, fermare la progressione e valutarne la causa.
6. Dopo prova, confrontare dinamica piastra/Sanwa, R_NTC e aria chiusa con storico open/fan OFF; derivare solo da qui i limiti successivi. Niente incremento automatico delle soglie e nessuna attivazione termostato autonomo.

## 5 — Esito per Step05 e gate OPEN

Lo step05 è concluso **soltanto come safety preflight e predisposizione software** quando il commit esatto avrà compile ESP32, test host e diff review PASS. Qualificazione del profilo reale, comando heater ON, ventilazione fisica e integrità termica NON verificati né autorizzati. La discussione dei parametri proposti e la separata approvazione della nuova prova avverranno **prima** dello Step06.

Per la V1 continua (Step07+) serviranno almeno una guardia NTC qualificata in Ω o una curva di termistore realmente provata, watchdog fisico per il controllo ON, cooldown coerente con Sanwa post-OFF, gestione fallimento fan/airflow, limiti materiali/TCO e verifica autonoma reale; i test con dati sintetici non bastano.
