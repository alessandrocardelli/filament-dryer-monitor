# STEP07-S7U — Analisi indipendente del CSV della prova fisica (26 ottobre 2026)

**Stato:** analisi effettuata sui 717 campioni reali forniti dall'utente, in collegamento con [il verbale](2026-10-26-S7U-RETEST-SESSION.md) e con il [CSV originale](2026-10-26-S7U-retest-raw.csv). **NON** è una qualificazione della V1, non autorizza altre prove a caldo né modifica le soglie. Sorgente codice S7U: PR #12 / ramo `fix/step07-clear-test-feedback-2026-10-26`. La firma firmware **non è presente nel CSV**, l'identificazione S7U deriva dal resoconto utente `PROVA S7U`. Vedere il verbale per osservazioni Sanwa, che non sono contenute nel file.

## 1. Integrità, provenienza e interpretazione

- **Originale utente:** allegato `filament-last-test (2).csv`; copiato *byte-per-byte* nella repo come `docs/evidence/thermal/2026-10-26-S7U-retest-raw.csv`.
- **SHA-256 originale = SHA-256 copia:** `c417313414eede75f2269dbfb8717ac6d8246625172e315a007b8056def834ae`.
- **Git blob SHA-1 verificato:** `891ea18eb00efdb6690e7b7e3c0c6466e6a39674` (calcolato dai 29142 byte originali e confrontato con il blob GitHub dopo il commit). Il file ha 718 righe di testo, di cui una intestazione e **717 campioni**, 29142 byte.
- Header esatto: `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent`.
- Indici `phase` dalla vera enumerazione `HeaterPulseCommissioning.h`: 0 Idle, **1 Prerun**, **2 Pulse**, **3 Gap**, 4 Cooldown, 5 Complete, **6 Aborted**. La presenza di phase=6 significa **test abortito**, non fase di raffreddamento normale completata.
- `ms` è **millisecondi di uptime ESP32**. Non è ora civile né tempo dall'inizio preciso del ciclo; usare differenze fra timestamp. Nessun video/event-log con orario civile è stato allegato.
- `heater_command=1` indica **comando software GPIO19 attivo**, NON misura indipendente della corrente. `fan_percent=100` indica **PWM richiesto al 100%**, NON RPM o portata misurata. Non inferire comportamento reale da soli campi di comando.
- SHT45 `air_c` misura **aria camera**; `ntc_ohms` è resistenza del sensore incorporato nel riscaldatore; Sanwa RD701/K misura **contatto locale vicino alla piastra/NTC**. **Nessuna** delle tre grandezze è intercambiabile con un'altra.
- Controllo dati: `ms` strettamente crescente, 0 timestamp duplicati, 0 celle mancanti, `air_valid=1` e `ntc_valid=1` in **717/717 righe**; campionamento tra righe mediana **250 ms**, minimo 250 ms, massimo 271 ms (solo il primo intervallo; non interpretarlo come frequenza esatta).

## 2. Profilo EFFETTIVAMENTE osservato (non quello massimo teorico)

Profilo firmware S7U programmato: ventola comandata100%, 5s prerun dopo startup, **massimo 6×20s ON**, gap12s, abort `R_NTC <= 40000Ω` / `SHT45 aria >= 40°C`, cooldown circa90s da ultimo heater ON. I 6 impulsi NON sono obbligatori: un guard trip deve interrompere.

