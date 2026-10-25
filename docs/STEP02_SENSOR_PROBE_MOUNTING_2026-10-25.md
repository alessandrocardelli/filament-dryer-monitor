# Step 02 — ispezione fisica e fissaggio Sanwa (25 ottobre 2026)

**Status:** PARTIAL / OPEN. Descrizione dell'utente e **tre foto fornite in chat** il 25 ottobre; le immagini non sono archiviate come file nella repository. Non dichiarare PASS fotografico oltre le parti visibili, né sostituire queste descrizioni a un collaudo a dispositivo chiuso.

## Evidenze dall'utente / fotografie

1. **Parete riscaldante:** pellicola flessibile arancione applicata a lastra metallica; ventola assiale nera affiancata a destra, su supporto bianco; alcuni cavi passano nel suo intorno. Una punta metallica di termocoppia tipo K del **Sanwa RD701** è visibile **non ancora fissata**.
2. **NTC originale:** piccolo sensore visibile circa al centro della pellicola riscaldante (posizione indicata dall'utente). **TCO monouso da 105°C**, in serie secondo stato precedente del progetto, dichiarato dall'utente **sotto il nastro metallico a sinistra**; le foto non certificano il modello, il contatto o la portata elettrica del TCO.
3. **SHT45:** PCB del sensore visibile nella parte alta della camera del filamento, su supporto distanziato, rivolta all'aria interna, non montata sulla piastra. La rappresentatività termica in esercizio e l'eventuale effetto della parete/ventilazione non sono ancora convalidati.
4. **Passaggio cavi provvisorio:** l'utente fa passare la termocoppia Sanwa e due fili dell'alimentazione 24 V attraverso il foro normalmente destinato al jack DC. Verificare protezione meccanica dei bordi, isolamento e assenza di sforzo/schiacciamento alla chiusura senza cambiare ora il cablaggio in esercizio.
5. **Difetto osservato/riferito:** la punta Sanwa, appoggiata vicino all'NTC, si è staccata più volte dal supporto anche con Kapton e nastro di alluminio. Il cavo è semirigido, pieghevole, e durante spostamento/chiusura trasferisce forza al punto di misura. **La ripetibilità del contatto e quindi della misura Sanwa non è dimostrata.**

## Diagnosi e prova meccanica proposta (NO POWER)

**Ipotesi di lavoro:** la trazione elastica del cavo aggrava l'insufficiente adesione del nastro al film riscaldante. Non attribuire il distacco esclusivamente alla qualità del nastro, né assumere compatibilità adesiva senza prova.

- Con alimentazione **24 V e USB scollegate** e piastra fredda, fissare il **cavo, separatamente dalla punta**, a un supporto strutturale della scocca **lontano dal lato caldo e dalla rotazione della ventola**; il fissaggio non deve schiacciare conduttori o isolanti, né appoggiarsi alla pellicola riscaldante.
- Lasciare una **ansa morbida priva di tensione** tra il fissaggio e la punta; evitare che la chiusura o lo spostamento del dryer tenda il cavo.
- Senza forare, graffiare, schiacciare o cortocircuitare le piste della pellicola, appoggiare la giunzione della termocoppia direttamente al punto scelto in prossimità dell'NTC e applicare sopra un piccolo tratto di nastro *con adesivo siliconico e specifiche appropriate*, se la superficie può essere pulita in modo compatibile. **Niente nastro fra giunzione e punto misurato.** Il nastro deve mantenere il contatto, non assorbire la trazione.
- Eseguire **solo a freddo** 3–5 cicli di apertura/chiusura e un lieve spostamento del corpo del dryer, verificando su fotografia che la posizione e il contatto della punta restino invariati. Annotare ogni eventuale distacco. Verificare che nessun cavo tocchi ventola/pale, spigoli scoperti del foro jack DC o parti elettriche esposte.
- Se il metodo si stacca ancora, **non ripetere incollaggi identici**. Valutare, previa analisi della geometria, un fermo meccanico con spinta elastica isolato e ancorato a una parte strutturale preesistente (**mai** fissato attraverso piste/film), oppure una specifica termocoppia K piatta autoadesiva con cavetto molto flessibile e connessione elettrica corretta al Sanwa. Il punto di misura eventualmente diverso dai 7 plateau originali deve essere documentato; non trattare misure di siti diversi come equivalenti.
- Il supporto Sanwa, il supporto TCO e le protezioni elettriche devono rimanere distinti; **non spostare o rimuovere il TCO da 105°C**, non cortocircuitare alcuna pista e **nessun test di riscaldamento autorizzato**.

## Esito dello step 02

**Accertato da foto/dichiarazione:** ubicazioni generali piastra/fan/NTC/TCO/SHT45 e percorso di uscita cavi provvisorio.

**OPEN e unica attività fisica immediata:** realizzare **scarico di trazione + fissaggio di contatto** della punta Sanwa; prova meccanica a freddo con coperchi realmente chiusi. L'esito andrà riportato qui e nel piano. Fino ad allora Step 02 non è PASS. Se il cavo via foro jack DC resta libero su uno spigolo, dovrà essere protetto/fissato prima di successive prove termiche.
