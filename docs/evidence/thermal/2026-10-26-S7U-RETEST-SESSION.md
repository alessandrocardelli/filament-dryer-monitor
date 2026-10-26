# STEP07-S7U — Verbale completo della sessione fisica supervisionata

**Stato: PROVA FISICA ESEGUITA, FAULT NTC R LIMIT — ACQUISIZIONE G1 PASS LIMITATA, QUALIFICA V1 NON PASS.**

**Fonte diretta n.1:** messaggio dell'utente del 26 ottobre 2026 intitolato `PROVA S7U`, trascritto integralmente sotto.
**Fonte diretta n.2:** allegato utente originale `filament-last-test (2).csv`, conservato byte-identico come [2026-10-26-S7U-retest-raw.csv](2026-10-26-S7U-retest-raw.csv).
**Fonte derivata (non sostituisce dati grezzi):** [analisi numerica indipendente con tabella timeline](2026-10-26-S7U-retest-analysis.md).
**Protocollo prima della sessione:** [STEP07_S7U_PHYSICAL_RETEST_PROTOCOL_2026-10-26.md](../../STEP07_S7U_PHYSICAL_RETEST_PROTOCOL_2026-10-26.md).
**Firmware/codice:** test dichiarato `S7U` dall'utente; versione prevista dal protocollo `HOT-261026-S7U` / OLED `H26-S7U`, ramo `fix/step07-clear-test-feedback-2026-10-26`, PR #12, sorgente originario `8f804f5ef08c9ec91eeadd576fc35fc976250638`. Il firmware non scrive il proprio ID nel CSV: **non esiste uno screenshot/versione status fornito con cui confermarlo indipendentemente**. Commit di sola documentazione posteriori non modificano la logica termica. Sorgenti di autorità `firmware/firmware.ino`, `firmware/HeaterStep07GuardTrial.h`, `firmware/HeaterPulseCommissioning.h`, `firmware/SafetyController.h`, `firmware/ThermalLog.h`.

## 1. Testimonianza originale e non interpretata dell'utente

Riportata ESATTAMENTE come fornita in chat:

```text
PROVA S7U
Sanwa iniziale: 17
Sanwa primo HEATER ON: 17
Sanwa prima terzo impulso: 36
Sanwa all'arresto definitivo: 43
Messaggio finale: NTC R LIMIT
Raffreddamento: Temperatura scesa regolarmente
Note: Quando la ventola si spegne SANWA a 25°C
```

I numeri Sanwa sono **°C** secondo strumento e protocollo. Le letture sono osservazioni indipendenti dall'ESP32, non campioni del CSV, e non possiedono timestamp civili o millisecondi sincronizzati. Il messaggio finale `NTC R LIMIT` è trascrizione utente dell'OLED; il CSV non contiene il testo del guasto.

## 2. Identità, condizioni fisiche e strumenti (fatti vs storico/non documentato)

| Campo | Evidenza effettiva di questa sessione |
| --- | --- |
| Giorno del resoconto/acquisizione | **26 ottobre 2026**; ora civile *esatta della prova* **NON FORNITA**, timezone progetto Europe/Rome; non assumere che l'ora di upload coincida con l'inizio prova |
| Operatore | **utente del prototipo**, test supervisionato riferito dall'utente; nessuna attestazione metrologica esterna |
| Prova/firmware | `PROVA S7U` secondo utente; ID display `H26-S7U` **NON allegato visivamente**, versione CSV non autoidentificabile |
| Forma test | UNA sessione con impulsi attivi, non test heater-OFF. Dati `heater_command=1` nel CSV per 237 campioni |
| Alimentazione | 24 V nominali dell'allestimento documentato; valore effettivo durante questa sessione **NON MISURATO** |
| Involucro | box chiuso secondo protocollo adottato per S7G/S7U; **nessuna nuova foto** della sessione, nessuna verifica indipendente della chiusura |
| Sanwa | multimetro **Sanwa RD701**, termocoppia tipo K su contatto vicino al riscaldatore/NTC, setup storico fissato in Step02; **posizione/tenuta non fotografata né verificata nuovamente** |
| NTC | sensore embedded su J7→GPIO34, misura `ntc_ohms`, non temperatura di piastra calibrata |
| SHT45 | sensore aria camera, valori `air_c` e `rh` nel CSV, non equivalenti al Sanwa |
| Fan | comando **100% in tutte le righe**, **fermata ventola dichiarata dall'utente** a Sanwa25°C; portata/RPM non misurati |
| TCO | storico prototipo: **TCO monouso 105°C in serie**; non è stata fornita conferma fotografica/funzionale specifica del test |
| Cavi/jack | percorso provvisorio storicamente noto; integrità attuale e assenza di anomalie **NON descritte nel messaggio** |
| Materiale nel box | **NON DOCUMENTATO**, non inferire presenza o assenza di filamento |
| Intervento manuale | **NESSUNO RIFERITO**; non equivale alla prova che nessun tasto sia stato premuto |
| Veto operatore da SOP | **50°C Sanwa** per questo profilo (diverso da55°C S6C), non consta che sia stato raggiunto: ultimo dato al taglio43°C |
| Video/foto/event log | **NON ALLEGATI**. Manca registrazione di buzzer, MODE mute, schermata `CSV READY` e orologio civile |

