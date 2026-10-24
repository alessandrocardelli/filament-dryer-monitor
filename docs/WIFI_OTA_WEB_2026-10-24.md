# Wi-Fi + monitor live + OTA + CSV locale — firmware sperimentale 24 ottobre 2026

Branch: `feature/wifi-ota-web-logging-2026-10-24`, nato da `test/closed-case-heater-pulses-2026-10-23`. **Non modifica i tre impulsi già collaudati di 1,5 secondi, le condizioni di blocco termiche, il PWM ventola o le decisioni D029/TCO.**

Versione OLED: **WIFI-241024-R1** all'accensione. Test precedente `PULSE-241024-R1`: utente riferisce Sanwa piastra 19 -> 23 °C, +4 °C, HEATER ON tre volte, COMPLETE, senza anomalie. La registrazione CSV *non* può recuperare retroattivamente quel test: era stato eseguito col firmware precedente.

## Implementato nel sorgente

- Wi-Fi **STA 2,4 GHz** configurabile senza mettere le password nel codice o su GitHub; salvataggio separato in NVS (`dryer_net`).
- Se non configurato, ESP32 avvia AP WPA2 **`FilamentDryer-XXXX`** con password temporanea casuale di **12 caratteri** visibile sul display in STANDBY (dopo il logo/versione). La password dell'AP cambia al riavvio.
- Portal `http://192.168.4.1/setup`: inserimento SSID Wi-Fi di casa, password della rete e **password OTA separata di almeno 12 caratteri**; salvataggio NVS e riavvio. Non trascrivere password nella repository, screenshot pubblici o report.
- In STA, OLED STANDBY mostra l'indirizzo IP del dispositivo; servizio `filament-dryer.local` tramite mDNS come comodità, ma il **numero IP è il ripiego affidabile**. Se credenziali salvate ma rete irraggiungibile per 15 s si apre temporaneamente l'AP di setup, senza cambiare gli interlock.
- Pagina `http://<IP>/` con valori SHT45, RH, resistenza NTC (Ohm, **non temperatura calibrata**), comando heater, PWM ventola, fase del test e piccolo grafico della temperatura aria. API locali: `/api/status` (JSON), `/api/history.csv` (CSV campioni RAM), `/api/last-test.csv` (ultimo test completo salvato).
- `ThermalLog`: massimo **400 campioni da 250 ms = ~100 s** in RAM. Salva `/last-test.csv` su LittleFS una sola volta *dopo* il termine del riscaldamento e dell'eventuale cooldown; **nessuna scrittura flash mentre heater è attivo**. Il vecchio test viene sostituito dal nuovo, l'ultimo persiste dopo un riavvio normale **a parità di partizioni**, non dopo erase/format. Se manca alimentazione prima del salvataggio, traccia volatile persa. Il montaggio iniziale `LittleFS.begin(true)` può formattare la partizione dati se non è riconosciuta.
- `/ota` con autenticazione HTTP Basic (`admin` + password OTA). Accetta esclusivamente update firmware applicativo `.bin`, con scheda in **STANDBY**, riscaldatore OFF e nessun cooldown/fault; nessun endpoint remoto per comandare riscaldamento, impostazioni o pulsanti. Aggiornamenti firmware sempre a carico dell'utente; non eseguiti autonomamente.

## Primo caricamento, UNA VOLTA via USB

1. In GitHub Desktop seleziona `feature/wifi-ota-web-logging-2026-10-24` e aggiorna (Fetch/Pull). Apri `firmware/firmware.ino` dalla stessa cartella di Desktop, non lo sketch precedente da Recent Files.
2. Arduino IDE, core **esp32 by Espressif Systems 3.x** e U8g2; seleziona la scheda ESP32 appropriata e lo stesso port COM già usato.
3. **Tools -> Partition Scheme:** scegli una disposizione 4 MB con due partizioni OTA e una partizione dati SPIFFS/LittleFS. Per il profilo `ESP32 Dev Module` il default `Default 4MB with spiffs` supporta tipicamente OTA; non scegliere `Huge APP (No OTA)`, né `No OTA`. La compilazione in CI usa `esp32:esp32:esp32` con impostazioni predefinite.
4. Carica via USB e verifica sull'OLED **WIFI-241024-R1**. Per il primo caricamento resta USB, ma dalle prove successive puoi scollegarla e chiudere il dryer.

