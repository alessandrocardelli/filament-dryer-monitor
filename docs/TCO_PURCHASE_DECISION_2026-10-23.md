# TCO — decisione d'acquisto verificata, 23 ottobre 2026

## Esito operativo

**Selezionato per l'acquisto: MICROTEMP / Therm-O-Disc (Sensience) G4A00152C, Tf 152 °C, distribuito da Omega Fusibili.**
Negozio: ElcoTeam / Elco Elettronica Srl, SKU **3604-10326**.
Link diretto aperto nel browser: https://www.elcoteam.com/thermal-fuse-152-c-format-g4-10a-250v-microtemp-g4a00152c

Disponibilità mostrata nella pagina e nella riga del carrello: **In Stock**. Quantità 1 effettivamente aggiunta; checkout ospite raggiungibile. Nessun acquisto, pagamento, invio ordine, registrazione o contatto al venditore eseguito. L'aggiunta al carrello non riserva lo stock.

**Prezzo articolo: 2,59 EUR + IVA = 3,16 EUR.** Calcolatore spedizione eseguito per Italia / Emilia-Romagna / CAP 48018. Riepilogo effettivo: articolo netto 2,59; spedizione 7,00; IVA 0,57; totale mostrato **10,16 EUR**.
**Discrepanza commerciale esplicita:** il carrello applica IVA soltanto all'articolo, mentre condizioni ufficiali e avviso nel carrello indicano trasporto da **7,00 EUR + IVA**. Applicando il 22% anche al trasporto il totale è **11,70 EUR** = (2,59 + 7,00) × 1,22 arrotondato. Questa cifra è un calcolo prudenziale, NON un checkout finale verificato. Non presentare 10,16 o 11,70 EUR come prezzo definitivo garantito. Entrambe le cifre sono entro il budget preferito 10–15 EUR, ma la cifra finale rimane da controllare al riepilogo prima del pagamento. Nessuna tassa di importazione per questa offerta italiana risulta indicata; nessun costo ulteriore obbligatorio osservato.
Il checkout ospite richiede prima consensi privacy, poi dati fatturazione/spedizione, metodo trasporto, pagamento e riepilogo. Fermato prima dei consensi e dell'inserimento di dati personali. Non si presume completata la verifica del pagamento.

Evidenza screenshot: [carrello](evidence/tco-2026-10-23/elcoteam-cart.jpg).
Fonte condizioni spedizione: https://www.elcoteam.com/footer-menu-links/informazioni-per-lordine/costi-di-spedizione

## Fonti tecniche primarie e dati applicabili