Separare i dettagli di montaggio **storici documentati nella repo** dagli elementi **osservati proprio in questa prova**, che sono le cinque misure Sanwa, messaggio FAULT, discesa e fermata ventola, e le grandezze del CSV.

## 3. Sequenza delle osservazioni Sanwa

| Evento | Sanwa RD701 | Natura della misura / sincronizzazione |
| --- | ---: | --- |
| Prima di iniziare | **17°C** | Lettura diretta riferita |
| All'inizio di `HEATER ON 1` | **17°C** | Lettura diretta riferita, primo impulso presente anche nel CSV |
| **Prima** del terzo impulso | **36°C** | Lettura diretta riferita; nel CSV terzo ON comincia a `ms=76806`; nessuna sincronizzazione di precisione |
| **Arresto definitivo** | **43°C** | Lettura diretta riferita simultanea all'OLED `NTC R LIMIT`, CSV prima riga OFF a `ms=96056` |
| Durante il raffreddamento | **temperatura scesa regolarmente** | Descrizione qualitativa dell'utente, **nessun obbligo né evidenza di picco POST-OFF** |
| **Quando la ventola si è spenta** | **25°C** | Lettura diretta riferita, non presente automaticamente nel CSV; tempo esatto della fermata non misurato |

Non ci sono altri campioni Sanwa registrati; **non stimare** tempo di ritardo, Tmax superficie fra letture, temperatura bead NTC o corrente. **Non confondere Sanwa43°C con SHT45 aria19,52°C all'OFF**: punti fisici diversi.

## 4. Esito dei comandi e fault: righe del CSV

Dati conservati byte-identici; analisi completa e metodo nella [relazione numerica](2026-10-26-S7U-retest-analysis.md).

| Evento | Uptime MCU | Durata HEATER ON o GAP precedente | NTC (kΩ) | Aria SHT45 °C | Fase |
| --- | ---: | --- | ---: | ---: | --- |
| Primo campione fan/prerun | 6785 ms | — | 136,9517 | 18,20 | 1 / prerun |
| Primo ON | 12806 ms | ON1 inizia | 135,6839 | 18,21 | 2 / pulse |
| Primo OFF/GAP | 32806 ms | ON1 **20,000 s** | 62,2874 | 18,34 | 3 / gap |
| Secondo ON | 44806 ms | GAP **12,000 s** | 78,6969 | 18,47 | 2 / pulse |
| Secondo OFF/GAP | 64806 ms | ON2 **20,000 s** | 46,4015 | 18,79 | 3 / gap |
| Terzo ON | 76806 ms | GAP **12,000 s** | 61,2710 | 19,07 | 2 / pulse |
| **FAULT e definitivo OFF** | **96056 ms** | ON3 **19,250 s** | **39,7893** | **19,52** | **6 / aborted** |
| Ultima riga cooldown | 185806 ms | **89,750 s dopo OFF** | 97,9547 | 20,89 | 6 / aborted |

- **HEATER ON comandato totale = 59,250 s**, due ON da20s e uno troncato a19,25s. **Non** 6 impulsi effettivamente completati e **non** 120s di HEAT.
- R_NTC immediatamente prima del taglio a `ms=95806` = **40159,9 Ω** (HEATER=1); primo record al taglio a `ms=96056` = **39789,3 Ω** (HEATER=0 e phase6). **Attraversamento40kΩ entro 250ms di campionamento**, compatibile direttamente con il motivo OLED dichiarato `NTC R LIMIT`. Non è prova indipendente di MOSFET OFF elettrico.
- Valore minimo NTC nel file = **39593,4 Ω** a `ms=96556` (0,500s dopo prima riga OFF); ultimo97,9547kΩ.
- SHT45 aria primo→OFF→fine = **18,20→19,52→20,89°C**; la camera **continua a riscaldarsi di1,37°C durante il cooling dopo OFF**. Questo è un effetto nella **temperatura ARIA**, non significa che il Sanwa/piastra continuasse a salire: l'utente ha osservato Sanwa in discesa.
- RH primo→OFF→fine = **64,12→63,39→58,65%**.
- CSV in **phase6/Aborted** fino all'ultima riga; sensori air/NTC validi in **717/717**; fan commanded100% in **717/717**. Il file termina appena prima/attorno alla fine del cooling pianificato; il cambio effettivo della ventola a PWM0 **non è nel CSV**, ma l'utente ne conferma la fermata a Sanwa25°C.
- La registrazione della sequenza STOP automatico è **PASS limitato** sul comando software; i 90s circa di ventilazione sono documentati come **comando PWM**, non collaudo tach/flow.
- OLED `NTC R LIMIT` è noto per dichiarazione utente. **Buzzer una-tantum/MODE mute, presenza del testo `CSV READY`, `HEATER ON x/6` visualizzati puntualmente non riferiti**; non dichiarare testati.
- L'allegato disponibile implica che la registrazione CSV è stata ottenuta; non permette di dimostrare separatamente la gestione di guasto LittleFS.

