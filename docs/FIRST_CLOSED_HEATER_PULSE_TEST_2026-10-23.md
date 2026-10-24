# Prima prova termica CHIUSA con ventola — solo firmware sperimentale 2026-10-23

> **2026-10-24 — prova senza USB, ID firmware introdotto:** con il nuovo ramo, dopo i ~4.15 s di animazione Slewform appare sul display per **3 s** la schermata **HEATER TEST / FW PULSE-241024-R1 / 3 x 1.5s TEST**. In STANDBY compare **P24-R1** in basso a destra. Questo discrimina il nuovo firmware da quello precedente, ma da solo NON prova un comando elettrico sul MOSFET. Durante DRYING in basso: **FAN PRERUN - 0/3**, **HEATER ON - n/3**, **HEATER OFF - n/3**, **COOLING - 3/3**. La pagina SYSTEM accessibile con M durante DRYING riporta NTC (kOhm), SHT45 e fase numerica. Se scatta FAULT, appare anche una descrizione (es. NTC, SHT45, FAN, AIR LIMIT, NTC R LIMIT). Senza USB, riprendere con telefono il display OLED e il Sanwa esterno, poi riferire le scritte/temperatura massima: le righe `HOT_CSV` **non** vengono memorizzate e quindi non sono recuperabili scollegando la USB. Il test invariato eroga al massimo tre impulsi da 1.5 s; temperatura quasi invariata **non dimostra da sola** che non sia partito. Prima verifica pratica: identificativo visibile, non altre prove fan/UI già superate.

> **VERIFIED SOFTWARE CHECKPOINT — 2026-10-23:** [GitHub Actions build + native tests SUCCESS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40022340102) for commit `aabc5b647a60d99efaee956e6033ae5aaaffb8f8`. ESP32 Arduino core 3.x / U8g2 compilation succeeded; 354347 bytes flash (27%), 25472 RAM (7%). Host scenarios: 14 original policy + 12 pulse + normal/fixture GPIO mocks PASS. **Not flashed to hardware, no closed heating performed.**

## Perché questa prova, e non le precedenti

I sette punti 30–85 °C sono stati raccolti con eBOX Lite **aperto e ventola spenta**; il driver heater 24 V / ~1.59 A e la ventola sono già stati provati separatamente. Questo firmware produce invece **un unico breve ciclo di calore con ventola inserita** per caratterizzare la risposta combinata, senza fingere di conoscere già la curva assoluta NTC.

Branch: `test/closed-case-heater-pulses-2026-10-23`, derivato dal firmware integrato. Questo è un **firmware sperimentale**, NON firmware definitivo di essiccazione. Il TCO aggiunto è opzionale secondo D029; non richiede acquisto. Il 105 °C effettivamente montato resta collegato, senza alcuna modifica o bypass.

## Comportamento della prova

- All'avvio: GPIO19 LOW, nessun calore senza pressione ON/OFF in STANDBY. Nessun ripristino di un ciclo dopo reset.
- Pressione breve ON/OFF: avvia DRYING, ventola 100% per 1 s poi 40% (policy D025 già verificata); attende almeno **5 s con ventola in fase Running**.
- Se SHT45 e NTC sono validi e aggiornati, invia **3 impulsi da 1.5 secondi di HEATER ON**, separati da **12 s di OFF**.
- **Arresto immediato del riscaldatore** se: NTC elettricamente guasto/scaduto, resistenza NTC a o sotto **42.27 kΩ**, resistenza sopra 250 kΩ, SHT45 guasto/scaduto, temperatura aria **>=35 °C**, comando ventola non in stato Running, scadenza totale 120 s o timer hardware indipendente a 2.2 s.
- I valori 42.27 kΩ e 35 °C sono soglie cautelative **di questo primo test**, non calibrazioni né limiti di esercizio finali: 42.27 kΩ corrispondeva a Sanwa vicino piastra circa 39 °C **solo nella geometria aperta/fan OFF**. Non dedurre la vera temperatura del bead o del case.
- Dopo l'ultimo impulso: la ventola continua per **45 s** e il ciclo passa automaticamente a COMPLETE. Dopo STOP manuale o FAULT, il controller mantiene la ventola comandata durante il residuo raffreddamento se ne resta il tempo. Non riavvia gli impulsi nel medesimo boot.
- Il timer hardware di ESP32 ordina GPIO19 LOW dopo 2.2 s anche se `loop()` ritarda: è un interblocco aggiuntivo software/hardware del MCU, **non** una protezione da guasto MOSFET in corto, guasto alimentazione o perdita flusso d'aria; la ventola non ha tachimetro.
- Il setpoint e il timer normalmente salvati nel menu **non controllano** questa sequenza sperimentale.

## Un solo test fisico nuovo, da fare solo dopo compile PASS

1. Verificare a dispositivo spento che **entrambi i coperchi si chiudano**, i cavi non siano pizzicati e la ventola sia libera; disporre **SHT45 nell'aria della camera** e fissare la **termocoppia Sanwa alla piastra** come riferimento separato. J5 può rimanere collegato; non toccare il TCO.
2. Caricare **esattamente il branch di questo test**, non il branch production locked, e aprire monitor seriale **115200**. Non abbandonare il dispositivo durante la sequenza. La prima schermata del serial deve dichiarare `EXPERIMENTAL 3-PULSE HEATER TEST MODE`.
3. Quando la temperatura dell'aria è normalmente sotto 35 °C, premere **ON/OFF una volta**: la macchina inizierà automaticamente i brevi impulsi con la ventola. Non variare manualmente setpoint o timer: sono ignorati nel test.
4. Osservare il **Sanwa sulla piastra**, la rotazione della ventola e la sequenza; interrompere alimentazione se si nota calore inatteso, odore anomalo, fumo o cavi in sofferenza. ON/OFF lungo comanda stop software; la disconnessione 24 V rimane l'arresto fisico di emergenza.
5. Dopo l'esito, trasmettere **solo** righe seriali `HOT_CSV` dal principio alla fine, temperatura massima osservata dal Sanwa e se il dryer si è chiuso completamente. Questo è tutto: niente ripetizione di menu/LED/ventola, già convalidati.

## Stato sorgente e release

La configurazione attiva `AppConfig::kHeaterCommissioningMode=true` è deliberatamente confinata a questo branch di test. La configurazione `kHeaterOutputEnabled=false` resta invariata e mantiene bloccata la **regolazione finale**: i soli impulsi di commissioning usano un percorso separato nel SafetyController.

File: `firmware/HeaterPulseCommissioning.h`, `firmware/SafetyController.h`, `firmware/firmware.ino`, `firmware/AppConfig.h`, `firmware/Ui.h`; test: `firmware/tests/host/test_heater_pulse.cpp` e test regressione. Compilazione e test CI da associare al commit dopo esecuzione, senza attribuire un PASS prima dei risultati.

**Non è il firmware definitivo:** il profilo di asciugatura continua sarà impostato dopo i dati a scatola chiusa e la verifica dei limiti termici/cavi. Il TCO aggiuntivo resta opzionale.