| Evento CSV | Uptime `ms` | Tempo relativo al primo campione | Phase/pulses/heater | R_NTC | SHT45 aria | RH |
| --- | ---: | ---: | --- | ---: | ---: | ---: |
| Prima riga / prerun fan | 6785 | 0,000 s | 1 / 0 / 0 | 136951,7 Ω | 18,20 °C | 64,12% |
| HEATER ON 1 | 12806 | +6,021 s | 2 / 0 / 1 | 135683,9 Ω | 18,21 °C | 64,16% |
| Fine ON1 / GAP | 32806 | +26,021 s | 3 / 1 / 0 | 62287,4 Ω | 18,34 °C | 64,68% |
| HEATER ON 2 | 44806 | +38,021 s | 2 / 1 / 1 | 78696,9 Ω | 18,47 °C | 64,78% |
| Fine ON2 / GAP | 64806 | +58,021 s | 3 / 2 / 0 | 46401,5 Ω | 18,79 °C | 64,58% |
| HEATER ON 3 | 76806 | +70,021 s | 2 / 2 / 1 | 61271,0 Ω | 19,07 °C | 64,30% |
| **FAULT / OFF definitivo** | **96056** | **+89,271 s** | **6 / 2 / 0** | **39789,3 Ω** | **19,52 °C** | **63,39%** |
| Minimo NTC rilevato | 96556 | +89,771 s | 6 / 2 / 0 | 39593,4 Ω | 19,52 °C | 63,39% |
| Ultimo campione | 185806 | +179,021 s | 6 / 2 / 0 | 97954,7 Ω | 20,89 °C | 58,65% |

**Durata dei comandi HEATER ON** fra cambio 0→1 e primo 1→0:
- ON1: 12806→32806 = **20,000 s** (80 campioni a1).
- ON2: 44806→64806 = **20,000 s** (80 campioni a1).
- ON3: 76806→96056 = **19,250 s** (77 campioni a1, **interrotto in anticipo**).
- Gap1 e gap2: **12,000 s** ciascuno.
- Totale finestre ON: **59,250 s nominali secondo timestamp CSV**, non 120 s. **Nessun quarto/sesto impulso**.
- Uptime ultima riga − OFF definitivo = **89,750 s**, compatibile con finestra di raffreddamento ~90 s. **Ultima riga ha ancora `fan_percent=100` e phase=6**: il CSV non registra la transizione di spegnimento ventola. La **fermata ventola** è riferita separatamente dall'utente, con Sanwa 25 °C.
- Intervallo prima riga→ultima = **179,021 s** (2m59,021s); il ciclo reale può aver avuto un preambolo non presente nel CSV, non equiparare automaticamente la durata del file all'intera durata civile.

## 3. Risposta NTC ed evento FAULT

Alla **ultima riga HEATER=1** (ms95806): `R_NTC = 40159,9 Ω` (**sopra**40kΩ).
Alla **prima riga HEATER=0 / phase=Aborted** (ms96056): `R_NTC = 39789,3 Ω` (**sotto**40kΩ). Il trip firmware avviene dunque nell'intervallo di campionamento di circa **250 ms** fra questi due record, senza misura elettrica indipendente di GPIO/corrente. Il minimo registrato `39593,4Ω` arriva circa **0,5s dopo** il primo record OFF; la latenza di lettura e il campionamento NTC non consentono di farne una precisa inferenza termica.

**Evidenza congiunta:** l'utente ha trascritto il messaggio OLED **`NTC R LIMIT`** e il CSV mostra nello stesso punto l'attraversamento 40kΩ e phase=6. Questo fornisce **conferma coerente del meccanismo automatico di arresto per resistenza NTC** nel collaudo. Non dimostra elettricamente che la corrente reale heater fosse zero, il guasto di transistor bloccato ON, né che il cutoff abbia un margine di sicurezza termica qualificato.

`pulses=2` nella riga di arresto NON significa che non sia iniziato il terzo impulso: è il **numero di impulsi completati**, e la sequenza fase2 è il terzo impulso in corso al momento dell'abort. In fase Aborted il contatore resta2. Confrontare il log e `HeaterPulseCommissioning.h` prima di interpretare questo campo.

## 4. Aria, umidità e raffreddamento

