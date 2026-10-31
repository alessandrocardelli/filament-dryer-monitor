# HOT-261031-N8580 — prova fisica AIR55 del 31 ottobre 2026

## Identità e condizioni realmente note

- **Esito:** sessione completata per scadenza temporizzatore 60 minuti; **AIR55 non raggiunta**. AIR massima **45,84°C**; Sanwa PLATE massimo **83°C**. Il CSV dimostra un solo HEATER ON continuo dopo prerun e un solo OFF a scadenza, senza pause NTC, riarmi o fault registrati.
- **Firmware attribuito:** esperimento corrente `HOT-261031-N8580` / `H31-8580`, sorgente compilata `1cd96dd76500dfb4d161e1b445d8df6c9d8e524b`, [CI#41142714373 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/41142714373), branch `experiment/step10-heater-diagnostics-air25-2026-10-28`. **Limitazione:** CSV privo di campo firmware/build ID; l'identità installata NON è verificata direttamente nel file, ma il comportamento osservato è coerente con tale firmware.
- **Operatore:** prova riferita in chat il **31 ottobre 2026**, zona Europa/Roma. L'ora assoluta START e STOP NON è stata fornita; la colonna `ms` è il tempo da accensione MCU, non un timestamp assoluto.
- **Impostazione desiderata:** AIR 55°C per 60 minuti, secondo contesto operatore e scadenza CSV; campi setpoint/durata del foglio Google non compilati e CSV non contiene il setpoint, quindi NON sono independentemente verificati dalla telemetria.
- **Geometria:** eSUN eBOX Lite modificato, vano filamento **vuoto**, temperatura stanza stimata **16–17°C** durante la prova, non misurazione precisa né condizione iniziale certificata. Non è stata comunicata una nuova posizione del Sanwa: riferimento alla sonda Sanwa RD701/K sul contatto PLATE vicino NTC già documentato. Stato chiusura effettiva della camera in questa sessione NON esplicitamente riconfermato; non inventare dato.
- **Sensori indipendenti:** Sanwa = contatto PLATE esterno, registrato manualmente; `ntc_ohms` = resistenza NTC incorporata nel riscaldatore, telemetria ADC; `air_c/rh` = SHT45 in camera. Nessuno dei tre indica la temperatura esatta del corpo del TCO.
- **Ventilazione:** CSV registra `fan_percent=100` durante prerun/risc. e nei 90 secondi di cooldown, poi 0. Campo PWM comando, NON misura corrente, portata o rotazione fisica; RPM non registrati.
- **Hardware di sicurezza:** TCO 105°C indipendente in serie secondo documentazione/precedenti rapporti operatore; nuova verifica fisica del corpo/cablaggio, temperatura del TCO, limiti dei materiali e capacità d'interruzione DC NON registrata. Non è stato rimosso o bypassato dall'assistente.

## Evidenze originali e riproducibilità

1. [CSV originale byte-identico](2026-10-31-AIR55-N8580-raw.csv), caricato come `filament-last-test (1)(1).csv`: **747 righe dati**, 15 colonne, **41.578 byte**, LF e newline finale conservati, SHA256 **`902948a50d920feb1b11af65cbff4982636108ce481a55c66c7d3f3c8b4ab8b4`**, Git blob SHA1 **`9e6940b3c57e6a400e85392c6e19eb452ab669d2`**.
2. [Foglio Sanwa operatore](https://docs.google.com/spreadsheets/d/1I5utkzGP1zvD168jLghpYB_y_myGjxMb1ccLens4Cks/edit?gid=525622499), scheda **Nuova prova** al momento dell'analisi. Qui sotto le **37 letture** prima/durante il ciclo, più **4** post-STOP, conservate testualmente perché il foglio è riutilizzabile e potrà essere modificato.
3. **Allineamento:** `ms=237518` è la prima registrazione in prerun/approssimazione di `0:00`; `ms=243518` prima riga HEATER ON dopo prerun; `ms=3837518` prima riga cooldown/HEATER OFF, esattamente **3600 s** dopo la prima riga. Le letture Sanwa sono manuali ai minuti indicati, non timestamp di misura sincronizzati in hardware. Ogni riga NTC/AIR è il campione CSV con `ms` più vicino al minuto, quindi è una **corrispondenza temporale approssimata**, NON una coppia termometrica simultanea garantita.

## Cronologia: Sanwa indipendente affiancato a NTC e AIR reali

| Da START / dopo OFF | Sanwa PLATE °C | NTC Ω circa | SHT45 AIR °C | RH % | Stato CSV |
| --- | ---: | ---: | ---: | ---: | --- |
| Prima di START | 15 | — | — | — | Sanwa indipendente prima di START |
| 0:00 | 15 | 149025.3 | 16.15 | 62.84 | FAN PRERUN |
| 1:00 | 31 | 39682.4 | 16.96 | 64.29 | HEATER ON |
| 2:00 | 41 | 26338.7 | 19.16 | 61.24 | HEATER ON |
| 3:00 | 47 | 21458.5 | 21.59 | 55.60 | HEATER ON |
| 4:00 | 51 | 18628.6 | 24.04 | 49.72 | HEATER ON |
| 5:00 | 55 | 16619.8 | 26.34 | 44.68 | HEATER ON |
| 6:00 | 58 | 15245.2 | 28.39 | 40.60 | HEATER ON |
| 7:00 | 60 | 14066.7 | 30.21 | 37.28 | HEATER ON |
| 8:00 | 62 | 13244.7 | 31.89 | 34.55 | HEATER ON |
| 9:00 | 64 | 12521.2 | 33.38 | 32.44 | HEATER ON |
| 10:00 | 66 | 11933.6 | 34.70 | 30.66 | HEATER ON |
| 12:00 | 69 | 11072.4 | 36.89 | 27.87 | HEATER ON |
| 14:00 | 72 | 10361.4 | 38.63 | 25.98 | HEATER ON |
| 16:00 | 73 | 9932.7 | 39.96 | 24.60 | HEATER ON |
| 18:00 | 75 | 9528.0 | 41.03 | 23.62 | HEATER ON |
| 20:00 | 76 | 9164.9 | 41.89 | 22.95 | HEATER ON |
| 22:00 | 77 | 8966.1 | 42.63 | 22.57 | HEATER ON |
| 24:00 | 78 | 8758.9 | 43.20 | 22.25 | HEATER ON |
| 26:00 | 79 | 8594.5 | 43.68 | 21.96 | HEATER ON |
| 28:00 | 79 | 8491.4 | 44.09 | 21.61 | HEATER ON |
| 30:00 | 80 | 8422.6 | 44.39 | 21.28 | HEATER ON |
| 32:00 | 80 | 8345.5 | 44.64 | 21.11 | HEATER ON |
| 34:00 | 81 | 8272.2 | 44.78 | 20.99 | HEATER ON |
| 36:00 | 81 | 8242.2 | 44.92 | 20.95 | HEATER ON |
| 38:00 | 81 | 8209.8 | 45.04 | 20.88 | HEATER ON |
| 40:00 | 81 | 8160.8 | 45.14 | 20.78 | HEATER ON |
| 42:00 | 82 | 8059.4 | 45.23 | 20.67 | HEATER ON |
| 44:00 | 83 | 8083.2 | 45.33 | 20.63 | HEATER ON |
| 46:00 | 82 | 8070.1 | 45.33 | 20.59 | HEATER ON |
| 48:00 | 83 | 8028.4 | 45.47 | 20.47 | HEATER ON |
| 50:00 | 83 | 8005.8 | 45.57 | 20.31 | HEATER ON |
| 52:00 | 83 | 7982.1 | 45.64 | 20.22 | HEATER ON |
| 54:00 | 83 | 7976.1 | 45.68 | 20.16 | HEATER ON |
| 56:00 | 83 | 7973.8 | 45.74 | 20.08 | HEATER ON |
| 58:00 | 83 | 7922.8 | 45.78 | 20.02 | HEATER ON |
| 60:00 | 83 | 7973.8 | 45.82 | 19.97 | COOLDOWN / timer scaduto |
| OFF +0:30 | 72 | 15115.9 | 45.67 | 19.96 | COOLDOWN; fan 100% |
| OFF +1:00 | 64 | 20849.1 | 45.12 | 20.22 | COOLDOWN; fan 100% |
| OFF +1:30 | 59 | 25303.7 | 44.36 | 20.63 | COMPLETE; fan 0% |
| OFF +2:00 | 58 | 26523.7 | 43.62 | 21.12 | COMPLETE; fan 0% |

Nessuna lettura Sanwa presente per `OFF +3:00` o `OFF +5:00`; non inferire un valore. Nessuna nota cronologica inserita in C sulle righe. L'operatore ha riferito durante la prova HEATER sempre ON e al termine che Sanwa non ha mai raggiunto85°C, AIR55 non raggiunta.

## Analisi quantitativa originale (CSV + foglio)

| Variabile | Misura verificata |
| --- | --- |
| Campionamento | 747 righe CSV, 15 colonne, uptime MCU da237518 a3957518ms (span3720s); campioni circa ogni5s con registrazioni aggiuntive a cambio fase |
| Prima fase | Fan prerun/HEATER OFF fino a `ms=243518` |
| Comando HEATER | Esattamente **1 transizione OFF→ON** e **1 ON→OFF**, `ms=243518`.. `3837518`, durata comandata **3.594,000 s** (~59min54s); nessun OFF intermedio |
| Terminazione | `phase=4` e `heater_command=0` a +60:00 (COOLDOWN); `phase=5` dopo90s (COMPLETE). Quindi il normale timer e cooldown risultano effettivamente nel file |
| SHT45 AIR | inizio **16,15°C**, massimo **45,84°C**, allo spegnimento **45,82°C**, alla fine log dopo120s **43,62°C**; **setpoint AIR55 non raggiunto** |
| RH SHT45 | inizio **62,84% RH**, minimo **19,90% RH** vicino fine ON, allo spegnimento **19,97%**, ultima **21,12%** (aria si raffredda); non prova estrazione umidità da filamento assente |
| NTC incorporata | primo **149.025,3Ω**, minimo globale **7.920,4Ω** negli ultimi secondi ON, al timer stop **7.973,8Ω**. Non ha raggiunto `<=7.120Ω` per pausa locale né `<7.120Ω` per fault |
| Esempi a Sanwa79°C | +26:+28min → NTC **8594,5Ω /8491,4Ω** |
| **Sanwa80°C** | +30:+32min → NTC **8422,6Ω /8345,5Ω** (media campionata **8384,05Ω**); confronto con precedente interpolazione OPEN/fanOFF di8236,6Ω per Sanwa80, NON una nuova temperatura calibrata del bead |
| Sanwa81°C | +34:+40min → NTC **8272,2→8160,8Ω**, quattro letture a tempo; attraversamento 8236,6Ω durante il riscaldamento **non provoca OFF** perché è la soglia di RIARMO, non quella di pausa |
| Sanwa82°C | +42 e+46min → NTC **8059,4 /8070,1Ω** |
| **Sanwa83°C** | +44,+48,+50,+52,+54,+56,+58,+60min → NTC **7922,8..8083,2Ω**. La massima temperatura Sanwa campionata resta83°C |
| Gradiente termico | a +60:00 Sanwa83°C vs AIR45,82°C = **37,18°C** nei due punti differenti, non temperatura esatta di corpo NTC/TCO |
| Rallentamento | AIR +40min **45,14°C** → +60min **45,82°C**, solo **+0,68°C** in20min **nonostante HEATER ON continuo**. Forte indizio di avvicinamento a equilibrio termico in questa configurazione; non prova l'esatto valore asintotico |
| Protezioni software registrate | `pulse_fault=0`, `ntc_fault=0`, `air_valid=1`, `ntc_valid=1` in tutte le747 righe; `loop_gap_ms=1` nei campioni del log. Un CSV sano non prova watchdog ISR, sensori sotto fault, corrente heater o RPM reali |
| Afterheat / Sanwa | dopo OFF **+30s72°C; +60s64°C; +90s59°C; +120s58°C** (solo letture documentate); nessun aumento post-OFF osservato ai tempi campionati. Tra un campione e l'altro non è possibile escludere variazioni |

### Confronto storico eSUN, senza promozione a specifica

Il [manuale originale eSUN eBOX Lite](https://www.esun3d.com/uploads/eBOX-Lite-Manual.pdf) riporta per livello3 *Hot plate118°C* e *Inside55°C* in ambiente20°C e55% RH. Il nostro Sanwa misura **un punto vicino alla NTC**, che non è dimostrato essere lo stesso punto della Hot plate del costruttore; in questa prova ambiente16–17°C, camera vuota e ventilazione comandata100%. Queste differenze **non permettono né di smentire i dati eSUN né di autorizzare118°C nel prototipo modificato**.

## Esito e prossimo gate

**Provato:** il nuovo controllo non impone più pause artificiali20s; termostato locale7.12kΩ NON raggiunto, quindi OFF e riarmo NTC **non collaudati**. L'AIR non arriva a55°C in60min; era quasi stabile a45–46°C. Timer e90s cooldown funzionano nel perimetro registrato, sensori validi, senza fault dichiarati. Nessuna prova che si mantengano55°C né che il prodotto sia sicuro per uso autonomo.

**Prossima sola attività prioritaria:** revisione tecnico-termica dell'insieme **potenza elettrica realmente disponibile al riscaldatore, circolazione e dispersioni, differenza tra contatto Sanwa e NTC, posizione/protezione effettiva TCO105 e limiti dei materiali**. Separare il limite artificiale sperimentale NTC7.12kΩ dal livello di potenza fisicamente ottenibile; non alzare soglie, non modificare firmware termico e non eseguire un'altra prova calda senza proposta documentata, analisi dei rischi e autorizzazione fisica. Richiesta futura: web dashboard aggiornabile durante HEATER ON senza ritardare il controllo, con validazione di carico/tempi e OTA disabilitata durante heat.

## Informazione emersa dopo l'analisi, 31 ottobre 2026

**Segnalazione dell'operatore:** dopo aver confrontato fotografie dell'interno dell'eSUN eBOX Lite originale, l'utente ha riconosciuto di aver **montato la ventola centrifuga al contrario** nel prototipo usato per questa prova. È un'osservazione fisica riferita dall'utente, non verificata dall'assistente tramite foto dell'assemblaggio; al momento della segnalazione **non risulta comunicata la correzione né una prova a freddo del nuovo orientamento**.

**Impatto sulle conclusioni:** il CSV ha rilevato il solo **comando PWM ventola 100%**, senza misurare verso, portata o flusso reale. Il montaggio inverso è una spiegazione plausibile e potenzialmente dominante della differenza fra PLATE83°C e AIR45.84°C, ma non è ancora dimostrata quantitativamente. Il dato sperimentale originale e la sua analisi rimangono validi per la configurazione **con ventola montata al contrario**; non devono essere presentati come prestazione termica della configurazione OEM ricostituita.

**Next:** ripristino meccanico a dispositivo scollegato e raffreddato; verifica del flusso **con heater assicurato OFF**; solo successivamente eventuale test termico distinto, esplicitamente autorizzato e preparato. Nessun cambio a firmware, NTC o TCO consegue automaticamente dalla scoperta.
