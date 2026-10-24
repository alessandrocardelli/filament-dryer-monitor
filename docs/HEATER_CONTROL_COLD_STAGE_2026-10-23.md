# NTC e prima logica di controllo termico — checkpoint 2026-10-23

**Scope:** codice di richiesta logica del riscaldamento solo per test nativi, senza GPIO né integrazione nel firmware ESP32. Branch: feature/heater-policy-cold-tests-2026-10-23, derivato da plan/closed-heater-qualification-2026-10-20.

**Decisione D029:** il TCO aggiunto è opzionale, quindi la ricerca commerciale NON blocca lo sviluppo del riscaldamento. Sul prototipo è tuttora installato un fusibile da 105 °C in serie, la cui presenza va considerata per le prove. Questo lavoro non autorizza a bypassarlo o effettuare prove a caldo.

## Fonti realmente consultate

- hardware/IO_Power.kicad_sch: R28 = 47 kΩ, J7 NTC, J5 riscaldatore, Q4 IRLR3636TRPBF; confrontata la netlist esistente hardware/Filament_Dryer_Monitor.net, non rigenerata.
- firmware/AppConfig.h: 16 letture ADC ogni 500 ms, correzione 0.984369 e -2.322 mV, alimentazione divisore equivalente 3311 mV, pullup 46572 Ω, soglie elettriche 150–3000 mV. Sono parametri già caratterizzati, non una nuova taratura.
- firmware/NtcSensor.h: mappa empirica donor 25–50 °C e stima beta esterna con 82.5 kΩ / 4100 K; fuori intervallo extrapolazione non qualificata.
- docs/NTC_CALIBRATION_ANALYSIS.md: sette coppie Sanwa vicino piastra °C / resistenza NTC kΩ: 30/64.62, 39/42.27, 47/28.72, 56/19.68, 64/13.73, 74/9.81, 85/7.12. Entrambi i coperchi aperti, ventola OFF. A freddo R_DMM 122.3 kΩ e R_ESP32 circa 124.4 kΩ vicino a 20 °C.
- firmware/SafetyController.h: l'app di produzione comanda sempre GPIO19 LOW, heaterPermitted() è false; FanController ha controllo PWM ma non tachimetro.

## Conclusioni sull'NTC (fase C)

1. ADC e divisore sono già stati verificati tramite resistenze sostitutive: non ripetere l'intera calibrazione elettrica senza nuove evidenze.
2. Le sette coppie misurano due *punti termici diversi*: temperatura Sanwa vicino piastra e resistenza NTC interno. Non sono una curva temperatura del bead, né definiscono un limite termico di sicurezza.
3. Il freddo 122.3 kΩ vicino a 20 °C contrasta con l'estrapolazione dei sette punti (103.91 kΩ previsti a 20 °C). Un NTC 100 kΩ / B3950 resta una possibilità, NON una specifica dimostrata. Anche il precedente dato donor 82.5 kΩ a 25 °C non è co-localizzato.
4. C per la temperatura assoluta resta **OPEN**. Il numero NTC attualmente mostrato su OLED è diagnostico, non utilizzabile come soglia affidabile per comando del riscaldatore.

## Nuovo lavoro implementato, senza alcun effetto sulla scheda

- firmware/HeaterControlPolicy.h: classe C++ pura, NON collegata al firmware esistente, senza API Arduino, GPIO o PWM. Calcola solamente un possibile *requestOn* per un test host. Controllo della temperatura aria (SHT45) con isteresi; NTC per osservazione del limite heater; richiede configurazione esplicitamente approvata e NTC calibrato, entrambi di default falsi. Nessun valore termico di produzione preimpostato.
- Disabilitazione della richiesta per letture mancanti/scadute, limiti e durate mancanti, ventilazione non comandata, sovratemperature, troppo tempo di riscaldamento o ciclo, stop. Fault persistente, da cancellare solo dopo stop e conferma esplicita.
- firmware/tests/host/test_heater_policy.cpp: 14 casi autonomi di start/prerun/isteresi/stop, configurazione non approvata, fault persistente/reset, NTC invalido/non calibrato/scaduto, SHT45 invalido/scaduto, doppia sovratemperatura, timeout ciclo e continuo, perdita comando ventola, rollover millis.

**Test nativo realmente eseguito** in ambiente Linux con g++ 14.2:

    g++ -std=c++17 -Wall -Wextra -Werror -pedantic firmware/tests/host/test_heater_policy.cpp -o /tmp/test-heater-policy
    /tmp/test-heater-policy

**Risultato:** PASS: 14 native heater policy scenarios (no GPIO, no heat).

Non è stata eseguita una compilazione Arduino/ESP32, non c'è integrazione nel loop dell'app e non è stato eseguito alcun collaudo reale del riscaldatore. firmware/firmware.ino, firmware/SafetyController.h, UI, ventola e KiCad invariati.

## Per integrazione e successivi test a caldo: ancora necessario

- **NTC**: validare una vera relazione resistenza-temperatura del bead o un limite affidabile tramite misure indipendenti. Un solo punto freddo non identifica R25 e beta insieme: il dato minimo aggiuntivo utile sarebbe un secondo punto **R_NTC + temperatura di equilibrio co-localizzata** a temperatura diversa dal primo, ottenuto con heater OFF se praticabile. Non ripetere i sette plateau.
- **Chiusura**: disposizione della sonda SHT45 nel volume bobina, Sanwa a contatto piastra, cablaggi protetti, aria e ventola. Attuale TCO 105 °C fisicamente in serie va considerato; acquisto di TCO diverso non obbligatorio.
- **Sicurezza effettiva**: limiti termici materiali/cavi, guasti sensori, controllo della ventola fisica (non c'è tachimetro), stallo del firmware, MOSFET bloccato acceso, criteri di arresto, watchdog e comportamento al riavvio. Il comando software della ventola non garantisce il flusso d'aria.
- **Abilitazione riscaldatore**: solo dopo revisione e autorizzazione separata di firmware e prova sorvegliata limitata. I numeri sintetici del test host (90 °C, 60 °C, 5000 ms) NON sono limiti raccomandati del dispositivo.

**Prossima azione fisica unica:** verificare a dispositivo spento posizione NTC/SHT45/Sanwa e chiusura effettiva della scatola; alla successiva occasione favorevole per misure a freddo, raccogliere un secondo punto di equilibrio del NTC a temperatura diversa. Non serve acquistare un TCO per procedere con il codice.
