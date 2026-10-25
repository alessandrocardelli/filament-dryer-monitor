# Step 04 — NTC piastra: protezioni e piano di caratterizzazione

**Data:** 25 ottobre 2026  
**Ramo verificato:** `feature/wifi-ota-web-logging-2026-10-24`; HEAD di partenza `c7836a4a42445f275df497d02b00190195bcf525`.  
**Risultato:** **PASS ANALISI / STRATEGIA**, **NON PASS qualifica termica**. Gli step 05–06 rimangono necessari. Questo file è un protocollo di progetto e **non autorizza** un upload heater-enabled, una prova calda, un bypass del TCO o l'assegnazione di valori di protezione definitivi.

## 1. Principio fisico — sensori NON co-localizzati

- **SHT45 (J3):** temperatura **aria nella camera del filamento** e RH. È la variabile da regolare al setpoint scelto dall'utente. Installazione documentata con foto 25 ottobre, ma rappresentatività in dinamica non ancora verificata.
- **NTC (J7):** termistore inglobato nel riscaldatore; si misura elettricamente la sua **resistenza**, che per un NTC decresce in riscaldamento. Non misura direttamente la temperatura dell'aria, né possediamo oggi una specifica R25/B verificata del bead.
- **Sanwa RD701/K:** termocoppia ora **fissata secondo conferma utente** a contatto presso la piastra vicino all'NTC. È una **temperatura superficiale locale** di riferimento, non la temperatura interna del termistore. Le fotografie precedono il fissaggio; nuova stabilità da osservare al preflight della futura prova autorizzata.
- **Ventola:** trasporta calore dalla piastra alla camera. La velocità/aria influenzano gradienti e tempi, quindi i dati a box aperto e fan OFF non identificano i gradienti a box chiuso con fan ON. Il solo comando PWM non certifica rotazione/flusso (tachimetro assente).
- **TCO fisico:** 105 °C monouso **riferito** montato esternamente, in serie al riscaldatore, sotto nastro metallico a sinistra. Aggiunta **opzionale** per decisione D029; ma il componente realmente presente è un vincolo, non un'autorizzazione a rimuoverlo. Datasheet/portata DC/temperatura del suo corpo non qualificati.

Nessun confronto `T_SHT45 == T_NTC`. Non scegliere `100k/B3950` dal precedente allineamento casuale a freddo né dichiarare temperatura NTC assoluta.

## 2. Evidenza elettrica: KiCad vero + firmware reale

Nel foglio `hardware/IO_Power.kicad_sch` e nel netlist esportato `hardware/Filament_Dryer_Monitor.net` (snapshot repo; riesportare se lo schema cambia):

- `J7 pin 1 → GND`; `J7 pin 2 → NTC`; `R28 pin 1 → NTC`, `R28 pin 2 → 3V3_MCU`; `C19` NTC/GND; `NTC → U4 IO34 (pin 6)`. Quindi R28 è il **pull-up** verso 3,3 V e il termistore chiude a massa. `J5 pin 2 → 24V_PROT`, `J5 pin 1 → HEATER_SW`. Il TCO in serie è **cablaggio esterno al PCB**, non una componente netlist che il CAD possa certificare.
- `firmware/AppConfig.h` in sorgente freddo `COLD-251025-S3`: intervallo NTC ogni **500 ms**, 16 letture; correzione `mV_corr = 0.984369*mV_letti - 2.322`; alimentazione effettiva 3311 mV, `R_pullup_eff=46.572 kΩ`, diagnostica `150 < mV_corr < 3000`. `NtcSensor.h` calcola `R_NTC = 46572 * mV_corr / (3311 - mV_corr)`.
- Convertendo le finestre ADC con la formula sopra: **~2.21 kΩ a 150 mV**, **~449.25 kΩ a 3000 mV**. Sono finestre **elettriche** short/open, non soglie di sovratemperatura. Per esempio **7.12 kΩ** a Sanwa 85 °C nella prova aperta sarebbe ancora **un valore elettricamente valido**. Un NTC rimasto su un valore plausibile ma scorretto può non generare un guasto elettrico: servono guardie di coerenza e fallimenti temporali nella futura logica termica.
- Il partitore 3311 mV / 46.572 kΩ dissipa teoricamente nel termistore non più di **~0.059 mW** (massimo elettrico ideale a R_NTC=R28_eff), ma non abbiamo la costante di dissipazione del bead integrato: non trasformare il piccolo valore in un'affermazione di accuratezza termica.
- SHT45 nominalmente campionato ogni **2000 ms**, timeout stale configurato **10000 ms** nel sottosistema aria. Il sensore NTC aggiorna ogni **500 ms**, ma ADC non è una prova di temperatura reale e la risposta fisica dipende dal bead/supporto. In futuro definire un criterio di fault su *stale* coerente con questi periodi, senza confondere il firmware host fixture con quello fisico.

