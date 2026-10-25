# STEP06B — Prova dinamica più lunga con ventilazione al 100%

> **2026-10-25 — SESSIONE FISICA ESEGUITA, NON SOLO PROPOSTA.** L'utente ha condiviso CSV S6C `filament-last-test (1).csv`, più misure indipendenti Sanwa: **18°C all'avvio, max53°C durante riscaldamento, 30°C a fine cooling**, con osservazione esplicita **Sanwa in ritardo rispetto alla variazione dell'NTC durante riscaldamento**. Analisi dettagliata (fonte fisica, 652 campioni, sei pulse heater-command di20s, fan PWM100%, SHT45 aria 18,64→23,98°C, NTC 131,95→min30,32kΩ): [report sul ramo operativo](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/feature/wifi-ota-web-logging-2026-10-24/docs/STEP06B_FAN100_PHYSICAL_RESULTS_2026-10-25.md). Il software compilato e testato per la sessione resta **SHA `e6a0d782eda690630f9b496c4eea1e5d37f5acaa`**, i commit di questo aggiornamento documentale NON modificano quel binario. **Non proseguire con test più lunghi o termostato V1** finché limite automatico NTC/margine rispetto a Sanwa53→veto55°C non venga analizzato. La relazione fra NTC(Ω), Sanwa locale(°C) e SHT45 aria(°C) **non è equivalenza di temperatura**.

