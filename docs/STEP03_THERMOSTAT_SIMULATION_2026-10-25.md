# Step 03 — termostato SHT45 simulato, GPIO19 bloccato (25 ottobre 2026)

**Stato:** PASS **SOFTWARE / host + build ESP32**, non collaudo PCB, non qualifica termica.

- Repository `alessandrocardelli/filament-dryer-monitor`.
- Branch di sviluppo `feature/step03-heater-locked-thermostat-sim-2026-10-25`, basato sull'HEAD operativo `9df0f04bb6fe9d8cbd6f8b5c9ee7668f2a4dd811`.
- PR [#5](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/5) verso **solo** `feature/wifi-ota-web-logging-2026-10-24`; `main` escluso.
- **Verifica CI PASS:** [GitHub Actions #40192603027](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40192603027). Runner Ubuntu 24.04; Arduino ESP32 core **3.3.12**, U8g2 **2.36.19**, `arduino-cli compile --fqbn esp32:esp32:esp32 firmware --warnings all`.
- Firmware compilato: **1.116.910 bytes (85% di slot applicativo)**, RAM globale **67.968 bytes (20%)**. La compilazione emette tre avvisi `snprintf` sul display `Ui.h` preesistenti e *non* introdotti da questa modifica; non impediscono la build ma meritano eventuale review UI in seguito.

## Modifica minima al firmware eseguibile

- `firmware/AppConfig.h`: `kHeaterCommissioningMode=false` **e** `kHeaterOutputEnabled=false`; `kHeaterProfileApproved=false`, `kHeaterNtcModelApproved=false`; tutti i limiti termici restano zero/non configurati. Non compare alcuna curva NTC approvata; non si prova a riscaldare la piastra.
- `firmware/FirmwareBuild.h`: identificatore nuovo `COLD-251025-S3` (`C25-S3` su OLED), distinto da `WIFI-241024-R4` nel sorgente di provenienza e dalla R3 riportata come fisicamente caricata in precedenza.
- **Blob IDENTICI al branch operativo** per `SafetyController.h`, `HeaterPulseCommissioning.h`, `FanController.h`, `NetworkService.h`, `NtcSensor.h`, `Sht45Sensor.h`, `Ui.h`, `CycleController.h`, `firmware.ino`, come certificato dallo step CI. La simulazione non può cambiare GPIO o output della ventola.
- `firmware/tests/host/test_thermostat_cycle.cpp`: test host puro dei componenti **reali** `HeaterControlPolicy.h` e `CycleController.h`, con una sequenza di aria simulata e limite NTC **esplicitamente artificiale**. Emette `SIM_ONLY,...,HEAT_REQUEST_ONLY=0/1,GPIO19=NOT_CONNECTED` esclusivamente nel log CI. **Non esiste un heater shadow-test attivo in ESP32** e la simulazione non è una validazione del termistore.
- `firmware/tests/host/run_step03_tests.sh`: esegue i test esistenti, poi il ciclo simulato; il vecchio runner host usa una copia temporanea di AppConfig per testare separatamente la policy approvata con HIGH simulato, **mai** la build firmware reale.
- `.github/workflows/step03-thermostat-ci.yml`: gate config di quattro booleani heater e confronto SHA dei file fisicamente sensibili, build reale + test host. Lo storico workflow step01 non si attiva più su ogni PR: il controllo SHA step01 di `AppConfig` rispetto alla base sarebbe erroneamente FAIL nello step03 poiché proprio il commissioning viene disabilitato.

## Risultati effettivi dai log

- **PASS** config "no pulse commissioning, no normal heater output, no thermal approval".
- **PASS** hash invarianti per safety / pulse / fan / network / NTC / SHT45 / UI / ciclo / sketch.
- **PASS** mock di `SafetyController` con config corrente, **GPIO HIGH writes = 0**; il mock con approvazione totalmente inventata su copia temporanea ha **HIGH writes = 1** e serve solo a dimostrare il percorso software.
- **PASS** 14 scenari host di `HeaterControlPolicy`; **PASS** 12 scenari di `HeaterPulseCommissioning`, unitari.
- **PASS** **41 assertion** dello script di ciclo SHT45/camera: avvio ventola e pre-run, richiesta teorica ON sotto setpoint, OFF a setpoint, isteresi senza chatter, STOP, nuovo start con nuovo pre-run, timer a scadenza e assenza di auto-resume; fault fan comandata spenta, latch / reset solo esplicito, stale/invalid SHT45 e NTC, temperatura locale artificiale sopra limite, aria sopra limite, limite durata e massimo ON.
- Traccia nominale `SIM_ONLY` conferma: `ms=1100 air=32 HEAT_REQUEST_ONLY=1` con **nessun GPIO**; `ms=1500 air=40 HEAT_REQUEST_ONLY=0`; `ms=1600 air=39.5 OFF`; `ms=1700 air=39 ON`. Tutte le temperature limite e il target 40°C nel test sono **fixture sintetiche**, NON setpoint/soglie autorizzate per heater reale.

## Ambito e successivo step

**NON verificati:** flash su PCB, pagine OTA/OLED della nuova build, flusso d'aria fisico, riscaldamento reale, modello T NTC assoluto, trip piastra, comportamento termico closed-loop, TCO o materiali in regime. Il firmware nuovo, anche se venisse caricato, è **fan-only** e non un dryer autonomo termico. Il firmware precedentemente caricato sull'hardware resta invariato.

**Passo successivo STEP 04:** analisi di NTC in resistenza, errori elettrici separati da limiti termici fisici, rapporto locale Sanwa/NTC tenendo conto della diversa posizione, definizione di una strategia per caratterizzazione progressiva e dei gate delle future prove a caldo. Non riaprire 12h history o acquisti TCO opzionale. Step 04 non autorizza nessuna prova a caldo di per sé.
