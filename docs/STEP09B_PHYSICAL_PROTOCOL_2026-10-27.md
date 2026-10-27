# STEP09B — Seconda prova proposta HEATER ON, controllo aria SHT45 a25°C

**STATO: PROTOCOLLO PREPARATO, NON ESEGUITO, NON ANCORA AUTORIZZATO DALL'UTENTE.** Questa procedura sarà utilizzabile **solo dopo** CI S9B PASS e nuovo consenso esplicito al **budget ON cumulativo200s**, che è maggiore del S9A approvato a120s. Non flashare né avviare il riscaldamento sulla sola base di questo documento.

## Introduzione per l'operatore (COSA / PERCHÉ / TEST PRECEDENTE / DOPO)

**COSA facciamo:** facciamo accendere realmente il riscaldatore e lasciamo che l'SHT45 gestisca la temperatura dell'**aria**. Quando l'aria arriva a **25,0°C** deve cessare la richiesta di HEATER ON; potrà riaccendere solo se l'aria scende a **24,0°C** e se l'NTC lo consente. La piastra sarà più calda dell'aria, e l'NTC sulla piastra impone pause e guasti separati. Rimangono massimo **20s consecutivi ON**, fan100% comandata, cooldown e Sanwa sotto osservazione.

**PERCHÉ:** nella S9A ci siamo fermati **perché finito il budget di120s HEATER ON**, non perché l'aria sia arrivata25°C. Proviamo un budget maggiore, senza toccare soglie di protezione, ventola o setpoint.

**TEST PRECEDENTE S9A:** Sanwa iniziale17°C, ON/OFF cicli6–12 **35/38°C**, 13° ON35°C poi cooling, Sanwa26°C a fine; CSV originale: aria17,64°C→22,59°C a cooling→massimo22,97°C, NTC minimo48,7167kΩ, zero FAULT. Tredici finestre e ~120s HEAT ON comandati.

**DOPO:** se l'aria raggiunge25°C possiamo confrontare il suo passaggio con lo stato HEATER ON/OFF e successiva inerzia termica. Se non raggiunge25°C, registriamo il limite senza forzare il test. Prima di passare a40°C aria serviranno ulteriori qualifiche/protocollo specifico.

## Differenza UNICA del nuovo profilo

| Parametro | S9A eseguito | S9B candidato |
| --- | ---: | ---: |
| HEATER ON **cumulativo** max | **120s** | **200s** (**+80s**) |
| HEATER ON per finestra | max20s (timer ISR20,5s) | invariato |
| OFF minimo | 12s | invariato |
| NTC SOFT PAUSE | R<=50kΩ | invariato |
| NTC nuova accensione | R>=65kΩ | invariato |
| NTC HARD FAULT | R<=40kΩ / R>=250kΩ | invariato |
| SHT45 setpoint e isteresi | OFF>=25°C, ON<=24°C | invariato |
| FAULT aria e sensor age/invalid | aria>=40°C + invalid/stale | invariato |
| Ventola / prerun / cooldown | PWM100%, prerun5s, cooling90s | invariato |
| Durata assoluta sessione | max600s | invariato |
| **STOP umano Sanwa** | **>=50°C** | **invariato** |

**200s è un tetto, non una durata obbligatoria o continua**: le pause possono rendere il tempo effettivamente HEATER ON più corto e il timeout complessivo600s può fermare prima. È un aumento del 66,7% rispetto a S9A, non qualificato dal punto di vista della massima temperatura di ogni componente. Nessun aumento della potenza heater o conversione NTC→°C. Vedere [analisi su dati reali S9A e review rischi](evidence/thermal/2026-10-27-S9B-budget-engineering-analysis.md).

## Identità software e sicurezza

