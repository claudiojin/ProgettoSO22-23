# ProgettoSO22-23
PandOs è un "sistema operativo" creato per essere rappresentabile tramite più layer astratti che implementano le funzioni chiave di un sistema operativo Unix. L’ambiente di riferimento per l’esecuzione del software e’μMPS3. Si tratta di un emulatore per architettura MIPS con interfaccia grafica integrata per l’esecuzione, l’interazione e il debugging del software.

# Fase 1
In questa fase del progetto si implementa il livello 2 del nostro Sistema Operativo. Di seguito vengono descritti i vari moduli dove vengono implementate le strutture dati che manipolano e gestiscono i PCB e i messages. Ricordiamo che il numero massimo di processi concorrenti è 40.

## Modulo PCB
I Process Control Blocks(descrittori di processi) sono organizzati in code o alberi.Le code sono implementate come liste bidirezionali circolari tramite un nodo "sentinella": un concatenatore che collega il primo e l'ultimo nodo della coda; la coda si dice vuota se rimane solo la sentinella. Gli alberi sono implementati similmente, infatti ogni processo ha un puntatore alla lista dei figli e il primo nodo di questa lista è la sentinella.    

## Modulo msg
L'accesso alle risorse avviene tramite message passing. I Message Blocks (msg) rappresentano i messaggio nel sistema e sono organizzati in code.

# Come compilare
Basta lanciare make dalla directory del progetto, dopodichè lanciare umps3. Il file che configura la macchina è già presente nella directory: "fase1". 
