# S9B180 — Prima sessione fisica reale HEATER ON, 28 ottobre 2026

> **Precisazione temporale ON20:** nel CSV l'ultimo OFF (ciclo19) avviene al `ms=569285`; `phase=6` compare a `ms=575851`, **6,566s dopo**. Il codice `HeaterStep09AirFeedback.h` richiede `minimumOffMs=12000`, quindi **non può aver legalmente iniziato il20° ON nei6,566s successivi** se era installata la build S9B180 prevista. Il20° ON è una osservazione manuale da preservare, ma **non corroborata** dai comandi registrati; possibilità di diversa interpretazione OLED/tempi, firmware differente o altro comportamento da verificare, non di impulso certo sfuggito. Recuperare `pulse_fault` e `firmware` da status; non inventare un ON non mostrato dal CSV. 

**STATO: PROVA FISICA ESEGUITA; ANALISI PRELIMINARE DEI DATI COMPLETATA; CHIUSURA NON REGOLARE (fase 6, ABORTED), CAUSA SPECIFICA ANCORA SCONOSCIUTA.** Questo è un record di evidenza, non un nuovo protocollo di riscaldamento. **NON RIPETERE S9B IDENTICA, NON AUMENTARE IL BUDGET INDEFINITAMENTE E NON ABBASSARE LE SOGLIE NTC PRIMA DELLA DIAGNOSI DELLO STOP.**

## Cosa / perché / prima / futuro

- **COSA:** verifica reale di S9B180 con HEATER comandato a impulsi variabili dall'NTC locale, setpoint aria SHT45 25°C e budget cumulativo HEATER ON 180s.
- **PERCHÉ:** in S9A, 120s di riscaldamento avevano portato aria solo a22,59°C prima di COOLING; S9B allunga il budget per tentare la validazione dell'arresto termostatico dell'ARIA.
- **TEST PRECEDENTE:** S9A vera, 13 ON (12 completati+ultimo0,5s), Sanwa17°C iniziale / 35–38°C durante cicli stabilizzati / 26°C fine cooling, SHT45 aria17,64→22,59°C al normale COOLING (fase4), picco22,97°C, NTC min48,7167kΩ. Tutti i suoi dati restano su main.
- **FUTURO:** il problema dominante ora è la limitazione locale intorno R_NTC50kΩ/valore Sanwa37°C e la fase6 non diagnosticata. Progettare un **regolatore a due temperature**: setpoint ARIA SHT45 distinto da guardia/obiettivo PIASTRA NTC da giustificare con misure reali; non più solo un budget sempre crescente. Prima recuperare `pulse_fault` dal sistema SENZA RIACCENDERE e confrontare con eventi/sicurezze. Nessun nuovo flash o HEAT implicitamente autorizzato.

## Fonti e integrità

- **File originale utente:** `filament-last-test (4).csv`, montato localmente, **44.461 byte**, **1.067 righe dati + header**, registrazione `ms` iniziale105365, finale659021, durata traccia **553,656s** (non ora assoluta). **SHA256 `1e17e6e131ce7cf9074bfe8d23d31f5e8ea1a605833ff36c0a21fe6568314beb`; Git blob SHA1 `db944d63d5a97ad4a3d39bdae4753a9d87822c97`.** Conservato **BYTE-IDENTICO** su questo branch e su `main`, SHA Git verificato dopo il push.
- [CSV raw originale S9B180](2026-10-28-S9B180-raw.csv). Questo è IL file S9B, non S9A. Una singola CRLF sulla riga header, altre righe LF come nell'upload, EOF newline; non ripulire e non rigenerare durante l'analisi.
- **Letture Sanwa comunicate dall'utente il 28/10/2026:** `Iniziale16; 1° ciclo on16 off28; 2° ciclo on28 off35; 3° ciclo on34 off36; 4° ciclo on34 off36; 5° ciclo on35 off36; dal6° al19° ciclo on35 off37; 20° ciclo on36 poi parte il cooling; Fine cooling26°C`. Questi sono **dati manuali**, non colonne del CSV e non sincronizzati al millisecondo. Non inventare un Sanwa OFF del 20° ciclo. Data/ora locale effettiva di avvio non data (data ricezione28 ottobre).
- Firmware candidato previsto: `HOT-261027-S9B180`, OLED `H27-B180`, branch `experiment/step09b-air25-budget180-2026-10-27`, PR#17 draft. L'identità esatta non è stata ridichiarata dall'utente al momento della prova, ma il CSV è compatibile con il profilo. Non presentare il checksum del bin come prova che tale bin sia sicuramente quello caricato.

