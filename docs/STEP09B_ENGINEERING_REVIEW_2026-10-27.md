# STEP09-B — Review ingegneristica dei dati S9A e scelta candidata del budget180s

> **Identity correction 2026-10-27:** the sole active draft S9B 180-second firmware is **`HOT-261027-S9B180` / OLED `H27-B180`** in [PR #17](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17). Superseded [200s PR #15](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/15) and duplicate [PR #16](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/16) are **CLOSED WITHOUT MERGE**. Older images labeled `H27-S9B` are **AMBIGUOUS** and MUST NOT be flashed. Real physical S9B still UNRUN; new operator approval for 180s budget and physical setup review still REQUIRED; NO flashing until latest firmware identity CI PASS.

> **CI S9B SOFTWARE ESEGUITA E PASSATA (non prova fisica):** [Actions run #40562778719](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40562778719), job `122395977499`: test C++ sul `SafetyController` reale con GPIO simulato **PASS** (incluso precedente120s NON termina e nuovo180s termina con cooldown, target aria25°C/isteresi, NTC soft50/resume65/hard40, fault/STOP/stale); compilazione Arduino ESP32 **PASS** con 1.126.854 byte FLASH (85%) e 92.096 byte RAM statica (28%), senza overflow. Commit testato `ca5c0e5bf6ea3f9cccd531456649e384c19eda6e`; SHA blob di tutti i file firmware e workflow verificati identici fra tale commit e il branch dopo le aggiunte dei soli documenti. **Non abilitato né installato su prototipo e nessuna qualifica di sicurezza fisica:** `H27-B180` è solo l'identità che l'utente dovrà verificare dopo un eventuale flash, previo consenso specifico alla prova. [PR #17 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17), base branch S9A PR14.

**Stato del 27/10/2026:** review quantitativa e review del perimetro del codice ESEGUITE; **nuovo test fisico S9B NON ESEGUITO, limite180s non ancora esplicitamente approvato dall'utente, margini/qualifica hardware non chiusi**. Questo documento non attiva automaticamente HEATER, non autorizza nuovi limiti V1, non rimuove alcun TCO.

## Introduzione obbligatoria (cosa / perché / prima / futuro)

**COSA:** verificare se un unico aumento del budget cumulativo HEATER ON da120 a180 secondi, senza cambiare NTC, fan o setpoint, permetterebbe alla camera eBOX Lite di arrivare all'obiettivo SHT45 25°C in una nuova prova sorvegliata.

**PERCHÉ:** in S9A il ciclo ha concluso senza FAULT al budget120s con aria22,59°C, e il target25°C non è stato raggiunto. È necessario osservare la decisione reale del termostato alla soglia, che S9A non ha messo alla prova.

**PRECEDENTE:** [CSV grezzo S9A](evidence/thermal/2026-10-27-S9A-raw.csv), [Sanwa13 finestre](evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md) e [analisi S9A](evidence/thermal/2026-10-27-S9A-analysis.md). S9A: 778 righe; aria17,64°C primo campione,22,59°C al COOLING,22,97°C picco cooling; circa120s comandati ON; R_NTC minimo48,7167kΩ, nessun FAULT; Sanwa17°C iniziale, tra35–38°C durante le finestre5–12 e26°C a fine cooling. S6C con120s ON ha prodotto +5,34°C di incremento aria e max Sanwa53°C con soglie diverse, quindi la «stabilità» Sanwa S9A NON dà certezza sulla sicurezza per un test più lungo.

**FUTURO:** se test e revisioni software PASS e operatore approva espressamente l'esposizione180s, un unico test reale S9B con Sanwa e CSV, poi rivalutazione dei dati prima di qualsiasi aumento a>25°C di aria.

## Evidenze numeriche e motivo del valore candidato

- S9A ha accumulato circa120s ON in **307,6s di tempo effettivo fino a COOLING**. Gli ultimi impulsi sono durati soprattutto circa8s, limitati dalla pausa NTC50kΩ, anziché i20s massimi. Nessun dato suggerisce che si raggiungesse25°C esattamente al termine S9A: mancavano **2,41°C rispetto a22,59°C alla transizione**, o2,03°C rispetto al picco22,97°C in cooling.
- In S9A l'aria è cresciuta da17,64 a22,59°C mentre il software comandava HEATER, con una ulteriore salita di+0,38°C al picco raffreddamento. Negli ultimi60s attivi la crescita della camera tende a rallentare, quindi **NON assumere una proporzionalità affidabile fra i secondi ON e la crescita SHT45**.
- Il profilo **180s** aggiunge al massimo60s ON cumulativi (**+50%**) alle finestre già sorvegliate dal controllo NTC. Con duty simile a quello di S9A potrebbe necessitare una quota significativa dei **292,4s residui** prima del limite di runtime600s. È un tentativo numericamente motivato, **non una previsione certa di raggiungere25°C** né un massimo sicuro di temperatura.
- La sessione massima rimane **600s prima di COOLING**; 90s di post-cooling portano la finestra registrabile teorica a <=690s. Il buffer esistente da **1400 campioni a intervallo nominale500ms copre fino700s**, senza aumentare consumo RAM (il precedente overflow da3000 campioni in S9A è documentato nel report storico).
- Ogni HEATER ON dura max20s comandati (timer ESP32 ISR cutoff20,5s), pausa OFF min12s. A livello di codice il limite180s è un budget sommato nei `tick` quando la fase è HEAT: **NON è wattora misurati e le durate ON da CSV possono essere distorte dalla frequenza di campionamento**. La corrente/potenza reale non è presente nel log.
- Target ARIA25°C; ON possibile solo con SHT45<=24°C e altri interblocchi; OFF a>=25°C. Se la soglia è raggiunta durante PAUSA NTC, il comando heater era già OFF: **questa situazione permette di verificare il blocco di riaccensione, non lo spegnimento «causato» dal termostato**. Non attribuire causalità senza guardare `phase`, `heater_command` e NTC.

## Sicurezza e limiti del setup — esito review

| Controllo | S9B candidato | Evidenza e limite |
| --- | --- | --- |
| Veto locale termistore | Soft OFF<=50kΩ; resume>=65kΩ; hard FAULT<=40kΩ oppure circuito aperto/invalido | Identico S9A; la resistenza non è una temperatura bead calibrata. Il soft impedisce riscaldamento fino al punto NTC più caldo già visto in S6C, ma NON garantisce ogni punto materiale |
| Sanwa a contatto piastra | **STOP manuale se >=50°C**, sempre sorvegliato | S9A misure discrete max38°C al contatto. Ulteriori60s ON in più possono modificare l'equilibrio e non sono dimostrati sicuri da sole |
| TCO indipendente | **105°C riferito montato in serie**, invariato | Componente opzionale ai sensi D029; integrità/montaggio e DC rating non qualificati in questo report. Non bypassare o sostituire automaticamente |
| Fan | PWM100% comandato + kickstart/prerun/cooldown90s invariati | S9A mostra fan_percent100 in tutte le righe, **nessun RPM/flow/tach**: conferma fisica prima della prova obbligatoria |
| Fault GPIO/timing | MAX20s finestra con ISR20,5s, RUN<=600s, CUM<=180s, NTC/AIR fault latched | Necessita build ESP32 e mock di regressione reali per branch S9B; non supera fault MOSFET stuck ON non misurabile |
| Camera/setpoint | SHT45 target25°C, air hard limit40°C, sensors age/invalid and fan command loss | I 25°C non sono mai stati raggiunti in S9A, quindi nessun PASS reale del cutoff per target |
| Dati | buffer1400 a500ms + snapshot LittleFS solo a heater OFF/cooling completato | ~700s copertura teorica, nessuna garanzia di rete/OTA nella prova; non ripetere HEAT solo per mancato download |

**Conclusione della review:** il **solo aumento180s** è una **proposta sperimentale controllata per studiare il comportamento al target**, con variabili locali ferme e sorveglianza umana, *non* un nuovo limite di progetto qualificato. L'estensione di esposizione termica è una modifica reale; se Sanwa cresce verso50°C, il test finisce anche se non raggiunge25°C. Prima di eseguirlo l'utente deve accettare il nuovo budget e controllare il montaggio. I limiti operativi V1 e le decisioni D0xx restano immutati.

## Gate

1. Codice S9B solo branch `experiment/step09b-air25-budget180-2026-10-27`, base S9A PR14, senza merge; firmware ID **`HOT-261027-S9B180` / OLED `H27-B180`**.
2. Test host reale SafetyController su **180s cum**, vecchio budget120s **NON** fermante, ingresso in COOLING e cooldown90s, contatto Sanwa non simulato, fault NTC/air/fan/stale e STOP. CI effettiva deve risultare PASS, anche compilazione ESP32 e consumo DRAM, sul commit specifico.
3. **Nuovo consenso esplicito** sull'estensione a180s e conferma preflight fisico prima di qualunque attivazione.
4. [Protocollo completo con cronologia Sanwa non ambigua](STEP09B_REAL_AIR25_FEEDBACK_PROTOCOL_2026-10-27.md), [modulo evidenze S9B NON ESEGUITO](evidence/thermal/2026-10-27-S9B-PHYSICAL-SESSION.md).
5. Nessun intervento reale/OTA comandabile dall'assistente; la prova viene svolta dall'operatore sul dryer, con dati successivamente riportati. Nessun «eseguito» dichiarato da CI.

