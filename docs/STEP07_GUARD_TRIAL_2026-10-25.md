# STEP07-G — un solo esperimento di arresto NTC anticipato

**Sorgente:** branch `experiment/step07-ntc-guard-verified-history-2026-10-25`, [PR #11](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/11) draft. **Firmware `HOT-251025-S7G` / OLED `H25-S7G`**. Deriva dall'S6C fisicamente provato senza modificare componenti o KiCad. Questo test NON è il termostato automatico di essiccazione.

**Fondamento storico:** come già registrato il 15 ottobre in PROJECT_STATE e FIRST_ARTICLE_TEST_MATRIX, il Sanwa vicino alla piastra ha mostrato durante la salita **ritardo/gradiente rispetto alla precedente indicazione NTC**; a cutoff diagnostico 50°C NTC indicato, il Sanwa indicava44°C, e le differenze si invertirono in cooling. Nel test fisico S6C (25ott), box chiuso + ventola100%, a 6×20s ON, Sanwa superficie max53°C (30°C a fine cooling), R_NTC minima30,32kΩ, mentre l'aria SHT45 ha raggiunto23,98°C a fine cooling. La **vecchia soglia di abort in 19,68kΩ** (Sanwa56°C stabilizzato in box aperto fan OFF) non ha fornito protezione rispetto al veto manuale55°C.

## Nuova build sperimentale S7G — cosa cambia e cosa non cambia

| Campo | S6C provato | S7G pronto da revisionare |
|---|---|---|
| Fan PWM | 100% | **100%** senza altri cambi |
| HEATER finestre MAX | 6 ×20s | **6 ×20s** MAX, stop anticipato all'NTC |
| Gap nominale | 12s | 12s |
| Timer indipendente GPIO LOW per singola finestra | 20,5s | 20,5s |
| Cooldown | 90s | 90s |
| Tempo massimo sessione | 330s | 330s |
| ARIA SHT45 corte | >=40°C | >=40°C (NON T piastra) |
| NTC min R abort | **19,68 kΩ** | **40,00 kΩ, blocco FAULT latched** |
| Altro | fault/stale fan command/NTC/SHT45/STOP | invariato |
| Termostato V1 autonomo | OFF | **OFF** |

Il salto di R da19,68k a40k è **un anticipo precauzionale dell'abort**, non calibrazione in °C del termistore. Sulla curva S6C autentica, R_NTC ha incrociato40kΩ **t ≈89,897s dall'inizio del CSV, durante l'ON numero3**; perciò S7G dovrebbe fermare prima, ma il momento reale dipende da condizioni iniziali, contatto, airflow e iterazioni. **Non è garantito** un Sanwa max<55°C né l'assenza di hotspot: Sanwa ha ritardo già osservato e il suo dato non entra nella MCU. Non forzare il completamento di sei impulsi se FAULT scatta presto: quel FAULT è lo **scopo** del test.

## Gate fisico da verificare prima del caricamento/prova

1. Solo dopo CI PASS sullo SHA preciso e lettura completa del report storico [REPOSITORY_FULL_REVIEW_2026-10-25.md](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/badf8dc30b9f16949a406feee80fff9a21edbcd7/docs/REPOSITORY_FULL_REVIEW_2026-10-25.md). La creazione del ramo NON equivale ad approvazione di un nuovo riscaldamento; condividere con utente limiti numerici e attendere consenso esplicito alla prova. **Nessun invio/flash automatico dalla CI**.
2. Stessa geometria S6C: dryer freddo con entrambi i coperchi chiusi, Sanwa fissato sulla stessa area e leggibile, sensori J3/J7 validi, ventola che *muove davvero aria* (PWM100% non è tach), cavi Sanwa/DC nel foro jack ben protetti, TCO105 esterno attualmente in serie ancora presente, alimentazione24V interrompibile immediatamente. Non aprire/pizzicare cavi durante alimentazione.
3. Dopo approvazione, flusso **GitHub Desktop Fetch/Pull → selezione branch S7G → Arduino IDE apre `firmware/firmware.ino` → porta OTA già collaudata**. Visualizzare `H25-S7G` dopo upload. Non usare per default la web OTA o file ZIP. **Una sola** sessione di verifica, supervisionata, non una serie di prove alla ricerca di40°C aria.
4. Perché Sanwa è ritardato, annotare **valore Sanwa e secondo/impulso** almeno all'avvio, appena prima del 3° impulso e nel momento dell'arresto automatico; osservare anche il suo **massimo dopo heater OFF** (video con display Sanwa/OLED e orologio visibili è utile, non una condizione che giustifichi esposizioni più lunghe). Veto manuale Sanwa a55°C *al massimo storico*, interrompere prima a qualunque anomalia; 50°C come livello cautelativo ulteriore da discutere prima della sessione. Se il contatto è incerto, fermare subito e non inferire sicurezza da una lettura bassa.
5. Attendere cooldown, scaricare `/api/last-test.csv` da dashboard al termine, segnalare **FAULT NtcResistanceLimit** e Sanwa istantaneo/max post-OFF; non tentare riavvio senza analisi. Anche dopo fault la ventola rimane comandata durante il cooldown se alimentata. In emergenza staccare24V prioritariamente.
6. Il protocollo serve a qualificare **l'efficacia dinamica di un arresto elettrico anticipato**, non la produttività del dryer. Se il Sanwa post-stop sale molto o intervengono condizioni anomale, bloccare la progressione e valutare fissaggio/gradiente/limiti di protezione.

**Prodotto V1 rimane OPEN:** setpoint aria40°C SHT45, controllo termostatico continuo, protezione automatica piastra NTC valida nel box in tutte le condizioni, timer hardware continuo, flusso aria reale e gestione TCO105 vs piastra nominale OEM105/116/118°C non dimostrati. Non cambiare D025/D029 senza nuova decisione.
