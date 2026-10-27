# STEP09-B — Review quantitativa del budget HEATER prima di un nuovo test fisico

**Data di preparazione: 27 ottobre 2026. Stato: review analitica ESEGUITA; S9B PROPOSTO, TEST FISICO NON ESEGUITO E NON ANCORA AUTORIZZATO.** Nessuna decisione di limitazione V1, montaggio, TCO, fan o NTC modificata. Il progetto ha già effettuato STEP09-A; non ripetere i dati.

## Introduzione (COSA, PERCHÉ, PRECEDENTE, FUTURO)

- **COSA:** verificare quantitativamente la ragione di stop a22,59°C in S9A e preparare, **come singola ipotesi** da sottoporre a consenso, S9B che mantiene il vero controllo termostatico SHT45 aria25°C e alza **solo il limite cumulativo di comando HEATER** da **120s a200s** (non la singola finestra). La durata massima10min non cambia.
- **PERCHÉ:** la prova S9A ha acquisito13 accensioni reali e funzionamento della pausa SOFT NTC, ma l'aria non ha raggiunto il target25°C: serve verificare lo spegnimento per raggiungimento setpoint, non altri impulsi fissi.
- **PRECEDENTE:** [S9A verbale originale](2026-10-27-S9A-PHYSICAL-SESSION.md), [S9A CSV raw](2026-10-27-S9A-raw.csv), [S9A analisi](2026-10-27-S9A-analysis.md). Sanwa massimo riferito38°C sulle finestre stabilizzate, 26°C dopo cooling; SHT45 aria17,64°C iniziale,22,59°C allo stop,22,97°C max a fine cooling; R_NTC minima48,717kΩ, no FAULT. Budget120s esaurito; non è intervenuto termostato25°C.
- **FUTURO:** primo vero cutoff SHT45 a25°C da misurare sul prototipo, con data/CSV+Sanwa; poi valutare se il loop termostatico e l'isteresi della V1 sono appropriati. Un singolo test non qualifica sicurezza termica, fan AUTO o i target OEM40/50/55°C.

## G1. Evidenza numerica e modello strettamente esplorativo

CSV sorgente: 778 campioni /33.096 byte, [Git blob SHA1 `9312a724743632e62e28368e27cba7241e55401f`](2026-10-27-S9A-raw.csv), SHA256 `6632f79b9e815965fe7f78b6ed31ec39f4a4bcb35b3c4f0c17da655ab4b877e0`. Dati veri: tempo relativo S9A=0s alla prima riga `ms=2406435`, picco pre-cooling al t307,595s con SHT45 aria22,59°C, massimo finale22,97°C. Finestre successive6–12 duravano7,5–8,5s ON, una circa ogni23–28s, con R_NTC frequentemente vicino a50kΩ al SOFT OFF e Sanwa on35/off38°C. Frazione HEATER comandata sul tratto **t180→307,6s** ≈**32,1%**, non corrente elettrica.

L'aria cresce ancora a fine fase HEAT, ma la pendenza diminuisce. Regressione lineare OLS descrittiva (soltanto ultimi campioni **prima** del cooling; non misura di costanti fisiche):

| Intervallo dati usato per fit | Pendenza descrittiva | Se invariata, crossing teorico25°C da inizio CSV |
| --- | ---: | ---: |
| t100–307s | 1,008°C/min | t440s (7min20s) |
| t180–307s | 0,886°C/min | t466s (7min46s) |
| t220–307s | 0,826°C/min | t480s (8min00s) |
| t250–307s | 0,786°C/min | t491s (8min11s) |

**NON USARE QUESTE DATE COME PREVISIONI GARANTITE**: l'estrapolazione oltre gli ultimi 307s è vulnerabile a temperatura esterna, inerzia, perdita termica, posizione SHT45, temperatura di partenza, diversa potenza e plateau della piastra. L'aria potrebbe stabilizzarsi sotto25°C anche lasciando più tempo. Nessun modello identificato di impianto, nessuna banda d'incertezza statistica/strumentale validata.

Dal comportamento osservato: t307,6→t440–491 implica grossolanamente **132–183s in più di tempo reale** e circa **40–60s di HEATER ON addizionale** se (ipotesi non confermata) il duty ratio ~32% rimanesse simile. Il cutoff target potrebbe quindi verificarsi con un totale di comando HEATER indicativamente160–180s. In S9A il cap era120s, quindi non poteva osservare il target. **Il candidato 200s** (in luogo di180s) lascia margine per andamento più lento e resta entro10min di durata globale; non è dimostrato che200s bastino e non è dichiarato sicuro per uso autonomo. 

## G2. Confronto rischio e limiti — variazione a una sola dimensione

