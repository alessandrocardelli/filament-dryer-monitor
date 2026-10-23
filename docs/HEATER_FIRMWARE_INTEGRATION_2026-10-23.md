# Firmware principale con controllo riscaldatore integrato — 23 ottobre 2026

## Stato

**IMPLEMENTATO nel codice sorgente, NON ancora collaudato sull'ESP32, heater OFF nel build corrente.**

Branch: `feature/integrated-heater-controller-2026-10-23`, derivato da `feature/heater-policy-cold-tests-2026-10-23`. Tutte le modifiche restano in questo ramo: non cambiano il firmware eventualmente già flashato sul prototipo. La decisione D029 rimane valida: TCO aggiunto **opzionale**; l'esemplare 105 °C fisicamente montato in serie resta da considerare, senza bypass automatico.

### Funzione completa scritta ora

1. `firmware/firmware.ino` chiama realmente `SafetyController::update()` nel ciclo principale, **dopo** il refresh dei sensori, della ventola e delle transizioni del ciclo; invia temperatura e durata, stato del motore ventola e setpoint camera. Forza immediatamente heater LOW all'uscita da DRYING.
2. `firmware/SafetyController.h` integra `HeaterControlPolicy.h` con la **reale uscita ESP32 GPIO19/Q4**: può comandare HIGH/LOW **solo dopo** validazione esplicita e configurazione di tutti i limiti; fault latch e stop interrompono la richiesta. Il modello regola l'aria SHT45 e sorveglia temperatura NTC, freshness, prerun ventola e timeout.
3. `firmware/AppConfig.h`: doppio consenso termico + abilitazione fisica, **tutti FALSE**; limiti termici e temporali non qualificati posti a **ZERO**, con `static_assert` che impedisce di compilare un'abilitazione parziale. **Non** inseriti setpoint fittizi nel firmware reale.
4. `firmware/CycleController.h`: esporta il tempo di ciclo effettivo per deadline.
5. `firmware/Ui.h`: il pannello DRYING e la pagina SYSTEM espongono `HEATER LOCKED`, `HEATER OFF` oppure `HEATER ON` a seconda dello stato reale comandato. Serial logging del lock e del fault codice numerico.

Nessun cambio KiCad, NTC di produzione, registrazioni delle impostazioni, pin dei tasti, PWM ventola o policy buzzer. L'attuale conversione NTC donor rimane **non sufficientemente validata**; il software non pretende che i sette punti Sanwa-vicino-piastra siano una curva bead.

## Test effettuati, con limiti espliciti

- **14 casi C++ nativi della policy: PASS**, già nel ramo precedente, rieseguiti il 23 ottobre.
- **Controller fisico simulato su GNU g++ C++17** con `digitalWrite` e `pinMode` fittizi: **PASS**, configurazione reale `kHeaterOutputEnabled=false` -> **ZERO scritture HIGH**; copia temporanea con dati sintetici qualificati -> richiesta HIGH dopo prerun e ritorno LOW allo STOP e al fault NTC. Questa è una prova del controller con stub, non una prova sulla PCB.
- Riproduzione host automatizzata in `firmware/tests/host/run_heater_tests.sh`, che compila **gli header veri del repository** in copie temporanee; il secondo run modifica **solo la copia** della configurazione con soglie artificiali da simulazione. Eseguire: `bash firmware/tests/host/run_heater_tests.sh`.
- **NON eseguiti:** compilazione completa con core ESP32 Arduino/U8g2 (toolchain assente nell'ambiente), flash su PCB, test riscaldatore o test temperatura a box chiuso. Non descrivere il firmware come già pronto per asciugare filament o come collaudato su eBOX Lite.

## Condizioni mancanti prima di un vero GPIO19 HIGH

- Verifica concreta della disposizione/ventilazione e dei cavi nel dryer chiuso; il comando PWM della ventola **non** dimostra il flusso fisico, mancando tachimetro.
- Calibrazione attendibile della temperatura NTC (la vecchia tabella donor 82.5 kΩ / B4100 e l'ipotesi 100 kΩ / B3950 non sono entrambe verificate); limiti termici componenti e case non deducibili dai valori OEM 118 °C piastra / 55 °C aria.
- Test firmware compilato per ESP32 prima del flash; dopo revisione, autorizzare separatamente una **prova breve, progressiva e sorvegliata** con termocoppia indipendente, limiti e criteri d'arresto. La prova non impone l'acquisto di un nuovo TCO, ma valuta la configurazione fisica realmente presente compreso il 105 °C.
- La logica richiede soltanto un comando PWM ventola `FanPhase::Running`, **non** misurazione della portata. Assenza di tachimetro, protezione da MOSFET in corto, eventuale guasto MCU e politica fan cooldown richiedono valutazione sistemica prima di autonomia prolungata.

## Prossimo lavoro prioritario

Compilare il firmware integrato per la scheda **ESP32-WROOM-32E e Arduino ESP32 core 3.x** con le librerie attuali; risolvere qualsiasi errore, mantenendo `kHeaterOutputEnabled=false`. Solo dopo test di regressione software/UI/ventola con riscaldatore elettricamente isolato e review della misura NTC, passare alla preparazione del primo test termico supervisionato. Non riaprire la ricerca dei TCO per procedere con la compilazione.

**Riferimenti:** D025 (fan), D029 (TCO opzionale), `docs/NTC_CALIBRATION_ANALYSIS.md`, `docs/HEATER_CONTROL_COLD_STAGE_2026-10-23.md`.
