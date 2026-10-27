# STEP09-A — Prima prova fisica con HEATER ON e feedback SHT45

**STATO 27 OTTOBRE 2026: TEST FISICO NON ANCORA ESEGUITO; AUTORIZZAZIONE DELL'UTENTE ACQUISITA PER PRIMO TEST HEATER-ON CON TARGET ARIA 25°C E VETO MANUALE SANWA 50°C.** L'utente ha risposto «Confermo» alla domanda esplicita «Confermi di voler eseguire questo primo test a 25 °C, mantenendo il limite manuale Sanwa a 50 °C?». **Il setup fisico corrente (posizione e contatto Sanwa, fan realmente funzionante, cablaggio, TCO105 in serie e 24V scollegabili) NON è stato ancora confermato in questo messaggio:** deve essere verificato sul posto prima del flash/test. Firmware sperimentale HEAT-CAPABLE, non la V1. Ultima CI PR#14 con mock SafetyController e link ESP32 PASS: [run 40486080140](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40486080140), compilazione RAM statica 92096 byte, FLASH1126854 byte. Ultima build testata prima di queste modifiche ai soli documenti. L'autorizzazione a questo test non autorizza modifiche ai limiti/cablaggi né ulteriori cicli o test a temperature superiori.

## Introduzione obbligatoria: cosa, perché, precedente, futuro

**Cosa faremo:** azioneremo REALMENTE il riscaldatore sotto la regolazione della temperatura ARIA della camera mediante SHT45. Il nuovo software richiede HEATER ON sotto 24°C di aria e HEATER OFF a 25°C, con protezione separata NTC sulla zona heater, ventola 100% comandato e limiti temporali.

**Perché:** verificare finalmente il feedback fisico anziché un altro schema a impulsi predefiniti. La piastra deve essere più calda dell'aria, e non si cerca di renderle uguali. Il primo target modesto 25°C è vicino ma sopra il massimo SHT45 di 23,98°C osservato nello S6C, e consente un primo esito interpretabile prima di tentare 40°C dell'aria.

**Test precedenti:** S6C ha comandato 120s di HEAT/6 impulsi, raggiunto Sanwa massimo 53°C e camera23,98°C a fine CSV. S7U ha fermato il terzo impulso a R_NTC=39,789kΩ, con Sanwa43°C e aria19,52°C; Sanwa25°C quando ventola ferma. STEP03 e STEP08 hanno PASS software, non prova fisica di controllo.

**Test futuro:** se SHT45 mostra il raggiungimento di25°C e HEATER passa OFF quando raggiunto il target, abbiamo la prima evidenza di controllo aria. Se non raggiunge25°C entro i limiti, registriamo anche questo risultato. Solo successivamente si valutano termica/protezioni e ulteriori setpoint come30°C e40°C, senza aumento automatico del cutoff.

## 1. Identità e limiti specifici, SOLO PROPOSTI

Repository alessandrocardelli/filament-dryer-monitor; branch experiment/step09-real-sht45-feedback-2026-10-27; PR #14 DRAFT, non mergiata. Identificatore proposto HOT-261027-S9A, OLED H27-S9A (da verificare in build e sul dispositivo dopo eventuale flash).

- Camera: target **25,0°C**, ON permesso a **<=24,0°C**, OFF da **>=25,0°C**; su questa build è un target FISSO, non cambia dalla memoria SETUP.
- NTC in OHM, NON temperatura calibrata: pausa preventiva HEATER OFF se **R<=50.000Ω**; nuovo ON solo a **R>=65.000Ω**, OFF da almeno12s e aria<=24°C. Non è un FAULT; la ripresa può non avvenire se non si raffredda abbastanza.
- FAULT separato e latched, con uscita immediata OFF, su **R_NTC<=40.000Ω** (limite duro uguale allo S7U), R>=250.000Ω, sensori invalid/stale, SHT45 aria>=40°C, perdita del comando fan, watchdog. Non c'è tachimetro: fan=100% indica PWM comandato, non RPM.
- Ogni accensione dura max **20s**, con ISR ESP32 di cutoff hardware indipendente da loop a **20,5s**; OFF minimo12s; tempo cumulativo comando heater ON non oltre **120s** (come S6C). Durata massima del nuovo esperimento **10min**. Il profilo termina anche senza raggiungere25°C.
- Ventola **100% comandato**, spunto circa1s, poi 5s di prerun; cooldown minimo **90s** dopo ultimo HEATER ON; one-shot senza ripartenza automatica.
- Verifica umana: **Sanwa 50°C** è soglia di intervento manuale come nella prova S7U. È un veto sperimentale, non una temperatura di sicurezza di prodotto.
- Il fusibile TCO105 monouso installato in serie resta invariato e NON va bypassato; specifica/DC rating, temperatura corpo e comportamento delle parti restano non qualificati. Non eseguire se l'assemblaggio è cambiato o ha parti danneggiate.

Questi parametri sono una PROPOSTA da revisionare sulla PR, non decisione definitiva per la V1. Nessuna nuova curva NTC temperatura viene introdotta.

## 2. Gate obbligatori prima della prova (nessun vecchio collaudo a heater-OFF)