- Repository `alessandrocardelli/filament-dryer-monitor`. **Branch S9B dedicato:** `experiment/step09b-chamber25-extended-budget-2026-10-27` (non `main` né S9A). **PR DRAFT** rispetto al branch S9A, non mergiare in firmware V1.
- Build **`HOT-261027-S9B`**, OLED **`H27-S9B`**. Il codice è HEAT-CAPABLE e può energizzare **GPIO19** quando flashato. **CI S9B e commit esatto da verificare dopo le ultime modifiche**.
- Uscita su `SafetyController.h` / `HeaterStep09AirFeedback.h`, guardia NTC RAW in ohm, NTC bead non calibrato in°C nel closed box. Fan100 è comando PWM non misurazione tach. MOSFET stuck ON non misurato indipendentemente.
- Ultimo CSV su LittleFS: [endpoint `/api/last-test.csv`](http://filament-dryer.local/api/last-test.csv), salvato dopo heater OFF e cooling. Buffer massimo1400 righe e campionamento nominale500ms, sufficiente al tetto600s+90s se nessun altro rallentamento anomalo.
- Fusibile termico monouso **105°C attualmente montato in serie**: NON scollegarlo, bypassarlo o sostituirlo per questo test. La sua qualifica DC e condizioni corpo/placement restano aperte. Non è un consenso implicito ad aumentare oltre50°C sul Sanwa.

## Prerequisiti obbligatori prima di flash e HEAT

1. **CI su commit esatto S9B PASS per build ESP32 e host mock**: verificare run completo e log del linker (nessun falso verde `tee`), test fault, watchdog/cooldown, target/isteresi e cap200s. Non utilizzare vecchio CI PASS S9A come prova di build S9B.
2. **Controllo fisico effettuato dall'operatore**: montaggio invariato rispetto a S9A, Sanwa saldamente fissato sulla stessa zona di contatto presso l'NTC heater, sensore SHT45 nella camera, ventola libera e realmente ruotante durante prova, cablaggi integri, box chiuso, TCO105 in serie invariato, alimentazione24V facilmente isolabile. Se differenze/anomalie, non iniziare.
3. **Consenso esplicito distinto** dell'utente al nuovo incremento HEATER cumulativo **120→200s** mantenendo il veto manuale **Sanwa50°C**, le soglie NTC e il massimo10min. L'autorizzazione alla sola preparazione software NON è approvazione alla prova calda S9B.
4. Dryer sufficientemente raffreddato, Sanwa e SHT45 iniziali annotati. **SHT45 aria<=24°C** per aspettarsi HEATER ON da standby; se più calda, non forzare né alterare la sonda: registrare e fermarsi.
5. Flash/OTA soltanto da **STANDBY** usando il branch/commit di S9B approvato; dopo reboot leggere **`H27-S9B`** su OLED. Se l'identificatore non corrisponde, **NON avviare**.

## Procedura cronologica e letture Sanwa ESATTE

**Non scrivere durante un evento critico sacrificando la sorveglianza.** Se possibile registra video OLED+Sanwa nello stesso quadro e trascrivi dopo. Il Sanwa NON è registrato dal CSV; devi **osservarlo continuamente mentre HEATER può essere ON**.

| Quando | Che cosa annotare (se l'evento avviene) |
| --- | --- |
| **PRIMA di premere ON/OFF** | Sanwa °C e aria SHT45 °C iniziali |
| **Primo `HEATER ON`** | Sanwa °C in quell'istante |
| **Primo `HEATER OFF`** | Sanwa °C e aria SHT45 °C |
| **OGNI nuovo `HEATER ON` e `HEATER OFF`** | Sanwa °C per ciascuna finestra, oppure cattura video |
| **Se l'aria SHT45 arriva a25°C** | Ora/evento, SHT45 °C, Sanwa °C e se HEATER è ON o OFF |
| **Se appare `COOLING`, STOP, FAULT o esaurimento budget** | Sanwa °C e aria SHT45 °C all'arresto definitivo (se non già annotate) |
| **Alla fermata effettiva della ventola dopo cooldown** | Sanwa °C e aria SHT45 °C |

Se il primo OFF coincide con l'ultimo, basta una lettura. Se il target25°C arriva mentre HEATER **è già OFF per NTC**, questo **non prova da solo** lo spegnimento termostatico dal comando ON: annota precisamente lo stato visualizzato, poi valuteremo dal CSV il veto SHT45 sulla successiva domanda. Non provocare manualmente oscillazioni per verificarlo. Se SHT45 raggiunge25°C e non c'è altro motivo per proseguire, **puoi fare STOP lungo e lasciare raffreddare**, mantenendo osservazione; non lasciare acceso inutilmente per tentare un secondo evento.

**STOP IMMEDIATO se Sanwa raggiunge 50°C**, se la ventola si ferma, compaiono odori/cavi caldi o altri comportamenti anomali: ON/OFF lungo (circa1,5s) durante DRYING; in emergenza togli subito24V. **Non aspettare il successivo `HEATER OFF` per leggere Sanwa**. Se la prova termina senza25°C, non modificare firmware/limiti al volo e non fare automaticamente un secondo ciclo.

## Chiusura/archiviazione

- Dopo cooling osservato e HEATER OFF definitivo, esportare **nuovo CSV S9B**, non il precedente S9A, da `/api/last-test.csv`. Non riaccendere per recuperare un download fallito. Annotare eventuali errori.
- Allegare CSV **originale** e tutte le temperature Sanwa cronologiche (anche se solo alcune misure disponibili), eventuali messaggi OLED, ora effettiva e foto/video facoltativi. Archiviare su GitHub con hash, conteggio righe, rapporto e confronto S9A.
- Conclusioni possibili: a) target25°C raggiunto con OFF termostatico osservato; b)25°C raggiunto durante pausa NTC, si verifica la persistenza OFF; c) budget200s o timeout600s scaduti prima target; d) FAULT/STOP per sicurezza. Nessun successo viene dichiarato prima di dati reali.

**Stato:** SOFTWARE IN PREPARAZIONE; **NESSUNA PROVA FISICA S9B ESEGUITA**; **CONSENSO S9B NON ANCORA ACQUISITO**. Gli esiti S9A non sono sostituiti né riutilizzati come nuovo CSV.
