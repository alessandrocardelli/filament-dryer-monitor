# STEP09-A — Rapporto firmware candidato e CI (27 ottobre 2026)

**Stato: BUILD ESP32 PASS + HOST GPIO MOCK PASS. PROVA FISICA NON ESEGUITA, NON AUTORIZZATA.** Il test HEATER-ON è ora una possibilità tecnica di questo branch sperimentale, non una qualifica sicurezza né un risultato di temperatura misurata.

## Introduzione (cosa/perché/precedente/futuro)

Cosa: preparare vero controllo HEATER ON regolato dall'aria SHT45 a25°C, con protezione indipendente resistenza NTC (soft pause a50kΩ e hard FAULT a40kΩ), ventola100% comandata e limiti di energia/tempo.
Perché: S7U ha dimostrato il trip a40kΩ, ma non il feedback aria. La prima prova fisica deve essere con heater reale e variazione HEATER ON/OFF comandata dal SHT45, non altra prova OFF.
Precedente: S6C 120s ON, Sanwa53°C massimo, aria23,98°C a fine log; S7U 59,25s ON, Sanwa43°C allo stop, SHT45 aria19,52°C a FAULT e20,89°C a fine cooling. STEP03/STEP08 host test PASS.
Futuro: se approvato il profilo e verificato il setup, una sola prova reale con Sanwa+CSV; poi analisi della capacità di stabilizzazione e dei margini, prima di un eventuale test aria30/40°C.

## Identità firmware e differenze

- Branch: experiment/step09-real-sht45-feedback-2026-10-27; PR #14 DRAFT, non mergiata.
- Firmware: HOT-261027-S9A, OLED H27-S9A, mode S09 AIR FEEDBACK.
- Nuovo HeaterStep09AirFeedback.h, integrato nel percorso **GPIO19 realmente attuabile** di SafetyController.h.
- AppConfig.h: kHeaterStep09TrialMode=true; kHeaterCommissioningMode=false; le tre approvazioni/regolatore di produzione restano FALSE. Modalità prova fisica indipendente e isolata.
- Controllo aria SHT45: HEAT OFF >=25°C; ON <=24°C (1°C isteresi).
- Heater NTC **resistenza raw**: soft OFF <=50kΩ; new ON solo quando >=65kΩ e pausa di almeno12s; HARD fault <=40kΩ o >=250kΩ; nessun modello NTC°C promosso a vero.
- In ogni finestra max20s HEAT, ISR GPIO19 a20,5s; tempo cumulativo HEAT<=120s, runtime<=600s, fan prerun5s dopo spunto1s, fan100 comandata, cooldown90s. Air >=40°C, NTC/air invalid/stale, comando FAN perso = fault latch. No auto restart.
- UI distingua AIR FEEDBACK, HEATER ON e OFF, NTC OHM e FAULT; target25°C fisso. Nuovo CSV RAM a500ms su buffer1400: 600s run +90s cooling = circa1380 righe al massimo, senza perdere inizio.
- NON modificati NtcSensor.h, Sht45Sensor.h, FanController.h, HeaterControlPolicy.h e HeaterPulseCommissioning.h. Nessuna PCB/KiCad o TCO modificata.

## CI reale: build e test

Workflow: .github/workflows/step09-feedback-build-and-guard-ci.yml
**Run GitHub**: https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40485007834
**Job**: 122124497852, Ubuntu24.04. Check: setup/branch config PASS; comparazione SHA driver originali PASS; native C++ mock SafetyController PASS; ESP32 core3.3.12 + U8g2 2.36.19 install PASS; build Arduino ESP32 FQBN esp32:esp32:esp32 **PASS verificato nei LOG**. La pipeline usa ora set -euo pipefail, quindi un errore linker fa fallire il job.

**Output autentico da job logs:**

    PASS STEP09 actual SafetyController GPIO-MOCK: real ON, SHT45 25C feedback/hysteresis, NTC soft50k/hard40k, 20s/12s windows, 120s cumulative heat cap, STOP, cooldown, stale/fault/no restart. NO REAL GPIO.
    PASS STEP09 native fixture. NO ESP32 connected or heater energized.
    Sketch uses 1126758 bytes (85%) of program storage space. Maximum is 1310720 bytes.
    Global variables use 92096 bytes (28%) of dynamic memory, leaving 235584 bytes for local variables. Maximum is 327680 bytes.
    PASS: ESP32 binary linked with no DRAM overflow

Il test nativo non esegue l'ISR hardware: la build ESP32 ne verifica compilabilità, mentre il limite reale20,5s deriva dal timer previsto e non è stato misurato col tester. Nessuna corrente heater o RPM ventola misurati in questa fase.

## Errore iniziale individuato e risolto — non nasconderlo

La prima CI dello STEP09 (run 40484256175) aveva un bug di workflow: comando compile in pipe con tee senza pipefail, così il job riportava SUCCESS anche se il linker mostrava **DRAM overflow di 5.920 byte**. Inoltre una prima modifica aveva portato ThermalLog a3000 righe e consumato troppa DRAM. Sono stati corretti entrambi: capacità ritornata1400 come S7U, campionamento Step09 rallentato a500ms per coprire l'intero periodo, e CI aggiunta set -euo pipefail. **Non chiamare PASS il primo run**; i numeri RAM e l'effettivo binario PASS sono quelli del run successivo sopra.

## Limiti e gate rimasti OPEN

- La futura prova è **fisicamente HEATER-ON**, ma non è avvenuta e richiede consenso sul nuovo specifico protocollo prima di flash/start.
- L'NTC del bead è identificata elettricamente in ohm, NON calibrata in temperatura nel box chiuso. Il trip hard40k è stato osservato in S7U in relazione al Sanwa43°C, senza che questo valga come limite materiale certificato. Soft 50/65kΩ è una proposta di controllo termico prudenziale da verificare.
- La ventola 100% è PWM comandato, nessun tachimetro; è necessaria osservazione diretta prima/durante HEAT. Materiali/cablaggi e temperatura del corpo TCO105 installato non qualificati.
- Sono stati usati limiti di test più restrittivi delle temperature nominali heater eBOX Lite105/116/118°C; non si raggiungono per decisione implicita.
- **Sanwa manuale veto50°C** preservato da S7U, nessun bypass TCO105, nessun flash automatico e nessuna modifica di produzione.
- **Protocollo fisico:** docs/STEP09_REAL_HEAT_AIR_FEEDBACK_PROTOCOL_2026-10-27.md. L'utente deve approvare specificamente il profilo e confermare lo stato fisico del setup, prima di una prova.

**Valutazione:** firmware sperimentale tecnicamente compilabile e mock testato; revisione rischi fisici G3 rimane OPEN, e PR14 DRAFT. Nessun dato sperimentale fisico S9A, nessun CSV S9A, nessun PASS di qualifica finale.