## Configurare il Wi-Fi senza USB

1. Dopo la schermata firmware, attendi STANDBY: il display mostra **WIFI SETUP**, il nome `FilamentDryer-XXXX`, una **KEY** generata all'avvio e `192.168.4.1`.
2. Con telefono/PC, scegli la rete `FilamentDryer-XXXX`, inserisci KEY. L'avviso "nessun accesso a Internet" è normale: rimani su questa Wi-Fi.
3. Apri `http://192.168.4.1/setup` nel browser. Inserisci il nome della rete domestica **a 2,4 GHz**, password di casa e password OTA **diversa** e lunga almeno 12 caratteri (salvala in un gestore password).
4. Premi **Save and restart**; dopo il riavvio torna sulla Wi-Fi domestica e apri l'IP mostrato sull'OLED, ad es. `http://192.168.1.123`, oppure prova `http://filament-dryer.local` se il tuo sistema risolve mDNS.
5. Nella pagina live verifica *senza riscaldamento* che i numeri cambino con nuove letture; in STANDBY gli endpoint CSV possono essere vuoti fino al prossimo test.

## Aggiornamenti successivi OTA

1. Con dryer **in STANDBY**, nessun ciclo in corso e nessun raffreddamento, in Arduino IDE usa **Sketch -> Export Compiled Binary** dopo aver selezionato il medesimo schema di partizioni OTA. Cerca il file **`firmware.ino.bin`** (nome può variare). Non caricare bootloader, partitions.bin, firmware merged/full flash o filesystem.
2. Apri `http://<IP>/ota`, inserisci utente **`admin`** e la password OTA scelta nella configurazione.
3. Seleziona `.bin`, invia e aspetta la risposta di completamento e il riavvio. Se il binario è troppo grande o la partizione di destinazione manca, l'ESP32 deve rifiutare il file e mantenere il firmware corrente; ciò richiede verifica fisica la prima volta.
4. Se la pagina risponde `409 OTA only in idle STANDBY`, completa/termina prima ciclo e cooldown; il firmware non riavvia per favorire un update.

## Limiti e verifiche fisiche ancora aperte

**Nessun test Wi-Fi, AP, flash LittleFS, browser OTA o ripristino dopo perdita rete è stato eseguito sulla PCB in questa modifica.** Le prove software CI non verificano RF/flash effettiva. Non dedurre operatività reale dalla sola compilazione.

- HTTP locale **non ha TLS**; Basic Auth protegge l'OTA da accessi casuali, ma la password può essere intercettata da chi controlla la LAN. **Usare esclusivamente rete domestica privata e fidata, mai inoltrare porte Internet**. La telemetria REST è leggibile da dispositivi della stessa rete.
- L'ESP32 eBOX Lite ha antenna interna: controllare connessione e qualità del segnale con scatola chiusa. Wi-Fi non deve cambiare stop/interlock locale. Non vi è tachimetro che confermi il moto reale della ventola.
- Per la prima verifica usa soltanto STANDBY + lettura dashboard: **non serve rifare il collaudo dei tre impulsi**. Dopo testare una prima OTA del firmware, preservando i parametri heat attuali.
- La prova dell'utente `PULSE-241024-R1` su scatola chiusa ha riportato +4 °C sul Sanwa vicino piastra, non verifica assoluta della curva NTC o del limite termico finale.

### Console diagnostica in tempo reale

È presente anche `GET /api/events`, con un buffer circolare degli ultimi **32 messaggi**: versione, entrata negli stati BOOT/STANDBY/DRYING/COMPLETE/FAULT, transizioni PRERUN/HEATER ON/GAP/COOLDOWN e cause numeriche di eventuale FAULT. La dashboard aggiorna la console ogni 2 s. È un estratto strutturato delle informazioni diagnostiche utili, **non** un mirror byte-per-byte di tutta la UART `Serial`. Il buffer eventi è volatile; lo storico numerico CSV, quando salvato, è recuperabile anche dopo il riavvio.
