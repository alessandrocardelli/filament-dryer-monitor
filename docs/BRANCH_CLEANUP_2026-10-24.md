# Pulizia e consolidamento branch — 24 ottobre 2026

> Historical protocol/technical record. Its dated NEXT and pre-test status are not the current work queue; see [PROJECT_STATE](PROJECT_STATE.md) and [TODO](TODO.md). Original checkpoint preserved in archive tags.

## Risultato eseguito

**Prima: 12 branch; dopo: 6 branch.** Operazione deliberatamente conservativa: ogni eliminazione è stata verificata con antenato Git oppure con identità dei file già mergiati. Nessun ref con modifiche hardware o diagnostiche non integrate è stato eliminato.

### Merge completato

- **PR #3**, `feature/heater-policy-cold-tests-2026-10-23` → `main`: [Merge 78278575dd97cb7b244ffe46df1bd87f80b73221](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/3).
- Sono confluiti i documenti di qualifica/D029, il modulo **non attuante** `firmware/HeaterControlPolicy.h` e 14 test C++ host. Non sono stati portati in `main` i 3 impulsi sperimentali, Wi-Fi OTA, né il controllo riscaldatore non convalidato.
- Verifica blob pre/post merge: `firmware/AppConfig.h`, `firmware/firmware.ino` e `firmware/SafetyController.h` sono **identici**. Main mantiene firmware Safe Bring-up e nessuna attuazione heater in questo merge.

### Branch eliminati sul remoto (6)

| Branch | Verifica prima di cancellare |
|---|---|
| `redesign/buck-sourcing` | antenato di `main` |
| `plan/closed-heater-qualification-2026-10-20` | antenato sia del ramo attivo sia del merge PR #3 |
| `feature/heater-policy-cold-tests-2026-10-23` | PR #3 mergiata, antenato del ramo attivo |
| `feature/integrated-heater-controller-2026-10-23` | antenato del ramo attivo |
| `test/closed-case-heater-pulses-2026-10-23` | antenato del ramo attivo |
| `archive/recover-tested-firmware-2026-10-19` | PR #2 già mergiata; tutti i file di `firmware/test_archive` equivalenti a `main` (controllo `git diff --quiet`) |

La cancellazione remota dei 6 riferimenti è confermata dai log del workflow GitHub Actions [run 40109675713](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40109675713). Il workflow temporaneo è stato rimosso subito dopo; non resta un processo automatico di cancellazione branch. I commit continuano a esistere attraverso la storia Git dei branch discendenti o i merge documentati.

### Branch conservati (6)

| Branch | Motivazione |
|---|---|
| `main` | base conservativa, firmware Safe Bring-up con policy pura mergiata |
| `feature/wifi-ota-web-logging-2026-10-24` | **branch corrente della PCB:** Wi-Fi, pagina web, OTA da Arduino IDE e test heater 3 × 1,5 s validati dall'utente. NON firmware termostatico V1 definitivo |
| `diagnostics/fan-gpio16-static-2026-10-20` | sorgente autonomo e note diagnostiche non integrate; mantenerlo finché la sua utilità storica non è verificata |
| `fix/isolated-fan-ledc-2026-10-19` | divergenza con modifiche FanController/firmware non presenti per identità nella linea attuale; merge automatico rischia regressione |
| `pcb/l7987l-layout` | PR #1 già mergiata a suo tempo ma punta a un ulteriore draft `docs/FIRST_ARTICLE_TEST_MATRIX.md` differente: non assumere che sia già integrato |
| `variant/mouser-footprints` | variante storica KiCad/footprints distinta dalla scheda corrente, con file hardware unici: **non** mergiare o cancellare senza revisione delle differenze schematiche/PCB |

## Implicazioni per le attività successive

- Il lavoro di **regolazione termica continua** deve partire dal branch `feature/wifi-ota-web-logging-2026-10-24` o un suo discendente, che contiene il firmware effettivamente collaudato sulla PCB, non da `main` come se fosse aggiornato.
- Consultare i commit per i vecchi nomi di branch ora eliminati, anziché usare URL `/tree/<branch>` non più validi.
- Conservare `kHeaterOutputEnabled=false` e `kHeaterCommissioningMode=true` finché non sono caratterizzati NTC, condizioni di temperatura della camera, limiti/fault e una prova a caldo graduale. Non promuovere il firmware sperimentale in `main` per mera pulizia branch.
- La voce `12h ambient history` è stata esplicitamente **rinviata** dall'utente: priorità all'essiccatore che mantiene la temperatura impostata.
