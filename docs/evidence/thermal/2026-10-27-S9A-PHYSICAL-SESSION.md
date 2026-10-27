# STEP09-A — Verbale fisico originale HEATER-ON con feedback SHT45

**STATO aggiornato 27 ottobre 2026: PROVA FISICA ESEGUITA, misure Sanwa fornite e CSV originale archiviato e verificato.** L'esito è **PASS di acquisizione e comportamento limitato della protezione/pausa NTC**, **NON** dimostrazione del raggiungimento/setpoint aria25°C, né qualifica termica della V1. Questa sessione NON deve essere ripetuta per acquisire i dati che ora ci sono.

**Sorgenti:** resoconto testuale fornito dall'utente in chat 27/10/2026: "Sanwa Iniziale 17; 1° ciclo on17 off30; 2° ciclo on29 off36; 3° ciclo on34 off37; 4° ciclo on35 off37; 5° ciclo on35 off38; dal 6° al 12° ciclo on35 off38; 13° ciclo on35 poi parte il cooling (vedrai che il sht era a22,7); Fine cooling26°C". File allegato originale `filament-last-test (3).csv`, copiato fedelmente nel repository come [S9A raw CSV](2026-10-27-S9A-raw.csv). I valori **Sanwa sono trascrizioni delle osservazioni dell'utente**, non sono nel CSV; non crearne timestamp indipendenti.

## Introduzione test / collegamenti

- **Cosa:** prima prova fisica del firmware S9A capace di attivare l'HEATER in un controllo della temperatura ARIA SHT45 (target25°C), subordinato alla pausa NTC (50kΩ/ripresa65kΩ) e guasto duro40kΩ, con ventola PWM100% e arresti temporali.
- **Perché:** distinguere la gestione della piastra più calda dalla temperatura obiettivo dell'aria, osservare in un sistema vero la sequenza HEAT/pausa senza ripetere altri impulsi fissi S7U.
- **Prima:** S6C 6 accensioni fisse e Sanwa max53°C / NTC min30,318kΩ; S7U 3 accensioni, guasto sperimentale NTC40kΩ e Sanwa43°C/aria19,52°C. STEP08 aveva PASS soltanto in host.
- **Dopo:** analizzare i limiti di duty/budget emersi e progettare **un nuovo percorso di verifica del controllo aria**, possibilmente con profilo sorvegliato rivisto **solo dopo review e consenso esplicito**, senza abbassare automaticamente i cutoff o scaldare subito.

## Provenienza e integrità dati

- File **originale** allegato `filament-last-test (3).csv`; repository [`docs/evidence/thermal/2026-10-27-S9A-raw.csv`](2026-10-27-S9A-raw.csv). Salvato preservando i bytes: **33.096 byte**, **778 righe dati + 1 header**, header con CRLF e righe dati LF come nell'upload. Hash Git blob SHA1 **`9312a724743632e62e28368e27cba7241e55401f`**; hash SHA256 originale **`6632f79b9e815965fe7f78b6ed31ec39f4a4bcb35b3c4f0c17da655ab4b877e0`**. Blob su GitHub letto dopo il commit e confrontato.
- Sessione eseguita e dati trasmessi **27 ottobre 2026**; ora di inizio/fine **NON FORNITA** e non deducibile da `ms` (millis da boot). Identificatore S9A **inferito dalla corrispondenza delle transizioni con il codice** e dal protocollo usato, ma **lettura diretta OLED `H27-S9A` NON ripetuta nel messaggio**. Sorgenti del trial su branch `experiment/step09-real-sht45-feedback-2026-10-27`, PR #14 DRAFT, build ESP32 + test GPIO mock precedentemente CI PASS (run40486080140).
- L'utente ha riportato 13 finestre e Sanwa e l'ultima temperatura a fine cooling; non è fornita conferma indipendente dello stato fisico del TCO105, del fissaggio sonda/cablaggi o giri ventola. Il CSV registra `fan_percent=100` ma non RPM/portata; non desumere da questo che il fan abbia realmente girato al100%.

## Dati della prova — SHT45/NTC dal CSV e Sanwa comunicato dall'utente