Fonti esterne applicative, **non datasheet del nostro componente**:
- Vishay Dale, *Selecting NTC Thermistors*, documento 33001, spiega il ruolo di dissipazione, montaggio, mezzo termico e costante di tempo: https://www.vishay.com/docs/33001/seltherm.pdf
- TDK Electronics, *Digital Readout of NTC Thermistors: Circuit Optimization* (2024), spiega partitore, autoriscaldamento e limiti del sistema di acquisizione: https://www.tdk-electronics.tdk.com/download/3229252/813b1afa3fb473d7a434b927186744b2/ntc-thermistors-readout-optimization-an-for-design-tools.pdf

## 3. Misure già raccolte (non ripeterle)

Fonte: `docs/NTC_CALIBRATION_ANALYSIS.md`: **coperchi entrambi aperti, ventola OFF, due punti fisici diversi**.

| Sanwa piastra superficiale (°C) | R_NTC (kΩ) | Uso legittimo |
|---:|---:|---|
| 30 | 64.62 | riferimento storico aperto |
| 39 | 42.27 | riferimento storico; già usato nel precedente limite di aborto impulsivo |
| 47 | 28.72 | riferimento storico aperto |
| 56 | 19.68 | riferimento storico aperto |
| 64 | 13.73 | riferimento storico aperto |
| 74 | 9.81 | riferimento storico aperto |
| 85 | 7.12 | riferimento storico aperto |

Le resistenze decrescono monotonicamente con l'aumento del Sanwa, indicando **coerenza qualitativa**. La correlazione numerica non può eliminare le differenze di posizione e l'offset dinamico. Il fit 7 punti `beta 4421.57 K/R25 80.69 kΩ` è un fit **al Sanwa**, non una proprietà identificata del bead. La tabella interpolata `R → T_Sanwa_equivalente`, se usata per diagnosi, è valida soltanto nell'intervallo **7.12–64.62 kΩ** e nelle condizioni della prova di origine. **Nessuna estrapolazione** per la protezione del dryer chiuso.

A freddo precedentemente: **122.3 kΩ** al multimetro con NTC scollegato, **~124.4 kΩ** col partitore, presso temperatura ambientale prossima a 20 °C. Questi dati non provano che NTC interna, Sanwa locale e SHT45 aria fossero perfettamente in equilibrio. L'ipotesi `100k/B3950` resta aperta, **non approvata**.

## 4. Tre livelli indipendenti di logica di protezione

| Livello | Rilevazione / comportamento richiesto | Stato reale oggi |
|---|---|---|
| **Elettrico** | NTC open/short/not finite e ADC fuori finestra: heater immediatamente OFF, fault latched; verificare campionamento e stale. SHT45 invalid/stale deve impedire richiesta calore. | `NtcSensor` implementa open/short e la prima catena di fault; controller termico host gestisce stale, ma produzione heater DISABILITATA. |
| **Guardia termica per sola caratterizzazione** | Durante piccoli impulsi sorvegliati, usare **R_NTC grezza** come arresto cautelativo, combinato a limite aria SHT45, timeout/energia, fan comandata e **Sanwa indipendente**. Le soglie sono iniziali di **test**, NON limiti di normale essiccazione. | Il precedente `HeaterPulseCommissioning.h` contiene una baseline; attualmente `kHeaterCommissioningMode=false`, quindi **non attiva**. |
| **V1 autonoma** | Richiesta heater regolata da **T aria SHT45** e protetta da limite piastra NTC **qualificato nella geometria finale**, guasti sensori, timeout e politica di arresto/cooldown che non crei rischi. | Non qualificata. `SafetyController` usa oggi per la futura abilitazione `ntc.temperatureC` calcolata da una curva storica non validata e richiede il flag `kHeaterNtcModelApproved`. **Non impostare tale flag true senza evidenza né usare la vecchia conversione in sicurezza**. In implementazione successiva va studiato un **guard in Ω** con campo provato oppure qualificata la conversione locale, e testato il fail-safe. |

### Criticità riscontrate nel codice da risolvere prima del calore continuativo

