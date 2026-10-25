# Project state

> **2026-10-25 — STEP 01 PASS SOFTWARE ([GitHub Actions PASS #40175231036](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40175231036)):** il firmware `WIFI-241024-R4` della PR [#4](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/4) compila con ESP32 core 3.3.12 e U8g2 2.36.19; eseguiti con successo mock controller, 14 test policy e 12 test pulse. GitHub Actions conferma file `AppConfig.h`, `SafetyController.h`, `HeaterPulseCommissioning.h`, `FanController.h` identici al branch operativo. Unica correzione funzionale visibile: SHT45 temperatura aria distinta da NTC resistenza della piastra, senza pubblicare T NTC non qualificata; l'NTC interno non è stato calibrato. **Resta OPEN il controllo visivo su PCB del nuovo OLED/web**; nessun flash, prova termica o merge. **NEXT: step 02 preparazione fisica a dispositivo spento; step 03 sviluppo termostato OFF simulato possibile in parallelo.**

> **2026-10-24 — piano finale operativo definito:** [EXECUTION_PLAN_2026-10-24.md](EXECUTION_PLAN_2026-10-24.md) traccia 11 step con responsabili, evidenze e gate. Nessuna modifica tecnica/flash/test caldo da questa pianificazione. Prima azione: rimuovere qualsiasi falso confronto automatico tra temperatura SHT45 (aria camera) e temperatura derivata NTC (piastra), conservando NTC Ω; poi simulazioni termostato senza GPIO e preparazione fisica. Branch operativo Wi-Fi preservato, firmware a impulsi ancora sperimentale.

> **2026-10-24 — housekeeping repository eseguito:** PR #3 `feature/heater-policy-cold-tests` → `main` mergiata commit `78278575dd97cb7b244ffe46df1bd87f80b73221`. I blob `firmware.ino`, `AppConfig.h`, `SafetyController.h` in `main` sono risultati identici prima/dopo merge. Verificati e cancellati **6 branch** (5 già ancestrali a main/branch attivo, più `archive/recover-tested-firmware` già mergiato PR #2 e stessa cartella firmware/test_archive). Restano **6 branch**, di cui 4 con modifiche non integrate da preservare. Branch operativo e source of truth per il prototipo **`feature/wifi-ota-web-logging-2026-10-24`**; `main` non contiene ancora il firmware sperimentale/OTA. Obiettivo utente: completare controllo temperatura SHT45/NTC e fail-safe; storico 12h rinviato. [Audit](BRANCH_CLEANUP_2026-10-24.md).

> **2026-10-24 — aggiornamento su prototipo:** Arduino IDE 2.3.10 ha riconosciuto e usato con successo la porta ESP32 via Wi-Fi, come confermato dall'utente. Web OTA e monitor SHT45/NTC funzionano. Priorità corrente: completare la regolazione termica reale del dryer, non uno storico di 12 ore. Rimossi dal branch i sorgenti della funzione ambient-history anticipata senza autorizzazione; codice R3 collaudato preservato. Controllo continuo riscaldatore non ancora attivo: `kHeaterCommissioningMode=true`, `kHeaterOutputEnabled=false`, calibrazione NTC e limiti definitivi aperti. Prossima fase C/D/E: NTC, regolazione camera SHT45, arresti e test a caldo graduati.


> **2026-10-24 — ArduinoOTA IDE implementato nel sorgente Wi-Fi, branch `feature/wifi-ota-web-logging-2026-10-24`, ID `WIFI-241024-R3`.** Il Web OTA R2 è stato collaudato fisicamente dall'utente via Edge e Chrome incognito, e la dashboard mostra le nuove etichette SHT45/NTC. Aggiunti `ArduinoOTA` (core ESP32) su porta UDP 3232, password NVS OTA esistente, unico proprietario mDNS `filament-dryer` con servizio HTTP + `_arduino._tcp` e log `IDE OTA READY/START/COMPLETE/ERROR`. Il metodo `ArduinoOTA.handle()` viene eseguito **solo** con `canOta` già determinato dall'app (STANDBY, nessun ciclo, heater OFF, nessun cooldown/fault); `onStart` forza ulteriormente `safety.stop()` e `fan.stop()`; azioni pulsante bloccate durante trasferimento e reboot. **Non** cambiati parametri heater, TCO, timer o GPIO. La CI compila anche la nuova libreria e produce il `.bin` web-OTA, ma **discovery della porta IDE, primo upload ArduinoOTA, guasti rete e flash CSV restano da validare in hardware**. Il primo R3 si installa dal Web OTA già verificato; non chiedere di rifare precedenti test. [Guida](WIFI_OTA_WEB_2026-10-24.md).

> **2026-10-24 — primo upload OTA fisico riuscito:** utente ha utilizzato Microsoft Edge per caricare il firmware `WIFI-241024-R2` sulla ESP32 via `/ota`. La dashboard al riavvio mostra effettivamente `Temperatura camera (SHT45)`, `Umidità camera (SHT45)`, `Resistenza piastra (NTC)`. Web OTA e aggiornamento in campo PASS, sulla rete hotspot già collaudata. **Chrome ha mostrato HTTP 401** sulla medesima pagina OTA, Edge ha funzionato; nessuna causa tecnica accertata, né modifica di sicurezza autorizzata. La funzione `ArduinoOTA` per caricare direttamente dalla porta di rete Arduino IDE **non è presente nel sorgente**; WebServer/Update è un protocollo distinto. Compilare in IDE ed esportare .bin è già possibile; upload diretto tramite Arduino IDE richiederà sviluppo con interlock heater OFF e autenticazione. Il CSV persistente non è ancora testato sulla PCB.

> **2026-10-24 — primo collaudo Wi-Fi sulla PCB confermato dall'utente:** rete del telefono/hotspot `gina` configurata su ESP32, pannello web e diagnostica visibili dal PC tramite IP e nome mDNS `filament-dryer.local`, API sensori/status/events funzionanti. Nuovo checkpoint sorgente firmware `WIFI-241024-R2`: tre etichette della dashboard esplicite per SHT45 camera temperatura e umidità e NTC della piastra; compilazione automatica predisposta per pubblicare l'app `.bin` su GitHub Actions e poter testare OTA senza riaprire. Il test **OTA in hardware e lettura post-reboot di CSV LittleFS rimangono OPEN**. Controllo termico sperimentale invariato; non attribuire verifica degli impulsi al nuovo R2. [Manuale](WIFI_OTA_WEB_2026-10-24.md).

> **2026-10-24 — implementato branch WiFi/OTA/dati:** `feature/wifi-ota-web-logging-2026-10-24` da `test/closed-case-heater-pulses-2026-10-23`. `firmware/NetworkService.h` offre provisioning locale AP WPA2 con password casuale mostrata su OLED, client WiFi STA, web status/events/history, credenziali in Preferences NVS e OTA HTTP Basic admin solo con ciclo idle / heater OFF; `firmware/ThermalLog.h` registra 400 misure in RAM ogni 250 ms e salva `/last-test.csv` in LittleFS al termine del cooldown. UI mostra ID `WIFI-241024-R1` e IP/setup. Nessun intervento hardware, parametro termico o TCO. Firmware ancora sperimentale 3x1.5 s. La compilazione ESP32 e i test host in GitHub Actions non sostituiscono verifica RF, provisioning e OTA su PCB; devono essere marcati separatamente. [Procedure](WIFI_OTA_WEB_2026-10-24.md).

> **2026-10-24 — risultato test reale precedente:** l'utente conferma firmware `PULSE-241024-R1` verificato sul display, 3 HEATER ON, COMPLETE, Sanwa piastra da 19 °C a massimo 23 °C (+4 °C), nessuna anomalia. Questo valida soltanto il primo test termico breve, **non** la curva NTC, flusso ventola misurato o essiccazione continuativa. Non richiedere di ripetere il collaudo a tre impulsi. [Verbale](FIRST_CLOSED_HEATER_PULSE_TEST_2026-10-23.md).

> **2026-10-24 — diagnosi onboard test heater, branch `test/closed-case-heater-pulses-2026-10-23`:** l'utente segnala che con l'ultimo firmware caricato il riscaldamento non è evidente; manca una conferma del firmware effettivamente flashato e non vi è log USB possibile a scatola chiusa. Aggiunto `firmware/FirmwareBuild.h` con ID **PULSE-241024-R1**, mostrato 3 s su OLED dopo il logo, sigla P24-R1 in STANDBY e seriale; schermate DRYING e FAULT riportano fase di commissioning e causa del blocco. Il riscaldamento resta il test separato di tre impulsi da 1.5 s, NON controllo continuativo. Non si è dedotta la causa prima di osservare la nuova schermata. Verifica di compilazione CI della modifica da associare al commit. [Test sheet](FIRST_CLOSED_HEATER_PULSE_TEST_2026-10-23.md).

> **2026-10-23 — heater physically energizing code prepared/tested, not yet flashed:** branch `test/closed-case-heater-pulses-2026-10-23` adds actual bounded GPIO19 pulse experiment with fan 5 s prerun, three 1.5 s heater pulses, 12 s gaps, 45 s cooling, raw NTC and SHT45 abort gates and ESP32 HW timer at 2.2 s. [CI 40022340102 SUCCESS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40022340102) Arduino ESP32 + U8g2 compilation and native tests: normal mock OFF=0 HIGH, enabled fixture HIGH=1, 14 policy scenarios, 12 pulse scenarios. No thermal data yet from closed dryer, no physical validation, no claim V1 is safe/complete. User's J5 can stay connected during this intentionally supervised test; do not bypass installed 105 C. TCO purchase remains optional D029. [Test sheet](FIRST_CLOSED_HEATER_PULSE_TEST_2026-10-23.md).

> **2026-10-23 — firmware heater in build ESP32 verificata:** [CI build+host SUCCESS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40009733825); Arduino core 3.x, U8g2, test host. Sorgente `firmware/firmware.ino` integra GPIO19 tramite SafetyController/HeaterControlPolicy, display stato e fault; **GPIO19 OFF nella build predefinita**. Nessun flash e test hardware eseguito. Calibrazione NTC e validazione fisica restano aperte, TCO opzionale D029. [Rapporto](HEATER_FIRMWARE_INTEGRATION_2026-10-23.md).

> **2026-10-23 — ramo firmware integrato `feature/integrated-heater-controller-2026-10-23`:** `SafetyController.h` collega la policy di regolazione a GPIO19/Q4 con controlli NTC/SHT45, prerun ventola, deadline, fault latch e stop. `firmware.ino` aggiornato nel loop, OLED e serial riportano HEATER LOCKED/OFF/ON, guardia statica in `AppConfig.h`. **Il codice è completo come integrazione sorgente ma la build reale resta heater-LOCKED**, valori termici non approvati; ESP32 build non verificata, flash e calore non eseguiti. GNU g++ mock OFF/fixture e 14 test policy PASS. La fase C di calibrazione non è ancora superata; D integrazione sorgente completata, collaudo target aperto. Il TCO è opzionale D029, quello montato da 105 °C è invariato. [Dettagli](HEATER_FIRMWARE_INTEGRATION_2026-10-23.md).

> **2026-10-23 — nuova attività C/D, branch feature/heater-policy-cold-tests-2026-10-23:** aggiunti modulo teorico senza attuazione firmware/HeaterControlPolicy.h e 14 casi host sotto firmware/tests/host; compilazione g++ nativa PASS. I 7 valori NTC/Sanwa non sono vera calibrazione bead; punto freddo 122.3 kΩ a ~20 °C ancora inconciliato con fit a caldo. Nessun flash, test a caldo, modifica ESP32 principale o KiCad. Prossimo: geometria sensori chiusa a freddo e, se praticabile, secondo punto di equilibrio del NTC. TCO opzionale D029, quello montato 105 °C non alterato. [Rapporto tecnico](HEATER_CONTROL_COLD_STAGE_2026-10-23.md).

> **2026-10-23 — CORREZIONE D029:** TCO separato *non presente nell'originale secondo quanto riferito dall'utente*; quello introdotto è **protezione opzionale**, non componente necessario al funzionamento né gate obbligatorio all'intero percorso di qualifica termica. La precedente D028 imponeva erroneamente la B come blocco assoluto per le prove heater. Proseguire A (meccanica), C (NTC/fault), D (controllo sicuro revisionato), E (prove termiche progressive previa approvazione); G logging/web in parallelo. Resta vero che un 105 °C è fisicamente cablato in serie sul prototipo: per una prova calda considerare i suoi limiti e non bypassarlo automaticamente. Nessuna nuova abilitazione heater o test eseguito. [Decisioni D027–D029](DECISIONS.md).

> **2026-10-23 — decisione D028 sul flusso di lavoro:** TCO/protezione esterna resta da qualificare per prove con calore e funzione autonoma, ma **non blocca** sviluppo e verifiche senza riscaldamento (UI/fault simulati, logging CSV, web locale, analisi sensori, predisposizione fisica a freddo). Codice `firmware/SafetyController.h` mantiene heater hard-OFF; nessuna modifica a firmware/KiCad, né prova a caldo o bypass autorizzata. Operativamente prioritario un branch separato per logging/web heater-OFF, mentre gate A1 e B restano OPEN. Documento [piano aggiornato](COMPLETION_PLAN_2026-10-22.md); decisioni D027–D028.

> **2026-10-23 — TCO, acquisto ora identificato:** prima scelta **MICROTEMP G4A00152C da WTT Elettronica**, articolo FT152G, [eBay291602208480](https://www.ebay.it/itm/291602208480). **1 pezzo7,75 EUR nel carrello verificato =1,85+5,90 trasporto**, oggetto **Italia**, stima27–29 ottobre al CAP48018. Il sito dello stesso WTT dichiara esplicitamente **G4A00152C** e lo stesso EAN8029564057789: risolto il precedente dubbio sul codiceTC152. Trasporto5,90 è un compromesso esplicito; nessun acquisto. [Fonti, prove e limiti](TCO_PURCHASE_DECISION_2026-10-23.md). D027 invariata (Tf152/Th137/Tm205;5 A/24 V DC); qualifica termica ancora OPEN, sviluppo heater-OFF prosegue secondo D028.


Checkpoint date: **2026-10-04**  
Active hardware branch: **`main`**  
Fabrication-source checkpoint (before documentation updates): **`74a3000127371ac6c3b3b7197f9036dec5b58eef`** (`Production files`, 2026-09-17). Documentation-only commits after this checkpoint do not change the released hardware.

## Executive state

The L7987L + AutoEN revision was released for a **bare-board, manual-assembly prototype**. The stored current KiCad reports date from 2026-09-17: ERC **0 errors / 0 warnings**; DRC **0 active errors / 0 unconnected pads / 0 footprint errors**, with **one excluded** U4 B.Silkscreen-to-board-edge warning. The stored netlist was regenerated on 2026-09-17 and has the corrected USB-C D+/D− mapping. The released production BOM and position file are `hardware/production/Filament_Dryer_Monitor_bom.csv` and `hardware/production/Filament_Dryer_Monitor_positions.csv`; the unsuffixed `hardware/production/bom.csv` / `positions.csv` are legacy AP66200-era exports and **must not** be used for the current design.

Gerbers/drills were uploaded to JLCPCB for bare-PCB production. Its production/CAM package was received and checked against the uploaded Gerbers in the order discussion. **On 2026-10-01 the user reported that the bare PCBs and component shipment had arrived.** First-article hand assembly and staged bring-up are now in progress. Gate 1A, isolated buck, AutoEN normal-operation, 3V3_MCU rail-link, and Gate 3 MCU power/reset/boot checks have passed as recorded below. USB enumeration at Gate 3 currently fails at the device-descriptor stage. No PCBA/stencil service is planned; all components are hand-assembled.

**Procurement (2026-09-21):** the live `BOM TME` reconciles to the current production BOM/netlist with 94 PCB-mounted component references for one assembled board: 61 references in 43 TME purchase rows and 33 references on 14 rows marked `In casa`. TME order was placed 2026-09-20; shipment and receipt are unconfirmed. **C11, Samsung CL21A226MAYNNNE (22 µF / 25 V X5R / 0805), is an open supply issue.** The order confirmation listed it as available; TME later reported its warehouse unit could not be located and is searching for it. TME offered a cancellation/refund or reordering route, but no cancellation, replacement or revised PCB MPN has been authorized. TME's later "week 48/2026" delivery notification does not establish that the rest of the order is delayed; dispatch of other in-stock items remains **unconfirmed**.

A candidate alternative mentioned by TME is TDK `C2012X5R1C226M125AC` (TME catalog code `C2012X5R1C226MAC`), 22 µF / 16 V X5R / 0805. It is **not ordered, not installed and not a committed BOM change**. The preferred next step is TME's definitive warehouse and shipping reply, not a second paid-shipping order. Keep the current C11 in schematic and BOM until an actual supply decision is reached.

**Outside the PCB purchasing BOM:** the OLED fitted at J4, off-board SHT45 sensor, independent heater thermal cutoff, mating connector housings/terminals/cables, and mounting hardware need a separate inventory check; their physical availability is not established by the 94-reference PCB BOM. J5 and 13 resistor rows are marked `In casa` in the sheet, but their actual packages/values/quantities still need confirming at assembly. Follow `docs/ASSEMBLY.md`: R5 is deliberately unpopulated at first 3.3 V power-up and fitted after the initial rail check.

The historical D012/C22 buck-bypass conflict remains closed (D018): C1 10 µF / 100 V covers VIN, C3 1 µF / 100 V covers VCC, and C22 now belongs exclusively to the CP2102 REGIN bypass. No circuit decision is changed in this documentation checkpoint.

## Sources inspected for this checkpoint

- Active branch `pcb/l7987l-layout`, fabrication-source commit `74a3000`; `AGENTS.md`, `README.md`, `docs/PROJECT_STATE.md`, `docs/DECISIONS.md`, `docs/TODO.md`, `docs/ASSEMBLY.md`, `docs/BUCK_L7987L_DESIGN.md`, `hardware/docs/PROCUREMENT.md`.
- Actual design/production source: `hardware/Power.kicad_sch`, `hardware/MCU.kicad_sch`, `hardware/Filament_Dryer_Monitor.net` (2026-09-17), `hardware/Filament_Dryer_Monitor.kicad_pro`, the `hardware/Filament_Dryer_Monitor.kicad_pcb` identity and released production files, `hardware/ERC.rpt` and `hardware/DRC.rpt` (2026-09-17). The whole PCB layout was not re-reviewed in this documentation-only checkpoint; the stored DRC and prior CAM review are the cited evidence.
- Live Google Sheet `Filament Dryer Monitor — BOM finale Mouser`, tabs `BOM TME` and `TME_IMPORT`, inspected 2026-09-21; original TME order confirmation dated 2026-09-20 and supplier correspondence of 2026-09-21.

The actual KiCad source files override descriptions, and supplier order/ship status must be confirmed directly rather than inferred from purchase-list availability.

## Electrical design state

The accepted L7987L design basis remains:

| Ref | Value / part | Function |
|---|---|---|
| U5 | ST L7987L | 24 V to 3.3 V asynchronous buck |
| U1 | TLV1701AIDBVR | AutoEN fault comparator |
| Q7 | MMBT3904 | EN pull-down |
| L1 | SRN6045-150M, 15 µH | Buck inductor |
| D7 | PMEG6030EP,115 | Catch Schottky, Nexperia 60 V / 3 A, CFP5/SOD-128 |
| C1 | 10 µF / 100 V X7S | Main local input ceramic |
| C3 | 1 µF / 100 V X7S | Current local VIN/VCC bypass present in source |
| C10 | 47 µF / 10 V X7R | Main pre-bead output capacitor |
| C21 | 1 µF / 25 V | VBIAS bypass |
| R4 | 47 kΩ | FSW, about 516 kHz nominal |
| R29 | 47.5 kΩ | ILIM, about 1.705 A nominal |
| R33/C9/C8 | 16 kΩ / 18 nF / 39 pF | Type-III compensation branch |
| R35/C20 | 1.13 kΩ / 560 pF | Type-III compensation branch |
| R36/R34 | 49.9 kΩ / 16 kΩ | Feedback divider, about 3.295 V nominal |

Output architecture remains `3V3_BUCK -> FB1 -> 3V3_MCU`. `PGOOD` and `SYNCH` remain intentionally NC.

The 2026-09-04 design calculations/review remain accepted: nominal ILIM about 1.705 A, engineering selection envelope about 1.42–2.15 A, and recorded compensation simulation about 59.1 kHz crossover / 64.9° phase margin / 19.6 dB gain margin.

### VIN/VCC local ceramic requirement — resolved

The original D012 interpretation treated ST's VIN and VCC bypass guidance as requiring two additional dedicated 1 µF / 100 V parts. The later review corrected that interpretation.

ST requires:

- a ceramic capacitor of **1 µF or higher** across VIN and power GND, close to U5;
- a ceramic capacitor of **1 µF or higher** across VCC and IC/signal GND, close to U5.

Current implementation:

- **C1 = 10 µF / 100 V X7S** is the local VIN ceramic and therefore already exceeds the VIN-side minimum;
- **C3 = 1 µF / 100 V X7S** is the local VCC bypass.

The extra 1 µF / 100 V buck C22 added in commit `6c6414b3` was redundant and was deliberately removed in commit `24a9170a`. Decision D012 is retained only as historical context and is superseded by D018 in `docs/DECISIONS.md`.

Current C22 belongs to the MCU sheet and is the CP2102 REGIN 1 µF / 25 V capacitor.

## MCU / USB state

### CP2102-GM

U2 is the classic CP2102-GM QFN28 implementation.

Current custom footprint state:

- perimeter pads: **0.95 × 0.28 mm**;
- exposed pad: **3.25 × 3.25 mm**;
- footprint solder-mask expansion: **+0.06 mm**;
- exposed-pad paste: **3 × 3 apertures, 0.9 × 0.9 mm**;
- U2 pad 3 and exposed pad 29 are GND.

The new REGIN decoupling capacitor is **C22 = 1 µF / 25 V / X7R / 0603, Murata `GCM188R71E105KA64J`**, connected between `3V3_MCU` and GND.

### USB differential routing

Accepted/configured USB geometry:

- signal layer: **B.Cu**;
- reference: **In2.Cu**;
- B.Cu-to-In2 dielectric: **0.10 mm FR4, Er 4.5**;
- target: **90 Ω differential**;
- track width: **0.20 mm**;
- differential gap: **0.25 mm**;
- tuning profile: **`USB_90R`**.

Current PCB mapping:

- U2 pin 4 = `USB_D+`;
- U2 pin 5 = `USB_D-`;
- U3 pin 1 ↔ pin 6 = D+ channel;
- U3 pin 3 ↔ pin 4 = D− channel;
- J2 A6/B6 = `USB_CONN_D+`;
- J2 A7/B7 = `USB_CONN_D-`;
- J2 GND pins and shield are GND.

U2 -> U3 D+/D− stays on B.Cu with **no vias**. On the short Type-C breakout, D+ uses **two signal vias** for the duplicated-pad crossover; D− remains on B.Cu. A nearby GND stitching via supports the reference-layer transition.

### Stored netlist is stale

`hardware/Filament_Dryer_Monitor.net` was regenerated on **2026-09-17 21:13:01** from the current schematic, after the earlier USB correction. The actual netlist contains J2 A6/B6 on `/MCU/USB_CONN_D+` and J2 A7/B7 on `/MCU/USB_CONN_D-`, consistent with the current schematic/board mapping above. The older warning about the 2026-09-16 reversed USB netlist is historical and **resolved**. USB enumeration and signal integrity still require first-board testing.

## PCB state

Current board file: `hardware/Filament_Dryer_Monitor.kicad_pcb`.

Current B.Cu/local-zone implementation includes:

- `24V_PROT` zones;
- local `/Power/3V3_BUCK` zone;
- local `3V3_MCU` zone;
- `/IO_Power/HEATER_SW` zone;
- controlled LX / `Net-(D7-K)` zone;
- local GND zones;
- full-board GND planes on In1.Cu and In2.Cu.

All nets are presently connected according to the fresh DRC. That closes the **connectivity/ratsnest** phase, not the engineering review/release phase.

### Power-distribution policy

Decision D017 records the power-copper policy for the current layout:

- `/Power/3V3_BUCK` stays a **compact local B.Cu copper area** around the buck output/C10/L1/FB1 path;
- `3V3_MCU` may remain **primarily 0.50 mm Power_3V3 traces after FB1**. The local zone currently present in the PCB is optional layout copper, not a requirement for a broad 3.3 V plane;
- `24V_PROT` may use wide traces and local pours where current/voltage-drop needs justify them;
- `LX` and other switched nodes remain geometrically compact;
- both inner layers remain uninterrupted GND.

Capacitance from an external DC power area to the nearby GND plane is not considered a reason to avoid the area. The relevant parasitic/EMI concern is excessive copper on fast switched nodes, plus any optional pour crowding the USB pair or ESP32 antenna keepout.

### U5 exposed pad — resolved for planned assembly

U5 is centered at approximately `(174.649, 85.116)` on B.Cu. Pad 17 is the **3.2 × 3.2 mm** GND/SGND exposed pad.

The accepted implementation is deliberately **not via-in-pad**. Six GND vias, each **0.60 mm diameter / 0.30 mm drill**, are placed immediately outside the pad in two rows of three:

- one row at approximately y = 82.9 mm;
- one row at approximately y = 87.35 mm;
- x positions around 173.75 / 174.65 / 175.55 mm.

This geometry was implemented in commit `c9dc4129` (`updated vias on U5 and D7`) and remains present in the current PCB. It gives the EP a short thermal/GND path to the internal GND planes without open vias directly under the solderable pad.

For the planned prototype process, U5 is hand assembled with a very light pre-tin on the exposed pad, flux and hot air. The current full-size B.Paste definition therefore does not control solder deposition and is **not a fabrication blocker** for this flow. If the process later changes to stencil/reflow or external PCBA, the paste aperture and via-treatment strategy must be reopened for that process.

First-board measurement of U5 temperature remains required, but that is prototype validation rather than unfinished PCB implementation. Decision D019 records this closure.

## Stackup / planes

| Layer | Copper / spacing | Role |
|---|---|---|
| F.Cu | 35 µm | front components/signals |
| dielectric 1 | 0.10 mm FR4, Er 4.5 | F.Cu to In1 |
| In1.Cu | 35 µm | **solid GND plane** |
| core | 1.24 mm FR4, Er 4.5 | inner separation |
| In2.Cu | 35 µm | **solid GND plane** |
| dielectric 3 | 0.10 mm FR4, Er 4.5 | In2 to B.Cu |
| B.Cu | 35 µm | back components/signals + local power copper |

Closed decision: **both internal layers remain continuous GND planes**. Power distribution stays on external layers.

## Netclasses

| Class | Clearance | Track | DP width / gap | Via dia/drill |
|---|---:|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.20 / 0.25 mm default | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | — | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | — | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | — | 0.80/0.40 mm |
| USB | 0.20 mm | **0.20 mm** | **0.20 / 0.25 mm** | 0.60/0.30 mm |

USB data nets are assigned to the USB class and use tuning profile `USB_90R`.

## ERC / CI state

Current stored `hardware/ERC.rpt` is dated **2026-09-17 22:18:03** and reports **0 errors, 0 warnings** (with the report's listed ignored check classes). No new ERC is claimed for documentation-only commits.

The branch-specific CI workflow trigger should be checked before assuming automatic ERC will run on documentation or future hardware changes; the successful stored report is the last confirmed check.

## DRC state

Current stored `hardware/DRC.rpt` is dated **2026-09-17 22:18:19**. It reports **0 active errors, 0 active warnings, 0 unconnected pads and 0 footprint errors**. One `silk_edge_clearance` warning for U4 B.Silkscreen touching Edge.Cuts is **explicitly excluded**, not an active defect. Check the released fabrication layers/board edge in the physical prototype; no DRC was rerun for documentation-only commits. The earlier 14-active-warning report from 2026-09-16 is obsolete.

The released bare PCB still requires incoming inspection and all first-board electrical, thermal and fault-recovery tests.

## Procurement/BOM reconciliation

The live Google Sheet `Filament Dryer Monitor — BOM finale Mouser`, tab `BOM TME`, was read on **2026-09-21**. Its 94 PCB-mounted references match the current released `hardware/production/Filament_Dryer_Monitor_bom.csv` and schematic netlist; no PCB-mounted ref or MPN discrepancy was found in the previous full 2026-09-19 audit. The sheet currently lists **61 PCB items across 43 supplier rows**, with **33 refs marked in house**; one PCB is selected for assembly. `TME_IMPORT` is the dynamically generated 43-row purchasing/export tab, **not an order-status ledger**.

The TME purchase was placed 2026-09-20; all **43 current `TME_IMPORT` supplier catalog codes** were found in the original order confirmation during a 2026-09-21 check (this does not verify shipment or received quantities). Its original confirmation lists C11 `CL21A226MAYNNNE` as `Disponibile a magazzino` (page 1, position 3), but TME later acknowledged the stocked unit is missing and is searching for it. Its week 48/2026 ETA is a supplier notice for **C11 alone**. Do not infer the delivery date of the remaining order from that notice. The supplier has **not confirmed** release of all other lines or resolution of C11. No cancellation/replacement has been authorized.

C11 is **required in the assembled design**: it is a 22 µF MLCC from `3V3_MCU` to GND. Current authorized MPN: Samsung `CL21A226MAYNNNE`, 25 V X5R 0805. The 16 V TDK candidate `C2012X5R1C226M125AC` / TME `C2012X5R1C226MAC` is a *candidate only*; verify stock, dimensions, DC-bias/usable capacitance and order resolution before installing or changing the purchasing BOM. There is no reason to change the PCB footprint based on the nominally identical 0805 size. Do **not** omit C11 for final assembly.

Do not treat the old unsuffixed `hardware/production/bom.csv` (legacy AP66200/CP2102N-era) as this design's current BOM. The suffixed export's `LCSC Part #` column is a mixed historical/supplier field; the **live Google Sheet's `Codice TME`** is authoritative for TME purchases. Reconcile actual deliveries (including supplier-imposed minimum quantities) against the TME order confirmation, not only the nominal per-board quantities.

Off-board OLED/J4, SHT45/J3, independent heater cutoff, mating housings/crimps/cable assemblies and mounting parts are **not included** in the 94-component PCB order. Treat them as inventory/assembly checks, not as newly selected PCB parts. See `hardware/docs/PROCUREMENT.md` for follow-up.

## Immediate handoff / next gates

1. Wait for TME's definitive result of the C11 warehouse search and a clear statement about shipping all other available order lines; do not cancel/reorder or silently replace the BOM until agreed.
2. Reconcile received TME packages against `BOM TME`, the confirmed order quantities and all 33 in-house refs. In particular, confirm the actual capacitor supplied for C11 before soldering.
3. Verify physical availability, pinout and fit of J4 OLED, external SHT45, thermal cutoff, mating connectors/cables and enclosure fasteners. The procurement sheet does not establish these are in house.
4. Inspect the received bare PCBs for mask, drills, component fit, exposed pads and assembly clearances; reconcile the received component packages/quantities against the released BOM before soldering.
5. Hand-assemble and bring up the first PCB following `docs/ASSEMBLY.md` (**R5 initially not populated**); verify 3.3 V, USB, sensor/display, buck temperature and fault handling before heater operation.
6. Keep experimental PCB changes and future-revision review separate from the already submitted fabrication revision; do not retroactively rewrite the released Gerber/netlist/BOM checkpoint without new evidence and a deliberate revision.

## 2026-10-01 bare-board preflight

Bare-board preflight continuity/isolation checks were reported passed before assembly: no short to GND was observed on `JACK_24V_RAW`, `24V_PROT`, `3V3_BUCK` or `3V3_MCU`; the unpopulated FB1 pads were isolated from each other; and J1 pin 3 was isolated from TP1 with F1/Q1 still unpopulated. Exact resistance values were not recorded. Gate 1A assembly may proceed.

## 2026-10-02 Gate 1A powered bring-up

Gate 1A powered test passed on 2026-10-02: supply current decayed to ~0 after capacitor charging at 5 V, 12 V and 24 V; TP1 tracked the applied input voltage; Q1 gate measured 0 V at 5 V, 0 V at 12 V and 9.31 V at 24 V, consistent with D1 clamping Q1 |VGS| to about 14.7 V.

- 2026-10-03: first-board power bring-up reached the 3V3_MCU rail. With R5, C11 and FB1 fitted, 24 V input produced ~1.8 mA no-load input current, 3V3_BUCK ~3.3 V and TP2/3V3_MCU ~3.3 V. Gate 1 power-path/buck/rail-link checks passed at no load; load/ripple/thermal validation remains pending.


## 2026-10-04 Gate 3 MCU / USB status

Gate 3 was reported fully populated. With 24 V input and a 50 mA bench limit, the board drew about 8 mA; 3V3_MCU, EN_ESP and IO0 measured 3.3 V. SW1/RESET and SW2/BOOT each pulled their respective node to about 2.5 mV while pressed and returned to 3.3 V when released. Thermal-camera inspection found no concerning hotspot.

USB enumeration remains open: Windows creates an unknown USB device but fails the device-descriptor request with Code 43 (`USB\\DEVICE_DESCRIPTOR_FAILURE`), and no CP2102 COM port or VID/PID is exposed. Changing the USB cable and host USB port did not change the result. Hardware checks so far found USB VBUS present (~4.7 V), the CP2102 VBUS-sense divider active (~3.1 V at R7/R8), CP2102 reset high, U3 correctly powered/oriented, D+/D− continuity from J2 through U3 to U2, no D+/D− short, and approximately 3.3 V on the directly probed U2 VDD/REGIN/VBUS-sense-/RST pins. U2 orientation appears visually correct; hidden QFN joint quality is not yet proven. Continue diagnosis before any rework or hardware revision.

Oscilloscope follow-up on 2026-10-04: CP2102-side D+ rises to ~3.3 V while D− idles low; a 50 ms/div capture shows a host-driven USB reset (D+ forced low, then returning high). A 2 µs/div single-shot trigger on a D− rising edge did not trigger. Continue by distinguishing connector-side USB activity from U3/U2-side activity before any rework.

USB follow-up: no D− rising-edge trigger was obtained even with a single 10× probe on U3 pin 3 (connector side). This shifts the immediate diagnosis toward checking whether D− is being clamped/shorted low (or otherwise prevented from transitioning) before considering CP2102 rework.

USB fault isolation update: U3 pin 3 was observed to move physically under probe contact, indicating an unreliable solder joint on the connector-side D− path. Repair and re-inspect U3 before further USB measurements or any U2 rework.

U3 was re-soldered on all six pins after the loose pin-3 finding. USB enumeration remains unchanged (unknown device / descriptor request failed), so further USB signal diagnosis is still required before any U2 rework or design conclusion.

Post-rework USB scope update: after re-soldering all U3 pins, D− at U3 pin 3 (connector side) now triggers on a rising edge when USB is connected. The available capture was at 100 µs/div and is too slow to assess 12 Mbps waveform amplitude/shape. This confirms the U3 repair changed the D− signal path, but Windows still reports descriptor failure; continue with faster-timebase packet capture before further rework.

After re-soldering U3, oscilloscope probing now shows clear high-speed activity on D− at U3 pin 3 (connector side), including a resolved burst at 2.5 µs/div. This is a real change from the pre-rework no-trigger result and confirms the loose U3 pin-3 joint was affecting the USB signal path. Descriptor enumeration is still not yet confirmed fixed; compare the same activity at U3 pin 4 before considering U2 rework.

USB propagation update: after U3 rework, D− activity measured at U3 pin 4 (CP2102 side) is practically identical to pin 3 (connector side), indicating the signal is propagating through U3. Continue diagnosis toward U2/USB protocol behavior rather than U3 continuity.

Gate 3 completeness was re-audited against the current MCU source and CP2102 datasheet: no additional essential component is missing from the populated USB/UART group. Required VDD/REGIN bypassing, /RST pull-up, VBUS sensing, USB-C CC resistors and ESD/data path are present. A post-rework 100 ns/div D− capture shows full-swing high-speed transitions; this confirms bit-level bus activity but does not yet prove successful CP2102 descriptor response.

CP2102 ground verification: U2 pin 3 to TP3/GND measured ~0.3 Ω unpowered, confirming the peripheral GND connection. The hidden exposed-pad joint remains unverified visually, but loss of the chip's only ground path is excluded.

Android cross-check: an Android USB device-info app reported no device detected when the powered board was connected to the phone. Treat this as provisional until phone-side host/OTG operation is confirmed by measuring USB VBUS at J2/R7 during the connection.

Android host validation: USB_VBUS measured only ~3.3 V with the phone connected, rather than the ~5 V expected from an active USB host. The Android no-device result is therefore inconclusive and must not be treated as an independent confirmation of the Windows descriptor failure.

USB VBUS backfeed check: with USB disconnected and only 24 V applied, the USB_VBUS node measured ~1 mV. The PCB is not backfeeding VBUS; the earlier ~3.3 V with the phone was external to the board and did not establish valid USB host mode.

Android host test confirmed valid: USB_VBUS measured 5 V from the phone while connected, yet no USB device was detected. Since Windows also fails at the device-descriptor stage, the remaining leading suspects are the CP2102 device or its QFN soldering/hidden pad rather than host software.

U2 GND clarification: pin 3 and exposed pad 29 share GND, and the PCB EP copper is tied to the GND network. The ~0.3 Ω pin-3-to-TP3 reading confirms board-side grounding; only the hidden package-EP solder bond remains unobservable. With peripheral GND working, perimeter U2 pins (especially USB/power/reset pins 4–9) are the next rework focus rather than the EP connection itself.

Gate 3 USB is now operational: following U2 perimeter-pin rework, Windows enumerates U2 as **Silicon Labs CP210x USB to UART Bridge (COM5)**. The prior descriptor failure is resolved. This strongly indicates a marginal U2 solder joint was the remaining assembly fault; exact pin not identified.

Gate 3 UART progress: ESP32 ROM boot output is readable on COM5 at 115200 after SW1 RESET, confirming U4 TXD0 -> CP2102 -> USB and ESP32 reset/boot operation. Reverse UART and programming/auto-reset remain to be validated.

Automatic programming is not yet passing: esptool on COM5 reaches `Connecting...` but reports `Invalid head of packet (0x65)`. Since ESP32 ROM TX output is readable, next isolate automatic DTR/RTS boot control from the PC-to-ESP32 UART path using manual BOOT/RESET download mode.

Manual BOOT/RESET proves bidirectional UART and ROM download mode: esptool identifies ESP32-D0WD-V3 rev 3.1 and reads chip information, but then fails while uploading/starting the RAM stub with `Invalid head of packet (0x65)`. Focus now shifts from UART connectivity to rail stability during stub start and possible esptool 5.3.x stub behavior.

Gate 3 programming update: with the 24 V bench current limit raised from 50 mA to 100 mA, manual BOOT/RESET programming completes successfully; esptool stub runs, flash writes and hash verification pass. The prior stub-stage corruption was caused by the too-low 50 mA current limit. Auto-download entry still needs a separate no-button upload test.

At 100 mA, automatic no-button upload still fails with `No serial data received`, while manual BOOT/RESET flashing succeeds. This isolates the remaining Gate 3 issue to the DTR/RTS automatic boot-control path. Capture TP4/EN_ESP and TP5/IO0 simultaneously during esptool connection.

Gate 3 auto-program root cause is now identified from scope plus source review: the released Q2/Q3 DTR/RTS cross-coupled network implements the opposite EN/IO0 truth table from Espressif's ESP32 auto-download reference. Manual BOOT/RESET works, but automatic esptool entry cannot generate the required GPIO0-low-at-reset-release condition. Treat this as a released-hardware design defect pending an agreed prototype bodge and later source correction; do not silently alter the schematic/PCB.


Gate 3 auto-program fix validated on the first article (2026-10-04): the Q2/Q3 collector destinations were crossed so Q2 collector drives EN and Q3 collector drives IO0. After this bodge, no-button Arduino/esptool programming succeeds through COM5, including stub start, flash verification and automatic RTS reset. This experimentally confirms the released PCB's auto-download truth table was reversed. The design sources have not yet been silently altered; the correction is recorded in D020 for the next hardware revision. Runtime output from the newly flashed diagnostic sketch remains to be checked before closing Gate 3.

Gate 3 is closed on the first article (2026-10-04). After the Q2/Q3 collector-cross bodge, automatic USB/UART programming works without manual buttons, and the flashed diagnostic sketch runs correctly at 115200 baud (startup banner after reset, then periodic `alive`). Next assembly/test gate is Gate 4A I2C support: J3 plus R14-R17 first, with the unloaded bus checked before separately attaching the actual J4 SSD1309 OLED module and the external SHT45.

Gate 4A hardware population has begun: J3 and R14-R17 are fitted; J4 OLED and SHT45 are not connected yet. J3 mapping is 1=GND, 2=SDA, 3=SCL, 4=3V3_MCU. Unloaded I2C bus checks are next.

Gate 4A unloaded I2C bus validation passed: J3 power, SDA and SCL continuity/idle levels are correct with no off-board modules attached. Next hold point is physical pinout/orientation verification of the actual SHT45 board and J3 harness before connection.

Gate 4A harness status (2026-10-04): the external SHT45 is still **not connected** to J3. The sensor currently has four loose color-coded leads: red = 3V3, yellow = SCL, green = SDA, black = GND. The first-article PCB already has J3 fitted as JST PH 4-way `B4B-PH-SM4-TB` (2.00 mm pitch), with released-netlist mapping pin 1 = GND, pin 2 = SDA, pin 3 = SCL, pin 4 = 3V3_MCU. The user currently has JST-XH connector parts, which do not mate with J3. A JST-PH 2.00 mm assortment with 2/3/4/5/6-way housings, pre-crimped 22 AWG leads and female crimp terminals is under consideration; do **not** mark it ordered/received until confirmed. The intended next bench step is to terminate the SHT45 leads in a 4-way PH mating housing with the above pin order, verify orientation/continuity with a meter, and only then connect/power the sensor for the I2C diagnostic. Historical README wording mentioning a STEMMA QT cable is not the released PCB interface; the fabricated PCB interface at J3 is JST-PH.

## 2026-10-04 Gate 4B buttons + diagnostic LED

Gate 4B is **PASSED** on the first article. SW3, SW4, SW5 and SW6 all read approximately 3.3 V at rest and pull their respective input net to approximately 0 V when pressed.

During bring-up, SW6 / BTN_DOWN initially measured about 0.91 V at rest. The cause was a poor solder joint on R24, not the switch or ESP32. Reworking R24 restored the expected ~3.3 V idle level.

D3/R18 was tested by flashing a temporary GPIO26 blink sketch; the LED blinked correctly, confirming the GPIO26 -> R18 -> D3 path. Because D3 is not visible outside the enclosure, D021 defines it as an **internal diagnostic/service LED**, not a user-facing status indicator.

Gate 4A remains at the JST-PH SHT45 harness hold point. Gate 4C buzzer population/testing can proceed independently while the PH harness is pending.

## 2026-10-05 Gate 4C buzzer

Gate 4C is **PASSED** on the first article. BZ1, Q6, D6, R27 and R37 were populated and the buzzer path was verified from ESP32 GPIO33.

A temporary firmware test drove the passive buzzer at approximately 2.7 kHz and produced the expected audible tone. A second test played a three-note ascending melody once at startup, confirming that firmware can vary pitch and timing.

The initial 50% duty-cycle test was subjectively too loud. Future firmware should therefore expose buzzer loudness as a software-controlled setting using reduced PWM duty cycle (for example OFF / LOW / MED / HIGH after final tuning), with the understanding that this is an approximate acoustic level control rather than a calibrated linear volume control.

Proceed to Gate 4D NTC input before fan/heater power-output bring-up.

## 2026-10-05 Gate 4D NTC analog-front-end validation

Gate 4D analog front end is **PASSED** on the first article with J7 intentionally still unpopulated pending reuse of the dryer-side mating connector.

R28 = 47 kΩ and C19 = 100 nF were fitted. With J7 open, the NTC node measured 3.3 V as expected. A temporary 100 kΩ resistor was then connected from J7 pad 2 (NTC) to J7 pad 1 (GND), simulating the external NTC. The node measured 2.253 V by DMM, close to the ideal divider value of about 2.245 V.

ESP32 GPIO34 / ADC1 then reported a stable raw reading around 2643-2647 and calibrated readings around 2300-2302 mV using Arduino `analogReadMilliVolts()`. The ADC therefore sees the divider correctly and stably. The ~47 mV offset versus the DMM is acceptable for functional validation but must not be treated as final temperature calibration.

Remaining Gate 4D work: fit J7 when the dryer connector is available, connect the real NTC, validate the already-recorded empirical NTC characterization through the PCB ADC path, and implement the final conversion and fault limits before any heater operation. The original eSUN NTC characterization is already recorded in README.md (82.5 kΩ at 25 °C through 28.8 kΩ at 50 °C, with a single-beta approximation near 4100 K).


## 2026-10-07 Gate 4A SHT45 live validation

The external SHT45/J3 portion of Gate 4A is **PASSED** on the first article. The JST-PH 4-way mating harness is now available and was assembled to the released J3 mapping (J3-1=GND/black, J3-2=SDA/green, J3-3=SCL/yellow, J3-4=3V3/red).

With the SHT45 connected, J3 pin 4 remained at approximately 3.3 V. An I2C scan detected the sensor at address `0x44`. A direct SHT4x high-precision measurement test (command `0xFD`, heater off) then returned stable valid readings with CRC checks passing: approximately 22.28-22.30 °C and 65.44-65.58 %RH over repeated samples.

This closes the SHT45 harness, power and I2C functional checks. The 1.54-inch SSD1309 OLED at J4 is still to be connected and tested separately on the already validated I2C bus before Gate 4A is considered fully complete.


## 2026-10-08 Gate 5A fan-driver bench validation

Gate 5A driver-path validation is **PASSED without the actual fan load**.

The originally installed Q5 was removed after J6 pin 1 measured approximately 1 ohm to GND. With Q5 removed, the same node rose to approximately 68 kohm. A GM328 component tester later failed to recognize the removed device as a MOSFET and instead reported low resistive paths, including approximately 1.69 ohm, confirming the removed Q5 was faulty.

A replacement IRLML2060TRPBF-class N-channel SOT-23 device was checked on the GM328 before installation and was recognized as an enhancement-mode N-MOSFET with plausible body-diode/device readings. After fitting the replacement Q5, J6 pin 1 to GND measured about 130 kohm and rising with the board unpowered, so the previous hard short was gone.

Powered at 24 V with the bench current limit at 100 mA and no fan connected:
- Q5 gate (pad 1) measured 0 V in the safe-OFF state.
- With a temporary 100 kohm pull-up from J6 pin 2 (+24 V) to J6 pin 1, the switched node measured 23.96 V with Q5 OFF.
- A temporary firmware test then drove FAN_PWM / GPIO16 HIGH after a delay; Q5 switched the J6 pin 1 node low as expected.

This validates the gate pull-down, MOSFET switching path and J6 switched output without a real load. Do not treat the actual fan as validated yet. Before connecting it, verify its rated voltage/current and polarity, then choose an appropriate bench current limit and perform static ON/OFF load testing before PWM characterization.


## 2026-10-08 Gate 5A real fan load test

The actual 24 V fan was connected to J6 after the replacement-Q5 no-load checks passed. With the temporary static-ON firmware, the fan started normally. Bench-supply current peaked at approximately 102 mA during startup and settled at approximately 89 mA while running. With the fan running and Q5 commanded fully ON, J6 pin 1 measured only 7.7 mV to GND.

This confirms successful first-article operation of the J6/Q5 fan power path under the real fan load, with negligible switched-node voltage drop in the static-ON state. The measured currents are total input current from the 24 V bench supply, not an isolated fan-only current measurement. PWM-speed characterization and any final firmware control policy remain separate follow-up work.


## 2026-10-08 Gate 5A PWM characterization

Initial PWM characterization was performed on the actual fan using GPIO16/FAN_PWM at 25 kHz.

- 50% duty: fan starts and runs; bench input current ~49 mA.
- 75% duty: fan starts and runs; bench input current ~72 mA.
- 100% static ON reference: bench input current ~89 mA after startup.
- The fan produces audible noise/fan whine even outside the PWM test, so the observed sound is not attributable solely to PWM frequency. A quieter replacement fan may be considered later without changing the validated driver architecture.

Further characterization should determine the minimum reliable startup/running duty before final firmware limits are chosen.


## 2026-10-08 Gate 5A PWM characterization

Initial fan PWM checks were performed at 25 kHz with the actual fan connected. The fan started and ran at all tested duty cycles. Bench input current was approximately 49 mA at 50%, 72 mA at 75%, and 89 mA at 100% (the 100% value is consistent with the prior static-ON result).

The fan remained audibly "whistling" at 50%, 75%, and 100%. Because the same acoustic character remains at 100% duty, where the MOSFET drive is effectively continuously ON rather than chopping the fan supply, the observed whistle is attributed primarily to the fan itself rather than to PWM switching. A quieter replacement fan may be considered later without changing the validated driver topology.


## 2026-10-08 Gate 5A minimum fan duty characterization

At 25 kHz PWM, the actual fan was tested for minimum reliable operation in 5% duty-cycle steps. From a complete stop, 20% did not start the fan and 25% was the first tested duty that started it. Once already rotating, the fan continued to run down to 15%; 10% did not sustain rotation.

Measured first-article thresholds are therefore 25% minimum observed start duty and 15% minimum observed running duty. These are characterization points, not yet final firmware limits; production firmware should retain margin above the observed thresholds and may use a brief full-duty startup boost before settling to a lower commanded speed.


## 2026-10-08 Gate 4A J4 I2C enumeration

With both off-board I2C devices connected, the bus scanner detected the SSD1309 display at `0x3C` and the SHT45 at `0x44`. This confirms J4 power/bus connectivity and coexistence of both devices on the first-article I2C bus. Graphical initialization and pixel output of the SSD1309 remain to be tested before Gate 4A is fully closed.


## 2026-10-08 Gate 4A OLED graphical test

The installed 1.54-inch 128x64 SSD1309 OLED at J4 was initialized successfully over I2C at address `0x3C` using U8g2 while the SHT45 remained present at `0x44`. A graphical test frame and text rendered correctly on the display.

Gate 4A is therefore **PASSED** on the first article: unloaded I2C electrical checks, SHT45 harness/live measurements, coexistence of both I2C devices, and SSD1309 graphical output are all validated.


## 2026-10-08 J7 OEM connector mismatch

First-article physical fit check found that the installed J7 JST-GH BM02B-GHS-TBT (1.25 mm) does not mate with the original eSUN heater NTC plug. The earlier assumption that the OEM NTC plug was JST-GH-compatible was not physically validated and is incorrect for this dryer.

Electrical J7 mapping remains valid (pin 1 GND, pin 2 NTC). For this first article, J7 will remain installed. The current recovery plan is to remove the incompatible plug from the original NTC leads and terminate those leads directly into a JST-GH 2-position housing, after validating a sample crimp first. The NTC leads are believed to be about 30 AWG. A PEBA micro-crimping kit/tool advertised for JST-GH 1.25 mm and 0.03–0.34 mm² conductors was purchased on 2026-10-08 and is expected on Saturday 2026-10-10. Do not cut the NTC harness until a sacrificial GH crimp has been checked mechanically. The next hardware revision should still revisit J7 connector choice deliberately.


## 2026-10-08 Gate 5B unloaded switching test

Q4, D4, R20 and R21 were installed with J5 still unpopulated and no heater connected. J5 pin 1 / HEATER_SW was not shorted to GND with power removed. At 24 V with a 100 mA bench limit, Q4 gate measured 0 V in the safe-OFF state. With a temporary 100 kΩ pull-up from J5 pin 2 (24V_PROT) to J5 pin 1, the OFF-state switched node measured 24 V. Driving HEATER_PWM (GPIO19) HIGH produced 3.3 V at the Q4 gate and approximately 1 mV at J5 pin 1.

The unloaded Q4 driver path therefore passes static OFF/ON validation. The real heater remains disconnected pending the real NTC/J7 path, firmware conversion/fault handling and independent series TCO.

## 2026-10-08 front-panel V1 interaction specification

The first-article front-panel hardware is now sufficiently validated to define application behavior: all four buttons are functional, the SSD1309 OLED renders graphics at `0x3C` while the SHT45 operates at `0x44`, and the passive buzzer can generate tones/melodies with software-adjustable PWM duty. D3 remains the hidden diagnostic LED under D021.

D022 now fixes the V1 interaction model and `docs/UI_SPEC.md` contains the detailed screen/button specification. The controller is standalone from the front panel, boots into safe `STANDBY`, does not auto-resume a cycle after reset/power loss, uses temperature + time + fan AUTO as the initial cycle-setting model, and treats RH as displayed/logged information rather than a standalone V1 cycle-completion criterion. Fault presentation has priority, and buzzer mute never clears a safety fault.

This is a firmware/UI decision only. It does not change the released PCB, connector recovery plan, current hardware bring-up gates or heater-safety prerequisites. Final parameter ranges, heater/NTC thresholds, fan AUTO/cooldown policy and material-profile values remain open until physical validation supports them.


## 2026-10-08 Slewform OLED branding asset

The Slewform logo **with wordmark** was selected for firmware branding / startup use on the 128x64 SSD1309 OLED. The original vector source is stored at `firmware/assets/slewform/Slewform_logo.svg`; the tested 1-bit U8g2/XBM derivative is stored at `firmware/assets/slewform/slewform_logo_128x64.h` (90x60 pixels, `slewform_full_logo`). The symbol-only variant is not the selected firmware branding asset.

## 2026-10-08 SAFE BRING-UP application firmware scaffold

Commit `030cb5c` adds the first application firmware scaffold under `firmware/`. This is deliberately a **SAFE BRING-UP** build rather than heater-control firmware.

Implemented now:

- exact first-article SSD1309 U8g2 path and the selected Slewform 90×60 startup logo;
- original short two-step rising startup chime, plus cycle-start and completion sounds;
- direct SHT45 high-repeatability command `0xFD`, non-blocking conversion wait, CRC validation and T/RH conversion;
- active-low four-button debounce, short/long press and UP/DOWN autorepeat;
- BOOT, STANDBY, SETUP, DRYING-demo and COMPLETE interaction states, plus technical/fault rendering scaffolding;
- OLED standby sleep/wake behavior;
- D021 internal diagnostic LED patterns;
- cooperative timestamp/state-driven main loop.

Critical safety limitation: GPIO19 (heater) and GPIO16 (fan) are configured as outputs and forcibly driven LOW on every loop iteration. The current DRYING state is therefore only a UI/countdown demonstration. The UI prototype temperature/time bounds are not accepted heater-control limits.

A host-side C++ syntax check passed. The sketch has not yet been compiled/flashed through the actual Arduino ESP32 toolchain in this checkpoint, so first-article compile/flash and live UI validation are the immediate firmware test.

## 2026-10-08 first live UI corrections

The first SAFE BRING-UP firmware was flashed successfully to the first article and the application starts. Initial live observations:

- Slewform splash at 1.5 s was too short to view comfortably;
- brief horizontal/random OLED content appeared before the logo;
- displayed temperatures were missing the degree symbol.

The firmware was corrected to hold the Slewform splash for 3.5 s, initialize/blank the SSD1309 as early as practical before enabling its visible frame, and render temperature units consistently as `°C` using U8g2 UTF-8 rendering. Heater and fan remain hard-locked LOW.

## 2026-10-08 first button/UI usability test

The first-article SAFE BRING-UP UI was exercised through the front panel. The user confirmed that the tested behavior works: STANDBY -> SETUP, temperature/time edits, UP/DOWN autorepeat, FAN AUTO non-editability, start into DRYING-demo, M page switching, short ON/OFF ignored during drying, long ON/OFF stop, setup cancellation, and OLED sleep/wake with the first wake press consumed.

The mechanics are therefore validated on the first article, but the interaction was judged somewhat cumbersome. D024 records the next usability trial: M-long becomes SETUP back/cancel and small context-sensitive button hints are drawn at the OLED edges aligned with the four physical buttons. Heater and fan remain hard-locked LOW.

## 2026-10-08 contextual button labels rejected

The first contextual-label layout trial was rejected before further first-article testing because the small edge labels competed with existing screen content on the 128×64 OLED. The labels were removed and the prior clean layouts restored.

The independent usability improvement remains: in SETUP, M short advances to the next field and M long returns to STANDBY; ON/OFF remains dedicated to starting the cycle.

## 2026-10-08 application phase 2: fan, controllers, persistence and buzzer

The SAFE BRING-UP application was expanded after the first UI/button tests.

New firmware structure:

- `CycleController.h`: cycle start/stop/countdown/completion;
- `FanController.h`: real 25 kHz fan PWM with 100% 1 s kick-start then provisional 40%;
- `SafetyController.h`: heater safety scaffold that continuously forces GPIO19 LOW; no heater-enable path exists;
- `SettingsStore.h`: ESP32 Preferences/NVS persistence for cycle UI values, buzzer volume and key-click preference;
- expanded `Buzzer.h`: OFF/LOW/MED/HIGH levels, optional key click, startup/start/complete/warning/fault sequences;
- SETTINGS UI entered by long M from STANDBY.

The fan is now the only application-controlled power output. It operates only during DRYING test cycles and is stopped on stop, completion or fault. The heater remains hard-locked OFF on every loop pass. Fan cooldown and final FAN AUTO behavior remain open.
 
## 2026-10-10 Slewform SVG-to-OLED bitmap refinement

The canonical `firmware/assets/slewform/Slewform_logo.svg` remains unchanged.
After the first-article OLED test revealed some unattractive/potentially
erroneous pixels in the old rasterized logo, a repeatable export was added at
`firmware/assets/slewform/generate_logo.py`. It uses a combined geometry
alpha mask (not brightness-based grayscale of the original blue/orange fills),
high-resolution SVG rendering, Lanczos reduction and adjustable 1-bit
thresholding. Three candidates were compared visually (light, balanced,
strong); the **balanced** candidate with threshold 115 is selected for the
current 90×60 XBM header, retaining the logo **with wordmark**.

The generator verifies the XBM pack/unpack round trip. The resulting header
contains exactly 720 data bytes (12 bytes × 60 rows) and preserves the
existing macro/symbol names used by `Ui.h`. The selected 128×64 black/white
preview is committed alongside the generator. Actual on-device appearance
remains to be checked after reflashing.

## 2026-10-10 Slewform animated splash and synchronized wordmark chime

After approval of a larger standalone emblem and separate SLEWFORM page, the
SAFE BRING-UP firmware now draws the stationary 54×60 upper emblem and reveals
root pixels radially in 11 timestamped stages. It then switches to a completely
separate centered 122×18 SLEWFORM wordmark at 2.1 s and starts the existing
two-note rising startup chime at that instant, rather than at application entry.
The BOOT interval remains 3.5 s: growth for 1.5 s, full emblem silent through
2.1 s, and solo wordmark through 3.5 s.

All 1,128 bytes of bitmap data derive from the unchanged SVG using
`firmware/assets/slewform/generate_animation_compact.py`, which checks XBM
pack/unpack. `Ui::requestRender` forces the wordmark redraw in the same loop
iteration as the startup chime trigger; there is no blocking animation delay.
The existing heater hard-OFF safety control and fan operation are unchanged.

**Outstanding:** run actual ESP32 Arduino build/flash, then inspect OLED
smoothness and verify the sound/display synchronization on the first article.

## 2026-10-11 OLED first-article boot observations and software mitigation

Physical first-article user report: some cold starts briefly show horizontal
lines or the previous screen; the final roots frame can briefly look like the
upper emblem without roots before the wordmark; the initial static emblem hold
is too short. Software changes in this revision:

- replace U8g2 `begin()` with `initDisplay()`, which leaves the panel off,
  and upload the first **rootless emblem** before enabling the panel;
- hold the rootless emblem 350 ms, then 11 growth steps at 150 ms, completing
  the symbol at 1.85 s; wordmark begins at 2.1 s and BOOT ends at 3.5 s;
- skip duplicate transfers of unchanged boot frames;
- temporarily turn off visible OLED pixels while writing the complete
  wordmark image at the page transition, preventing partial framebuffer
  transfer from momentarily displaying the rootless emblem;
- trigger the existing chime **after** sending and revealing the new wordmark
  frame rather than before its I2C transfer completes.

An OLED controller that powers up before ESP32 application initialization may
still briefly expose stale/undefined RAM; this cannot be fully eliminated by
firmware alone, especially without an independently driven OLED reset.
**Hypothesis requiring hardware validation:** the rootless moment was
full-frame I2C transfer tearing, not a deliberate animation step.
Nothing in this revision enables the heater or changes fan control.
Compile/flash and observe repeated power cycles and ESP32 resets before
marking the boot artifacts solved.

## 2026-10-11 follow-up: actual causes identified for remaining boot symptoms

The user reports that horizontal lines/previous-screen power-on artifacts seem
resolved after the previous OLED startup fix, but (a) the full-grown roots
briefly disappear before the wordmark and (b) the initial static frame is no
more noticeable.

Source inspection found **two deterministic faults**:

1. `bootStartedMs` had been taken before blocking OLED initialization and
   subsequent Preferences/buzzer/sensor setup. By the time the first frame was
   displayed, the nominal 350 ms hold was partly (or largely) consumed.
   The BOOT timer now starts after all initialization, with a 550 ms
   rootless static frame and 120 ms per growth step. The full emblem now
   remains visible from ~1.75 to 2.1 s.
2. `slewform_root_radius_sq_x4[SLEWFORM_ROOT_STEPS]` declared **11**
   elements but initialized only **10**. In C++, the final element silently
   became zero; during the final 150 ms step, roots disappeared exactly as
   observed. An explicit 11th radius of 4039 (root-mask maximum squared
   radius in 2× coordinates: 4034) now restores all 308 root pixels. The
   table is unsized and protected by `static_assert` against missing
   entries; the SVG generator also validates stage count and final coverage.

No OLED rendering refactor is required to fix the confirmed second cause.
The previously effective OLED cold-start preload and hidden wordmark
transition remain unchanged. No heater/fan/safety logic changed.
**Flashing and checking both corrections on the physical first article
remain necessary.**

## 2026-10-11 continued OLED field observations and longer rootless intro

User confirms the preceding fixes appear to have resolved (2) the brief
root disappearance at the end of growth and (3) the too-short rootless frame,
but asks for a longer static symbol of approximately 1–1.5 s. Selected
**1.2 s** hold while retaining the existing 11 × 120 ms root-growth cadence,
the ~350 ms complete-emblem hold and ~1.4 s solo wordmark display:

- 0–1.20 s rootless static emblem;
- 1.20–2.40 s root growth;
- 2.40–2.75 s complete emblem;
- 2.75–4.15 s wordmark and chime;
- 4.15 s STANDBY.

Only `firmware/AppConfig.h` timing constants and their documentation changed:
no bitmap, buzzer, safety, fan or OLED initialization modifications.

User also reports **(1) startup horizontal lines or previous screen have
returned**, after having appeared resolved during earlier testing.
The OLED initialization code is unchanged in the prior/this revision:
`initDisplay()` followed by a preloaded first emblem frame, then
`setPowerSave(0)`. The initial observation of success was therefore not
conclusive; intermittent power-on/reset sequencing or retained controller
display RAM remains possible, not confirmed. Before changing hardware or
claiming an OLED fix, compare cold power-on, fast power-cycle, and ESP32 RESET
with the OLED still powered. Note when the artifact appears relative to the
first emblem; inspecting actual power rise/reset with a scope may be warranted
if a controlled test supports that hypothesis.


## 2026-10-11 Gate 4D real NTC path and ADC calibration

The original eSUN heater NTC has now been reterminated to the installed J7 JST-GH connection and connected to the first article. With the heater power connector still disconnected, the real NTC path is electrically functional.

Initial room-state checks:
- J7 pin 2 / NTC node: 2.368 V by DMM.
- Direct NTC resistance after settling: 114.9 kΩ (an earlier reading was 116.7 kΩ).
- SHT45 air temperature at the same time: 21.8 °C. This is not used to recalibrate the heater NTC because the SHT45 measures chamber air rather than the heater body.
- ESP32 GPIO34 reading with the real NTC: RAW 2779-2783 and `analogReadMilliVolts()` 2409-2412 mV.

A five-point first-article ADC/divider calibration was then performed with known resistors substituted at J7 while the heater remained disconnected:

| Measured resistor | DMM node voltage | ESP32 `analogReadMilliVolts()` |
|---:|---:|---:|
| 33.03 kΩ | 1.373 V | 1.394 V |
| 47.0 kΩ | 1.663 V | 1.696-1.698 V |
| 68.0 kΩ | 1.967 V | 1.998-2.002 V |
| 81.9 kΩ | 2.111 V | 2.143-2.146 V |
| 99.9 kΩ | 2.257 V | 2.294-2.296 V |

A linear first-article correction over this range is `V_DMM_mV ≈ 0.984369 * V_ESP32_mV - 2.322`. Fitting the corrected divider data gives effective parameters of about 3.311 V supply and 46.572 kΩ pull-up, which absorb the actual first-article R28 tolerance and rail behavior.

Application firmware now contains `NtcSensor.h`. It averages ADC readings, applies the measured ADC correction, converts node voltage to NTC resistance, and converts resistance to heater temperature using piecewise interpolation of the existing empirical 25-50 °C table. Outside that measured interval it reports a diagnostic estimate from the previously recorded β≈4100 K model and marks the value as outside the empirical range.

The safety controller now latches obvious electrical NTC faults (open/short/invalid) while continuing to force HEATER_PWM LOW. These are bring-up electrical fault windows only; final thermal trip thresholds, heater control limits and control gains remain deliberately open. The real heater must remain unpowered until the independent series TCO is physically installed/verified and the new NTC firmware is live-tested.


## 2026-10-11 Gate 4D live NTC firmware / open-fault validation

The calibrated NTC application firmware was flashed and tested on the first article with the real heater power path still disconnected.

With J7 connected, serial diagnostics were stable around 2.33 V corrected, 110-111 kΩ and about 18.6-18.8 °C. The SYSTEM page simultaneously displayed chamber temperature from the SHT45 (~22.8 °C) and heater temperature from the NTC (~18.8 °C), confirming the two independent sensor paths are presented correctly. These temperatures are not expected to match because the SHT45 measures chamber air while the NTC is thermally coupled to the heater.

Unplugging J7 while powered triggered the intended latched NTC open-circuit FAULT, including the user-visible fault state, while HEATER_PWM remained hard-locked OFF. Reconnecting J7 requires reset to clear the current latched fault behavior.

The real NTC sensing path and the first electrical safety-fault path are therefore live-validated. Final heater thermal trip thresholds, heater control policy and the independent series TCO remain open prerequisites before any real heater energization.

## 2026-10-14 Gate 5B real-heater load validation

Gate 5B has now progressed from the previously validated unloaded switch test to a real-heater load test on the first article.

Physical first-article configuration:
- original eSUN heater element measured about **15 ohm cold**;
- J5 is populated and the original heater cable is reused;
- an off-board one-shot thermal cutoff marked **105 °C / 15 A / 250 V** is crimped in series with **HEATER+ / J5 pin 2**, with insulated crimp sleeves/heat-shrink and the TCO body held against the flex heater with Kapton;
- complete heater path measured about **15 ohm across J5 pins 1-2** after TCO installation;
- real heater NTC remained connected at J7.

A standalone diagnostic sketch (not the normal application firmware) forced GPIO19 LOW at boot, required an explicit serial `H` command, allowed only one pulse per reset, aborted on invalid NTC or a conservative temporary temperature ceiling, and automatically returned HEATER_PWM LOW.

Measured results:
- **1 s pulse:** bench current about **1.59 A** at 24 V; NTC responded upward and continued rising briefly after switch-off from thermal inertia.
- **5 s pulse:** stable heating response, NTC approximately 16.8 -> 23.0 °C during ON with a post-off peak around 24.0 °C.
- **10 s pulse:** start **16.55 °C**, temperature at switch-off **28.90 °C**, peak **29.36 °C**, temperature after 10 s cooldown **25.13 °C**, actual ON time **10.000 s**, shutdown reason **timer**, final NTC status **OK**.

At 24 V and 1.59 A, observed heater input is about **38.2 W**, consistent with the measured ~15 ohm cold resistance and the intended ~1.6 A load. No current-limit event, NTC fault, or abnormal switch-off was observed.

**Gate 5B real-load switching / short thermal-response test: PASS.** This validates the first-article Q4/J5/heater current path and live NTC response for short controlled pulses. It does **not** establish final closed-loop heater control, final thermal fault thresholds, control gains, long-duration thermal equilibrium, or the TCO's destructive 105 °C trip point. The normal application firmware remains heater-hard-locked until those control/safety limits are deliberately implemented and validated.

## 2026-10-15 Heater characterization to 40 C

A controlled real-heater characterization was extended to a **40 °C NTC target** using the standalone diagnostic sketch with GPIO19 default-OFF, explicit single-run serial start, continuous NTC validity checks, a 60 s absolute heater timeout, automatic shutdown at the target and 20 s post-off logging.

Reported first-article result:
- time to 40 °C target: **26.000 s**;
- temperature at heater switch-off: **39.97 °C**;
- post-off peak: **40.31 °C**;
- peak occurred **26.600 s** after heater start, i.e. about **0.600 s after switch-off**;
- temperature after 20 s cooldown: **31.45 °C**;
- actual heater ON time: **26.000 s**;
- shutdown reason: **40 °C TARGET REACHED**;
- final NTC status: **OK**.

The observed overshoot above the switch-off temperature was only about **0.34 °C**, confirming that the heater NTC responds quickly enough for short controlled characterization in this range and that thermal inertia after power removal is modest at 40 °C. This remains characterization data, not a final control law or safety threshold.

## 2026-10-15 Heater characterization to 50 C

Controlled first-article real-heater characterization to the top of the **existing empirical NTC calibration range (50 °C)** completed with the standalone diagnostic sketch (not integrated application firmware). GPIO19/HEATER_PWM defaults OFF; an explicit one-shot `H` command starts the heater, and NTC-invalid fault, 50 °C target, and 60 s maximum-ON timer all stop heater drive; 20 s cooldown is logged.

Actual user-reported diagnostic summary:
- Start temperature: **17.32 °C**.
- Threshold-crossing times from heater ON: **25 °C 6.000 s**, **28 °C 7.900 s**, **30 °C 10.600 s**, **35 °C 18.000 s**, **40 °C 23.800 s**, **44 °C 31.500 s**, **50 °C 42.800 s**.
- Temperature at OFF: **50.00 °C**.
- Maximum temperature: **50.14 °C**, **43.300 s** after heater start (**0.500 s** after switch-off).
- Temperature after 20 s cooldown: **39.28 °C**.
- Actual heater ON time: **42.800 s**.
- Shutdown reason: **50 C TARGET REACHED**; final NTC status **OK**.

**PASS for controlled real-load NTC response and automatic target stop to 50 °C.** Observed post-OFF NTC overshoot was only **0.14 °C**. This does not independently prove absolute NTC accuracy, thermal uniformity across the heater, case/plastic temperatures, crimp/J5 joint temperatures, or suitability for higher settings. Original empirical NTC points end at 50 °C; above that the temporary sketch uses beta-model extrapolation, not new measured calibration points. Before intentionally extending tests to 60 °C or higher, inspect the nearby case, heater and TCO/J5 cabling for heat effects and obtain independent surface/plastic temperature measurements at representative locations. The standalone sketch remains separate from normal firmware, which still hard-inhibits heater operation.


## 2026-10-15 Independent Sanwa thermocouple / NTC comparison — calibration OPEN

With the first-article heater OFF at room state, the original eSUN NTC connected to J7 was read by a standalone diagnostic sketch as **124.4 kΩ / 16.35 °C (beta extrapolation below 25 °C)**; the SHT45 located in ambient air about 10 cm from the case read **19.99–20.01 °C**, and a Sanwa RD701 with type-K probe held against the heater near the original NTC read **20 °C**. With J7 disconnected and power removed, direct NTC resistance was **122.3 kΩ**; these two resistance readings were made in **different electrical configurations**, NOT simultaneously, and agree to about 1.7%. The 2026-10-11 checkout had already measured direct NTC resistance **114.9 kΩ** (earlier 116.7 kΩ), while SHT45 read ~21.8 °C, and the then-running application showed ~18.6–18.8 °C NTC with room air ~22.8 °C. Do not request redundant baseline resistance measurements.

A temporary standalone 50 °C NTC-target diagnostic was used for direct comparison with the Sanwa probe fixed near the NTC. User-recorded pairs (manual readings, not synchronized instrument datalogging):

| Phase | Sanwa type-K | ESP32 NTC estimate |
|---|---:|---:|
| Ambient/rest | 20 °C | 16.5 °C |
| Heating | 30 °C | 34.7 °C |
| Heating | 40 °C | 45.6 °C |
| At NTC-target shutdown | 44 °C | 50 °C |
| Cooling | 40 °C | 40.7 °C |
| Cooling | 35 °C | 33.97 °C |

**Interpretation, not yet proven calibration:** heating-phase offset increasing to +6 °C (NTC estimate minus thermocouple), followed by convergence and sign reversal during cooling, is consistent with probe/attachment thermal response lag and/or heater temperature gradients. The persistent ambient NTC estimate below both independent measurements also indicates the previously recorded 25–50 °C NTC resistance/temperature curve plus beta extrapolation must be revalidated. Original eSUN NTC manufacturer/part identity is unknown; 25–50 °C values were collected during original-system cooldown and must not be treated as certified calibration. Sanwa has integer-degree readout and non-negligible measurement uncertainty. Dynamic readings must NOT be blindly used as equilibrium calibration points, and 50 °C indicated NTC must NOT be assumed to equal actual heater surface 50 °C.

**Next gate:** evaluate time lag by watching the Sanwa during the first seconds after automatic heater switch-off (NTC log continues). For accurate resistance/temperature mapping obtain stabilized, co-located reference measurements at several temperatures; only then update firmware's NTC conversion. Keep upper-temperature tests (>50 °C indicated) paused until thermal reference and enclosure safety checks are addressed. This is distinct from the SHT45: SHT45 measures chamber air, NTC measures heater body.

## 2026-10-15 NTC steady-state thermal reference checks (30 and ~40 C)

The user completed two supervised temporary diagnostic heater-hold tests with the Sanwa RD701 type-K thermocouple positioned near the eSUN heater NTC. Unlike the preceding rapidly rising temperature comparison, these readings were taken near the end of **120 s holds**:

| Sanwa reference | ESP32 ADC-derived NTC resistance | Old firmware curve equivalent |
|---|---:|---:|
| 30 °C | 64.62 kΩ | 30.89 °C |
| 39 °C | 42.27 kΩ | 40.69 °C |

These are steady-state *comparison points*, not proof of absolute NTC calibration: the Sanwa reads integer °C with finite uncertainty; contact/placement and residual ON/OFF cycle thermal ripple also matter. The first-article ADC calibration was already separately established. Note that a prior room-state measurement showed 122.3 kΩ directly at J7-disconnected NTC with both independent temperature instruments near 20 °C, for which the old below-25 °C beta extrapolation predicts ~16.6 °C. The room-state inconsistency remains unresolved; do not apply a blanket offset.

Next proposed gate: one stabilized 50 °C NTC hold with thermocouple supervision and bounded heater control, then jointly assess 20/30/39/50 °C resistance-temperature evidence before editing NtcSensor.h. The previously proposed intermediate 35 °C hold is not necessary if the 50 °C hold yields a suitable reference. Do not extrapolate beyond the independently characterized temperature/safety envelope.


## 2026-10-15 Heater identity / thermal-range correction — model identification REQUIRED

Primary eSUN manuals show **two different hot-plate envelopes**, contrary to prior unqualified use of "80 °C original eBOX maximum":

- Original **eBOX**: **12 V / 3 A**, 36 W heater, nominal hot plate **50/60/70/80 °C**, indicative chamber **36/40/45/50 °C**. Source: https://www.esun3d.com/uploads/eBOX-Manual_User-Guide.pdf (original eBOX manual; also https://au.esun3d.com/news/ebox-filament-dryer-review).
- **eBOX Lite**: **24 V / 2 A**, 35 W heater per manufacturer user guide, nominal hot plate **105/116/118 °C**, indicative chamber **40/50/55 °C**. Source: https://www.esunchina.net/uploads/eBOX-Lite-Manual.pdf.

Our first article uses a **24 V** supply and measured **~1.59 A / ~38 W heater** at 24 V, compatible with the Lite supply/heater order of magnitude but **not proof of specific model, original temperature-control policy or sensor position**. Identify the exact donor unit before applying either manual's nominal hot-plate levels. **80 °C must not be recorded as an accepted production limit for our heater or as its verified OEM maximum.** Conversely, **50 °C** was only the upper edge of the old NTC characterization table and the validated transient test, not an OEM maximum temperature.

Safety impact: the currently installed independent heater-series one-shot **105 °C TCO**, thermally attached to the heater, may be incompatible with the normal **105–118 °C nominal hot-plate range** in the eBOX Lite manual. Do **not** replace or defeat the TCO on inference alone; verify donor model, physical TCO location/thermal coupling and safe operating envelope, then obtain explicit engineering agreement before changes to hardware safety decisions. No high-temperature tests beyond the already completed 50 °C transient/40 °C hold are authorized solely by the manual.

Thermistor curve analysis using only the two **stabilized thermocouple comparisons**: Sanwa 30 °C -> ADC-derived NTC **64.62 kΩ** and Sanwa 39 °C -> **42.27 kΩ** yields local two-point beta **~4463 K**, extrapolated **R25 ~82.7 kΩ**, agreeing closely with the old recorded **82.5 kΩ at 25 °C**. Yet this local two-point model predicts **R20 ~106.8 kΩ**, inconsistent with **122.3 kΩ** measured directly on the NTC disconnected from J7 with both the independent Sanwa heater probe and ambient SHT45 indicating near 20 °C. This inconsistency is **not resolved** by adding a uniform temperature offset. It may reflect sensor temperature/placement, thermal history, original source-point uncertainty, or another non-modeled effect. Do not claim proven cause.

Sanwa RD701 stable display well before the 120 s hold, as explicitly reported by user: do **not** portray an insufficient 120-second hold as established cause of error. Instrument specifications: temperature display 1 °C; multimeter accuracy ±(2% reading + 3 digits/°C) excluding type-K probe; Sanwa K-250PC sensor independent stated accuracy ±1.1 °C (0–250 °C). This bounds *absolute* calibration; it does not mean the plateau comparisons should be disregarded. Manufacturer sources: https://www.sanwa-meter.co.jp/japan/pdf/manual/digital_multimeters/RD700-701_EN.pdf and https://overseas.sanwa-meter.co.jp/products/accessory-fuse/k_250pc.html.

Maintain separate **heater NTC** (heater body) and **SHT45** (chamber air) roles; final chamber temperature and fan/airflow effectiveness must be characterized as a separate gate. Set a **provisional test envelope only after verifying donor model**, then step and hold at representative heater temperatures with independent surface, casing, cable and TCO checks. Do not silently approve the final firmware heater upper bound, chamber setpoint range, or TCO revision.


## 2026-10-15 Donor eSUN dryer identity confirmed from purchase record

The user explicitly requested an inbox purchase-record check. The **3DJake order confirmation (2024-11-25) and shipment confirmation (2024-11-26)** unambiguously identify the purchased donor unit as **eSUN eBOX Lite** (3DJake item ESUNCN-6922572201521), **not original eBOX**. Full personal Gmail message IDs and order-identifying information are intentionally not stored in the public repository. This establishes the purchased model; the first-article reused heater and 24 V supply are consistent with this model, but physical sensor placement/part revisions are not confirmed by email.

Official eSUN eBOX Lite manual: https://www.esun3d.com/uploads/eBOX-Lite-Manual.pdf. Manufacturer's *reference* nominal hot-plate temperatures at levels 1/2/3 are **105/116/118 °C** and corresponding inside-air temperatures **40/50/55 °C**, under stated room conditions 20 °C and 55% RH. Values are guidance for OEM complete equipment, **not a validated setpoint map for this retrofitted heater, NTC, thermostat, software or enclosure**. The previous original eBOX 80 °C plate value was from the wrong donor model and must **not** be used to choose the new operating maximum.

**Safety compatibility hold:** current first article has an independently mounted **105 °C one-shot series TCO** against the heater. eBOX Lite manual's nominal plate levels start at ~105 °C, so that safety cutoff **may trip during normal OEM-equivalent heating** depending on actual TCO temperature and manufacturing tolerances; do not defeat, replace or raise its rating without a separate explicit protection review/decision. The NTC current old empirical characterization only reaches 50 °C; it is *not* the heater's design upper range. Final operating upper limit and TCO selection remain OPEN pending thermal mapping and safety validation. Chamber air temperature measured by SHT45 remains distinct from heater NTC temperature.


## 2026-10-15 Steady-state heater NTC checkpoint at 50 C indicated

After the earlier stabilized reference readings (Sanwa 30 C / J7 ADC-derived NTC 64.62 kOhm; Sanwa 39 C / 42.27 kOhm), a further **120 s nominal 50 C NTC hold** yielded **Sanwa 47 C / J7 NTC 28.72 kOhm**. Under the *existing* 25-50 C NTC table, 28.72 kOhm corresponds to approximately **50.0 C** (at the empirical table endpoint); the reference is therefore approximately **3 C lower** at the probe position. The readings are plateau comparisons, not dynamic ramp samples. Do not confuse Sanwa surface-contact readings with SHT45 chamber-air readings.

A log-resistance versus inverse-Kelvin least-squares fit to the three stabilized Sanwa/NTC pairs (30, 39, 47 C; 64.62, 42.27, 28.72 kOhm) gives an *exploratory local approximation* **beta approximately 4624 K, R25 approximately 83.86 kOhm**, reconstructing the three Sanwa temperatures to within roughly 0.23 C *mathematically*. That figure represents internal agreement of only three points and **not physical accuracy**: the independent Sanwa RD701 type-K reference has several degrees of stated absolute uncertainty and may be positioned at a different local temperature than the embedded NTC. The fit predicts about 109.3 kOhm at 20 C, which is inconsistent with the 122.3 kOhm directly measured at the resting NTC (when both Sanwa and SHT45 were near 20 C); that discrepancy is OPEN. Thus a single fitted beta must not be claimed validated across the entire 20-118 C OEM envelope.

Retain the raw checkpoint; no integrated application NTC conversion has been changed. Any firmware conversion update requires an agreed scope (provisional 30-47 C surface reference vs intrinsic thermistor calibration) and safety testing. Do not repeat the completed 30/39/47 C plateau tests by default.


 
## 2026-10-16 Fourth stabilized NTC/Sanwa point — 60 C diagnostic target

The supervised standalone diagnostic for 60 C NTC target with 120 s nominal hold produced **Sanwa heater-contact temperature 56 C** and **J7 ADC-derived NTC resistance 19.68 kOhm** (user report on 2026-10-16). Under the **old table through 50 C and beta=4100 K extrapolation anchored at 50 C/28.8 kOhm**, 19.68 kOhm corresponds to approximately 60.0 C. This is the **fourth stabilized comparison**, in addition to Sanwa 30 C/64.62 kOhm, 39 C/42.27 kOhm, 47 C/28.72 kOhm. These temperatures are the Sanwa contact-probe readings near the NTC, not the SHT45 chamber-air readings.

A *provisional mathematical fit only* to these four data points, with ln(R) linear in 1/T(K), yields **beta=4585.8 K**, **R25=83.49 kOhm**. The four back-converted Sanwa temperatures agree within about 0.23 C under that fit (fit residual, not instrument accuracy). This model predicts NTC resistance **~108.53 kOhm at 20 C**, whereas the direct earlier cold-state NTC reading was **122.3 kOhm** with Sanwa contact temperature and SHT45 air temperature near 20 C. The cold-end discrepancy remains unresolved; do not silently force that datum onto the four-point fit or use one uniform offset.

This is a useful empirical mapping of **resistance versus the contact-probe temperature** over Sanwa **30–56 C**, not a manufacturer calibration of the thermistor and not yet an absolute heater surface safety calibration, given Sanwa RD701 type-K uncertainty and possible local gradient between probe and integrated NTC. No production firmware NTC conversion or heater safety threshold has been changed as a result. User previously reported the series 105 C cutoff bypassed; current protection configuration has not been reverified. Do not infer the independent cutoff is connected from the fact a test was run.


## 2026-10-16 TCO connection reported restored; next heater plateau planned

The user confirms the independent 105 C series thermal cutoff is again connected in the heater circuit. This report updates the current first-article test setup; no new destructive TCO trip test was performed and the final eBOX Lite full-temperature safety choice remains open. A temporary **70 C old-NTC-indicated plateau test** with 120 s hold is the next planned checkpoint (not yet executed). It retains the same legacy 25-50 C table and 50 C-anchored beta=4100 K extrapolation used in the 60 C diagnostic, rather than changing production NTC calibration. Log Sanwa near-heater contact temperature and J7-derived kOhm after the 120 s hold. Maintain short tests and independent thermal protection; the regular firmware heater stays disabled.


## 2026-10-16 Fifth stabilized NTC/Sanwa comparison: nominal 70 C NTC

At the end of the 120-second diagnostic hold with the *existing* conversion nominally targeting 70 C NTC, user measured **Sanwa 64 C** at the heater-mounted type-K probe and **J7 ADC-derived NTC resistance 13.73 kOhm**. The 50 C / 28.8 kOhm-anchored beta=4100 K extrapolation in the temporary diagnostic maps 13.73 kOhm to **70.04 C**. Difference **NTC estimate minus nearby Sanwa: +6.04 C**. The user had explicitly reported that the independent 105 C heater-series TCO was reconnected before this test; its operating condition/trip was not measured during the test.

The complete five stabilized Sanwa-C / NTC-kOhm pairs are **30/64.62, 39/42.27, 47/28.72, 56/19.68, 64/13.73**. Exploratory straight-line regression of ln(R) against reciprocal absolute Sanwa temperature yields **beta approx. 4645.8 K**, **R25 approx. 84.20 kOhm**, largest back-converted residual about **0.33 C** across those five points (mathematical internal agreement, NOT metrological accuracy). At Sanwa 20 C the five-point model predicts **about 109.83 kOhm**, not the prior directly observed **122.3 kOhm** at ambient. The room-temperature discrepancy remains open. The systematic growing difference between the old firmware estimate and nearby Sanwa reading could combine inaccurate R-to-T conversion and a heater surface thermal gradient; neither effect has yet been isolated. The Sanwa and NTC are distinct thermal points and the Sanwa RD701 K-probe absolute accuracy is limited.

No production NTC conversion or heater-safety threshold was changed. Do not confuse the 70 C old-NTC diagnostic target with a verified heater surface temperature or full-range calibration.

 
## 2026-10-16 Diagnostic nominal 80 C heater test aborted by premature firmware timeout

The first temporary 80 C indicated NTC / 120 s hold sketch **did not reach its 80 C NTC target**: it automatically transitioned to COOLDOWN due to **PREHEAT_TIMEOUT_MS = 150000 ms**. The user confirmed the serial stop reason was the preheat timeout. This timeout was selected without measured 70->80 C ramp-time evidence and caused a **test-procedure limitation**, not a confirmed electrical fault, TCO operation, or NTC characterization result. Do not record any sixth 80 C plateau point from this aborted attempt.

Next diagnostic is the **same nominal 80 C NTC, 120 s hold** with an explicitly announced longer **300 s bounded preheat observation**, continuous NTC validity and 83 C software stop, and auto-off; the 300 s remains an experiment-time safety cap, not a predicted time-to-target or final-control setting. All prior 30/39/47/56/64 C Sanwa points and old 25-50 C table plus 50 C-anchored beta=4100 extrapolation are retained unchanged for comparability. The user reported the 105 C off-board series TCO restored before these trials, though no destructive cutoff test was performed.


## 2026-10-16 Sixth stabilized NTC-Sanwa comparison — nominal 80 C target

After the revised standalone diagnostic with 300 s bounded preheat and 120 s nominal 80 C NTC hold, user reports **Sanwa 74 C** (contact thermometer near heater NTC), **NTC 9.81 kOhm** (J7 ADC-derived). Existing diagnostic above 50 C uses beta=4100 K anchored at 50 C/28.8 kOhm and computes **79.97 C** for 9.81 kOhm; difference to nearby Sanwa approximately **+5.97 C**. No Sanwa-independent heater hotspot or enclosure/cable temperatures were reported; do not treat nominal 80 C indication as an actual safe temperature limit.

Six stabilized Sanwa-C / NTC-kOhm points: **30/64.62, 39/42.27, 47/28.72, 56/19.68, 64/13.73, 74/9.81**. Exploratory six-point least-squares fit ln(R)=A+B/T(K) yields **beta about 4560.81 K, R25 about 82.98 kOhm**, maximum back-conversion discrepancy about **0.74 C** on the six data points (internal fit only, not Sanwa accuracy). Predicted R20=**107.72 kOhm**, in tension with 122.3 kOhm earlier directly recorded near ambient temperature. Fit estimates a nearby Sanwa contact temperature of about **82 C** at the **old** NTC diagnostic 90 C point, which is a prediction not verified or absolute heater measurement.

Next proposed diagnostic checkpoint: hold **90 C legacy NTC indication for 120 s**, only after confirming no heat damage to housing, wires, crimps, and independent 105 C TCO remains mounted/in-series as reported. Keep same old NTC lookup/beta conversion in temporary test for comparability and bounded 300 s preheat, NTC thermal fault stop and independent supply disconnection. High-range manufacturer 105-118 C plate levels remain unvalidated; do not equate Sanwa adjacent-probe temperatures with the integrated NTC thermistor or the hottest heater spot. No production firmware changes.


## 2026-10-16 Nominal 90 C NTC plateau attempt: preheat timeout, not a characterization point

User reports the temporary **90 C legacy-NTC-indicated** standalone heating diagnostic ended in **timeout**. As coded, the diagnostic has a 300 s maximum preheat; successful entry to a 120 s hold and completion would occur by ~420 s, ahead of the 425 s active watchdog. Thus this report most likely indicates **90 C was not reached within 300 s**. No final NTC resistance, Sanwa contact temperature, bench supply current or exact serial stop line were yet supplied. **Do not add a 90 C point to the NTC table.**

Next action is **diagnosis from the existing last serial log and bench power indication**, not increasing the preheat timeout blindly. Ask for last `NTC=` temperature and whether supply current remained near the earlier measured ~1.6 A heater load (allowing for board current) or dropped sharply despite firmware HEATER=ON. Sustained heater current with a slowly asymptotic NTC reading suggests thermal equilibrium/insufficient transfer, not necessarily a fault. Current collapse with HEATER=ON suggests an open heater path, potentially the non-resettable 105 C TCO, connector or wiring; with 24 V disconnected, inspect continuity using the previously measured ~15 ohm complete heater path at J5. Keep 105 C TCO in series; do not bypass it or escalate the 90 C test until diagnosis. All six 30-74 C Sanwa data remain unchanged.


## 2026-10-17 Completed legacy NTC-indicated 90 C hold — seven-point checkpoint

The user supplied the complete standalone serial record (613 lines) and an independent heater-contact **Sanwa reading of 85 C** at the end of the **90 C indicated** hold. The 120-second HOLD phase completed normally: **HOLD COMPLETE**; at **t=531.2 s from start**, **NTC=89.98 C**, **NTC resistance=7.12 kOhm**, **HEATER=OFF**. Recorded maximum indicated NTC **90.09 C**. PREHEAT entered HOLD at about **t=411.2 s** (6m51s): the prior **300-second timeout** happened because indicated NTC temperature at **t=299.8 s was 85.12 C**, while the heater was still reported **ON**. Additional PREHEAT log samples: t=239.8 s => 81.16 C, t=359.8 s => 88.33 C, t=409.8 s => 89.92 C. Thus the prior 300 s abort was simply too short for this heating ramp; **this run alone is not evidence for TCO interruption or a true plateau**.

At the 90 C hold, among the **121 approximately 1-second HOLD readings**, the heater was reported **ON for 110, OFF for 11** (~91% nominal ON indication). This is a coarse sample-based proxy for substantial duty, **not a precision measurement of electrical power, instantaneous current, TCO case temperature or thermal safety**. It suggests thermal headroom is decreasing at the current ambient/casing/fan configuration. Status of enclosure/ventilation during this trial is not confirmed in the available evidence; do not assume heater was enclosed or fan running (firmware PIN_FAN commanded LOW). The local Sanwa-to-old-NTC indicated difference was **+4.98 C**, smaller than the 6 C differential at 80 C indicated.

Seven stabilized independent Sanwa-C / J7-NTC-kOhm pairs: **30/64.62; 39/42.27; 47/28.72; 56/19.68; 64/13.73; 74/9.81; 85/7.12**. A purely exploratory 7-point regression ln(R) vs 1/T(K) returns **beta ~4421.6 K, R25 ~80.69 kOhm**, max residual ~1.64 C when converted back to reference temperature. The 7th point deviates by about 3 C from the six-point surface-reference model at that same resistance; this does not prove thermistor nonlinearity vs thermal gradient and Sanwa uncertainty. The earlier ambient 20 C / 122.3 kOhm discrepancy persists. Do not silently replace production thermistor parameters, do not claim intrinsic thermistor calibration, and do not extrapolate to OEM 118 C heater plate.

**Next gate, before attempting 95 C or 100 C indicated**: verify actual thermal setup (heater inside closed eBOX Lite enclosure vs open/exposed, cover and fan conditions), nearby polymer/cable/TCO thermal contact, and intended independent thermal protection for OEM temperature range. The **105 C one-shot TCO** is reported restored, but is not necessarily compatible with full OEM ~105-118 C nominal plate range; do not bypass it. Once configuration and protection are confirmed, choose a bounded thermal-capability test with supply current/airflow observation rather than blindly extending time targets.


## 2026-10-17 Critical physical-configuration clarification for all heater measurements

User confirms that during the prior heater/NTC/Sanwa characterization the original eSUN eBOX Lite enclosure had **both the bottom/base closure panel removed/open and the top hinged spool-loading lid open**. The temperature measurements 30/39/47/56/64/74/85 C at the heater-adjacent Sanwa probe therefore characterize an **open enclosure**, with strong heat loss, **NOT the complete closed dryer under OEM reference conditions**. In particular, the ~411-second rise to 90 C under the old NTC conversion and ~91% sampled heater ON indication during the 120-second hold cannot support a conclusion that the 35–38 W heater cannot reach the manufacturer’s 118 C nominal hot-plate reference in the closed original enclosure. It also cannot verify that it will reach that temperature; manufacturer plate location, built-in thermistor and Sanwa thermocouple location may differ.

eSUN eBOX Lite manufacturer manual states OEM level 1/2/3 nominal hot-plate reference **105/116/118 C**, corresponding *inside air* reference **40/50/55 C**, tested at room temperature 20 C/55% RH, and advises not opening the cover during drying. Source: https://www.esun3d.com/uploads/eBOX-Lite-Manual.pdf .

**Next meaningful validation is a controlled closed-assembly versus open-assembly thermal-behavior comparison, not another blind higher target.** If the bottom panel and upper lid can be restored without trapping or damaging wiring/thermocouple, inspect heater assembly and TCO wiring first and use the existing bounded 90 C diagnostic sketch (no calibration firmware changes) to compare warm-up time and time-on fraction, while supervising adjacent plastics and hardware cutoff. If the enclosure cannot yet be restored safely, do not claim equivalence to OEM specs; instead first plan the physical sensor/wire routing. The original 105 C one-shot independent TCO remains series-connected per user's prior confirmation but can trip at high real plate temperature, making it an unresolved compatibility item before validating the upper nominal OEM 105–118 C levels. Normal firmware continues to inhibit heater.

 
## 2026-10-17 Testing plan clarification: OEM fan, accessibility and calibration versus system qualification

User confirms that in the **original eBOX Lite the fan normally runs throughout the drying/heating operation**. All standalone heater/NTC/Sanwa temperature plateau tests so far were performed with **fan commanded OFF**, bottom and top enclosure panels open. These heater-adjacent measurements are useful **only as comparative, setup-specific NTC-resistance versus reference-probe-temperature data**; they do not simulate OEM ventilation, closed-case thermal distribution, or the complete user-operational dryer. Even a closed-case repeat while fan OFF would still not reproduce OEM conditions. Both closing panels around temporary exposed wiring/thermocouple and repeating 90 C purely to compare closure were judged impractical at this development stage.

Agreed practical separation: **(1) Bench sensing/calibration work** may continue with heater accessible and enclosure open, avoiding repeat plateau tests already completed; explicitly limit interpretation to reference-probe thermal location and the tested configuration. Seven measured stable pairs through Sanwa 85 C are in the prior checkpoint; ambient 20 C anomaly and probe-location/thermal-gradient uncertainty remain open. Next focus: inspect the seven-point fit, diagnostic conversion scope and reason for ambient discrepancy, without extending high-temperature heater tests for the sake of the OEM nominal 118 C. **(2) Final thermal system validation** later, after enclosure, routing and fan control are sufficiently integrated, must be under representative closure and airflow, measure NTC heater location plus chamber SHT45 temperature and accessible hotspot/material temperatures, and verify suitable independent thermal protection. The independently restored 105 C series TCO remains a compatibility issue with OEM nominal heater plate temperatures, and must not be bypassed.

The earlier “next meaningful validation requires immediately restoring the whole enclosure and using the fan-OFF 90 C sketch” recommendation is **superseded** by this practical two-stage test plan; it is not a new hardware design decision. Final heater temperature limits and closed-loop fan strategy are still open.


## 2026-10-17 Seven-point bench calibration analysis and passive diagnostic viewer implemented

Completed the promised no-heating reanalysis in **`docs/NTC_CALIBRATION_ANALYSIS.md`** and added **`firmware/diagnostics/ntc_reference_viewer/ntc_reference_viewer.ino`**, an *optional*, standalone, non-heating serial viewer. **The integrated `firmware/NtcSensor.h`, `firmware/AppConfig.h` and `firmware/SafetyController.h` are unchanged**; heater remains forcibly OFF under the actual application firmware.

Using seven *open-case/fan-OFF* stabilized Sanwa-contact-C versus ADC-NTC-kOhm pairs 30/64.62,39/42.27,47/28.72,56/19.68,64/13.73,74/9.81,85/7.12, the one-beta fit to Sanwa-contact temperature gives beta **4421.571 K**, R25 **80690.93 Ohm**; within-sample max abs contact-temperature residual **1.64 C**, RMSE **0.89 C**. Piecewise linear contact-temperature interpolation over **ln(Ohms)** exactly passes through seven recorded pairs *by construction* and is valid **only in observed R=7.12–64.62 kOhm**. Five interior leave-one-out neighbor interpolation errors -0.10,+0.59,-0.29,+1.31,+0.75 C (RMSE ~0.74 C); neither model's residual establishes thermistor absolute accuracy.

Crucial new **hypothesis, not decision or thermistor identification**: earlier direct cold/room-temp NTC **122.3 kOhm at ~20 C** (J7 ~124.4 kOhm) is consistent within ~2.5% of **100kOhm at25C with beta3950**, which predicts **125.35 kOhm at20C**; one-beta contact-Sanwa fit extrapolates to only **103.91 kOhm at20C** (~16.87 C at observed 122.3k). A genuine embedded 100k/B3950 NTC might be hotter than a neighboring externally attached Sanwa thermocouple when heated; from measured R it would compute ~35.2 C versus Sanwa30 and ~99.3 C versus Sanwa85. This is unproven and also requires reconciling original donor cooldown report 82.5k at nominal25 C. This may be a *location/measurement mismatch*, not a bad component. Do not silently treat provisional equivalent Sanwa contact temperature as the thermistor's physical temperature or use its lookup for heater closed-loop control.

The optional passive diagnostic uses the same ADC calibration, always commands heater GPIO19 LOW/fan GPIO16 LOW, and displays resistance, actual integrated legacy firmware temperature, historical standalone-heater-sketch temperature, range-limited piecewise Sanwa-equivalent, range-limited 7-point-beta Sanwa prediction, and explicitly labelled *unverified* 100k/B3950 NTC hypothesis. Outside observed 7.12-64.62kOhm it prints OUT_OF_MEASURED_RANGE for Sanwa-equivalent, especially at room temperature. Sketch was committed but has **not yet been compiled/flashed or tested physically**; no change to production firmware or accepted safety protections. Next optional test, **without heating**, is a paired independently measured thermistor-body temperature and resistance at stable ambient; final thermal performance waits for closed box plus fan operation and suitable independent series cutoff.


## 2026-10-17 Project integration sequencing and front-panel print

The user wants to proceed with NTC assessment **and** the project-wide next steps, and is considering printing a **new front panel** while the eBOX Lite heater is disassembled (so the user cannot use the dryer to pre-dry filament). This is **not a blocker** for an initial print if the available filament prints acceptably. Printing a **dimensional/fit prototype now** is the recommended critical-path step: it will unlock mechanical mounting of the first-article PCB, OLED and buttons and eventually safe close-up. A test print and final temperature/material validation are distinct. Never assert PLA (or another polymer) has sufficient heat performance for the finished enclosure without testing local temperatures. The current GitHub tree has a board-named `hardware/Filament_dryer_pcb.stl`, but **no identifiable front-panel/cover CAD master or printable export**. The user's local Inventor panel geometry and its readiness/clearances were NOT independently verified; request it for review when possible and add source/export to the repo once approved. The PCB source and actual module/button geometry are authoritative, not chat recollection.

The full gated project sequence is now documented in **`docs/PROJECT_ROADMAP.md`**: (0) front-panel dimensional print; (1) PCB mounted UI/SHT45/fan demo with application heater forcibly OFF; (2) optional passive NTC ambiguity resolution without retesting seven completed hot plateaus; (3) final mechanical/harness assembly; (4) independent heater TCO/thermal controller engineering and fault approvals; (5) actual **closed box, fan-on** heater/air qualification; (6) standalone V1 firmware; (7) logging/web UI; (8) deliberate PCB revision/DRC only as needed. The existing `docs/NTC_CALIBRATION_ANALYSIS.md` and safe passive `firmware/diagnostics/ntc_reference_viewer/ntc_reference_viewer.ino` have been inspected. Normal `firmware/SafetyController.h` still hard-locks GPIO19 LOW; no heater enable is authorized by this roadmap.


## 2026-10-19 Front-panel print confirmed by user; advance to closed-box no-heater integration

The user reports that the **new front panel has been printed and is OK**; physical printing is no longer a blocker. This is a user-reported mechanical fit, not an independently inspected CAD revision or confirmation that the printed polymer will tolerate eventual nearby heater temperatures. The repository currently does not contain a verified editable source/export of the newly printed panel; obtain it if/when available, rather than requiring another print before testing.

A new **`docs/CLOSED_ENCLOSURE_NO_HEAT_TEST.md`** documents the immediate next supervised prototype acceptance run **with top lid and bottom panel closed, heater's J5 power harness physically DISCONNECTED and insulated, actual PCB mounted in printed panel, and current integrated SAFE BRING-UP app active**. NTC/J7 MUST remain connected for this test because `SafetyController.h` latches NTC electrical faults and inhibits the fan; SHT45 and fan must also be properly connected. From STANDBY a short ON/OFF begins a DRYING **fan-only** demo (100% one-second kick-start, then 40% at 25 kHz), with heater GPIO19 forced LOW continuously; ON/OFF long press stops cycle and returns STANDBY, fan OFF. Verify no rubbing, cable pinching, fault, supply reset, UI pressure/button sticking, SHT45 activity, and no automatic restart on power-cycle. This is **NOT closed-box thermal qualification** and no experiment was run by the assistant; report results after user execution.

The previously requested seven-point NTC comparison and overarching roadmap were already completed in `docs/NTC_CALIBRATION_ANALYSIS.md`, `firmware/diagnostics/ntc_reference_viewer/ntc_reference_viewer.ino` (heater-disabled), and `docs/PROJECT_ROADMAP.md`. No further hot open-box repeats or changes to `NtcSensor.h`, `SafetyController.h` or heater controller safety status are needed to perform the immediate mechanical+fan test. The 105 C one-shot TCO's fit with OEM nominal 105/116/118 C plate reference remains unresolved; actual closed **heater-energized** tests await a deliberate independent cutoff/safe operation gate. Never bypass it to make OEM target temperature. Cold room 20 C /122.3kOhm NTC discrepancy remains open; 100k/B3950 is only a hypothesis, not adopted calibration.


## 2026-10-19 Assembled front panel and PCB; heater J5 left connected — resume integrated UI application

User reports **all parts are now physically assembled** and explicitly intends to **leave the heater wiring connected**. Previous proposed initial hardware-isolated UI/fan acceptance (`docs/CLOSED_ENCLOSURE_NO_HEAT_TEST.md`, J5 heater DISCONNECTED) therefore **has not been carried out in its specified configuration**. Assembly, mechanical clearance, actual heater output current and software-only safe-operation have **not** been independently observed or accepted. No heater-powered testing has been authorized.

Read current `main` firmware at HEAD: the existing **Slewform animated startup, standby/setup/settings, DRYING TEST, SYSTEM technical page, SHT45, NTC, buzzer, four buttons and fan PWM** are already integrated in `firmware/firmware.ino`. `SafetyController.h` initializes GPIO19 LOW and drives it LOW **each loop**, exposes `heaterPermitted() == false` with **no heater-enable route**. The actual schematic/netlist has R20 100 ohm between GPIO19 and Q4 gate, R21 100 kohm gate-to-GND pull-down, Q4 IRLR3636TRPBF. No firmware edit is required merely to keep heater commanded OFF; the series TCO remains installed per prior user report and should not be bypassed. Firmware lockout is **not equivalent to physical disconnection** and cannot mitigate a failed-short MOSFET/incorrect loaded image, so supervision and verification of no heater current/temperature rise are still essential.

**Next operational step:** restore/reflash the **integrated SAFE BRING-UP sketch**, not the earlier 90 C `H`-activated heater diagnostic. This is crucial while heater J5 is physically connected. If flash/upload requires 24V with old heat diagnostic still installed, temporary heater-path isolation during firmware replacement is the conservative safeguard. Then run only supervised UI/SHT45/fan tests, with no heating command, verifying no actual heater activity, panel/button/OLED fit, 1s 100% then 40% fan duty, SHT45, J7/NTC fault behavior, stop/reset without auto-restart. User-specific variation and checklist in `docs/ASSEMBLED_UI_SOFTWARE_HEATER_OFF_TEST.md`. Results not yet reported; do not mark as passed. The earlier J5-disconnected protocol remains available when physical isolation is required. Future heating still requires separate safety/TCO and sensor-calibration gate.


## 2026-10-19 historical test source recovery — merged archive

After the user flagged that previously working temporary Arduino sketches were not archived, repository history and recoverable 2026-10-07/08/11 project-chat test evidence were inspected. PR #2 merged an archive to `main`: `firmware/test_archive/TEST_FIRMWARE_ARCHIVE.md`.

**Exact source:** the complete integrated 2026-10-08 source at historical commit `3535dba9643e77d48e14eccc109606e3a5d758cf` is copied under `firmware/test_archive/validated_integrated_2026_10_08/` using 14 Git blob identities verified equal to the historical source, including the sketch, modular headers and Slewform assets. The chat later reports «Funziona tutto» after this commit. Important limit: this broad observation does not explicitly establish that every later safety or thermal gate had passed. The original integrated source already configured fan LEDC channel 9 and buzzer channel 8, just like the later `main` firmware, so a clash of those original channel numbers is **not established as the root cause** of the current fan-off regression.

**Chat-procedure reconstructions, NOT originals:** four fan standalones (static OFF→ON; 25 kHz 0→50%; 50/75/100% 7-second stages; 20→60% startup / 50→10% run thresholds), plus I²C scanner, SHT45 direct command/CRC, SSD1309 OLED text/frame, and GPIO34 NTC raw ADC. Each reconstructed `.ino` is marked as NOT byte-for-byte original and NOT yet re-run on the board. Historical measurements and chat dates are in the archive index.

**Source recovery incomplete:** exact original standalone contents for early I²C/OLED/logo, Q5 switching, button/buzzer, heater 80/90 C sketches remain unavailable in tracked Git history; the latest *heater-OFF* NTC reference viewer was already saved at `firmware/diagnostics/ntc_reference_viewer/`. No uncontrolled heater-enable sketch was fabricated. Do not present reconstructed scripts as proven identical to originals.

**Live issue outstanding:** user's assembled DRYING TEST showed `F 40%` with fan stopped. J6 pin 2 ≈24 V, switched J6 pin 1 ≈23 V OFF and commanded ON, Q5 gate ≈0 V, GPIO16-side R25 ≈0 V, U4 pin 27 ↔ R25 pin 2 0.3 Ω unpowered; touching U4 pin 27 with DMM probe made fan start. The user's subsequent NTC/STATE-only serial log shows **no `FAN PWM` diagnostic lines** introduced on the separate `fix/isolated-fan-ledc-2026-10-19` branch, so actual testing of that firmware remains unconfirmed. The fix branch is **not merged**, and current main app remains unchanged.

Next: identify flashed firmware version, use physically isolated fan tests as appropriate, distinguish GPIO waveform from UI command, and verify electrical conditions without powering heater. The hardware and heater safety decisions remain unchanged.


## 2026-10-20 U4 pin 27 solder repair — DRYING fan start restored; acceptance resumed

**New physical evidence (user-reported):** The fan failing to run under DRYING was traced to the actual **solder joint of the ESP32-WROOM-32E U4 physical pin 27 (GPIO16)**. Before correction, applying the positive DMM probe to that pin caused the fan to start; DC probes at R25 and Q5 had previously shown ~0 V, and J6 pin 1 stayed near 23 V. The unpowered U4-pin27 → R25 pin2 continuity of ~0.3 Ω did not rule out an intermittent pin-to-pad connection under probing pressure.

**After correcting the solder joint:** the user observes the fan starts immediately at **100%** and then continues at **40%**, as specified by the integrated SAFE BRING-UP policy. This is a direct observed recovery of the actual fan, not just the displayed `F 40%` or peripheral readback. The user had already reported passing five SETUP checks and five SETTINGS checks (including persistence after cycling power), entering DRYING and opening the technical page. Do not repeat those passes without reason.

**Diagnosis/correction:** GPIO16 physical joint on the first-article assembled board; **no evidence here that Q5 failed again**, nor proof of any LEDC timer collision. The experimental `fix/isolated-fan-ledc-2026-10-19` branch moved the fan from LEDC channel 9 to 0 and added serial diagnostics. An earlier log/screenshot indicates that version was flashed at one stage; the exact binary/commit running during the successful post-solder test has not been reidentified. Main still configures channel 9. **Do not treat the speculative LEDC change as a verified firmware fix** or merge it automatically. The `diagnostics/fan-gpio16-static-2026-10-20` test branch was prepared but no execution was reported; it is no longer the immediate next test.

**Acceptance remaining:** the 100% → 40% transition passes on user report; full 2–3-minute run under supervision, verification of actual heater physical OFF (particularly because J5 was previously left connected), stable fan airflow and case/wire clearance, SHT45 live chamber T/RH, deliberate ON/OFF long stop, power-cycle no auto-resume, and documenting startup anomalies/material remain pending. Continue at `docs/ASSEMBLED_UI_SOFTWARE_HEATER_OFF_TEST.md` rather than reopening the recovered fan sketch issue. A software heater lockout **does not substitute for physical J5 isolation**. The independent 105 °C TCO and hot closed-box qualification gates remain unchanged and open.


## 2026-10-20 assembled DRYING extended run and long STOP — PASS on user report

Following the repaired ESP32 U4 pin 27 solder joint, user was instructed to run **2–3 minutes at 40% fan**, monitor for stall/scraping/resets and abnormal heater warming, then use **long ON/OFF (~1.5 s)** to stop. User replied **«Tutto ok»**, confirming the requested test sequence passed without reported issues. This records **fan sustained operation and stop into STANDBY with fan stopped as user-observed PASS**. No quantitative SHT45 values, current measurements, waveform captures, heater temperatures, or electrical no-heat measurements accompanied the response; do not claim them. J5's physical connection/isolation was not freshly confirmed and software OFF remains the only confirmed control.

**Next:** perform a complete power-off/on after DRYING was stopped and verify fresh boot to STANDBY with fan OFF, no automatic cycle resume. Then record live SHT45 temperature/%RH on the assembled technical page, OLED/fit and front-panel material, and check heater physical inactivity. Hot heater activation remains gated by the previously documented independent TCO and thermal safety requirements. Firmware remains unchanged by this documentation checkpoint. Experimental channel0 branch was not merged.

## 2026-10-20 complete power-cycle no-autoresume — PASS (reported)

The user completed the requested complete 24 V + USB removal and subsequent reapplication following the long-ON/OFF DRYING stop, and reported **«Test passato»**. For the specified test this confirms user-observed normal Slewform boot, return to **STANDBY**, fan **OFF**, and **no automatic cycle restart**. This closes the application restart/no-auto-resume observation; it is not a validated fault-injection safety test. The earlier first-article **SETUP 5/5**, **SETTINGS 5/5** (including persistence), fan 100%→40% startup after GPIO16 solder repair, sustained fan-only DRYING run and long-ON/OFF stop are still accepted as user-reported passes.

**Next:** capture present SHT45 chamber temperature/RH and NTC on UI/SYSTEM page, with heater command locked OFF; record enclosure/wire route observations and actual electrical heater OFF separately. The series 105°C TCO compatibility and closed-box hot-use qualification remain unresolved. Physical heater J5 disconnection has not been reported for the currently assembled test. Do not change heater safety decisions or mark closed-enclosure heater qualification passed.


## 2026-10-20 photo evidence of OLED SHT45/NTC display — PASS for reporting, not calibration

The user supplied three photographs of the assembled control panel OLED:
- STANDBY: Filament Dryer, SHT45 chamber temperature **19.4 °C**, relative humidity **69.7%**, Ready.
- DRYING TEST: **19.4 °C**, **69.7% RH**, prototype target **SET 54 °C**, FAN **40%**, HEATER LOCKED.
- SYSTEM / BRING-UP: Chamber **19.4 °C** (SHT45), Heater **15.5 °C** (NTC/J7, provisional conversion), H **70%** (rounded from 69.7% by Ui.h), F **40%**, HEATER LOCKED.

No sensor WAIT/FAULT was shown. This is a PASS for observing valid-looking digital measurements and the correct screen/state formatting, *not* independent confirmation of sensor accuracy, steadiness over time, or final NTC calibration. The **3.9 °C discrepancy** between NTC-indicated heater temperature and SHT45 air is not automatically evidence of a defect; the two sensors have different locations and the cold NTC model remains unresolved in docs/NTC_CALIBRATION_ANALYSIS.md. Prior cold log showed NTC empirical=no.

The photographs show screens and panel, not a verified sealed-dryer mechanical/airflow acceptance. HEATER LOCKED is software/UI only, not measured Q4/J5 off-current; physical J5 status has not been freshly reconfirmed, and independent 105 °C TCO suitability / closed-hot operation remain OPEN. Images were supplied in chat; their observed readings, not the image binaries, are archived here.

Next: confirm J5 is actually connected/disconnected, check physical heater OFF independently before any hotter stages, document housing fit/wiring/filament and SHT45 location, then plan safety/NTC research without modifying heater control. Experimental fan LEDC-channel0 branch remains separate and not a proven repair.


## 2026-10-20 assembled heater connector J5 still physically connected — confirmed

User answered `Confermato`: the **actual heater power cable remains connected at J5** on the assembled prototype while the SAFE BRING-UP application is displayed with HEATER LOCKED. This establishes actual test-harness status by user report; does **not** demonstrate MOSFET Q4 electrically off. Do not mark the J5-disconnected isolated-enclosure acceptance as passed.

The released netlist directly confirms **J5 pin 2 = 24V_PROT**, **J5 pin 1 = /IO_Power/HEATER_SW = Q4 drain**. The off-board heater and series TCO bridge between J5 pins 2 and 1. In the DC **OFF** state with intact load/TCO, about 24 V is expected on both pins relative to GND (nearly zero differential voltage across heater). With Q4 on, pin1 can fall to about 0 V and approximately 24 V across the heater; do not deliberately energize Q4. Software `SafetyController.h` keeps GPIO19 LOW, but this must be verified electrically. An open TCO/connector can alter predicted floating-voltage readings; absence of heater temperature rise and supply current can independently corroborate OFF.

**Pending controlled nonheating measurement** (not physically performed by assistant): power fully removed before placing any insulated leads, DC voltmeter only, avoid bridging exposed neighboring J5 terminals. After wiring prepared and inspected, energize to STANDBY under supervision with emergency 24 V disconnection. Read J5 pin2→GND and pin1→GND; both nominally about +24 V if Q4 does not sink load, subject to circuit continuity; any pin1 near 0 V and/or thermal/current rise => stop immediately, do not proceed with heater experiments. Bench supply current can be a noninvasive first screen if direct connector measurements are not safely accessible. No current result or PASS has yet been reported.

Open thermal gate: independent series 105°C cutoff suitability and closed heater operation remain unverified; no deliberate heater enable is authorized.


## 2026-10-20 correction: do not repeat completed Q4/J5 heater switching tests

Following the user's objection to yet another J5 electrical-voltage-probing request, the existing engineering records were rechecked. **Already completed and passed:**

- **2026-10-08 Gate 5B unloaded drive test:** Q4 gate 0 V safe-OFF, 100k pull-up to the switched output showed J5 pin1 = 24 V OFF, GPIO19 commanded HIGH produced gate 3.3 V and J5 pin1 around 1 mV ON.
- **2026-10-14 Gate 5B actual heater load test:** J5 connected to actual heater with series 105 °C TCO and NTC J7; one-shot 1/5/10 s timed pulses, roughly 1.59 A at 24 V, rising NTC and automatic shutoff; no current-limit, fault or abnormal switching reported.
- **By 2026-10-17:** seven open-box, fan-OFF NTC/Sanwa heater temperature plateaus completed, with highest nominal 90 °C NTC / 85 °C nearby Sanwa. These did not validate sealed, fan-running thermal behavior.

Consequently **do not put repeated Gate 5B switch OFF/ON measurements on the immediate to-do list**, and do not ask the user to re-probe J5 for a previously passed bench functional test. On the current assembled fan-only firmware, the user confirms J5 is physically connected, the app is `HEATER LOCKED`, and no unexpected warming was reported during the 2–3 min DRYING test; this is **not proof of independently safe heating control or full electric OFF across all failures**. When commissioning a **new integrated heater-control firmware**, separately validate startup/off-state, sensor-fault inhibition, and physical heater disconnect/independent TCO under the approved **future safety gate**. These are not the same as repeating historical functional bench checks.

The next *new* work is mechanical sensor/TCO positioning and safety requirement review, then an intentionally authorized heater regulation design. No heating code, circuit decisions, or bench test results were changed by this documentation correction.


## 2026-10-22 active work order: close-case geometry first, TCO second

User explicitly directed immediate focus on **Step 1 (actual closed eBOX Lite geometry, SHT45 chamber sensor, Sanwa plate contact, heater/NTC/fan/TCO wire routing)**, followed by **Step 2 (independent 105 °C cutoff evaluation)**; do not reopen unrelated enclosure cosmetic design or previously completed Q4/ventola functional bench tests. A focused no-heat stage-1 inspection and test protocol was committed on `plan/closed-heater-qualification-2026-10-20` at `docs/CLOSED_CASE_STAGE1_SENSOR_PROBE_FAN_NO_HEAT.md`. This has not been physically completed or reported yet. Its immediate unknowns are actual SHT45 and Sanwa placement and safe probe/wire exit with both covers closed. For a formally isolated no-heat fan test, disconnect J5 with power removed, keep J7/SHT45/fan connected; if impracticable, record this limitation rather than marking that gate passed.

Step 2 will inspect actual TCO part marking/manufacturer and thermally effective mounted location, against OEM plate references, before any closed active heater run. No modifications to production heater safety, application firmware, or DECISIONS have been authorized by this plan.


## 2026-10-22 operational resumption: closed enclosure phases A then B (plan only)

The user has directed the project to proceed specifically through **(A) heater enclosure preparation, then (B) 105 °C independent thermal cutoff review**, and to skip any further discussion/measurements of the front panel. Detailed instructions, known vs missing evidence and acceptance gates recorded in `docs/PHASE_A_B_CLOSED_DRYER_PREP_2026-10-22.md` on branch `plan/closed-heater-qualification-2026-10-20` (not yet merged).

A: SHT45 must report representative chamber air in spool zone; Sanwa contact probe must remain at a reproducible location on heater surface while both covers can close; J3/J5/J6/J7/TCO cable routing safe and fan unobstructed, all inspected with power OFF. None of these new physical details have yet been evidenced by photos or direct observation of completed closure. Previous fan/OLED/NTC electrical functional test results remain valid and will not be repeated.

B: Existing one-shot TCO reportedly 105 °C / 15 A / 250 V in series at J5 HEATER+, body held in contact with flex heater with Kapton. Manufacturer eBOX Lite reference **105/116/118 °C plate vs 40/50/55 °C air** and current 105 °C cutoff mount create a potential incompatibility for the OEM high level. Exact TCO MPN, `Tf`, `Th`, `Tm`, DC interrupt compatibility and actual body temperature at full regime are unverified. Do not bypass, arbitrarily relocate or substitute. Verify source before new safety decision. No heater enable sketch or thermal hot operation authorized by this planning update.

## 2026-10-22 end-to-end completion plan — planning-only checkpoint

On the existing feature branch `plan/closed-heater-qualification-2026-10-20`, [`docs/COMPLETION_PLAN_2026-10-22.md`](COMPLETION_PLAN_2026-10-22.md) now defines a gated path A–H from closed mechanical/sensor setup through TCO qualification, NTC/safety, separately approved heater diagnostic firmware and thermally supervised closed tests, autonomous V1 firmware, CSV/web and final validation/release. It consolidates existing `PROJECT_ROADMAP.md` with current 2026-10-22 A/B preparation and does not supersede `DECISIONS.md` or actual source. **Current gate is A1**, physically checking SHT45, Sanwa, TCO and wire/probe routing with both covers; **B** is the exact TCO part/rating/location decision. All PASS statuses are unchanged and no heater activation or design change is authorized. Plan saved on feature branch, not yet merged into `main`.