| Finestra | HEATER ON a t relativo (s) | ON comandato (s) | Sanwa ON (°C) | Sanwa OFF (°C) | Aria ON SHT45 (°C) | Aria OFF SHT45 (°C) | NTC a OFF (kΩ) |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 6,00 | 20,0 | 17 | 30 | 17,65 | 17,77 | 63,32 |
| 2 | 38,00 | 16,643 | 29 | 36 | 17,91 | 18,16 | 49,59 |
| 3 | 66,14 | 10,0 | 34 | 37 | 18,43 | 18,61 | 49,67 |
| 4 | 88,64 | 9,0 | 35 | 37 | 18,87 | 19,04 | 49,58 |
| 5 | 110,75 | 8,5 | 35 | 38 | 19,32 | 19,53 | 49,66 |
| 6 | 133,25 | 8,5 | 35 | 38 | 19,80 | 19,97 | 48,97 |
| 7 | 156,75 | 8,0 | 35 | 38 | 20,28 | 20,43 | 49,82 |
| 8 | 179,97 | 8,0 | 35 | 38 | 20,71 | 20,83 | 49,65 |
| 9 | 203,97 | 8,0 | 35 | 38 | 21,14 | 21,25 | 49,42 |
| 10 | 228,83 | 8,0 | 35 | 38 | 21,50 | 21,64 | 49,44 |
| 11 | 254,34 | 7,5 | 35 | 38 | 21,91 | 22,01 | 49,54 |
| 12 | 281,60 | 8,0 | 35 | 38 | 22,23 | 22,35 | 49,98 |
| 13 | 307,10 | 0,5 | 35 | non letto; passa a COOLING | 22,59 | 22,59 | 65,36 |

*I tempi relativi sono ricavati da `ms` nel CSV a partire dal primo campione; la risoluzione è di circa500ms e ci sono brevi gap di telemetria. L'ON comandato non equivale a corrente heater misurata indipendentemente. Le misure Sanwa manuali sono associate agli eventi ON/OFF indicati dall'utente, non sincronizzate al millisecondo; il ciclo13 NON ha misura Sanwa specifica OFF.*

### Valori di sintesi verificati

- Prima riga CSV `ms=2406435` (fase1 PRERUN): SHT45 **17,64°C**, RH**65,33%**, NTC**138,825kΩ**, fan command100%; Sanwa prima avvio **17°C**.
- Tredici fronti HEATER ON, dodici passaggi regolari verso `phase=3` (pausa) e una transizione dalla finestra13 a `phase=4` (cooling). Dodici finestre completate contate in `pulses=12`; la finestra13 è iniziata e durata circa **0,5s**, non va classificata come completata dal contatore.
- Tempi ON comandato ricavati dai fronti CSV (13 finestre): **20,0; 16,643; 10,0; 9,0; 8,5; 8,5; 8,0; 8,0; 8,0; 8,0; 7,5; 8,0; 0,5 secondi**. Somma delle transizioni loggate **~120,643 s**, confrontabile entro campionamento/scheduling con il limite di budget software **120.000ms**. L'integrazione del firmware può raggiungere il budget all'inizio del tick di spegnimento; NON è prova di 120,643s di corrente realmente erogata. Il tempo di trial fino al passaggio a COOLING è 307.595s, molto inferiore al `maximumRunMs=600000`: **il criterio coerente col codice è budget cumulativo raggiunto, non tempo massimo né target aria**.
- Il primo ON dura20,0s (limite finestra). Dal secondo al dodicesimo OFF, `R_NTC` a OFF si attesta intorno **48,975–49,983kΩ** (per lo più <=50k), compatibile con **pausa preventiva NTC 50kΩ**, non con termostato aria o guasto40kΩ; l'ultima accensione n.13 termina per budget calore quando NTC è ancora ~65,365kΩ.
- **NTC minimo globale CSV 48,717kΩ**, quindi nessun campione raggiunge il guasto hard40kΩ; `phase=6` (ABORTED) **0 campioni**. NON inferire un certificato di massima temperatura locale o corrente hardware da questi numeri.
- Aria SHT45 **17,64°C iniziale**, **22,59°C al passaggio in COOLING (t=307,595s)**. L'utente sul display riporta **circa22,7°C** allo stesso evento, compatibile con ritardo/arrotondamento/letture non sincronizzate; non correggere la sua osservazione come se fosse una misura sbagliata.
- In cooling SHT45 sale dal valore al cutoff **22,59°C** a un massimo osservato **22,97°C**, quindi picco relativo post-OFF **+0,38°C**; ultima riga **22,93°C**, RH **51,36%**; inizialmente RH **65,33%**, calo apparente **13,97 punti RH**, fortemente influenzato dalla temperatura: non prova essiccazione del filamento.
- Ultima riga `ms=2803565` dopo **397,130s** dalla prima; `phase=4`, heater_command0, fan_percent100. Il cooling nei campioni dura **89,535s**: l'assenza di riga `phase=5` nel CSV è coerente con campionamento che termina al completamento e salvataggio. L'utente riporta **Sanwa26°C alla fine del cooling**; il CSV non prova da solo la fermata fisica della ventola né la lettura del Sanwa.
- Sensori dichiarati validi **778/778** entrambi, comando fan PWM100 in **778/778**; righe per fase (1:12, 2:241, 3:350, 4:175). Campionamento principale nominale500ms, alcuni gap di firmware/servizi: non dedurre un segnale analogico continuo perfetto dalle transizioni campionate.
- **Sanwa massimo riportato38°C (OFF cicli5–12)**; al tredicesimo ON **35°C**, niente lettura OFF, cooling terminato Sanwa **26°C**. Nessun superamento di50°C *riportato*; le letture sono discrete, non un logger Sanwa continuo.

