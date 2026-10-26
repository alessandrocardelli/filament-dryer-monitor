# Registro sperimentale — STEP07-S7U nuova sessione termica

**STATO: DA ESEGUIRE — nessuna lettura di questa NUOVA prova è stata ancora acquisita.**
Usare ESATTAMENTE [il protocollo pre-registrato](../../STEP07_S7U_PHYSICAL_RETEST_PROTOCOL_2026-10-26.md); NON copiare i dati S7G 17/36/43°C come se fossero di S7U. È la prova richiesta esplicitamente dall'utente per recuperare un insieme di dati completo; nessun test a heater spento e nessun recupero del vecchio CSV.

## A. Identità e setup — compilare prima di ON/OFF

| Campo | Valore effettivamente verificato |
| --- | --- |
| Data, ora (Europe/Rome), operatore | NON ANCORA ESEGUITO |
| Riferimento sorgente branch / commit | `fix/step07-clear-test-feedback-2026-10-26` / commit S7U da annotare all'upload |
| Firmware visualizzato OLED / `/api/status` | NON ANCORA VERIFICATO; previsto `HOT-261026-S7U` (`H26-S7U`) |
| Alimentazione e collegamenti | 24 V; confermare condizioni osservate |
| Box/coperchi, oggetti interni | NON ANCORA VERIFICATO |
| Sanwa RD701 / termocoppia K: posizione, fissaggio, punto di contatto e foto | NON ANCORA VERIFICATO |
| NTC integrato (J7), SHT45 aria, TCO105 in serie | Setup storico documentato; riconferma visiva limitata alla prova |
| Ventola: flusso reale, cavi/fori/pale | NON ANCORA VERIFICATO |
| Video sincronizzato Sanwa + OLED o cronometro | NON DISPONIBILE / da aggiornare |
| Profilo software | NTC <=40kΩ, fan comandata100%, max6×20s, gap12s, prerun5s, cooldown90s |
| Veto umano della nuova sessione | Sanwa >=50°C, interrompere prima per anomalie; distinguere da 55°C delle precedenti S6A/S6C |
| Altre modifiche rispetto all'ultima prova fisica S7G | NON ANCORA DICHIARATE |

## B. Letture indipendenti Sanwa — durante e dopo OFF

| Evento effettivo | Ora/timestamp/video | Sanwa °C | OLED / numero impulso / nota |
| --- | --- | --- | --- |
| Zero prima dell'avvio | — | NON MISURATO | STANDBY |
| Comparsa HEATER ON 1/6 | — | NON MISURATO | — |
| Prima di HEATER ON 3/6, **solo se raggiunto** | — | NON MISURATO | oppure NON RAGGIUNTO |
| Primo stop/HEATER OFF **definitivo** (FAULT/STOP/fine impulsi) | — | NON MISURATO | testo esatto da trascrivere |
| **Andamento Sanwa dopo OFF definitivo** | — | NON OSSERVATO | indica semplicemente se scende / resta stabile; annotare una risalita SOLO se effettivamente osservata. Non è obbligatorio misurare un secondo picco |
| Ventola ferma a fine cooldown | — | NON MISURATO | display finale |
| Altre letture numeriche con evento/tempo | — | — | — |

**NOTA:** essendo la termocoppia appoggiata direttamente alla zona riscaldante, dopo l'OFF ci si aspetta normalmente una stabilizzazione o discesa. Non presumere una salita post-OFF, non chiedere una seconda temperatura massima come condizione per completare il test. Registrare una risalita solo se si osserva. Il Sanwa non viene registrato dalla MCU.

## C. Esito reale del firmware / sicurezza

- Avvio ON/OFF breve da STANDBY: **NON ESEGUITO**.
- Sequenza ventola prerun, presenza flusso reale: **NON OSSERVATO**.
- HEATER ON / OFF effettivamente mostrati (elencare impulsi): **NON OSSERVATO**.
- Evento finale: FAULT / COMPLETE / STOP / emergenza 24 V: **NON OSSERVATO**.
- Testo **esatto** di eventuale motivo FAULT sull'OLED e codici `/api/events`: **NON OSSERVATO**.
- Allarme acustico singolo e MODE breve mute, se pertinenti: **NON OSSERVATO**.
- Raffreddamento, countdown, arresto ventola: **NON OSSERVATO**.
- CSV READY / CSV SAVE FAILED / TEST COMPLETE: **NON OSSERVATO**.
- Intervento manuale (soglia, osservazione, momento, azione): **NESSUNO REGISTRATO**.
- Temperatura/RH aria iniziale se osservata: **NON MISURATA**.
- Qualsiasi anomalia hardware/probe/cavi/fan/odore: **NON OSSERVATA/DA COMPILARE**.
- Eventuale assenza CSV e motivo: **DA COMPILARE**.

## D. File grezzi e provenienza (aggiungere realmente, NON anticipare)

| Tipo | Percorso repository / riferimento | Integrità / completezza |
| --- | --- | --- |
| CSV **raw** scaricato da `/api/last-test.csv` dopo cooling | NON ANCORA DISPONIBILE. Nome previsto nella stessa cartella: `2026-10-26-S7U-retest-raw.csv` | SHA-256 bytes: — ; righe: — |
| `/api/events` o trascrizione messaggi OLED | NON ANCORA DISPONIBILE | — |
| Foto punto Sanwa e box | NON ANCORA DISPONIBILI | — |
| Video Sanwa/OLED con identificazione tempo, se registrato | NON ANCORA DISPONIBILE | — |

CSV atteso: `ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent`.
Non contiene Sanwa né motivo FAULT; annotarli dalla misura indipendente/UI/eventi. Non trascrivere manualmente righe inventate. Non dedurre corrente heater o RPM dal comando.

## E. Analisi DOPO ricezione dati (non compilare prima)

- Identità e validità del file, campioni, primo/ultimo `ms`, frequenza effettiva: **OPEN**.
- Finestre `heater_command=1`, relative durate, gap, impulso di stop: **OPEN**.
- R_NTC inizio/min/fine, eventuale crossing40kΩ e sincronizzazione col comando OFF: **OPEN**.
- SHT45 aria inizio/alla fine HEAT/max durante cooldown/fine, RH corrispondente: **OPEN**.
- Trigger FAULT: dichiarare **solo se verificato** da messaggio/codice; CSV può non bastare: **OPEN**.
- Confronto Sanwa alla disattivazione/andamento dopo OFF e possibile ritardo rispetto a NTC: **OPEN**.
- Limitazioni strumentali (contatto, dinamica, non sincronizzazione): **OPEN**.
- Analisi di sicurezza (TCO105, fan command != airflow, materiali, comportamento FAULT, RTC): **OPEN**.
- Criterio G1 acquisizione: OPEN / PASS limitato / INCOMPLETO, motivazione: **OPEN**.
- Decisione e **un solo prossimo passo** verso G2/G3/G4: **DA DECIDERE SUI DATI**, senza automatizzare nuova prova.

## F. Chiusura e persistenza — obbligo di aggiornamento

La sessione si considera registrata **solo dopo commit GitHub** di questo registro completo (inclusi espliciti NON MISURATO), CSV raw se disponibile, eventuali eventi/foto referenziati, valutazione limitata, aggiornamento di `docs/PROJECT_STATE.md`, `docs/TODO.md`, `README.md` e handoff. Riportare gli SHA commit e link in questa sezione.

**Commit finale prova:** NON ANCORA ESISTENTE.
