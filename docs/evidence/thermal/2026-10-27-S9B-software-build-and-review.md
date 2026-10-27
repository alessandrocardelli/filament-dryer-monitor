# STEP09-B — Checkpoint codice e build con 200s massimo cumulativo HEATER

**Stato: SOFTWARE VERIFICATO / CI PASS, PROVA FISICA NON ESEGUITA E NON APPROVATA.** Data: 27 ottobre 2026. [PR #15 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/15) basata sulla PR #14 S9A, branch `experiment/step09-s9b-25c-200s-2026-10-27`. **Non flashare né avviare con questa nota; la precedente approvazione dell'utente per S9A a120s non copre il S9B a200s.**

## Riepilogo comprensibile (COSA, PERCHÉ, PRECEDENTE, DOPO)

- **Cosa:** realizzare candidato firmware HEATER realmente attuante con termostato SHT45 aria25°C e budget ON cumulativo aumentato a200s senza aumentare la durata della singola accensione né cambiare protezione locale NTC.
- **Perché:** S9A ha terminato per budget120s a SHT45 aria22,59°C, senza FAULT e senza raggiungere25°C; vogliamo verificare per la prima volta l'OFF per soglia aria25°C.
- **Prima:** [S9A dati fisici originali e 13 letture Sanwa](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md), [CSV S9A](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/main/docs/evidence/thermal/2026-10-27-S9A-raw.csv). NTC SOFT50kΩ funzionante e Sanwa35–38°C nei cicli stabili; non prova margine termico nella durata aumentata.
- **Dopo:** nuovo protocollo S9B su hardware HEATER ON **soltanto con nuova approvazione del profilo** e verifica fisica ventola/Sanwa/TCO; acquisizione nuova SHT45 aria in prossimità25°C e CSV, senza ripetere S9A.

## Diff effettivamente revisionato su PR #15

- `firmware/HeaterStep09AirFeedback.h` **unica variazione alle condizioni di riscaldamento**: `maximumTotalHeatMs = 120000` → `200000` (non modifica `maximumRunMs=600000`, `maximumOnWindowMs=20000`, ISR20,5s, min OFF12s, SOFT pausa R<=50kΩ/ripresa R>=65kΩ, hard fault R<=40kΩ / R>=250kΩ, camera fault40°C, pre-run/cooldown).
- `firmware/FirmwareBuild.h`: id esatto `HOT-261027-S9B`, OLED `H27-S9B`.
- `firmware/firmware.ino`, `Ui.h`: solo stringhe/stato test S9B; la modalità codice sperimentale può COMANDARE GPIO19 e NON deve essere scambiata per la produzione. Nessun flash da remoto effettuato.
- `firmware/tests/host/test_step09_feedback_gpio.cpp`: vero `SafetyController` con GPIO MOCK, verifica10×20s=200s cumulative HEAT, fine cooling90s, hard NTC40, SOFT50/65, STOP, sensori stale/invalid, fan command lost, time cap; NUOVO scenario di SHT45 che porta OFF25°C **dopo più di120s ON** e consente di riaccendere solo sotto24°C.
- `.github/workflows/step09-feedback-build-and-guard-ci.yml`: trigger per ramo S9B, `pipefail` sia test mock sia build ESP32 per evitare errore S9A di falso verde; assert su parametri noti.
- Nessuna modifica ai sensori, driver fan, PCB, `AppConfig.h`, TCO, calibrazione NTC, duty100%. Il PR diff conferma questa separazione.

## GitHub Actions — risultato verificato **nei log**

- **Run `40560081934`**: https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40560081934
- **Job `122386410479`**, risultato finale `SUCCESS`, incluso `Compile actual STEP09 ESP32 image (DO NOT FLASH)` e test C++ reale. Sorgente testato commit `6b7e67e08e61edc3a06dc8c7eeaf0f0d1313c8e4` (comprende tutti i cambiamenti firmware e il protocollo iniziale; eventuali successivi commit SOLO documentali da verificare confrontando blob).
- Compilatore `g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic` da script host; Arduino-ESP32 core3.3.12 e U8g2 2.36.19, target `esp32:esp32:esp32`.

**Estratto dai veri log CI:**

```text
PASS: candidate constraints match documented review, no V1 heater approvals
PASS: existing sensors/drivers and test trace original
PASS STEP09 actual SafetyController GPIO-MOCK: real ON, SHT45 25C feedback/hysteresis, NTC soft50k/hard40k, 20s/12s windows, 200s cumulative heat cap, STOP, cooldown, stale/fault/no restart. NO REAL GPIO.
PASS STEP09 native fixture. NO ESP32 connected or heater energized.
Sketch uses 1126854 bytes (85%) of program storage space. Maximum is 1310720 bytes.
Global variables use 92096 bytes (28%) of dynamic memory, leaving 235584 bytes for local variables. Maximum is 327680 bytes.
PASS: ESP32 binary linked with no DRAM overflow
```

**Questo PASS è SOFTWARE, non qualifica termica;** in particolare il test GPIO mock non prova la corrente del MOSFET, la riuscita dell'ISR fisico, giri/portata della ventola, temperatura materiale/TCO105, accuratezza assoluta R_NTC→°C, o che l'aria a25°C si possa raggiungere davvero.

## Valutazione e gate

L'estrapolazione dati S9A più recente [nel report budget](2026-10-27-S9B-budget-feasibility-and-safety-review.md) colloca ipoteticamente l'arrivo25°C in circa **7min20s–8min11s dall'avvio**, ma solo se trend e condizioni restano simili; non è previsione verificata o garanzia di stabilizzazione. Il tempo complessivo HEAT aggiuntivo (fino+80s rispetto a S9A) comporta rischio di esposizione maggiore per pareti, TCO e cablaggi che i singoli punti Sanwa non qualificano. Lasciare invariato il veto manuale Sanwa50°C non equivale a sicurezza automatica.

- **G0 archivio S9A: PASS completo**, il precedente test non va ripetuto.
- **G1 analisi numerica S9B: COMPLETATA**, limiti/differenze derivati esplicitamente dal CSV reale.
- **G2 codice + CI S9B: PASS SOFTWARE**, senza flash o HEAT fisico.
- **G3 review fisica e nuovo consenso specifico: APERTI**; non interpretare `Procedi` (autorizzazione all'analisi e preparazione) come approvazione a far girare 200s di HEAT.
- **G4 S9B esperimento fisico: NON ESEGUITO**, nessuna misura nuova né CSV. [Protocollo preregistrato S9B](../../STEP09_S9B_NEXT_PHYSICAL_TEST_2026-10-27.md) e [verbale pre-creato](2026-10-27-S9B-PHYSICAL-SESSION.md).

**Prossimo passo UNICO:** chiedere autorizzazione specifica per S9B target aria25°C, limite cumulativo200s, Sanwa STOP50°C, NTC invariata, con controllo sul posto delle condizioni fisiche; solo dopo eventuale flash OTA da STANDBY con OLED H27-S9B e UN test HEATER ON sorvegliato. Nessun merge PR#15 prima di review, nessuna autorizzazione a V1, niente prove heater OFF duplicate.