## Numeri reali CSV e Sanwa (non inferenze di sicurezza)

| Metrica | Valore verificato |
| --- | --- |
| Sanwa iniziale | **16°C** |
| Sanwa c1 ON/OFF | **16/28°C** |
| Sanwa c2 ON/OFF | **28/35°C** |
| Sanwa c3 ON/OFF | **34/36°C** |
| Sanwa c4 ON/OFF | **34/36°C** |
| Sanwa c5 ON/OFF | **35/36°C** |
| Sanwa c6–c19 ON/OFF, riportato per ciascuno | **35/37°C** |
| Sanwa c20 ON poi cooling/stop | **36°C ON**, **OFF non fornito** |
| Sanwa alla fine del cooling | **26°C** |
| SHT45 aria inizio | **16,77°C** |
| SHT45 aria all'ingresso fase6 | **23,54°C**, a t relativo470,486s |
| SHT45 aria massimo nell'intero CSV | **23,77°C**, dopo stop |
| SHT45 aria ultimo campione | **23,70°C** |
| SHT45 target | **25°C, MAI raggiunto in 1.067 campioni** |
| NTC resistenza minima | **48.687,8Ω** (~48,688kΩ), ben sopra hard40kΩ |
| Fronti HEATER ON/OFF visibili in CSV | **19 ON e 19 OFF**, tutti in fase2, con 19 pause fase3 |
| HEATER ON visibile dai fronti campionati | **~175,332s** (granularità/jitter campioni, NON misura indipendente di corrente/energia) |
| Fase6 ABORTED | **159 righe**, da t470,486s a553,656s (83,170s), ventola PWM comando100% |
| Fase4 COOLING regolare | **0 righe**; non equiparare fase6 a fase4 |
| Validità sensori nel CSV | air_valid1 e ntc_valid1 in tutte1.067 le righe; non esclude stale/fault al momento dello stop, perché non si registrano età/errori separati |
| Comando ventola | fan_percent100 in1.067 righe; **NON equivale a RPM/portata misurati** |
| RH inizio/fine | **66,34% → 48,53%**, variazione relativa, non verifica rimozione acqua dal filamento |

### Cronologia essenziale dei cicli osservati e registrati

- Accensione1 CSV a t=6,000s e OFF a26,000s: **20,000s ON**, NTC a OFF65,636kΩ, Sanwa16→28°C. Comando OFF spiegabile dal limite20s.
- Accensione2 t=38,000s→57,332s: **19,332s ON**, NTC a OFF48,728kΩ, Sanwa28→35°C; soft limite NTC50kΩ intervenuto.
- Dal ciclo3 al19 le durate ON sono **10; 9; 8,5; 8,5; 8,5; 8; 8; 8; 8; 7,5; 7,5; 7,5; 7,5; 7,5; 7,5; 7,5; 7s**. Gli OFF sono con NTC circa49–50kΩ: l'uscita segue le soglie SOFT locale, non il target aria25°C.
- **Ultimo ON registrato = ciclo19**, t=456,920s→463,920s, SHT45 23,46→23,50°C, NTC ON65,096kΩ→OFF49,983kΩ. Ultima fase3 visibile a t=466,420s: R_NTC50,282kΩ. **Dopo un gap di 4,066 secondi nel CSV**, il primo campione successivo è già **`phase=6` / `pulses=19` / heater_command0 a t=470,486s**, SHT45 23,54°C e NTC55,913kΩ.
- **Discrepanza importante:** operatore ha chiaramente osservato **il20° ON** con Sanwa36°C, ma la traccia a500ms mostra soltanto 19 fronti e non incrementa il contatore oltre19. **NON dichiarare inesistente il20° ON**: un evento breve può essere non campionato durante il gap, oppure la visualizzazione operatore usa una numerazione diversa; la causa esatta richiede `pulse_fault` o log eventi. Non attribuire automaticamente altri4,668s ON né dichiarare budget180s raggiunto.
- Il firmware distingue `Phase::Cooldown = 4` da `Phase::Aborted = 6`. Il CSV registra **6**: una transizione di abort/cancel/fault, non la normale conclusione per budget. Il display in FAULT può continuare a mostrare **COOLING** per il fan-overrun; per questo l'osservazione utente “parte il cooling” non è sbagliata, ma non dimostra la causa della fine. Il parametro esatto `pulse_fault` **non è nel CSV attuale**. Possibili cause da verificare includono errori/staleness sensori, timer ISR, STOP o guasto latch; **NON inventare la causa**.