## 5. Originale, integrità, file, dati mancanti

- File ricevuto: `filament-last-test (2).csv`, **29142 byte**, 718 righe compresa intestazione = **717 campioni**, timestamp MCU `6785→185806ms`, intervallo log **179,021 s**, mediana campioni250ms.
- Path **GitHub**: `docs/evidence/thermal/2026-10-26-S7U-retest-raw.csv` nel ramo S7U.
- **SHA-256 del file ORIGINALE**: `c417313414eede75f2269dbfb8717ac6d8246625172e315a007b8056def834ae`.
- **Git blob SHA-1 originale e repo confrontati**: `891ea18eb00efdb6690e7b7e3c0c6466e6a39674`, prova byte-identica dopo upload, inclusi fine-riga originali.
- Combinazione completa dei campi grezzi: `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent`; nessuna riga mancante, doppia o invalid sensor fra quelle presenti.
- Nessun `/api/events`, serial `HOT_CSV`, video/foto, firma del binario installato, misura amperometrica, tacchimetro, prova TCO o temperatura distribuita della piastra allegati. **Non richiedere per rifare a caldo una prova già conclusa**. Riportare le limitazioni in ogni futura decisione.

## 6. Valutazione, significato ingegneristico, NEXT

**G1 ACQUISIZIONE = PASS LIMITATA**: test reale dichiarato S7U, Sanwa a cinque eventi/valori (incluso termine raffreddamento), motivo FAULT esatto, log originale intero, crossing della guardia40kΩ riproducibile, comportamento qualitativo Sanwa in discesa, nuove misure SHT45. Sono presenti e persistenti **tutti i dati realmente ricevuti**; quelli non misurati sono documentati, non inventati.

**Cutoff NTC40kΩ osservato funzionare nella sequenza software**, ma NON è ancora approvazione della soglia per funzionamento continuativo del dryer o prova di sicurezza rispetto a hotspot/lag/fan mancante/TCO/materiali. **Aria SHT45 arriva solo20,89°C al termine della traccia**, quindi non abbiamo raggiunto il futuro obiettivo di40°C aria.

**Prossima attività unica e concreta (G2/G3):** valutare con i dati S7U+storici S6C la guardia NTC RAW40kΩ, termica differenziale locale Sanwa43°C / aria19,52°C al taglio, trend aria +1,37°C dopo OFF, condizioni TCO105/materiali/fan senza tach, rischio di MOSFET ON. Proporre soltanto modifiche motivate e verificabili, con review prima di ulteriori accensioni. **Non ripetere S7U**, non recuperare vecchio CSV S7G, non iniziare altro ciclo HEAT per verificare beep/UI. Una volta definita la strategia, avanzare verso termostato aria SHT45 40°C in *simulazione con riscaldatore fisicamente OFF*; poi eventuale piano reale distinto e approvato. Non modificare automaticamente il codice heater o fare merge della PR #12 su main.

## 7. Commit e passaggio di consegne

- **CSV originale già archiviato:** commit `57ac65ccb9987d7a4e27e6202deeb49e2bbc9363` ([GitHub](https://github.com/alessandrocardelli/filament-dryer-monitor/commit/57ac65ccb9987d7a4e27e6202deeb49e2bbc9363)).
- **Analisi numerica già archiviata:** commit `0792dd9d2e9d4c9190b5df78b840a67d0acd29e5` ([GitHub](https://github.com/alessandrocardelli/filament-dryer-monitor/commit/0792dd9d2e9d4c9190b5df78b840a67d0acd29e5)).
- **Questo verbale e i successivi aggiornamenti di stato** sono registrati tramite cronologia Git del file; non scrivere uno SHA del commit che non esiste ancora al momento di questa modifica.
- Dopo questa registrazione, allineare **README.md**, **docs/PROJECT_STATE.md**, **docs/TODO.md**, **docs/THERMAL_EXPERIMENT_LEDGER.md** e **docs/HANDOFF_2026-10-26_STEP07_S7G_TO_S7U.md** con la **priorità attuale G2/G3 e nuova prova S7U già eseguita**, mantenendo gli storici distinguibili. Tutti i file vivono in repo; lo stato in chat non sostituisce questo archivio.