| Parametro | S9A fisico | S9B **proposto, non autorizzato** | Razionale |
| --- | --- | --- | --- |
| SHT45 aria setpoint / isteresi | target25°C, ON<=24°C, OFF>=25°C | **IDENTICO** | Obiettivo è verificare lo spegnimento per temperatura aria |
| Soft NTC PAUSA / RESUME | OFF<=50kΩ, ripresa>=65kΩ | **IDENTICO** | Non alzare temperatura locale della zona sensore |
| Hard RAW NTC FAULT | <=40kΩ o >=250kΩ | **IDENTICO** | Fault elettrico + overheat test invariati |
| HEATER finestra ON | <=20s; watchdog ISR a20,5s | **IDENTICO** | Nessuna accensione continua lunga |
| Pausa minima | >=12s | **IDENTICO** | Nessuna crescita duty per cambio guard |
| Limite aria fault | >=40°C | **IDENTICO** | Veto aria indipendente dal target25°C |
| **HEATER cumulativo massimo comandato** | **120s** | **200s** (+80s, +66,7%) | Unica variabile termica cambiata, per tentare il crossing25°C |
| Durata massima esperimento | 600s (10min) | **IDENTICA** | Secondo blocco indipendente:200s non garantiti se i duty scendono |
| Fan / prerun / cooldown | PWM100%, spunto1s, prerun5s, post90s | **IDENTICO** | Confrontabilità e smaltimento del calore |
| Sanwa veto manuale | STOP a >=50°C | **IDENTICO** | Stop indipendente dall'NTC; non garantisce punti non misurati |
| TCO105 monouso esistente | riferito in serie | **NESSUNA MODIFICA** | Non bypassare/sostituire come parte del test |
| CSV RAM | 1400 righe @500ms (massimo700s) | **IDENTICO** | 600s run+90s cooldown = fino690s dal ciclo, lascia pochi secondi di margine |

**Valutazione G2:** plausibilità ingegneristica **LIMITATA** per predisporre una *PR di test*, **NON autorizzazione automatica a caricare/accendere**. Sanwa35–38°C in S9A mostra un certo plateau sulla sonda contatto durante~5min, ma il maggior budget e l'aumento graduale aria potrebbero cambiare i gradienti locali, il carico e la temperatura delle pareti. **Il Sanwa è una sonda puntuale, non un rilevatore di tutti gli hotspot; NTC raw non è un termometro qualificato in°C.** Nessun sensore RPM rileva blocco ventola. Il TCO105 è un'aggiunta opzionale secondo D029 ma è già in serie e va mantenuto; foto/contatto/rating DC/temperature del corpo non verificati qui.

## G3. Perché non modificare altri parametri o anticipare40°C

- Abbassare R_NTC hard40kΩ o SOFT50kΩ consentirebbe al sensore di scaldarsi di più, aggravando incertezza; la sessione S9A non lo giustifica.
- Ridurre il PWM fan contemporaneamente al budget cambia ricircolo e hotspot e toglie confrontabilità. `fan_percent=100` nel CSV è solo comando.
- Allungare il singolo ON o disabilitare ISR toglierebbe una protezione già presente; NON farlo.
- Passare direttamente al target40°C OEM senza aver visto SHT45 fermare il riscaldatore a25°C cambia insieme target, esposizione e rischi: non è il prossimo esperimento.
- Dalle misure S6C a120s ON e Sanwa53°C su impulsi20s si sa che differente duty/guard può portare il punto contatto oltre il veto50°C; non generalizzare il plateau38°C di S9A a qualunque firmware.

**Gate prima di S9B FISICO:** verifica stato e geometria attuale (Sanwa sulla piastra vicino NTC, SHT45 in aria camera, ventola realmente libera, cavi/materiali e TCO105 già in serie, alimentazione 24V rapidamente isolabile), CI/ESP32 verificata sul commit, review safety e **specifica approvazione dell'utente per l'incremento totale120→200s**. La precedente autorizzazione S9A non copre la modifica. Se i materiali/cavi/fan sono differenti o presentano anomalie fermare. Un singolo esperimento S9B sotto supervisione con cooling e veto Sanwa>=50°C; non un'utilizzazione autonoma.

## G4. Esito della review

**GO esclusivamente alla preparazione del codice/test CI e del protocollo S9B in un branch/PR DRAFT separati.** **NO-GO al flash o prova HEATER ON senza approvazione del nuovo profilo e verifica fisica**, indipendentemente da CI PASS. Nessuna nuova decisione D0xx o modifica al circuito approvata.

**Dati ancora da acquisire soltanto in S9B autorizzato:** se e quando SHT45 attraversa25°C; istante e ragione del primo HEATER OFF a target, eventuale successivo HEATER ON sotto24°C (può non accadere), Sanwa a ogni cambio stato, lettura al termine, R_NTC/aria/RH e fan command nel CSV, massimo temperatura contatto osservato e andamento cooling. Non duplicare S9A.