### Trend aria: il limite non è solo il tempo

| Intervallo di registrazione | Crescita aria SHT45 |
| --- | --- |
| t100→160s | **+1,25°C / min** |
| t200→260s | **+0,96°C / min** |
| t300→360s | **+0,69°C / min** |
| t400→460s | **+0,47°C / min** |

La crescita dell'aria rallenta mentre l'NTC impone circa7–8s ON e lunghe pause in tutti gli ultimi cicli e la piastra Sanwa resta intorno35–37°C. Lo scenario è **compatibile con una differenza termica / duty locale insufficiente a scaldare rapidamente la camera**; non prova il valore di equilibrio, né le temperature in ogni punto. Altri +60/80s ON potrebbero portare aria25°C più tardi (non garantito), ma **non rappresentano una strategia valida per i futuri target 40–55°C** del dryer. Serve un controllo distinto su piastra e aria.

## Proposta ingegneristica (NON nuovi setpoint di prodotto)

1. **Non ripetere S9A/S9B e non aumentare meccanicamente durata/cicli.** Progettare una logica a due soglie: domanda HEAT su SHT45 ARIA, e limite termico piastra NTC stimato da misura diretta nella geometria definitiva. La soglia soft R_NTC50kΩ è **quella che ha creato il collo di bottiglia locale**.
2. **La direzione è consentire alla piastra una temperatura più alta**, ma ciò equivale a **ridurre il valore R_NTC per la pausa SOFT** (NTC con dR/dT<0), non a cambiare i25°C dell'aria. NON proporre alla cieca valori arbitrari come cutoff di sicurezza. Il limite duro R_NTC40kΩ rimane separato, e il margine/inerzia dopo OFF deve essere verificato.
3. **Prima di una nuova prova calda:** recuperare, se la scheda non è stata riavviata, `http://<IP attuale>/api/status` e registrare `pulse_fault` e `phase`; `http://<IP attuale>/api/events` registra transizione e codici `FAULT ntc=... pulse=...`. Questi dati possono sparire con un reboot. **Nessuna accensione per recuperarli**. Se il codice è0 e fase6, ricercare STOP app; se diverso da0, mappare enum `HeaterCommissioning::Fault`. Un fault NTC elettrico può essere separato dal fault fase S9B. Non cambiare firmware prima di recuperare se ancora disponibile.
4. Prima di variare R_NTC soft, considerare misure S7U (hard 40kΩ con Sanwa43°C), S9A/B (soft50kΩ con Sanwa~37–38°C), posizione e inerzia effettiva del bead, TCO105 in serie, fan senza tach, materiali/cablaggi, e scelta dell'ampiezza di prova successiva con **nuovo budget e guardie motivati e consensuali**. Non si considera40kΩ un limite termico calibrato o sicuro per ogni hotspot. Il Sanwa contatto locale NON misura tutta la piastra.
5. Integrare nel firmware successivo **traccia di guasto nel CSV** (`pulse_fault` / `ntc_fault` / causa) e garantire chiarezza `COOLING` vs `ABORTED` sul display, così un eventuale successivo stop non sia ambiguo. Fare CI host/ESP32, non altro collaudo fisico a HEATER OFF.

**Outcome:** crescita aria reale +7,00°C fino picco23,77°C, target25°C non raggiunto; soft50k limita il calore locale; **stop/abort da diagnosticare**. La sessione fisica è finita e acquisita, non è un PASS del termostato aria25°C né un FAIL della capacità del riscaldatore originale.
