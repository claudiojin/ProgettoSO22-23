# ProgettoSO22-23
PandOs è un "sistema operativo" creato per essere rappresentabile tramite più layer astratti che implementano le funzioni chiave di un sistema operativo Unix. L’ambiente di riferimento per l’esecuzione del software e’μMPS3. Si tratta di un emulatore per architettura MIPS con interfaccia grafica integrata per l’esecuzione, l’interazione e il debugging del software.

# Fase 1
In questa fase del progetto si implementa il livello 2 del nostro Sistema Operativo. Di seguito vengono descritti i vari moduli dove vengono implementate le strutture dati che manipolano e gestiscono i PCB, i semafori e i namespace. Ricordiamo che il numero massimo di processi concorrenti è 20.

## Modulo PCB
I Process Control Blocks(descrittori di processi) sono organizzati in code o alberi. Le code sono implementate come liste bidirezionali circolari tramite un nodo "sentinella": un concatenatore che collega il primo e l'ultimo nodo della coda; la coda si dice vuota se rimane solo la sentinella. Gli alberi sono implementati similmente, infatti ogni processo ha un puntatore alla lista dei figli e il primo nodo di questa lista è la sentinella.    

## Modulo ASH
I Semaphore Descriptors(SEMD) sono gestiti tramite la Active Semaphore Hash(ASH), una hash a lista di trabocco. Viene inizializzata tramite la macro DEFINE_HASHTABLE con dimensione 5 bit, che corrisponde a 32 celle in questo modo abbiamo una chiave univoca(il puntatore al valore del semaforo) che viene calcolata tramite una macro della hash API: 5 bit dovrebbero evitare al meglio le collisioni. Ogni semaforo deve avere almeno un processo bloccato nella sua coda, altrimenti viene rimosso dalla ASH.

## Modulo Namespace
Ogni processo può vedere la sua parte di sistema operativo, ogni parte è detta namespace e la sua struttura dati è il NSD(Namespace Descriptor). Un processo può stare in più namespace a patto che siano tutti diversi tra loro, per questo si dichiara un'array di dimensione MAX_PROC per ogni tipo di namespace.In questa fase del progetto utilizzeremo un solo namespace, PID, ma per renderci le cose più facili in futuro abbiamo creato una funzione ausiliaria initNS_Free() che inizializza la lista di uno specifico tipo di ns. Inoltre le variabili nsFree_h e ns_list_h sono degli array di liste: ogni cella contiene la testa della lista dei ns liberi e occupati di un determinato tipo.  

# Come compilare
Basta lanciare make dalla directory del progetto, tutti i file compilati restano nella stessa directory.
Il file che configura la macchina è già presente nella directory: "FASE1". L'unica modifica che è stata apportata al file p1test.c è l'include degli header pandos_const.h e pandos_types.h, compila solamente se vengono inclusi con le "virgolette".
