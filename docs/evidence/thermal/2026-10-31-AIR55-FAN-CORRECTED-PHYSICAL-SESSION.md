# AIR55/60min, ventola originale rimontata correttamente — seconda prova fisica del 31 ottobre 2026

## Identità, ambito e provenienza

- **Esito tecnico:** prova fisica da 60 minuti completata dal timer, obiettivo AIR55 **NON raggiunto**. Dopo la correzione dell'orientamento della ventola OEM, AIR massima **49,84°C** (circa10 secondi dopo HEATER OFF); AIR all'OFF **49,82°C**. Prima prova con ventola invertita AIR max45,84°C, AIR all'OFF45,82°C; miglioramento AIR massimo circa **+4,00°C**, nelle condizioni non perfettamente identiche dei due test.
- **Sicurezza:** la misura esterna Sanwa RD701/K su **PIASTRA vicino alla NTC** ha raggiunto **89°C** da circa40 a60 minuti; **limite manuale sperimentale85°C oltrepassato dopo20min**, nessuno STOP anticipato visibile nel CSV. Questa violazione non è un "PASS sicurezza". La misura del corpo TCO105, hotspot, temperature reali dei materiali e margini non sono disponibili.
- **Dichiarazione dell'operatore:** ventola originariamente montata al contrario nella prima sessione, ora rimontata secondo orientamento OEM; le prestazioni indicano una differenza misurabile, ma non provano da sole direzione/portata della ventola o che sia l'unica variabile ambientale.
- **Software attribuito dal contesto:** `HOT-261031-N8580` / OLED `H31-8580`, sorgente sperimentale `1cd96dd76500dfb4d161e1b445d8df6c9d8e524b`, [CI#41142714373](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/41142714373). Il CSV non include un identificatore di firmware effettivamente flashato. `main` HEATER-off invariato, PR18 DRAFT.
- **Input dichiarati del confronto:** AIR setpoint55°C, durata selezionata60min, ventola OEM comando100%, camera senza filamento come nel test precedente, prerun fan5s e postHEAT90s. Non è documentata una **misura indipendente** della temperatura ambiente per la nuova sessione; SHT45 AIR iniziale17,13°C e Sanwa prima di START16°C non equivalgono a misura della stanza. Stato del controllo a freddo del flusso, lettura TCO e condizioni esatte di chiusura sono non confermati.
- **Originale caricato dall'utente:** `filament-last-test (2)(1).csv`, **40.938 byte, 747 righe dati, 15 colonne, SHA256 `52b00f8ec6ecc367e97f647141bfb5d6f1b6bc77dc48e4f11633f275f44d6454`, Git blob SHA `b15d351f96f9189f50d468c0881cb49fdbc79de2`**. Conservato byte-identico in [raw](2026-10-31-AIR55-FAN-CORRECTED-raw.csv), non riscritto o ricampionato.
- **Sanwa originale:** [foglio Google `Nuova prova`, colonne A-C](https://docs.google.com/spreadsheets/d/1I5utkzGP1zvD168jLghpYB_y_myGjxMb1ccLens4Cks/edit?gid=525622499): **B** misure della sessione con ventola inversa, **C** sessione ventola corretta, **D** note/eventi, **non presenti nuove note specifiche sui tempi degli eventi**; le righe 41-44,51/56-59 restano vuote. Sono presenti37 misure di prerun/heat e4 di raffreddamento nel foglio. L'orario civile della prova non è annotato nel CSV: il campo `ms` è `millis()`, non data/ora; data del contesto/chat 31 ottobre 2026 (timezone utente Europe/Rome, ora esatta di START/STOP ignota).

## Numeri verificati da CSV e foglio

| Variabile | Ventola corretta | Ventola invertita | Interpretazione |
| --- | ---: | ---: | --- |
| AIR iniziale CSV | 17,13°C | 16,15°C | +0,98°C di partenza; ambiente diverso/non misurato |
| AIR max | **49,84°C** | **45,84°C** | **+4,00°C** assoluti; non attribuire integralmente al flusso senza un ambiente controllato |
| AIR al passaggio HEATER OFF | **49,82°C** | **45,82°C** | +4,00°C |
| Crescita AIR da40 a60min | **+0,65°C** | **+0,68°C** | entrambe prossime ad appiattimento in queste condizioni |
| Sanwa PLATE massimo | **89°C** | **83°C** | +6°C, ma misura in unico punto, non TCO/body |
| Sanwa prima START | 16°C | 15°C | valore manuale |
| R_NTC minima CSV | **8.016,5Ω** | **7.920,4Ω** | nuova prova NTC NON arriva alla soglia firmware7.120Ω |
| R_NTC al HEATER OFF | **8.067,7Ω** | **7.973,8Ω** | vicino, NON sufficiente per dire che thermistor/bead=Sanwa |
| RH iniziale CSV | 53,21% | 62,84% | differente; controllo umidità non qualificato |
| RH minima CSV | **17.79%** | 19,90% | la riduzione RH non prova asciugatura filamento |
| Comando HEATER | una sola accensione per**3594s**, OFF a fine60min | una sola accensione per3594s | CSV comandi, non misura di corrente |
| Ventola comando | 100% durante HEAT e90s dopo | uguale | nessuna misura tachimetrica/portata |
| Fault `pulse_fault`/`ntc_fault` | 0 in tutte747 righe | 0 | assenza contatore non certifica protezione termica |

**Controllo:** `phase` passa1→2 al ms744120,2→4 +3600s dal primo campione (ms4338120),4→5 al ms4428119, con FAN100 poi FAN0. I 719 campioni HEATER=1 coprono3594s; 28 campioni HEATER=0. Sensori `air_valid` e `ntc_valid` sempre1, `pulse_fault=ntc_fault=0`, `loop_gap_ms=1` su tutti i record. I log riflettono le **richieste firmware**, non il funzionamento elettrico continuo verificato con amperometro. Nessuna pausa NTC a7.120Ω, OFF per AIR55, riarmo AIR54 o prova reale di fault osservati. Ventola ha completato90s di cooldown comandato e l'archivio continua fino120s dopo OFF.

### AIR e NTC per minuti selezionati (valori CSV più vicini; 0 = primo campione prerun)

| Minuto da START | AIR nuova °C | AIR precedente °C | Delta nuova−vecchia °C | Sanwa nuova °C | NTC nuova Ω |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 0:00 | 17.13 | 16.15 | 0.98 | 16 | 139928.7 |
| 5:00 | 29.36 | 26.34 | 3.02 | 66 | 16837.3 |
| 10:00 | 38.18 | 34.70 | 3.48 | 76 | 12093.8 |
| 20:00 | 45.79 | 41.89 | 3.90 | 85 | 9231.9 |
| 30:00 | 48.22 | 44.39 | 3.83 | 87 | 8495.1 |
| 40:00 | 49.17 | 45.14 | 4.03 | 89 | 8251.8 |
| 50:00 | 49.61 | 45.57 | 4.04 | 89 | 8136.9 |
| 60:00 | 49.82 | 45.82 | 4.00 | 89 | 8067.7 |

**Interpretazione:** a10min AIR38,18 vs34,70 (+3,48°C), a30min48,22 vs44,39 (+3,83°C), a60min49,82 vs45,82 (+4,00°C). Negli ultimi20min AIR aumenta solo ~0,65°C con comando HEATER continuo; è una **quasi stabilizzazione osservata**, non una dimostrazione matematica del limite definitivo o una misura della capacità massima in condizioni di ambiente identiche. L'aumento della temperatura iniziale di ~1°C può contribuire al delta.

### Tutte le letture Sanwa del foglio e NTC/AIR associati (prima di START e 0–60min)

Valori Sanwa **trascritti senza interpolazione** dalle righe2–38 del foglio; NTC/AIR **prelevati dal record CSV temporalmente più vicino** usando come t=0 il **primo campione PRERUN** `ms=738120`, non l'inizio HEATER ON che avviene6s più tardi. Gli allineamenti ai beep e tempi manuali sono approssimativi; nessun timestamp assoluto condiviso dai due strumenti. Temperature diverse delle NTC e del Sanwa: il Sanwa è in un punto vicino, non sul bead dell'NTC.

| Minuto | Sanwa nuova °C | Sanwa vecchia °C | NTC nuova Ω | AIR nuova °C | HEATER comando |
| --- | ---: | ---: | ---: | ---: | ---: |
| Prima START | 16 | 15 | 139928.7 | 17.13 | 0 |
| 0:00 | 16 | 15 | 139928.7 | 17.13 | 0 |
| 1:00 | 45 | 31 | 38856.2 | 18.42 | 1 |
| 2:00 | 54 | 41 | 26568.5 | 21.05 | 1 |
| 3:00 | 59 | 47 | 21669.6 | 24.00 | 1 |
| 4:00 | 62 | 51 | 18757.8 | 26.80 | 1 |
| 5:00 | 66 | 55 | 16837.3 | 29.36 | 1 |
| 6:00 | 68 | 58 | 15395.0 | 31.54 | 1 |
| 7:00 | 71 | 60 | 14280.1 | 33.56 | 1 |
| 8:00 | 73 | 62 | 13412.2 | 35.32 | 1 |
| 9:00 | 74 | 64 | 12667.8 | 36.85 | 1 |
| 10:00 | 76 | 66 | 12093.8 | 38.18 | 1 |
| 12:00 | 78 | 69 | 11154.7 | 40.39 | 1 |
| 14:00 | 81 | 72 | 10527.4 | 42.23 | 1 |
| 16:00 | 82 | 73 | 10030.9 | 43.75 | 1 |
| 18:00 | 84 | 75 | 9624.8 | 44.90 | 1 |
| 20:00 | 85 | 76 | 9231.9 | 45.79 | 1 |
| 22:00 | 86 | 77 | 9012.9 | 46.51 | 1 |
| 24:00 | 86 | 78 | 8871.5 | 47.07 | 1 |
| 26:00 | 87 | 79 | 8722.2 | 47.55 | 1 |
| 28:00 | 87 | 79 | 8600.5 | 47.88 | 1 |
| 30:00 | 87 | 80 | 8495.1 | 48.22 | 1 |
| 32:00 | 88 | 80 | 8467.3 | 48.50 | 1 |
| 34:00 | 88 | 81 | 8382.8 | 48.72 | 1 |
| 36:00 | 88 | 81 | 8328.6 | 48.91 | 1 |
| 38:00 | 88 | 81 | 8293.8 | 49.04 | 1 |
| 40:00 | 89 | 81 | 8251.8 | 49.17 | 1 |
| 42:00 | 89 | 82 | 8235.0 | 49.23 | 1 |
| 44:00 | 89 | 83 | 8209.8 | 49.40 | 1 |
| 46:00 | 89 | 82 | 8181.1 | 49.45 | 1 |
| 48:00 | 89 | 83 | 8153.6 | 49.51 | 1 |
| 50:00 | 89 | 83 | 8136.9 | 49.61 | 1 |
| 52:00 | 89 | 83 | 8120.2 | 49.68 | 1 |
| 54:00 | 89 | 83 | 8079.6 | 49.69 | 1 |
| 56:00 | 89 | 83 | 8108.2 | 49.75 | 1 |
| 58:00 | 89 | 83 | 8089.1 | 49.77 | 1 |
| 60:00 | 89 | 83 | 8067.7 | 49.82 | 0 |

**Coppie diagnostiche significative:** Sanwa85°C a20min ≈ **9.231,9Ω**; Sanwa88°C da32 a38min con NTC≈8,29–8,47kΩ; Sanwa89°C da40 a60min con NTC≈8,07–8,25kΩ. **La vecchia soglia7.120Ω proveniva dal contatto Sanwa85°C il17 ottobre con box aperto e ventola OFF**: questi dati dimostrano che non equivale a85°C Sanwa nell'attuale sistema chiuso/fan ON. Resta sperimentale, non convertire questo insieme in `NTC=xx°C` né promuovere nuove soglie senza verifica delle temperature locali e dei margini indipendenti.

### Raffreddamento misurato

| Da HEATER OFF | Sanwa nuova °C | Sanwa vecchia °C | AIR nuova °C (CSV) | NTC nuova Ω | Ventola comando % |
| --- | ---: | ---: | ---: | ---: | ---: |
| OFF +0:30 | 72 | 72 | 49.62 | 15301.7 | 100 |
| OFF +1:00 | 64 | 64 | 48.91 | 20778.4 | 100 |
| OFF +1:30 | 58 | 59 | 47.98 | 24989.7 | 0 |
| OFF +2:00 | 58 | 58 | 46.96 | 26230.7 | 0 |

Il Sanwa dopo OFF scende89→72→64→58→58°C; nessun overshoot Sanwa documentato ai quattro tempi, ma non si conosce il massimo istantaneo tra letture. AIR raggiunge max49,84°C circa10s dopo OFF, +0,02°C rispetto all'OFF; poi scende fino46,96°C a120s. Al90s FAN OFF. Non esistono misure oltre2min postOFF per questa sessione.

## Punti confermati, limiti, prossimo passaggio

**Confermato nei dati:** correzione dell'orientamento riferita dall'utente e incremento osservato AIR +4°C; AIR55 non raggiunta, Sanwa fino89°C e sopra veto85°C per circa40min, nessun intervento di guard nel CSV, ciclo60min e cooldown90s regolari. Le letture SHT45 sono numericamente coerenti e la conversione Sensirion nel firmware `Sht45Sensor.h` usa correttamente `-45+175*raw/65535` con due CRC: la correttezza della lettura assoluta nel sito di montaggio non è provata da un confronto termometrico indipendente. La posizione nota è nella parte alta camera, su supporto distanziato.

**Non verificato:** effettiva potenza elettrica24V sotto carico, reale velocità/portata ventola, temperatura nel punto del corpo TCO105 (la sua temperatura di intervento NON è deducibile dal Sanwa), gradienti/hotspot, ambiente misurato, accuratezza termometrica in installazione, sicurezza autonoma, capacità di termoregolare a55. La caratterizzazione storica17 ottobre era in altra geometria. Il limite manuale di prova85°C non coincide con un rating di guasto materiale: la sua violazione è registrata, non giustificata retroattivamente dalla mancata apertura del TCO.

**Prossima unica attività tecnica:** esaminare perché due prove con lo stesso riscaldatore e differente orientamento della ventola si appiattiscano rispettivamente a ~46/~50°C; prima di un altro test caldo, privilegiare una verifica **non riscaldante** della temperatura SHT45 contro termometro indipendente nella sua posizione e dell'effettivo percorso dell'aria, oltre al controllo elettrico della potenza del riscaldatore e del TCO/materiali con procedura sicura separata. Se questi controlli non spiegano i ~5°C residui, formulare **una** proposta supportata da misure per eventuali interventi. Non alzare limiti NTC, non rimuovere il TCO né alterare `main` automaticamente. Il web live durante HEATER ON rimane miglioramento secondario.
