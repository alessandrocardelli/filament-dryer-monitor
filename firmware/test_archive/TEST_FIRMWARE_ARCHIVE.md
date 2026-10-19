# Firmware di collaudo recuperati: registro delle evidenze

**Data recupero:** 2026-10-19. **Stato:** documentazione/archivio; nessun nuovo test fisico eseguito in questa operazione.

## Origine, originalità e stato delle prove

| Elemento | Archivio | Tipo di sorgente | Evidenza |
|---|---|---|---|
| Applicazione integrata 8 ottobre | [validated_integrated_2026_10_08](validated_integrated_2026_10_08/) | **ESATTA dal commit Git `3535dba9643e77d48e14eccc109606e3a5d758cf`** | Segnalazione utente «Funziona tutto», chat 2026-10-08 ~20:44; verificare che il riscontro non equivalga a ogni test di sicurezza |
| Ventola static ON/OFF | [fan_static_on_off](reconstructed_fan_2026_10_08/fan_static_on_off/) | **RICOSTRUITA**, non identica all'originale | Gate 5A, 2026-10-08: fan real load ON, J6-1 a 7,7 mV; totale alimentazione 102 mA spunto, 89 mA regime |
| Ventola PWM 50% | [fan_pwm_50](reconstructed_fan_2026_10_08/fan_pwm_50/) | **RICOSTRUITA**, non identica all'originale | Chat 2026-10-08 17:10: GPIO16, analogWrite PWM 25 kHz, 0→50% dopo 3 s; ventola avviata |
| Ventola PWM 50/75/100% | [fan_pwm_50_75_100](reconstructed_fan_2026_10_08/fan_pwm_50_75_100/) | **RICOSTRUITA**, non identica all'originale | Chat 2026-10-08 17:16: 7 s a livello; circa 49/72/89 mA dal banco; tutti funzionanti |
| Ricerca soglie 20→60% e 50→10% | [fan_pwm_thresholds](reconstructed_fan_2026_10_08/fan_pwm_thresholds/) | **RICOSTRUITA**, non identica all'originale | Chat 2026-10-08 17:18–17:22: 25% primo avvio riuscito, 15% minimo mantenimento |
| Scanner I²C SHT45/OLED | [i2c_scan](reconstructed_sensors_2026_10_07_11/i2c_scan/) | **RICOSTRUITA** | Chat 2026-10-07/08: SHT45 `0x44`, SSD1309 `0x3C` confermati |
| SHT45 misura diretta | [sht45_raw](reconstructed_sensors_2026_10_07_11/sht45_raw/) | **RICOSTRUITA** | Chat 2026-10-07: comando `0xFD`, CRC8; ~22.28–22.30 °C, 65.44–65.58 %RH |
| OLED SSD1309 frame U8g2 | [oled_ssd1309_frame](reconstructed_sensors_2026_10_07_11/oled_ssd1309_frame/) | **RICOSTRUITA** | Chat 2026-10-08: OLED 128x64 cornice/testi funzionanti |
| NTC lettura ADC grezza | [ntc_raw_adc](reconstructed_sensors_2026_10_07_11/ntc_raw_adc/) | **RICOSTRUITA** | Chat 2026-10-11: GPIO34, ADC_11db, raw 2779–2783, 2409–2412 mV prima correzione |
| NTC viewer senza riscaldamento | [ntc_reference_viewer](../diagnostics/ntc_reference_viewer/) | **Già salvato in repository** | Analisi NTC 2026-10-17; non implica test fisico completo di questo viewer |

L'archivio dell'applicazione integrata contiene i **blob sorgenti originali** di `firmware/` del commit `3535dba9643e77d48e14eccc109606e3a5d758cf`, incluso bitmap/header, copiati senza modifiche di contenuto (solo il nome dell'INO è allineato alla directory di sketch per l'IDE). Non è la versione più recente e non deve sostituire automaticamente l'app corrente.

## Sketch standalone non recuperati come sorgenti identici

Dalle conversazioni e dai resoconti in `docs/PROJECT_STATE.md` emergono anche le seguenti prove:
- Gate 3: USB/CP2102, UART, programmazione ESP32; codice di test integrale non rintracciato nell'albero Git.
- Gate 4A: scanner I²C, SHT45 con comando diretto, e OLED visualizzazione sono ora ricostruiti, **non** recuperati byte per byte; il nome locale `slewform_logo_test.ino` appare nei messaggi, ma quel file originale non è salvato in Git.
- Gate 4: pulsanti, buzzer, LED. La versione integrata del commit storico contiene questi controller; alcuni test standalone originali non sono stati archiviati.
- Gate 5A: test Q5 senza ventola con pull-up temporaneo 100 kΩ e pilotaggio statico; comportamento conservato nei resoconti, non i byte dello sketch originale.
- Gate 5B / successivi: sketch riscaldatore comandato via seriale `H` per 80/90 °C; **non recuperato come sorgente identico**; non si ricrea o rende eseguibile alla cieca un firmware che abilita il carico termico.

L'assenza di un sorgente originale va considerata una lacuna documentale, NON colmata spacciando codice ricostruito per quello effettivamente collaudato. Le ricostruzioni sono candidate di riproduzione **mai convalidate fisicamente come file attuali**.

## Regole per uso e diagnosi

1. **NON caricare questi sketch senza leggere l'intestazione e identificare la versione.** I test ventola sono solo per scheda ESP32-WROOM-32E, Arduino Core 3.x, alimentatore 24 V e J6. Il riscaldatore GPIO19 viene comandato LOW via software, ma per i test si preferisce isolare fisicamente J5.
2. Non confrontare automaticamente il valore UI `F 40%` con la tensione realmente misurata su GPIO16: il display mostra il comando software, non un segnale misurato.
3. La versione integrata Git del giorno 8 ottobre aveva già **ventola LEDC 9 e buzzer LEDC 8**, gli stessi canali della versione su `main` al 2026-10-19. Quindi il presunto conflitto di canale come unica causa del guasto **non è dimostrato**.
4. Il log fornito dall'utente nel collaudo del 19 ottobre (solo `NTC adc` e `STATE -> DRYING TEST`) **non presenta** i nuovi log `FAN PWM` introdotti sul branch `fix/isolated-fan-ledc-2026-10-19`; flash di quel branch non confermato.
5. Se uno sketch di diagnostica viene riutilizzato e validato, archiviare il file *identico* usato, hash/commit, versione core/librerie, wiring e log prima di passare al firmware successivo.
6. I risultati storici e i limiti di sicurezza continuano a essere documentati in `docs/PROJECT_STATE.md`, `docs/DECISIONS.md`, `docs/TODO.md`; non si abilita il riscaldatore senza la revisione sicurezza.

## Riferimenti

- [Originale Git del firmware integrato 8 ottobre](https://github.com/alessandrocardelli/filament-dryer-monitor/tree/3535dba9643e77d48e14eccc109606e3a5d758cf/firmware)
- `docs/PROJECT_STATE.md`: Gate 5A bench / fan real load / PWM / thresholds; firmware integrato ottobre.
- `docs/FIRST_ARTICLE_TEST_MATRIX.md`: Gate 5A e gate sensori.
- Conversazioni 2026-10-08: test statico circa 17:06; PWM 17:10 / 17:16 / 17:18; firmware integrato 20:38 / risposta 20:44.

## Stato attuale

**Test fisico del firmware in questa cartella: NON ESEGUITO dopo il recupero.** Questo commit recupera i materiali e rende esplicite le lacune.