- [Pagina del produttore Sensience, G4](https://www.sensience.com/products/microtemp-thermal-fuse-2/): esplicito 5 A / 24 V DC.
- [G4 MicroTemp Product Information, collegato dalla pagina ufficiale, p. 2](https://23653763.fs1.hubspotusercontent-na1.net/hubfs/23653763/G4%20MicroTemp%20Product%20Information.pdf).
- [Sensience Application Notes, pp. PDF 8–13 / stampate 14–19](https://www.sensience.com/wp-content/uploads/2025/01/Thermal-Fuse-Application-Notes.pdf): limiti continuativi, DC, montaggio e qualificazione nell'applicazione.
- [Catalogo ufficiale, p. 4](https://www.sensience.com/wp-content/uploads/2022/12/Microtemp_Catalog.pdf), conferma G4 152/137/205 e 5 A/24 V DC.
- [Scheda Omega, tabella G4 e terminali](https://www.omegafusibili.it/catalogo/stampa/Electronic/Protezioni%2Btermiche/Fusibili%2Bassiali/G4A01117C): codice G4A00152C, Tf 152, Th 137; il trattino Tm nella tabella distributore non sostituisce il valore del produttore.

| Parametro | Dato verificato | Implicazione |
|---|---|---|
| Tf | 152 °C; tolleranza +0/-5 °C | Apertura nominale di laboratorio 147–152 °C; non garantisce identica temperatura della piastra al momento dell'intervento |
| Th | 137 °C | Prova di mantenimento almeno 168 h, non garanzia di vita illimitata a 137 °C |
| Temperatura continuativa consigliata | non oltre Tf −25 °C = **127 °C** (Application Notes) | Il corpo TCO deve restare sotto questo riferimento con margine per misura, transitori e gradienti; non usare Th come normale limite continuo |
| Tm | 205 °C | Limite di sovratemperatura dopo apertura; non è temperatura di esercizio né limite ammesso delle plastiche |
| Rating resistivo DC | **5 A / 24 V DC**, tabella produttore UL/CSA | Copre nominalmente 24 V / ~1,59 A resistivi; non deriva da un declassamento inventato del rating AC |
| Rating resistivo AC | 10 A / 250 V AC; 15 A / 120 V AC | Valori distinti dal rating DC |
| Corpo G4, resina compresa | circa 14,7 mm × Ø4,0 mm | Verificare spazio effettivo; non è un ricambio dimensionalmente già provato |
| Reofori | Ø1,0 mm; lunghezza totale standard ~63,8 mm | G4A00 versione terminali 16/35 secondo Omega; G4A01 terminali 35/35 non è il codice selezionato |
| Tipo | monouso, corpo metallico elettricamente attivo | Isolamento elettrico verso piastra/sonda/parti metalliche e fissaggio termico devono essere idonei |

La stessa fonte produttore precisa che l'interruzione DC dipende dall'applicazione e raccomanda prove. **Il problema documentale del rating DC è risolto per la famiglia G4 e per il carico nominale dichiarato; la sicurezza dell'intero essiccatore non è ancora qualificata.** Nessuna estensione del rating a tensioni superiori a 24 V DC è documentata qui. Verificare alimentatore effettivo/tolleranza e corrente massima del carico prima del sign-off.

## Perché 152 °C e perché G4

L'obiettivo è preservare il campo OEM eBOX Lite (piastra nominale 118 °C; aria nominale 55 °C). Con G4 152 °C il riferimento continuativo consigliato è 127 °C: differenza nominale di 9 °C rispetto a 118 °C. È una valutazione preliminare: la temperatura locale del corpo fusibile non è stata misurata in configurazione chiusa e potrebbe essere diversa dalla piastra misurata altrove.

142 °C non offre un vantaggio sufficiente per questo obiettivo: SCHOTT SF139R ha raccomandazione superficie corpo 119 °C, appena 1 °C sopra il dato piastra OEM. Anche il vicino G4 144 °C porta Tf−25 a 119 °C, mentre G4 141 °C porta a 116 °C. Non sono incompatibili per ogni possibile montaggio, ma non sono la prima scelta per il contatto con la zona riscaldante non ancora mappata. Non alzare ulteriormente Tf per compensare un'installazione inadatta.

Il precedente SF150R0 rimane ben documentato termicamente: Tf 152 °C, Th 137 °C, Tm 210 °C, raccomandazione corpo 130 °C, 15 A/250 V AC resistivi. Tuttavia non è stato trovato un rating applicabile DC per questo preciso SF nel catalogo primario. Il G4 conserva Tf/Th e aggiunge un rating **esplicito 24 V DC**, perciò è preferito per l'acquisto. Ciò non dichiara il SEFUSE pericoloso né gli attribuisce una certificazione DC inesistente.
Fonte SCHOTT, pp. 7 e 21 stampate: https://media.schott.com/api/public/content/ff224fc379ec4eb09a770a547ac50b04?download=true&v=2fae6f08

## Ricerca commerciale: esiti e scarti

1. **Amazon.it prioritario:** pagina NEC 152 °C / 15 A, ASIN B01N4JLOI8 aperta: https://www.amazon.it/Fusibile-Termico-152-%C2%B0C-15-250/dp/B01N4JLOI8 . Venditore/speditore SPC-BOUTIQUE; pagina richiedeva aggiunta al carrello per mostrare prezzo. Il tentativo ha restituito «Non supportiamo la tua sessione o il tuo cliente». Non verificati prezzo, spedizione o MPN; nessuna raccomandazione d'acquisto Amazon, nessun Prime presunto. Non aggirato il blocco.
2. **eBay Italia economico realmente attivo:** https://www.ebay.it/itm/182203697116 , ROSSI RICAMBI, 152 °C, 1 EUR IVA inclusa + 6 EUR trasporto verso destinazione italiana mostrata, oltre 10 disponibili e pulsanti acquisto/carrello attivi. **Campo oggetto: Venetico, ME, Italia.** MPN non applicabile / senza marca, foto priva di marcature leggibili. Non attribuibili dati SCHOTT o MICROTEMP a questo generico. Escluso come protezione finale per identità/parametri non verificabili; prezzo basso da solo non è il motivo dello scarto. Carrello non necessario una volta emersa l'identità insufficiente.
3. Ricerca eBay “microtemp 152” con filtro Italia aperta: **0 corrispondenze esatte nazionali**, poi risultati internazionali esplicitamente etichettati Cina/USA esclusi. Inserzione G4A01152C 205975437957 recuperata dalla ricerca: oggetto a EPSOM, Regno Unito, esclusa per provenienza. Non è sufficiente eBay.it.
4. Inserzione scaduta 206514925231 NON riaperta né riproposta. RS non scelto come soluzione principale e nessuna disponibilità RS precedente presentata come nuova verifica.
5. **ElcoTeam G4A00152C**: pagina, inserimento carrello, CAP e checkout ospite verificati come sopra. Fonte concreta preferita.

Non si raccomandano cinque equivalenti: **un solo acquisto selezionato**, G4A00152C da ElcoTeam; nessuna alternativa economica viene spacciata per elettricamente qualificata.

## Verifiche residue prima dell'impiego — nessun test caldo autorizzato

- Ricevimento: controllare marcatura MICROTEMP / G4A00 / Tf152, codice lotto, integrità e corrispondenza al codice ordinato. Se viene consegnato un generico diverso, il rating G4 non è trasferibile.
- Montaggio a dispositivo spento: verificare ingombro 14,7 × 4 mm, percorso in serie a HEATER+, continuità e isolamento elettrico del corpo e dei giunti, assenza di bypass tramite corpo/termocoppia/parti metalliche. Non assumere idoneità termica di vecchia guaina o adesivo dal solo aspetto.
- Fissaggio: crimp idonei al reoforo e al filo, senza sforzo su sigillo/corpo; non deformare il cilindro. Seguire istruzioni MICROTEMP per taglio, piega e giunzione. Non applicare automaticamente al G4 distanze di montaggio specifiche SCHOTT.
- Identificare limiti reali di cavi, crimps, isolamento e polimeri; dimostrare che risposta e inerzia del cutoff proteggano tali materiali. 152 °C non è una temperatura automaticamente accettabile per la scocca.
- Dopo apposita autorizzazione e gate A/B/C/D: qualificare temperatura locale corpo TCO, piastra, NTC e aria con box chiuso e ventilazione reale; criterio iniziale corpo TCO ≤127 °C con margine, Th137 e Tm205 distinti. Nessuna prova già compiuta a 85 °C aperto può chiudere questa verifica.
- Per l'interruzione nel circuito reale: progettare separatamente una verifica supervisionata su campione sacrificabile, con carico resistivo equivalente ~15 ohm / almeno 40 W adeguatamente dimensionato, sorgente 24 V reale e rilevazione della corrente dopo apertura, senza usare la piastra eSUN come mezzo di attuazione termica. Il piano di prova va approvato prima: non è istruzione ad accendere ora. Un solo campione positivo è evidenza applicativa, non certificazione né prova statistica di affidabilità.
- Se si volesse comunque usare SF150R0 invece del G4, la via pratica è chiedere al produttore conferma scritta sul preciso SF150R0 a 24 V DC / ~1,6 A resistivi; non serve fare quella scommessa per l'acquisto G4 già documentato. Nessun messaggio inviato.

## Tracciabilità progetto

Bootstrap sul branch plan/closed-heater-qualification-2026-10-20, HEAD iniziale 7d707cafeee5ac556acf40a575d463870465ff4f: AGENTS, README, PROJECT_STATE, DECISIONS, TODO e COMPLETION_PLAN letti; ispezionati IO_Power.kicad_sch, PCB, progetto KiCad, netlist memorizzata e SafetyController.h. La netlist storica non è un nuovo export: è coerente per J5-2=24V_PROT, J5-1=HEATER_SW/Q4 drain, R20=100 ohm, R21=100 kohm; il TCO è fuori scheda. Firmware forza GPIO19 LOW e heaterPermitted=false. Non modificati sorgenti hardware/firmware; nessun nuovo ERC/DRC dichiarato.

**Ultimo PASS pertinente:** UI/SETUP/SETTINGS, ventola e stop/riavvio riferiti il 20 ottobre; Q4/carico reale e plateau aperti già registrati. **Gate A geometria e B qualifica finale restano OPEN.** La presente decisione chiude la selezione del codice d'acquisto, non il gate termico. Prossimo passo verificabile senza calore: controllo del montaggio e materiali effettivi, poi ricevimento e verifica dell'esatto G4. Prezzo finale da ricontrollare al riepilogo, prima di pagare.