**Data:** 2026-10-25. **Tipo:** esperimento supervisionato, **non** termostato autonomo V1. Branch esclusivo: `experiment/step06b-fan100-extended-2026-10-25` (da S6B, PR #10). **Versione sorgente:** `HOT-251025-S6C`, OLED `H25-S6C`. **Nessuna nuova prova fisica eseguita o dichiarata valida** da questo documento. Source: `firmware/AppConfig.h`, `HeaterStep06Extended.h`, `SafetyController.h`, `ThermalLog.h`.

## Perché questa prova, non il 40% storico

La decisione [D025](DECISIONS.md) fissò per il *bring-up elettrico* la prima soglia d'avvio dal fermo al 25% PWM, la minima velocità sostenuta già testata al 15%, e un comando convenzionale del **40% come margine contro lo stallo**; questo **non misura** né portata d'aria né trasferimento termico ed è esplicitamente *non finale*. Nella prima prova S6A il comando 40% e 4×10s ON hanno prodotto, su **411 campioni/141,542 s**, aumento aria SHT45 da **18,51 a 19,52°C**, R_NTC da **135,50 kΩ a min49,04 kΩ** e Sanwa di superficie **18→40°C**. La prova non ha misurato la corrente riscaldatore indipendentemente. La differenza tra piastra e camera è fisica e attesa.

**Nuova scelta sperimentale:** richiedere **100% PWM** all'avvio e *per tutto il test* così da avere un riferimento di ventilazione massima comandata, non una garanzia di flusso massimo né una prova che sarà l'impostazione ottimale per asciugatura. Questa modifica non sostituisce D025 come regola permanente: è un override specifico di laboratorio documentato in una build distinta.

## Profilo di misura proposto e implementato (una sessione)

| Campo | Valore |
|---|---:|
| Start fan kickstart | 100% PWM per 1 s |
| Fan regime e cooldown | **100% PWM**; rotazione e flusso *da osservare fisicamente*, no tach |
| Prerun dal primo RUNNING | 5 s |
| Comandi heater | **6×20 s ON** (max 120 s nominali); **5×12 s OFF** fra i sei |
| Watchdog hardware GPIO19 | timer ISR a **20,5 s per intervallo**, impone LOW se loop bloccato, da verificare ancora fisicamente |
| Sessione nominale | ~186s fino all'ultimo heater OFF (da avvio fan); +90 s cooling = **~276 s** |
| Overall watchdog | 330 s |
| Stop NTC | **≤19,68 kΩ** o ≥250 kΩ, invalid/short/open/stale oltre1,5s |
| Stop camera SHT45 | **≥40°C ARIA**, invalid/stale oltre5s |
| Stop altri | fan *comandata* non running, STOP, fault, overall timeout, ISR deadline |
| Veto termico **manuale** | Sanwa piastra **≥55°C** o contatto incerto; **NON** letto dalla MCU |
| Memoria logger | 1400 campioni/250ms, circa **350s**: preserva inizio e raffreddamento |
| Potenza elettrica ipotizzata | 24V×1,59A =38,16W da dati precedenti (non rimeasured) |
| Budget energetico **nominale teorico** | 120s×38,16W ≈**4.58 kJ** se tutti e sei i segmenti si completano; **3×** precedente S6A ~1,53kJ |

Non confondere **40°C ARIA SHT45** (arresto automatico del protocollo) con **55°C SANWA** piastra (veto operatore), né usare la vecchia correlazione NTC/Sanwa fatta a box APERTO e ventola OFF come termometria interna del bead. Una piastra più calda dell'aria è necessaria per trasmettere potenza termica; la differenza da ottenere/valutare dipende da materiali, ventilazione, sensori e dinamica, non è un errore di sensore.

### Stato approvazione e condizioni concrete

L'utente ha esplicitamente chiesto di **procedere senza altre fasi teoriche** dopo aver concordato che il prossimo test vada fatto a ventilazione piena e per durata significativamente maggiore. Questo è un **profilo numerico aggiuntivo scelto dal lavoro tecnico**, quindi va reso esplicito e accettato come impostazione della sessione fisica prima di avviarla; l'implementazione non certifica la sicurezza per futuri cicli autonomi.

Prima di eseguire:
1. Selezionare in **GitHub Desktop** solo il branch `experiment/step06b-fan100-extended-2026-10-25`, fare Fetch/Pull, caricare `firmware/firmware.ino` con **Arduino IDE, porta OTA già collaudata**, senza scaricare manualmente zip o attivare la pagina /ota. Accertare che l'ID visualizzato sia `HOT-251025-S6C`/`H25-S6C`.
2. Alimentatore 24V immediatamente isolabile, contatto Sanwa sulla piastra **fisicamente confermato**, passaggio cavi da jack DC protetto da spigoli/schiacciamenti e dall'elica, scatola chiusa, TCO105 già presente ancora in serie, nessun comportamento anomalo. Osservare reale movimento aria durante kickstart e regime al 100%; il comando PWM non prova rotazione.
3. Preparare Sanwa leggibile durante tutto il test e dopo spegnimento, ambiente e materiale circostante sicuri, presenza costante operatore, STOP pronto. **Non avviare** se Sanwa non resta fissata, aria non circola, cavi possono toccare piastra/pale, o stacco rapido24V non disponibile.
4. Avviare **una sola sessione supervisionata**. Fermare subito a Sanwa≥55°C, distacco sonda, anomalia meccanica/termica/elettrica, arresto della ventola, odore o comportamento non previsto. Per emergenza interrompere alimentazione 24V, anche se questo arresta la ventola; arresto d'emergenza prevale su cooldown.
5. Attendere termine cooldown90s e scaricare `/api/last-test.csv`; annotare massimi Sanwa ON e dopo OFF e se venivano raggiunti durante quali impulsi, eventuali interruzioni, giri/flusso ventola visivamente, temperatura camera da SHT45 nel CSV. L'HTTP non aggiorna durante HEATER ON: la dashboard S6B/S6C visualizza **NON AGGIORNATO** invece di un vecchio OFF; il CSV è registro dei *comandi* e non misura corrente.
6. Se la sequenza si interrompe per guardia, **non modificare le soglie o riavviare** per forzare una risposta più calda. La curva ottenuta, anche abortita, è il risultato tecnico utile. Solo dai dati decideremo il primo termostato limitato a 40°C aria dello Step07, quale duty fan comparare, e se/sotto quali limiti raggiungere il setpoint è compatibile con TCO e materiali.

### Garanzie non fornite

- Il valore 100% PWM non equivale ad una portata misurata.
- I 4.58kJ non dimostrano un aumento aria prevedibile né garantiscono che l'NTC non raggiunga il cutoff prima dei 120s.
- Il watchdog ISR è stato compilato e testato solo per integrazione di codice; non è stato misurato esternamente con un fault CPU sul dispositivo fisico.
- Nessuna verifica indipendente di amperaggio heater, vita TCO, Tmax materiali o sovraelongazione a un punto non strumentato.
- Non fare entrare questa build riscaldante in `main` né sostituire la V1 con una sequenza di commissioning.

**NEXT fisico:** dopo compilazione GitHub Actions PASS del commit esatto, eseguire la prova una volta, condividere CSV/Sanwa, analizzare il nuovo ritardo termico e il margine NTC. Non usare un altro ciclo a4×10s per cercare informazioni nuove.