1. **Modalità off != termostato che scalda:** `AppConfig.h` correntemente ha entrambe le modalità di riscaldamento `false`. La policy pura viene testata con dati inventati, non comanda GPIO19 nella build `COLD-251025-S3`.
2. **`SafetyController::update` per il termostato attuante richiede `ntcThermallyCalibrated` e controlla `ntc.temperatureC` da un modello non convalidato**. Serve progettare la logica di guardia in resistenza oppure dimostrare un modello termico locale, senza promuovere un flag per aggirare il gate.
3. **Cooldown in caso di fault:** `coolingRequired()` ritorna vero soltanto in modalità commissioning, e `enterState(Fault)` + `fanPermitted()` arrestano normalmente la ventola se il commissioning non è attivo. Con riscaldamento continuo futuro questo comportamento non è automaticamente sicuro: occorre preservare un **cooldown definito anche dopo guasti e STOP**, compatibilmente con tipo di guasto e reale disponibilità della ventola. Servirà una soluzione esplicita e simulazioni (STEP05/07), non una semplice abilitazione del flag heater.
4. **Ventola:** `fan.phase()==Running` prova il comando software, non il movimento reale né flusso d'aria. Trattare perdita comando come fault e richiedere monitoraggio fisico durante la qualifica; valutare mitigazione della perdita di flusso in uso autonomo anche senza tachimetro.
5. **Stale e falsa plausibilità:** l'ADC electrical-valid può restare nel suo ampio intervallo durante sovratemperatura o drift; valutare valori bloccati, incoerenza con duty/andamento termico, latenze di campionamento e sovraelongazione **dopo lo spegnimento**. Non assegnare soglie rate-of-change arbitrarie senza misure.
6. **TCO 105°C presente:** lasciare cablaggio invariato. Non considerare le temperature OEM 105/116/118°C della piastra automaticamente compatibili col componente esistente. Foto/marcatura non prova Tf/Th/Tm né idoneità ad interrompere 24V DC, ~1.59A; la decisione D029 resta vigente.

## 5. Soglie e budget numerici — baseline di test, NON protezioni definitive

I numeri **GIÀ presenti nel vecchio firmware impulsivo** sono una baseline tracciabile per la review STEP05, **non soglie da abilitare** automaticamente:

| Controllo storico `HeaterPulseCommissioning::Parameters` | Valore | Significato corretto |
|---|---:|---|
| `minimumNtcOhms` | **42.270 Ω** | Arresto impulsivo in Ω; coincidente con 39°C **Sanwa** nel test aperto fan OFF; **non** 39°C interna del bead |
| `maximumNtcOhms` | 250.000 Ω | Controllo di plausibilità nel commissioning, non identificazione NTC |
| `maximumChamberC` | 35 °C | Arresto su **aria SHT45**, NON piastra |
| `maxNtcAgeMs / maxAirAgeMs` | 1.500 / 5.000 ms | Stale nel vecchio commissioning; diversi dai 0 ancora non configurati del controllo V1 |
| `pulseMs / gapMs / pulseCount` | 1.500 / 12.000 ms / 3 | 3 impulsi già collaudati, **non** termostato continuativo |
| `prerun / cooldown / totalTimeout` | 5.000 / 45.000 / 120.000 ms | Limiti della sola sequenza impulsiva |
| hardware watchdog di impulso ESP32 | 2.200 ms | Taglio hardware configurato SOLO se commissioning abilitato; non è un watchdog generale del termostato |

Dal dato fisico registrato in prove precedenti **24 V × 1,59 A ≈ 38,16 W** heater, si ricava come stima ideale di budget energetico **~38 J per secondo ON**, **~57 J per impulso da 1,5s**, **~172 J per 3 impulsi**, presupponendo V/I relativamente costanti. È una stima di energia elettrica erogata, non della temperatura risultante, né un massimo indipendente dalla tensione reale. Qualunque **nuovo** profilo con maggiore energia/calore richiede protocollo e nuova approvazione nello STEP05; il completamento dello STEP04 **non** autorizza ad eseguire o ripetere alcun impulso.

**Criteri candidati per il primo incremento ragionato di prova:** preservare inizialmente l'arresto in R a **42,27 kΩ**, l'arresto dell'aria a **35°C** e il watchdog per impulso a **2,2 s** soltanto come parametri da sottoporre a review; includere il Sanwa come veto manuale, tendenza temporale e temperatura massima *misurata* sul punto piastra. Non fissare qui un valore Sanwa o un tempo/energia incrementato come se fosse già sicuro: scelta e intervalli appartengono al gate di approvazione STEP05, dopo la revisione di cablaggi/TCO e della misura. Questi limiti potrebbero arrestare il test molto prima di raggiungere una temperatura camera utile: ciò **non** giustifica allentarli automaticamente.

