# ProgettoSO22-23

# Fase 1
In questa fase del progetto si implementa il livello 2 del nostro Sistema Operativo realizzato su architettura umps3, ovvero le strutture dati che manipolano e gestiscono i PCB, i semafori e i namespace. 

# Modulo PCB
I Process Control Blocks(descrittori di processi) sono organizzati in code o alberi. Le code sono implementate come liste bidirezionali circolari tramite un nodo "sentinella": un concatenatore che collega il primo e l'ultimo nodo della coda; la coda si dice vuota se rimane solo la sentinella. Gli alberi sono implementati similmente, infatti ogni processo ha un puntatore alla lista dei figli e il primo nodo di questa lista è la sentinella.    

# Modulo ASH
I Semaphore Descriptors(SEMD) sono gestiti tramite la Active Semaphore Hash(ASH), una hash a lista di trabocco. Ogni semaforo ha una chiave univoca(il puntatore al valore del semaforo) che viene calcolata tramite una macro della hash API: questo dovrebbe evitare al meglio le collisioni. Nel caso ci fossero, viene preso in causa il primo semd della lista finchè non viene rimosso. Ogni semaforo deve avere almeno un processo bloccato nella sua coda, altrimenti viene rimosso dalla ASH.

# Modulo Namespace
// da completare
