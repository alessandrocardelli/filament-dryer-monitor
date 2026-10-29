# STEP10A — Profilo piastra/aria + diagnostica robusta (28 ottobre 2026)

> **STATO: FIRMWARE CANDIDATO, NON ANCORA PROVATO SULL'HARDWARE.** Questo file descrive UNA NUOVA prova HEATER ON proposta. Nessun flash caldo e nessuna sessione fisica eseguiti da ChatGPT. Verificare esito CI sull'ultimo commit e la sicurezza del montaggio prima dell'impiego. L'utente ha chiesto di preparare il prossimo firmware e la procedura; questa richiesta non certifica di per se' nuovi limiti termici. Nessuna autorizzazione implicita a produzione/autonomia.

## COSA / PERCHÉ / PRECEDENTE / DOPO

- **COSA:** prova **una sola volta** del controllo aria a25 °C con NTC locale meno restrittivo: pausa R<=45 kΩ, riarmo R>=60 kΩ, FAULT hard R<=40 kΩ (invariato), senza conversione non qualificata R→°C. Aggiungere diagnosi di tempi del loop, freschezza SHT45 e NTC, identificazione esatta del fault, acquisizione dei fronti HEATER/phase, e non rispondere a streaming CSV durante heating/cooldown.
- **PERCHÉ:** S9B180 con soft50/65 kΩ ha tenuto il Sanwa **sulla piastra vicino all'NTC** a35–37 °C a regime e l'aria ha raggiunto solo23,54 °C al FAULT. Ridurre un poco il cutoff locale è un esperimento controllato; non un limite di progetto valido. Gli 8 gap del loop crescono da1,707 a4,670s, ed il primo STOP vero è `pulse_fault=4 = AirStale` a ms575852. La ripetuta richiesta HTTP di CSV completo durante il ciclo è un'ipotesi tecnica plausibile di blocco, NON una dimostrazione conclusiva.
- **PRECEDENTE:** S6C Sanwa massimo53 °C con NTC minimo30,32 kΩ (differente profilo); S7U HARD40k è scattato con Sanwa43 °C; S9A R soft50k, Sanwa35–38 °C e aria picco22,97 °C; S9B soft50k, Sanwa35–37 °C, aria picco23,77 °C, terminato per SHT45 stale. Tutte le prove già archiviate: **NON RIPETERE** S9B per recuperare i dati.
- **DOPO:** se 25 °C raggiunti senza fault e termostato SHT45 aziona OFF/ON, correlare NTC/aria/Sanwa per revisione locale della piastra e successiva progettazione verso i 40/50/55 °C ARIA. Se fallisce, il nuovo CSV deve diagnosticare il motivo e portare a una sola modifica motivata, non a rilanciare test identici.

## Identità e limiti, esclusivi di questa build