- [ ] Leggere il risultato REALE della build ESP32 GitHub Actions su commit esatto della PR #14; verificare **assenza di linker error e DRAM overflow**, non fidarsi del verde superficiale. Il runner deve usare pipefail. Superati anche i nuovi mock del SafetyController.
- [ ] Verificare firmware, UI, fault, fan, CSV, ISR, OTA e comportamento STOP su quel commit; nessuna modifica pending. Verificare che il CSV registri tutto: per Step09 registra a500ms, 1400 campioni = fino700s (600s run +90s cooling).
- [ ] Review fisica condivisa: stesso setup chiuso di S7U, SHT45 nell'aria bobina, Sanwa RD701 termocoppia K fissato alla piastra vicino NTC, ventola libera e visibilmente rotante, cavi integri e non schiacciati, TCO105 presente in serie, alimentatore24V rapidamente disconnettibile. Non inventare controlli: segnare quali eseguiti dall'utente.
- [x] **L'utente ha confermato il primo test HEATER-ON aria25°C con limite manuale Sanwa50°C** nel messaggio «Confermo» del 27/10/2026. Controllare prima del flash che il profilo specifico del firmware rispetti quanto presentato: NTC soft pausa50k/ripresa65k, guasto40k invariato, tempo cumulativo120s, 20s finestra, ventola100%, cooldown90s. Non interpretare la conferma come autorizzazione a cambiare questi limiti.
- [ ] **Preflight fisico indipendente non ancora confermato**: stesso montaggio e stato dei componenti dopo S7U; nessuna prova nuova e nessuna misura Sanwa acquisita nel messaggio di conferma.

## 3. Procedura fisica — da eseguire SOLO dopo quei gate

1. Lascia raffreddare completamente il dryer. Verifica box chiuso, Sanwa ben a contatto con la piastra nella stessa posizione, cavo protetto, fan libera, TCO in serie e 24V subito scollegabili.
2. Con il dryer in STANDBY e senza riscaldamento, carica via OTA **solo il codice del commit STEP09 approvato**. Dopo riavvio leggi **H27-S9A** sul display. Se manca, non avviare.
3. Segna **Sanwa iniziale**, temperatura e RH SHT45 iniziali; se possibile filma insieme OLED+Sanwa per non dover prendere appunti durante HEAT.
   **Se l'aria SHT45 è già sopra 24,0°C**, il controllo con isteresi della prova può legittimamente non comandare HEATER ON: **non iniziare questa sessione per forzare calore, non cambiare il setpoint e non intervenire sul sensore per raffreddarlo artificialmente**. Riporta la lettura iniziale; il risultato sarebbe non valutabile come prima prova HEATER-ON.
4. Premi una volta brevemente ON/OFF da STANDBY: deve partire la ventola, seguita dal prerun e dalla scritta evidente **HEATER ON**. Se la ventola non ruota realmente, fermati.
5. **Letture Sanwa: esattamente quando annotarle**, in ordine cronologico:

   | Evento che vedi | Lettura da segnare |
   | --- | --- |
   | PRIMA di premere ON/OFF, con dryer freddo | Sanwa iniziale (+ aria SHT45 iniziale) |
   | Appena compare il PRIMO `HEATER ON` | Sanwa al primo ON |
   | Appena compare il PRIMO `HEATER OFF` | Sanwa al primo OFF (+ aria SHT45) |
   | Ad OGNI successivo passaggio `HEATER ON` e `HEATER OFF`, se avviene | Sanwa in corrispondenza di quel passaggio |
   | Se SHT45 raggiunge 25°C | Sanwa nello stesso momento (+ indicazione HEATER ON/OFF) |
   | Allo spegnimento DEFINITIVO per fine prova / STOP / FAULT | Sanwa all'arresto definitivo |
   | Quando la ventola si ferma a fine raffreddamento | Sanwa finale |

   **Se primo OFF e OFF definitivo coincidono, basta UNA lettura**: non annotarla due volte. La registrazione video contemporanea di OLED+Sanwa è preferibile a distrarsi per scrivere; dopo la prova trascrivere i valori dal video. Il CSV registra i comandi e l'SHT45 ma NON il Sanwa.
6. Le pause `HEATER OFF` possono essere dovute a NTC<=50kΩ, fine finestra20s o aria>=25°C, senza necessariamente implicare FAULT. Un nuovo `HEATER ON` è possibile solo con NTC>=65kΩ, OFF>=12s e SHT45 aria<=24°C. **Durante tutto il ciclo sorveglia in continuo il Sanwa**, anche fra gli eventi registrati.
7. **Se Sanwa raggiunge 50°C NON aspettare l'evento successivo: STOP IMMEDIATO.** Fermati anche per odore anomalo, ventola ferma, danneggiamento, riscaldamento inatteso o altro guasto. Durante DRYING ON/OFF lungo circa1,5s; in emergenza scollega24V. Non prolungare né alterare soglie per raggiungere25°C.
8. Attendi il raffreddamento con ventola (previsto fino a90s dall'ultimo HEATER ON); controlla che il Sanwa diminuisca o resti stabile. Registra il suo valore **alla fermata effettiva della ventola**, senza cercare un secondo picco obbligatorio.
9. Solo dopo heater OFF e raffreddamento, scarica il **NUOVO** CSV da http://filament-dryer.local/api/last-test.csv (o usando IP del dryer). Se il download fallisce, descrivi l'errore: non rifare un ciclo HEAT per recuperare un file mancante.

## 4. Cosa verrà salvato a test completato

Verbale originale con: data/ora sessione realmente osservate, build OLED, Sanwa iniziale/primo ON/primo OFF/ultimo OFF/cooling/fan stop, SHT45 e RH iniziali e finali, evento target25 oppure budget HEAT o FAULT o STOP, numero e durata delle finestre, comportamento fan reale, eventuali anomalie. CSV originale senza editing + SHA256 + campioni e analisi distinta. Registro in docs/evidence/thermal/2026-10-27-S9A-PHYSICAL-SESSION.md su branch di esperimento e aggiornamento README, docs/PROJECT_STATE.md, docs/TODO.md e docs/THERMAL_EXPERIMENT_LEDGER.md con commit. Mai inventare letture e mai scambiare R_NTC in ohm per Sanwa °C.
