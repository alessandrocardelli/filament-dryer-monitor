# S9B180 — Caricamento OTA e UN test fisico (pronto, consenso registrato)

**Stato al 27 ottobre 2026:** l'utente ha autorizzato esplicitamente di procedere alla **singola prova S9B180** quando, in risposta alla domanda specifica su180s di HEATER ON cumulativi, aria25°C e Sanwa STOP50°C, ha ordinato di farlo. Non chiedere una nuova conferma degli stessi parametri. **Software pronto/compilato e .bin applicativo reale verificato; caricato sul dryer? NO, non ancora osservato; prova fisica? NO.** Queste azioni richiedono l'operatore presso il dispositivo, non sono eseguibili da GitHub/questa chat.

## Cosa / perché / precedente / prossimo

- **Cosa:** ONE-SHOT riscaldatore realmente ON/OFF in controllo SHT45 di temperatura **ARIA** verso25°C, massimo180s cumulativi ON, fan e protezione piastra NTC indipendenti.
- **Perché:** validare se l'SHT45 interviene quando l'aria raggiunge25°C e osservare l'inerzia termica. La piastra NON è tenuta a25°C.
- **Test precedente S9A (già eseguito):** 13 ON, max budget120s terminato con aria22,59°C, picco22,97°C dopo HEATER OFF; Sanwa negli ultimi cicli 35/38°C, finale26°C; nessun FAULT. [Dati S9A](evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md).
- **Test futuro:** analizzare nuovo CSV S9B vs S9A, distinguendo OFF per NTC vs OFF per raggiungimento target25°C; nessun setpoint maggiore o nuovo test implicito.

## File pronto davvero, senza ricompilare

- **Firmware ESP32 SOLO applicazione OTA:** `HOT-261027-S9B180-OTA-app.bin` — **1.127.008 byte**, header ESP32 `E9`, ID reale `HOT-261027-S9B180` trovato al suo interno.
- **SHA256 del .bin, verificato anche dopo download del reale artifact ZIP dal CI:** `2e4cbdf3265f66bb6028ae04b7bff8441d7860700a901f5eb6c1a9ea88bdfedc`.
- **GitHub Actions CI effettivamente PASS:** [run40577584938](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40577584938); articolo scaricabile nella sezione **Artifacts** con nome `S9B180-HEAT-CAPABLE-OTA-APP` (ZIP contiene `.bin` + `.sha256`). Sono disponibili anche i log host GPIO-mock/ESP32; build 85% flash/28% RAM statica, nessun linker error.
- Build compilata a partire dal branch `experiment/step09b-air25-budget180-2026-10-27` della [PR #17 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/17). Identità **OLED `H27-B180`**. **Non usare** firmware `H27-S9B`: vecchie varianti180/200s avevano lo stesso ID ambiguo.
- Questo `.bin` è **applicativo OTA**, NON `merged.bin`, flash completo, bootloader o tabella partizioni. Caricare il file `.bin`, **NON lo ZIP**.

## Prima di iniziare: due verifiche sul posto (non altri test a freddo)

1. Confermare che il dryer sia **in STANDBY, riscaldatore spento e raffreddato**. Configurazione immutata S9A: Sanwa termocoppia K RD701 **stabilmente sulla piastra presso NTC**, SHT45 aria camera, TCO105 fisicamente in serie senza bypass, fan libera e funzionante, cavi senza pizzicature o anomalie, enclosure chiusa, alimentazione24V rapidamente scollegabile. Non assumere che queste verifiche siano già state effettuate o certificate a distanza; se qualche condizione non vale, non avviare il calore.
2. Da telefono/PC **sulla stessa rete del dryer**, raggiungere `http://filament-dryer.local/ota` oppure `http://<IP-del-dryer>/ota`; nome utente **`admin`**, password OTA già configurata nel dryer (**non condividere password in chat**). Questa pagina rifiuta un caricamento fuori STANDBY (`409 OTA only in idle STANDBY, heater OFF`). Se manca Wi-Fi/mDNS, usare l'IP già visualizzato nel dryer, non bypassare la sicurezza.

## Caricamento (nessuna operazione heater richiesta durante upload)

3. Nella pagina `/ota`, **selezionare il SOLO file `HOT-261027-S9B180-OTA-app.bin`** ed eseguire `Install firmware`. Durante l'upload non togliere alimentazione, non avviare un ciclo e non premere altri pulsanti. La pagina segnala `OTA completed. Rebooting...` oppure errore; in caso d'errore non avviare il riscaldatore.
4. Dopo reboot, confermare `H27-B180` sull'OLED (mostrato durante la schermata firmware), **oppure aprire `http://filament-dryer.local/api/status` e verificare la stringa esatta** `"firmware":"HOT-261027-S9B180"`. Se differisce, niente test.

## UNA sola prova reale HEATER ON a25°C

5. **PRIMA del tasto ON/OFF**: leggere **Sanwa iniziale** e **SHT45 aria iniziale**; se l'aria è oltre24°C potrebbe non attivarsi il riscaldatore — non forzare la sonda, non cambiare valori. Il Sanwa va sorvegliato **per tutta la prova**, non solo nei momenti di annotazione.
6. Premere **ON/OFF breve una volta**. La ventola deve girare realmente. Segnare **Sanwa a OGNI `HEATER ON` e a OGNI `HEATER OFF`**, con numero della finestra (come S9A). Se scrivere distrae, filmare contemporaneamente Sanwa+OLED e trascrivere dopo la prova. Il firmware può spegnere e riaccendere per NTC soft e pause.
7. Se **SHT45 aria raggiunge25,0°C**, annotare in quello stesso momento **Sanwa, aria SHT45 e stato HEATER ON/OFF**. Se HEATER era già OFF per NTC, non dichiarare erroneamente di avere visto uno spegnimento causato direttamente dal target.
8. **Arresto immediato** se **Sanwa>=50°C**, ventola fermata, odore, deformazione, fili danneggiati, fumo, altre anomalie/FAULT: tasto **ON/OFF lungo ~1,5s** durante ciclo. In emergenza togliere **24V**. Non attendere una lettura pianificata, non cercare di arrivare ai25°C a ogni costo. Max180s HEATER ON **cumulativi**, **non consecutivi**; max20s per ON, max durata fase calda600s, raffreddamento con ventola90s dopo ultimo HEATER ON.
9. Al passaggio **COOLING**, leggere **Sanwa e SHT45 aria**. A **fine COOLING, quando la ventola si ferma**, leggere **Sanwa e SHT45 aria finali**. Non avviare un secondo ciclo.
10. Dopo cooling, aprire `http://filament-dryer.local/api/last-test.csv` (o indirizzo IP) e scaricare il **nuovo CSV S9B**; allegarlo qui con le letture Sanwa cronologiche. Se non si scarica, descrivere il problema **senza ripetere l'accensione**.

## Integrità e limiti

- Protezione piastra RAW NTC: SOFT HEATER OFF se R<=50kΩ, ripresa con R>=65kΩ e pausa>=12s; guasto latched se R<=40kΩ o circuito aperto. Sono OHM non °C calibrati del bead.
- La ventola100% è **PWM comandato**, non giri misurati: osservarla fisicamente. Il fusibile TCO105 in serie non va toccato.
- Questo test non certifica materiali, hotspot, MOSFET guasto ON o temperatura assoluta di ogni punto; **non lasciare incustodito**.
- Documentare la prima esecuzione nel [modulo evidenze S9B (ancora NON ESEGUITO)](evidence/thermal/2026-10-27-S9B-PHYSICAL-SESSION.md) e caricare CSV originale con SHA256 prima di qualsiasi nuova decisione.