## 6. Protocollo misure STEP05→06: una sola nuova prova mirata

### Preflight (da definire e verificare nello STEP05, prima di qualsiasi nuovo heater-on)

- Documento firmware effettivo con SHA/ID e config pulita, review di watchdog/stop/cooldown/fault; flash separatamente approvato. La build `COLD-251025-S3` **non** ha heater e non va convertita in termostato continuo da un flag.
- Unità realmente **chiusa**; punto Sanwa fissato/ripetibile vicino NTC, termocoppia e cavi alimentazione nel foro jack DC protetti da spigoli, nessuno schiacciamento; ventola libera; 24 V disinseribile immediatamente; alimentatore e limiti materiali adeguati.
- SHT45 nell'aria camera, NTC J7 plausibile a freddo, Sanwa stabile, 105°C TCO realmente in serie preservato; attenzione al rating ancora non identificato. Senza conferma di contatto Sanwa e stop immediato **non** effettuare prova calda.
- La sequenza successiva deve limitare durata di **ogni impulso**, energia/cumulative ON time, intervallo tra impulsi, finestra complessiva e cooldown, anche se il sensore si blocca; **verificare la ventola sul dispositivo**. Arresto automatico su errori elettrici/stale e abort esplicito su osservazione termica imprevista.
- Scegliere preventivamente **un solo** profilo progressivo che aggiunga informazione ai tre impulsi già superati, con criteri numerici *derivati* e revisione, anziché ripetere meccanicamente il test del 24 ottobre. Non spingersi automaticamente ai riferimenti OEM 105–118°C.

### Traccia da acquisire (piastra e camera, NON intercambiabili)

`t_ms | heater_actual_ON | ciclo/fase | fan_percent | R_NTC_ohm | NTC_adc_corr_mV | T_SHT45_air_C | RH_SHT45 | T_Sanwa_plate_C | Vin_V | Iin_A | eventi/abort`.

- Serial/CSV corrente acquisisce molti dei campi MCU in test commissioning. **Sanwa e V/I sono misure esterne**: annotare il timestamp di sincronizzazione e i valori nella medesima timeline; **non fingere che il CSV le legga automaticamente**. Dati SHT45 2s, NTC 0.5s: campioni ripetuti nei log più veloci **non** sono nuove letture fisiche.
- Non basta il valore di picco durante ON. Osservare la **massima T Sanwa e R minima anche nel cooldown**, quando il punto misurato può continuare a scaldarsi mentre l'aria risponde con ritardo.
- Calcolare qualitativamente e poi numericamente `R_NTC(t)` / `T_Sanwa(t)` / `T_air(t)`, trend e ritardo, risposta della ventola, overshoot post-OFF, stabilità della sonda. Annotare eventuali impedenze/gradiente e l'evoluzione della lettura dopo STOP.

### Gate di accettazione (prossimo lavoro tecnico, non prova già superata)

1. File/firmware esatto con protezioni indipendenti, simulazione e compile PASS; valore massimi ON/energia motivato, stop manuale/automatico e cooldown approvati.
2. Accettazione installazione sonda/cavi/coperchi e osservabilità con alimentazione pronta a essere tolta.
3. Solo dopo autorizzazione separata: singola sessione fisica sorvegliata con log completi e nessun fault/danno/contatto perso.
4. Dati nel box chiuso + fan ON permettono scegliere **un intervallo in Ω** per test successivi, con margine motivato verso hot spots e afterheat. Se c'è discordanza NTC/Sanwa si ferma la progressione e si studia la causa; non si forza un curve-fit.
5. Solo in fasi successive, sulla base delle evidenze chiuse, autorizzare soglie e timer per V1 autonoma, politica di raffreddamento su fault, eventuale decisione esplicita TCO. Preservare `docs/DECISIONS.md` fino all'approvazione di un cambiamento duraturo.

## 7. Stato e attività successiva ESATTA

**STEP04 PASS come lavoro di analisi/definizione della strategia**, non come qualifica dell'NTC, del TCO o dell'uso riscaldato.

**STEP05 NEXT:** code/safety review per un profilo termico **supervisionato e temporizzato**, soprattutto risolvendo o isolando il rischio cooldown/fan OFF su fault. Scrivere i limiti numerici del prossimo protocollo *prima* di ogni eventuale flash/test caldo e richiedere il consenso specifico dell'utente. Gli step software possono essere elaborati senza interventi fisici; **nessun nuovo riscaldamento è consentito da questo documento**. Lo storico 12h resta differito.