- **SHT45 aria iniziale→OFF definitivo→ultima riga:** **18,20→19,52→20,89°C**.
- Variazione aria dall'inizio all'OFF: **+1,32°C**. Variazione **durante ~90s successivi all'OFF**: **+1,37°C**. Incremento totale osservato: **+2,69°C**.
- Massimo aria nel CSV **20,89°C**, raggiunto a `ms184306` e rimasto fino alla fine. Non c'è alcun avvicinamento reale al cutoff aria40°C in questa sessione. La risposta ritardata è quella **dell'aria camera**, non una dimostrazione di ulteriore incremento della superficie riscaldante.
- RH iniziale→OFF definitivo→fine **64,12→63,39→58,65%** (−5,47 punti percentuali complessivi). Interpretare RH con l'effetto della temperatura, NON automaticamente come massa di acqua espulsa o velocità di asciugatura.
- NTC iniziale→OFF→min→ultimo campione: **136951,7→39789,3→39593,4→97954,7Ω**. Andamento coerente con riscaldamento e raffreddamento, ma non convertire questi Ohm in temperatura del bead/piastra senza qualificazione termica.
- `fan_percent=100` **in tutte le 717 righe**, incluso il log abortito; questo indica che il firmware **ha richiesto** ventilazione residua. L'utente ha osservato ventola effettivamente ferma a Sanwa 25°C, ma non ha fornito RPM, portata o timestamp della fermata. È coerente con raffreddamento ma NON è una misura indipendente del flusso.

## 5. Letture SANWA originali dall'utente (NON nel CSV)

Dichiarazione user nel messaggio intitolato esattamente `PROVA S7U`:
- Sanwa iniziale **17°C**.
- Sanwa **al primo HEATER ON 17°C**.
- Sanwa **prima del terzo impulso 36°C**.
- Sanwa **allo stop definitivo 43°C**.
- OLED **`NTC R LIMIT`**.
- Durante cooling **«Temperatura scesa regolarmente»**; alla **fermata ventola Sanwa 25°C**.

Non sono stati forniti orari assoluti per le letture Sanwa, foto/video di contatto, cronologia completa Sanwa né picco continuo. Le letture «al primo ON», «prima impulso3», «all'arresto» sono **eventi approssimativamente associabili** ai tre timestamp CSV, non sincronizzate al millisecondo. Non assumere che Sanwa43°C = temperatura bead NTC né che l'aria20°C contraddica la lettura locale43°C.

## 6. Conclusioni ammesse e limiti

**Osservato/ben supportato:**
1. Nuova sessione physical `S7U` riferita dall'utente e CSV valido da **717 righe**; primo/secondo comando ON completi e terzo interrotto a **19,25s**.
2. **R_NTC attraversa 40kΩ** contemporaneamente all'interruzione software; `phase=6` e messaggio `NTC R LIMIT` riferito confermano il motivo del FAULT con evidenza forte.
3. Durante il cooling l'utente ha osservato **Sanwa in discesa, fino a25°C quando la ventola si è fermata**. **Non si è osservato né va ipotizzato un picco post-OFF Sanwa**.
4. Temperatura aria SHT45 è aumentata *dopo* heater OFF (da19,52 a20,89°C), dimostrando una risposta termica della camera ritardata; non attribuirla al Sanwa.
5. Traccia di fan PWM100% durante tutti i record; conferma del comando, non dei giri. Salvataggio CSV avvenuto perché file valido è stato fornito dall'utente; non è stato osservato il testo OLED `CSV READY` e la fine cooling non è rappresentata da phase5.

**Non dimostrato (OPEN):** temperatura effettiva massima del bead/hotspot/piastra, precisione assoluta e lag Sanwa, prova indipendente MOSFET OFF su guasto, tach/airflow, sicurezza materiali/TCO105 in chiuso, verifica di allarmi/STOP aggiuntivi, termostato autonomo a40°C, eventuale profilo ventilazione ottimale e prestazioni di essiccazione.

**Valutazione gate G1:** **PASS limitato per acquisizione e tracciamento della guardia NTC**, con limiti dichiarati. **NON** è PASS di qualifica termica/V1.
**Prossima UNICA attività raccomandata, SENZA NUOVA PROVA A CALDO:** revisione ingegneristica G2/G3 dei dati appena ottenuti, in particolare margine della guardia NTC 40kΩ rispetto alla temperatura Sanwa osservata e alla dinamica SHT45 in cooling, qualità della misura, limiti per futura regolazione aria e sistema di protezione. Successivamente progettazione controllo SHT45 40°C *prima in simulazione senza abilitazione fisica*. Non aumentare esposizione ON, non abbassare cutoff e non richiedere redo S7U soltanto per ottenere display/CSV.
