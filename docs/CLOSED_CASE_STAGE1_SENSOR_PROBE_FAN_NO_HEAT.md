# Fase 1 operativa — configurazione chiusa, sensori e sonda (NESSUN riscaldamento)

**22 ottobre 2026 — checklist da completare; non ancora eseguita/validata in questa configurazione.**

**Oggetto ristretto:** eSUN eBOX Lite con fondo e coperchio superiore chiusi, ventola che ricircola aria, sensore SHT45, termocoppia Sanwa presso la piastra, NTC originale J7 e cablaggi heater/TCO. La qualifica della scocca e del pannello di comando non fa parte di questa fase; nessun cambio firmware o elettronico.

## Dati già superati — non ripetere

- Ventola Q5/J6: GPIO16 giunto di saldatura ripristinato; 100% spunto, poi 40% stabile per 2–3 min, arresto da pulsante; no auto-ripresa dopo riavvio.
- Letture display SHT45: 19,4 °C e 69,7% RH a riposo; NTC heater indicato 15,5 °C nel SYSTEM (temperatura diagnostica, non calibrazione definitiva).
- Gate Q4/J5: prova elettrica a vuoto 8 ottobre e prove con heater reale 1/5/10 s 14 ottobre, già superate.
- Sette plateau NTC/Sanwa open-case fan-OFF fino a Sanwa 85 °C, già acquisiti e analizzati. Questi **non valgono per il dryer chiuso con fan ON**.

## Verifica fisica a impianto spento — PRIMA di qualunque prova nuova

1. **Identificare il punto fisico reale del SHT45**: deve rilevare aria di camera, non essere appoggiato a piastra riscaldante, né essere irraggiato direttamente o investito dal getto concentrato della ventola. Non è noto dalla repo dove ora sia fissato; acquisire foto/schizzo con posizione, distanza e orientamento. Non cambiare placement sulla base di ipotesi prima di vederlo.
2. **Ripetibilità della misura Sanwa**: fissare in modo stabile la termocoppia K presso la piastra e annotare posizione rispetto al sensore NTC originale, evitando contatto con terminali elettrici e parti conduttrici esposte. Il Sanwa misura la temperatura del proprio contatto, **non la temperatura interna del bead NTC**. Annotare eventuale diversa posizione rispetto ai sette test precedenti.
3. **Passaggio sonda termocoppia**: prevedere uscita del cavo senza schiacciamento sui bordi, sfregamento con ventola, contatto con hotspot, deformazione che renda la chiusura non rappresentativa. Se la sonda non può passare senza essere pizzicata, NON forzare i coperchi; correggere il percorso prima della prova.
4. **Cavi e aria**: controllare heater/J5, NTC/J7, SHT45/J3, fan/J6, TCO in serie, isolamento giunzioni, ancoraggi, vie dell'aria e rotore libero. Verificare entrambi i coperchi senza schiacciare o spostare sensore/sonda/TCO. Ispezionare con alimentazione completamente rimossa.
5. **Solo per il collaudo fan-only formalmente isolato**: scollegare J5 a scheda totalmente spenta, isolare il connettore del ramo heater e lasciare connessi J7, J3, J6. Non rimuovere/bypassare il TCO. Il firmware integrato HEATER LOCKED è una protezione logica, non sostituisce l'isolamento fisico in questa prova. Se non è praticabile accedere e isolare J5, annotare il problema e non registrare un PASS di isolamento.

## Prova chiusa ventilata SENZA HEAT — soltanto dopo il preflight

- Con entrambi i coperchi chiusi, alimentazione accessibile per lo stop e firmware attuale, avviare il già noto DRYING TEST (100% per 1 s, 40% continuo) per **2–3 minuti**. Non è un nuovo collaudo del driver ventola: si osservano unicamente **effetti della geometria chiusa** (ventola libera, passaggi aria, nessun cavo pizzicato, nessun reset/FAULT).
- Registrare T/RH SHT45 all'inizio e alla fine (e il riferimento ambientale se disponibile). Il confronto aria/NTC a freddo è orientativo; non effettuare correzioni NTC.
- Arrestare come già verificato; non serve ripetere ulteriori sequenze UI, prove Q4/J5 o i sette plateau caldi.
- **Esito fase 1:** PASS soltanto con sensore e Sanwa fissati/rintracciabili, cavo sonda instradato senza danno, due coperchi completamente chiusi, fan libero e campionamento SHT45; annotare se J5 fisicamente disconnesso per la prova.

## Fase 2 immediatamente successiva

Documentare e giudicare la **protezione termica indipendente TCO monouso 105 °C / 15 A / 250 V** già presente e riferita fissata con Kapton contro l'heater. Prima di qualsiasi prova a caldo chiusa o revisione, raccogliere il codice fabbricante, marcatura completa e scheda tecnica (Tf/Th/Tm, tolleranze), **posizione reale fotografata** e contatto termico. Confrontare con le temperature piastra OEM 105/116/118 °C e temperatura/ratings di cablaggi, plastiche, sensori e superfici. Non assumere che un TCO da 105 °C vada semplicemente sostituito con uno a rating maggiore; nessun bypass e nessun nuovo setpoint alto finché scelta e protezioni non sono approvate.

**Stato alla redazione:** non abbiamo prove fotografiche della posizione SHT45/Sanwa né esito chiuso con J5 isolato. La fase 2 è in agenda, non completata. Nessuna modifica a firmware, KiCad, protezione o DECISIONS.