## Interpretazione tecnica (non decisione di nuovi limiti)

1. **PASS G1 acquisizione prova e comportamento delle pause locali:** l'HEATER GPIO viene comandato ON in 13 finestre, ogni nuova finestra quando `R_NTC` è risalita verso>=65kΩ; la maggior parte dei passaggi OFF avviene vicino a50kΩ e il Sanwa si stabilizza 35–38°C nelle osservazioni riportate. Non si evidenzia un FAULT.
2. **OBIETTIVO 25°C NON RAGGIUNTO e feedback aria NON testato sul setpoint:** la camera raggiunge solo max22,97°C anche durante cooling. Tutti gli OFF di riscaldamento spiegabili da limite20s/NTC o budget120s, e non serve attribuirne nessuno all'SHT45 che non è arrivato25°C; quindi **NON dichiarare termostato25°C in closed loop convalidato fisicamente** solo perché il firmware possiede il controllo.
3. **La piastra Sanwa e camera SHT45 non devono coincidere**. Questo test esplora riscaldamenti brevi sotto una pausa NTC prudenziale. La stabilizzazione Sanwa35–38°C non prova né sufficiente trasferimento per obiettivo40°C né soglia definitiva di sicurezza.
4. **Vincolo sperimentale individuato:** `maximumTotalHeatMs=120000` viene raggiunto a circa308s da inizio log molto prima del limite globale10min, interrompendo il trial quando l'aria è solo22,59°C. Non alzare automaticamente budget, diminuire R_NTC soft50kΩ o hard40kΩ senza quantificare il margine termico e approvare nuovo protocollo. Valutare il confronto con S6C/S7U e la portata reale ventola e TCO installato.
5. **Outcome:** ciclo completato per budget (transizione a cooling, non FAULT), Sanwa si raffredda fino26°C a fine cooling. Temperatura aria25°C non raggiunta entro il budget. No modifica V1 o soglie permanenti approvata da questa sessione.

## Prossimo passo mirato (senza chiedere un altro S9A identico)

Analisi ingegneristica della limitazione **tempo totale HEAT vs pause NTC50/65kΩ vs progressione aria**. Definire, documentare e rivedere un eventuale nuovo envelope sperimentale per raggiungere il setpoint aria senza violare il veto Sanwa o il guasto NTC; separare review hardware TCO/fan/materiali da test autonomi. Un'ulteriore prova HEAT necessita **di nuova introduzione cosa/perché/precedente/futuro**, differenza esplicita da S9A e **approvazione del profilo successivo**; non chiedere all'utente di rifare questa per mancanza di dati.

**Dati non disponibili:** conferma esatta OLED build, ora effettiva, Sanwa a OFF finestra13, misura Sanwa massima continua tra gli eventi, misure tach/portata, corrente/potenza heater reale, documentazione fisica cablaggi/TCO e foto/video. Sono indicati come assenti, non inventati.
