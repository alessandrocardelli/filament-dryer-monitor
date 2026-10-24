# Qualificazione termica del dryer — fase NTC a freddo (24 ottobre 2026)

## Obiettivo operativo

Portare il filament dryer dalla modalità sperimentale tre impulsi alla **regolazione effettiva della temperatura in camera SHT45** con NTC indipendente di sorveglianza piastra. Questa fase è un passaggio necessario prima della verifica dei limiti e dell'abilitazione dell'uscita heater continua. Non si tratta di storico 12 ore, grafica, nuovo hardware, un test dei tre impulsi già riuscito, né sourcing TCO.

Branch di lavoro: `feature/thermal-qualification-2026-10-24`, derivato dal firmware fisicamente collaudato `feature/wifi-ota-web-logging-2026-10-24`. Identificativo nuovo firmware: **THERM-241024-R1** (non ancora confermato in hardware).

## Evidenza acquisita prima di questo aggiornamento

- SHT45 misurazione temperatura e umidità camera funziona; dashboard e Arduino OTA già riportati PASS dall'utente.
- NTC su GPIO34: correzione ADC/divisore misurata tramite sostituti resistivi il 2026-10-11; modello termico **non** validato.
- NTC 122.3 kΩ misurata col DMM e circa 124.4 kΩ nell'ESP32 a circa 20 °C in precedente prova a freddo.
- I sette punti 30–85 °C sono `R_NTC` vs **Sanwa vicino alla piastra**, con dryer **aperto e ventola OFF**: non sono una calibrazione temperatura del bead o una prova di sicurezza nel dryer chiuso; si vedano [NTC_CALIBRATION_ANALYSIS.md](NTC_CALIBRATION_ANALYSIS.md).
- Con box chiuso e ventola attiva, primo test firmware `PULSE-241024-R1` completato: tre indicazioni ON, Sanwa 19→23 °C, COMPLETE. Nessuna raccolta di un secondo punto di calibrazione NTC.
- Il TCO 105 °C è riferito come montato in serie sul prototipo. Il suo eventuale upgrade è opzionale D029 ma il pezzo presente **non** viene scavalcato.

## Confronto di modelli preesistenti: esclusivamente diagnosi

Il nuovo `firmware/NtcQualification.h` riporta due sole **ipotesi** già presenti nello studio:
- `Donor 82.5k/B4100`, dato storico non co-localizzato.
- `100k/B3950`, alternativa plausibile ma **non riconosciuta come componente effettivo**.

| Temperatura di riferimento *assunta* sul bead | R secondo donor 82.5k/B4100 | R secondo ipotesi 100k/B3950 |
| --- | ---: | ---: |
| 20 °C | circa 104.3 kΩ | circa 125.4 kΩ |
| 25 °C | 82.5 kΩ | 100.0 kΩ |
| 30 °C | circa 65.8 kΩ | circa 80.4 kΩ |

Queste resistenze sono calcolate per un NTC posto proprio alla temperatura indicata, **non** sono misure della PCB. L'aria SHT45 corrisponde alla temperatura del bead **solo approssimativamente e dopo un vero equilibrio termico**, se non vi sono gradienti/autoriscaldamento. Lo scarto di una singola lettura non identifica da solo R25/beta e **nessun modello viene approvato** dall'endpoint.

## Funzioni diagnostiche aggiunte

- API in sola lettura: `http://filament-dryer.local/api/ntc-qualification`.
- Dashboard: sezione **Verifica NTC a freddo (solo diagnostica)** con temperatura aria SHT45, resistenza NTC, lettura ADC mV corretta, resistenza prevista per entrambe le ipotesi e differenza percentuale.
- I valori di confronto sono visualizzati solo con **SHT45 e NTC validi e freschi**, dryer **non in ciclo**, heater OFF e ventola OFF. L'endpoint indica `qualified:false`, `use_for_heater:false`.
- Le formule sono pure, senza accesso a GPIO/heater, e verificate con test nativi C++ separati.
- `AppConfig.h`, `SafetyController.h`, soglie, PWM ventola, sequenza tre impulsi e schemi KiCad **non vengono modificati**.

## Unica misura richiesta sul dispositivo prima di qualsiasi nuovo test caldo

1. Lascia il dryer **spento come riscaldatore** e, dopo che si è raffreddato completamente ed è rimasto nell'ambiente fino a stabilizzarsi, accendilo in STANDBY senza iniziare un ciclo (ventola OFF). Se possibile lascia equilibrarsi anche la parte sensibile dell'SHT45 senza sorgenti di calore vicine. Nessuna nuova accensione heater per questa fase.
2. Installa **THERM-241024-R1** via Arduino IDE OTA usando il branch indicato; verifica la versione tramite API o OLED. Mantieni una rete Wi-Fi privata fidata. Attendi stabilizzazione, poi apri la dashboard e la nuova diagnostica.
3. Comunica **la coppia simultanea** `T_aria SHT45` e `R_NTC` della pagina, preferibilmente con un valore indipendente del Sanwa (e descrivi il suo esatto punto di contatto se presente). Fai due letture distanziate di 10–15 minuti **senza scaldare**, annotando se si sono stabilizzate. Una foto/screenshot della diagnostica e un valore Sanwa sono sufficienti.
4. Non attribuire alle ipotesi una precisione o un limite di sicurezza sulla piastra. Se il riferimento è solo aria e non termistore in contatto, trattare la misura come **screening a freddo**, non calibrazione definitiva.
5. Per stimare beta/R25 e un cutoff affidabile serviranno due o più temperature indipendenti del **bead** o un datasheet/specifica dell'NTC; l'eventuale secondo equilibrio a temperatura diversa si pianifica **senza attivare heater continuativo**. Una successiva prova supervisionata sul dryer chiuso dovrà registrare sensore camera e piastra con criteri di stop qualificati; non ripetere il vecchio protocollo di 3 impulsi solo per rifarlo.

## Gate prima del termostato V1

Non modificare `kHeaterCommissioningMode=true`, `kHeaterOutputEnabled=false`, `kHeaterProfileApproved=false` o `kHeaterNtcModelApproved=false` finché non vengono definiti e revisionati con evidenze: modello e posizione NTC, limite termico reale di piastra/parti, limite aria, timeout massimo di conduzione e ciclo, risposta sensore mancante/stale, funzionamento ventola durante fault, stop/restart e protezione contro blocco del loop durante heater ON. Il precedente hardware watchdog vale solo per i brevi impulsi sperimentali, **non** qualifica il controllo continuo. Il setpoint 55 °C è soltanto un valore UI salvato, non un criterio di sicurezza.
