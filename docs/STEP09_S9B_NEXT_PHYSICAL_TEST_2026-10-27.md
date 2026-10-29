# STEP09-B — Verifica fisica del controllo aria a 25 °C con budget HEATER esteso

> Historical protocol/technical record. Its dated NEXT and pre-test status are not the current work queue; see [PROJECT_STATE](PROJECT_STATE.md) and [TODO](TODO.md). Original checkpoint preserved in archive tags.

**STATO 27/10/2026: PREPARATO, NON ESEGUITO.** Il profilo S9B **NON È AUTORIZZATO** dalla precedente conferma S9A. È necessario consenso esplicito dell'utente alla sola variazione **120 → 200 secondi complessivi di HEATER ON**, dopo CI, review e controllo fisico. **Non caricare il branch S9B e non avviare il dryer finché i gate non sono chiusi.** Nessuna modifica alla PCB, al TCO105, a fan PWM o a guardia NTC.

## Introduzione obbligatoria in linguaggio chiaro

- **COSA:** durante una sola prova fisica con HEATER realmente acceso, portare l'aria misurata dall'SHT45 verso **25°C**. A differenza di S9A, concediamo **al massimo 200s complessivi di ON** invece di120s, mantenendo gli spegnimenti automatici quando il riscaldatore si scalda troppo. **Non significa HEATER ON continuo per 200s**.
- **PERCHÉ:** vogliamo verificare che il termostato **smetta di richiedere calore quando l'aria raggiunge25°C**, non perché sia terminato il budget temporale. Se aria25°C non viene raggiunta neanche con il nuovo budget, il risultato sarà archiviato senza cambiare soglie sul posto.
- **DA DOVE ARRIVIAMO:** S9A ha già acceso13 volte il riscaldatore, con Sanwa17°C iniziale,35–38°C nelle finestre stabilizzate e26°C al termine. CSV originale778 campioni: SHT45 aria17,64°C iniziale,22,59°C al COOLING,22,97°C nel successivo raffreddamento, R_NTC minimo48,717kΩ, nessun FAULT. Il test è terminato dopo120s HEATER ON cumulativi, **non** per il raggiungimento dei25°C.
- **DOPO:** solo se il raggiungimento25°C e il comportamento HEATER ON/OFF risultano interpretabili potremo studiare un ulteriore step con temperatura aria maggiore; non è un'autorizzazione a40°C o a una V1 autonoma.

## Cosa cambia e cosa resta uguale

