# Diagnostica statica GPIO16 / R25 / Q5 / J6

**Stato: sorgente di diagnostica nuovo, NON compilato o provato sulla scheda in questa sessione.** Non è lo sketch originale del collaudo 2026-10-08. Il programma corrente di produzione rimane invariato.

## Motivo

Durante il test DRYING della scheda assemblata:
- J6 pin 2 ≈ +24 V rispetto a GND.
- J6 pin 1 ≈ 23 V in STANDBY e in DRYING.
- R25 pin 1 / gate Q5 ≈ 0 V; R25 pin 2 / GPIO16 ≈ 0 V durante DRYING.
- U4 pin fisico 27 verso R25 pin 2 ≈ 0,3 Ω a scheda spenta.
- Toccare U4 pin 27 con puntale positivo DMM avrebbe fatto partire la ventola.
- La versione LEDC diagnostica riportava configurazione e scrittura apparentemente riuscite, non il GPIO elettricamente verificato.

L'obiettivo è eliminare il PWM e l'interazione con il buzzer: `digitalWrite(GPIO16, HIGH)` rende le misure DC non ambigue.

## Sicurezza

1. **Scollegare tutti gli alimentatori, USB incluso, e ISOLARE fisicamente il riscaldatore J5 prima del caricamento/prova**. Il programma forza GPIO19 LOW, ma non protegge da un MOSFET heater in corto né da errori di flash.
2. Verificare ventola 24 V collegata correttamente a J6, NTC/sensore solo se interessano a un test separato. Questo sketch non usa i sensori.
3. Non utilizzare un puntale metallico sul pin 27 dell'ESP32 alimentato: rischio corto con pad adiacenti. Posizionare eventuali sonde isolate o pinze a scheda SPENTA sui pad di R25/J6.
4. Usare alimentatore 24 V con limite di corrente appropriato e possibilità di interruzione rapida; sorvegliare il test.

## Procedura

- Aprire nell'Arduino IDE la cartella `fan_gpio16_static_check` con il file omonimo `.ino`.
- Flash core ESP32 Arduino 3.x.
- Monitor seriale **115200 baud**.
- 0–3 secondi: OFF; 3–18 s: GPIO16 HIGH; dopo 18 s: OFF definitivo (riparte solo dopo reset).
- Durante la finestra HIGH: verificare **movimento ventola** e, se misure accessibili in sicurezza, tensione DC rispetto a GND di **R25 pin 2** (lato U4: attesi circa 3,3 V), **R25 pin 1** (lato gate Q5: attesi circa 3,3 V), **J6 pin 1** (uscita switch Q5: attesi circa 0 V a MOSFET acceso con ventola collegata). J6 pin 2: circa +24 V sempre.

**Tabella decisionale (valori indicativi):**

| R25 pin 2 | R25 pin 1 / gate | J6 pin 1 | Interpretazione |
|---|---|---|---|
| ~3,3 V | ~3,3 V | ~0 V e fan ON | Comando digitale e path Q5 funzionano; indagare gestione PWM/LEDC dell'app |
| ~0 V | ~0 V | ~23–24 V | GPIO16 non porta HIGH oppure rete caricata in corto/contatto difettoso; verificare registro del flash, MCU, pad e percorso senza attribuire a priori |
| ~3,3 V | ~0 V | ~23–24 V | Possibile problema R25/saldature oppure forte carico/corto lato gate |
| ~3,3 V | ~3,3 V | ~23–24 V | Sospetto Q5, sua saldatura o rete di potenza J6 |

Se ventola parte solo applicando un puntale sul modulo ESP32, annotare il fatto senza riprodurlo rischiando cortocircuiti: anche pressione meccanica e riferimento del puntale DMM possono modificare la prova. Un multimetro in AC/DC va controllato per impostazione corretta. La continuità unpowered di 0,3 Ω non esclude un contatto intermittente.

Il comando `digitalWrite` non è una misura di tensione: il risultato elettrico va verificato sul pad o con uno strumento.

## Provenienza

- `docs/PROJECT_STATE.md`: collaudo ventola ottobre 8, sostituzione Q5, test PWM; regressione ottobre 19.
- `firmware/test_archive/TEST_FIRMWARE_ARCHIVE.md`: sorgenti storici esatti contro ricostruzioni.
- `fix/isolated-fan-ledc-2026-10-19`: candidato a parte, non convalidato.
