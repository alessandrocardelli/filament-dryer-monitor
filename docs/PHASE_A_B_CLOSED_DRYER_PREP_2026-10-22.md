# Fasi A e B — predisposizione dryer chiuso e verifica TCO

**2026-10-22 — Bozza operativa, NON un collaudo superato né autorizzazione ad attivare il riscaldatore.**  
**Scope richiesto:** solo (1) disposizione del dryer chiuso e (2) protezione termica. Si esclude la discussione del pannello frontale. Nessuna modifica firmware/KiCad/DECISIONS.

## Fase A — disposizione per prova termica a dryer CHIUSO

**Prove precedenti non da ripetere:** Q4/J5 e heater con carico reale, ventola PWM, SETUP/SETTINGS, SHT45/NTC display già testati. Le sette coppie NTC-Sanwa 30–85 °C misurate **fondo e coperchio aperti, ventola OFF** restano benchmark **aperto**, non prestazione chiusa. La configurazione chiusa non è ancora accettata.

A dispositivo spento (24 V e USB scollegati):
1. **SHT45/J3**: installazione nel volume d'aria rappresentativo della zona bobina, non a contatto con piastra/parete calda e non sotto irraggiamento diretto né immediatamente nel getto locale ventola. L'attuale posizione non è documentata con certezza; fare una foto dell'interno *solo se necessaria* per validarla. Adafruit SHT45 esistente non va cambiato arbitrariamente. La calibrazione 19.4 °C / 69.7% RH precedente prova solo che legge, non che rappresenti aria distribuita.
2. **Sanwa RD701 + tipo-K**: mantenere per la comparabilità una misura **di contatto della piastra riscaldante nello stesso punto** circa dei collaudi aperti; fissare la punta in modo da non scivolare o toccare parti elettriche. Instradare il cavo senza pizzicamento dei giunti e senza lasciare il box aperto in modo significativo. Distinguere T_piastra Sanwa da T_NTC interna e T_aria SHT45.
3. **Cablaggi**: J5 heater e TCO, J6 fan, J7 NTC, J3 SHT45, crimps e guaine lontani da pale e punti caldi, trattenuti senza trazione. Verificare gioco/chiusura del coperchio inferiore e superiore e nessun isolamento danneggiato. Non muovere il TCO durante queste verifiche senza documentarne la nuova posizione.
4. **Fan**: verificare accesso aria/ricircolo in camera e assenza di ostruzione, non ripetere il già superato test PWM da banco. Il comando FAN 40% non è una misura di flusso; la scheda non ha feedback tachimetrico.
5. **Preparazione misure**: annotare dov'è il Sanwa e se rilevare anche la **temperatura del corpo TCO** richiederebbe una seconda sonda oppure una prova distinta. Stabilire un modo immediato per interrompere 24 V, senza far passare puntali metallici esposti tra contatti.

**Gate A per procedere alla valutazione della prova a caldo**: configurazione *realmente chiusa* senza fili schiacciati, SHT45 in aria di camera, Sanwa a contatto verificabile con piastra, TCO mantenuto in serie e riconoscibile, percorso fan libero. Accettazione solo su osservazione reale dell'assemblato; attualmente OPEN.

## Fase B — protezione indipendente del riscaldatore: valutazione concreta

### Stato noto

- TCO esistente: **marcatura riferita 105 °C / 15 A / 250 V**, monouso, corpo contro flex heater fissato con Kapton, in serie a HEATER+ tra J5 pin 2 e riscaldatore (2026-10-14; ripristino 2026-10-16). Il modello completo e il datasheet non risultano identificati nel repo.
- Il manuale OEM eSUN eBOX Lite riporta per la piastra **105/116/118 °C** ai livelli 1/2/3 e per l'aria interna **40/50/55 °C**, da non confondere con T_NTC. Condizioni di riferimento 20 °C / 55% RH. Fonte nel progetto: `docs/PROJECT_STATE.md`; copia manuale indicizzata: https://www.esunchina.net/uploads/eBOX-Lite-Manual.pdf.
- Ultimo collaudo termico APRI+fan OFF: Sanwa adiacente piastra 85 °C, NTC indicato 90 °C, nessuna dimostrazione dei livelli OEM con scatola chiusa e fan ON.

### Valutazione, NON decisione

Un cutoff da **Tf nominale 105 °C** a contatto diretto con una piastra che dovrebbe potenzialmente raggiungere circa 118 °C potrebbe aprire **durante funzionamento normale**: incompatibilità probabile nel posizionamento attuale, ma senza la temperatura del **corpo TCO** nel regime chiuso e la scheda tecnica non possiamo quantificarla. Non dedurre che la sua temperatura coincida con la temperatura di riferimento OEM presa altrove.

Il solo valore **Tf** (temperatura nominale di intervento) non basta: va verificato **Th** (massima temperatura continuativa garantita in conduzione), oltre a Tf reale/tolleranza, **Tm** (temperatura ammissibile post-intervento), corrente, effettiva idoneità a interrompere **24 V DC circa 1.6 A**, eventuali omologazioni e limiti meccanici/termici dei terminali/crimp. Un rating 250 V/15 A indicato senza datasheet non specifica necessariamente le capacità **DC**. Il Tf nominale non è una temperatura di normale esercizio garantita.

Fonti tecniche di consultazione:
- SETsafe/SETfuse, *Over Temperature Protection*: https://www.setfuse.com/Products/Over-Temperature-Protection.html — Tf/Th, declassamento e misura reale al punto di installazione.
- Hiwachina, tabella SEFUSE SF/R: https://www.hiwachina.com/en/proshow.asp?id=268 — esempi di rating Th inferiori a Tf per tipi specifici; NON identificazione del nostro componente.

### Conclusione provvisoria e decisione non ancora presa

**Non approvato** tentare 118 °C piastra nella configurazione attuale finché TCO/protezione/limiti materiali non siano verificati. Non bypassare né rilocare liberamente il fusibile da 105 °C e non scegliere automaticamente 120/130/140 °C: un componente che evita falsi interventi a regime potrebbe proteggere troppo tardi i materiali in caso di guasto.

Prima di decidere una revisione di TCO o del suo posizionamento:
- documentare **foto ravvicinata con intera sigla/omologazioni del TCO**;
- documentare **punto fisico di contatto** sul riscaldatore, rispetto alla sonda Sanwa e ai materiali vulnerabili, a impianto spento;
- ottenere scheda tecnica e specifiche complete (Tf/Th/Tm e portata DC se pertinente);
- definire quali temperature massime sopportano i componenti nei punti esposti e che protezione originale abbia/avesse il donor;
- decidere protezione indipendente idonea e registrare ogni variazione in `docs/DECISIONS.md` **solo dopo approvazione**, senza bypass.

**Gate B attualmente OPEN.** Solo quando A e B sono soddisfatti passeremo al firmware sperimentale di riscaldamento e, separatamente, al test termico chiuso sorvegliato. La prova a 118 °C rimane obiettivo prestazionale di confronto OEM, non preset NTC né autorizzazione automatica al riscaldamento.