| Voce | S9A eseguito | **S9B proposto** |
| --- | --- | --- |
| Setpoint aria | 25°C (ON quando SHT45<=24°C; OFF quando >=25°C) | **Uguale** |
| **Budget di HEATER ON complessivo** | **120s** | **200s** (l'unica modifica al profilo di calore) |
| Singola accensione, timeout ISR | <=20s, timer GPIO19 20,5s | Uguale |
| OFF minimo / NTC SOFT | OFF>=12s, sospensione R<=50kΩ, ripresa R>=65kΩ | Uguale |
| NTC HARD FAULT | R<=40kΩ o R>=250kΩ | Uguale |
| Limite fault aria | >=40°C | Uguale |
| Durata massima test | 600s /10 minuti | Uguale |
| Ventola | PWM comandato100%, kickstart1s, prerun5s, cooling90s | Uguale |
| **Intervento operatore Sanwa** | **STOP immediato a50°C** | **Uguale** |
| TCO monouso presente | 105°C riferito in serie heater | Nessuna modifica né bypass |

**Evidenza e incertezza della scelta200s:** [review quantitativa](evidence/thermal/2026-10-27-S9B-budget-feasibility-and-safety-review.md). Sui soli ultimi campioni S9A la pendenza aria è circa0,79–1,01°C/min; un'estrapolazione NON validata collocherebbe25°C a t440–491s dall'avvio, entro600s. Non significa che 25°C verranno raggiunti né che il margine termico del box sia noto. Il limite è **proposto per prova supervisionata**, non per V1 o funzionamento unattended.

## Build, firmware, logging

- Repository: `alessandrocardelli/filament-dryer-monitor`; branch dedicato **`experiment/step09-s9b-25c-200s-2026-10-27`**, discendente dal branch S9A ma distinto, senza modificare S9A/PR#14 o `main`.
- Identificazione univoca proposta: firmware **`HOT-261027-S9B`**, OLED **`H27-S9B`**. Non usare `H27-S9A` per questa prova.
- La variante S9B **è fisicamente in grado di comandare GPIO19**. `kHeaterStep09TrialMode=true` ma i tre flag di abilitazione produzione restano FALSE; non equivale a firmware V1.
- CSV completo su RAM1400 righe campionate nominalmente a500ms (capacità fino700s; run<=600s +cooling<=90s), esportato **dopo** HEATER OFF e cooling: `http://filament-dryer.local/api/last-test.csv` oppure IP del dryer. Da conservare con hash, senza ritocchi.
- I cicli del test sono chiamati **finestre ON/OFF**. L'SHT45 misura aria camera; l'NTC in ohm misura la zona heater; il Sanwa legge un punto di contatto vicino all'NTC. Nessuna delle tre misure deve essere uguale.

## Gate prima del flash e prima di qualsiasi HEATER ON

- [x] **CI reale del commit di codice S9B PASS**: compilazione ESP32 core3.3.12/U8g2 2.36.19 e host mock `SafetyController` con soglia25°C dopo >120s HEAT, hard40k/latch FAULT, max200s cumulativi, stop/cooldown e maxRun600s. [Run40560081934](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40560081934) **SUCCESS verificato nei log**, testato sul commit `6b7e67e08e61edc3a06dc8c7eeaf0f0d1313c8e4`; le modifiche ulteriori sono solo ai documenti. [Rapporto codice/CI](evidence/thermal/2026-10-27-S9B-software-build-and-review.md). Entrambe le pipeline mock/Arduino usano `set -euo pipefail`.
- [x] **Review del diff software conclusa** [PR #15 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/15): `maximumTotalHeatMs` da120000 a200000, nuova build ID, test CI/documentazione; NTC hard/soft, timer20,5s, fan100, TCO e scheda invariati. **Questo non sostituisce la verifica fisica degli elementi installati**.
- [ ] **Check fisico sul dryer esistente prima del test:** box chiuso/nessun corpo estraneo, Sanwa correttamente a contatto zona piastra/NTC senza cavi pizzicati, SHT45 aria, ventola libera e realmente rotante, nessun odore/bruniture/danni dopo S9A, cablaggio HEATER e TCO105 in serie invariati, alimentatore24V subito scollegabile.
- [ ] L'utente approva esplicitamente **S9B, target25°C, tetto HEATER 200s e invariati gli altri limiti compreso Sanwa50°C**. La conferma S9A già registrata **NON si applica automaticamente**.

## Operazioni fisiche — **solo quando tutti i gate precedenti sono PASS**

1. Avvia dal dryer freddo, verifica la sonda Sanwa e annota **Sanwa iniziale + SHT45 aria iniziale**. Se SHT45 aria>24°C la logica potrà non accendere: non cambiare la temperatura target per forzare la prova.
2. SOLO in STANDBY e con HEATER OFF/cooling finito, compila/carica OTA il commit del branch S9B revisionato. Controlla che l'OLED visualizzi esattamente **`H27-S9B`**; se non corrisponde, **STOP**, non fare HEAT.
3. Attiva **una sola sessione** con pressione breve ON/OFF, con Sanwa costantemente sorvegliato e ventola fisicamente osservata. Dopo kickstart+prerun deve comparire `HEATER ON`. Se ventola ferma/sospetta, interrompi subito.
4. **Appunti Sanwa da fare ESATTAMENTE così:**

   | Quando | Cosa trascrivere |
   | --- | --- |
   | Prima di premere ON/OFF | Sanwa + aria SHT45 iniziali |
   | All'apparizione del primo `HEATER ON` | Sanwa ON1 |
   | All'apparizione del primo `HEATER OFF` | Sanwa OFF1 + aria SHT45 |
   | A **ogni** nuovo passaggio ON o OFF | Sanwa ONn o OFFn (anche se ripetitivo); se difficile, FILMA insieme Sanwa+OLED e trascrivi dopo |
   | Quando il display SHT45 arriva **per la prima volta a25°C** | Sanwa, lettura SHT45, e se in quel momento HEATER era ON o OFF |
   | Quando interrompi definitivamente (STOP/FAULT/budget) | Sanwa + causa dello stop |
   | Quando la ventola si ferma a fine cooling | Sanwa finale |

   Se il primo/ultimo evento coincide, una misura è sufficiente. **Sanwa va guardato continuamente fra gli eventi**, non solo al momento degli appunti: può muoversi anche durante HEATER OFF.

5. **Al primo raggiungimento di aria>=25°C:** se HEATER è ON e **non** diventa OFF, **interrompi immediatamente**. Se è passato OFF e non mostra anomalie, annota se il passaggio OFF è avvenuto mentre HEATER era ON (cutoff termostatico osservabile), oppure l'aria ha raggiunto25°C quando HEATER era **già OFF per pausa NTC** (caso non sufficiente a dimostrare il cutoff per target). Osserva al massimo altri~10s senza forzare condizioni, poi premi ON/OFF lungo (~1,5s) per STOP e lascia raffreddare. **Non è necessario aspettare un altro ON a24°C per questa prima verifica**.
6. Se25°C non viene raggiunto, lascia terminare automaticamente a budget200s o timeout10min, SENZA aumentare nessun limite; annota quale evento conclude. Se compare FAULT, segna il testo. Non fare ulteriori sessioni per inseguire il target.
7. **VETO SANWA50°C: interrompi SUBITO senza attendere il prossimo HEATER OFF.** Interrompi anche per ventola ferma, riscaldamento inatteso, odore, plastica/cablaggio sospetti o anomalie. STOP lungo durante DRYING; **in emergenza togli alimentazione24V**. Non scollegare sonde/parti sotto tensione.
8. Osserva il postheat fan100/cooling previsto90s, Sanwa che scende o rimane stabile e **Sanwa quando si ferma la ventola**. Se non riesci a salvare il CSV, annota errore: **non ripetere la prova solo per il file**.
9. Scarica e allega il **NUOVO** CSV `/api/last-test.csv` solo dopo raffreddamento, identificando H27-S9B. Non sovrascrivere il raw S9A; conservare letture Sanwa e dati separati su GitHub.

## Esiti ammessi e cosa NON dichiarare

- **A:** raggiunto target25 mentre HEATER ON e osservato HEATER OFF — test fisico del cutoff SHT45 promettente; non significa V1 sicura.
- **B:** aria raggiunge25 mentre HEATER era già OFF — temperatura raggiunta, ma cutoff per setpoint non isolato dal veto NTC; serve analisi, non forzare nuove accensioni.
- **C:** aria non raggiunge25 entro budget200s o max10min — limite sperimentale raggiunto, nessun test ripetuto automaticamente.
- **D:** FAULT o STOP veto — tutela primaria, registrare Sanwa/CSV/contesto; non riprovare in caso di guasto.

Sorgenti da aggiornare a prova realmente eseguita: `docs/evidence/thermal/2026-10-27-S9B-PHYSICAL-SESSION.md` (verbale predisposto), futuro CSV originale `docs/evidence/thermal/2026-10-27-S9B-raw.csv` solo se fornito, report analisi separato, `docs/THERMAL_EXPERIMENT_LEDGER.md`, `docs/PROJECT_STATE.md`, `docs/TODO.md`, `README.md`, `AGENTS.md`. Nessuna approvazione termostato V1 implicita.