- Branch: `experiment/step10-heater-diagnostics-air25-2026-10-28` (derivato dal branch S9B180, PR#17 non mergiata); **solo quel branch contiene il firmware candidato**.
- Firmware via `/api/status`: **`HOT-261028-S10A25`**, OLED **`H28-S10A`**.
- Aria target SHT45 **25 °C**, riarmo **<=24 °C**, fault ceiling aria **40 °C**. Si testa **25**, NON 40/55 °C aria.
- NTC heater raw: soft OFF **<=45,0 kΩ**, riarmo **>=60,0 kΩ**; hard FAULT **<=40,0 kΩ** e open **>=250,0 kΩ**. **La differenza 5k fra SOFT e HARD non è un margine termico certificato**. L'NTC non è una misura di °C validata.
- Un impulso ON al massimo **20,0s**, timer hardware di disinserimento **20,5s**, OFF minimo **12s**. Massimo ON **cumulativo 300s**, sessione completa **720s**, fan PWM100% (solo COMANDO, non tachimetro), prerun5s, cooldown90s. La logica può fermarsi molto prima.
- SHT45 AirStale rimane **5s**, NTC age1,5s. Non è stato allungato il timeout per nascondere il difetto S9B. No riscaldamento dopo FAULT/STOP fino a power-cycle.
- Il veto **Sanwa sulla piastra vicino all'NTC a 50 °C** è vincolante: STOP immediato se raggiunto, se compare un riscaldamento anomalo o se la ventola si arresta fisicamente. Non affidarsi a 50 °C come controllo elettronico autonomo.
- TCO105 segnalato installato **in serie e invariato**, nessun bypass; verificare fisicamente prima dell'uso. Materiali, hotspot e TCO non qualificati per profilo OEM. Heater production `kHeaterOutputEnabled=false`.

### Perché non 118 °C direttamente?

Il **Sanwa misura la PIASTRA**: è corretto confrontare il dato con la piastra nominale dell'eSUN eBOX Lite 105/116/118 °C, mentre l'aria nominale è40/50/55 °C. La posizione Sanwa non è necessariamente identica al punto OEM; il TCO105 aggiunto potrebbe interrompere prima dei livelli OEM. Queste differenze NON sminuiscono il confronto: mostrano che stiamo volutamente caratterizzando una piastra molto più fredda. Però non danno il permesso di disattivare limiti, bypassare TCO o assumere che tutti i componenti della modifica tollerino118°C.

## Verifica software GATE prima di usare Arduino IDE

1. Aprire **Actions → STEP10A - local-heater diagnostic candidate** sulla branch STEP10A. Accettare **solo PASS sull'ultimo commit che contiene firmware e workflow**. La workflow usa `g++` host GPIO-mock `firmware/tests/host/run_step09_feedback.sh`, Arduino-ESP32 **3.3.12**, U8g2 **2.36.19**, compila il vero sketch ESP32 e pubblica il binario OTA + SHA256. **Non proclamare CI PASS senza il report effettivo**.
2. Se ci sono fallimenti/nessuna esecuzione, non installare il candidato HEAT. Non usare vecchi binari `H27-S9B`, `H27-B180` o un download da altro branch.
3. Prima del nuovo firmware copiare dal dispositivo, se utile, `/api/last-test.csv`: S9B è già archiviato nel repo, completo di fault/event log; non avviare il riscaldatore per riottenere dati.

## Arduino IDE: caricamento completo della build STEP10A

1. Da GitHub selezionare la branch esatta STEP10A; **Code → Download ZIP**; estrarre il contenuto. Non usare ZIP del `main` (il main non contiene l'esperimento HEAT).
2. Installare Arduino IDE 2.x, se non già presente. In **File → Preferences → Additional Boards Manager URLs** controllare l'URL Espressif `https://espressif.github.io/arduino-esp32/package_esp32_index.json`; in **Boards Manager** selezionare **esp32 by Espressif Systems versione 3.3.12**. In Library Manager installare **U8g2 v2.36.19**. Non cambiare versioni a intuito.
3. Aprire **`firmware/firmware.ino`** all'interno dello ZIP estratto. Se Arduino IDE propone di spostare lo sketch, mantenerlo in una cartella chiamata `firmware` con **tutti** i file `.h` e sottocartelle accanto a `firmware.ino`.
4. In **Tools → Board** selezionare **ESP32 Dev Module** (`esp32:esp32:esp32`), il profilo usato dalla CI; preservare il **partition scheme OTA** usato dal prototipo. Collegare ESP32 alla USB/programmatore o selezionare la porta OTA Arduino IDE se disponibile/configurata. Il vecchio firmware S9B è **FAULT-latched**, perciò l'OTA non è disponibile finché rimane in FAULT; non tentare un aggiornamento nel mezzo del cooling. Dopo evidenze salvate e alimentazione isolata, un riavvio controllato per tornare a STANDBY può essere necessario.
5. Premere **Verify/Compile**. Non procedere se ci sono errori o avvisi su board/partizioni incompatibili. Per USB scegliere la porta seriale corretta e **Upload** solo con HEATER disalimentato e connessioni di programmazione già verificate; per ArduinoOTA utilizzare soltanto il percorso autorizzato e la porta configurata: se non compare, non improvvisare flash o cablaggi.
6. Avviare con HEATER alimentato solo **dopo** verifica codice caricato: nella schermata firmware/OLED deve comparire **`H28-S10A`**, e `/api/status` deve riportare **`HOT-261028-S10A25`**. Qualunque ID diverso: STOP.
7. **L'alternativa al caricamento da sorgente con IDE** è il file `HOT-261028-S10A25-OTA-app.bin` dell'artifact GitHub Actions, solo dopo CI PASS e SHA verificato, caricato via `/ota` in STANDBY. Non usare bootloader, partitions o merged flash sul browser OTA.

## Procedura fisica proposta — UNA sola sessione presidiata

**Gate: non eseguire finché non risultano superati CI dell'immagine esatta, controllo del montaggio e accettazione esplicita dell'esposizione diversa da S9B.** La semplice produzione di questo codice non certifica la sicurezza del nuovo envelope.

1. Preparare **Sanwa fissato sulla PIASTRA vicino all'NTC** come in S7U/S9B, consultabile senza aprire il box; fissaggio cavi e distanze da parti calde, TCO105 realmente **in serie**, NTC e alimentazione24V integri, ventola realmente libera e rotante, 24V isolabile subito, box nella configurazione definitiva. Niente materiale combustibile/filamento durante questo test. Annotare ambiente iniziale e aria SHT45 (meglio sotto24 °C per consentire avvio).
2. Prima di HEAT controllare versione `HOT-261028-S10A25`, i valori SHT45/NTC coerenti a freddo; se sensori invalidi, fan fermo, TCO/cablaggio dubbio, STOP.
3. Registrare in video **Sanwa sulla piastra e display** o prendere letture con tempo, soprattutto ON1/OFF1, primi cicli, ogni stabilizzazione, temperatura al primo25°C aria e dopo OFF. La pagina web **non aggiorna in tempo reale durante HEATER ON**; non affidarsi ad essa per verificare lo stato del MOSFET. Non scaricare CSV durante ciclo/cooling (l'endpoint risponde409).
4. Premere **ON/OFF una volta** per avviare la sessione sperimentale. Fan deve girare; prerun5s; riscaldamento a finestre <=20s e protezione R_NTC. Restare presente per **l'intera sessione** e cooldown.
5. **STOP IMMEDIATO** se Sanwa raggiunge50 °C, crescita troppo rapida o non plausibile, odore, cavi caldi, stop ventola, suono/FAULT, qualunque anomalia. Il comando STOP documentato è **pressione lunga ON/OFF** durante DRYING; se la GUI non risponde, interrompere esternamente i24V. NON riarmare durante la stessa sessione.
6. Se arriva a25 °C aria, annotare l'evento OFF automatico, poi l'eventuale riarmo a24 °C se avviene entro i limiti. Non forzare una prova aggiuntiva per vedere il riarmo; il test rimane una sola sessione.
7. Attendere il raffreddamento controllato senza spegnere la ventola prematuramente, salvo emergenza elettrica. Al termine scaricare `http://filament-dryer.local/api/last-test.csv`, poi `/api/status` e `/api/events`. NON affidarsi ai contatori `pulses` come numero di letture manuali Sanwa.
8. Archiviare **CSV ORIGINALE senza modifiche, hash, ID firmware, letture Sanwa, fault/eventi e condizioni** in `docs/evidence/thermal/`, aggiornare `docs/PROJECT_STATE.md`, `docs/TODO.md`, `README.md`, `AGENTS.md`. Se fault, non eseguire un secondo HEAT fino alla diagnosi.

## Nuovi dati CSV (15 colonne)

`ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent,pulse_fault,ntc_fault,air_age_ms,ntc_age_ms,loop_gap_ms`

- I 10 campi storici sono mantenuti nell'ordine originario; le cinque colonne nuove consentono di identificare cause ed età e blocchi del loop.
- I cambiamenti osservabili di `phase/heater_command/pulse_fault` forzano la registrazione anche prima dei 750ms. **Questo NON può recuperare transizioni mentre il loop è davvero bloccato**: il cutoff hardware resta separato.
- `phase=6` = ABORTED; `pulse_fault=4` = AirStale; `pulse_fault=11` = ManualStop. `fault=0` non significa fine regolare se altre cause di latch.
- L'ultima prova S9B aveva `AirStale` e `FAULT ntc=0 pulse=4`, non un manual stop. L'ipotesi CSV web sincrono deve essere verificata confrontando i `loop_gap_ms` della nuova esecuzione.

## Esiti non promessi

Questa revisione NON garantisce che la camera raggiunga25 °C (men che meno55 °C), NON qualifica la temperatura reale dell'NTC o i punti caldi della piastra, NON valida il TCO né dimostra RPM della ventola. È un passo **unificato** di software affidabile + profilo termico più informativo, non un rilascio prodotto.
