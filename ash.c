#include "ash.h"
// array di semd con dimensione massima 20
semd_t semd_table[MAXPROC];

// testa della lista dei semd liberi o inutilizzati(come PCBFree_h)
// ogni semd ha accesso alla lista dei semd liberi semdFree_h tramite il campo s_freelink
semd_t *semdFree_h;

/* Hash dei semafori attivi (ASH), su un singolo semaforo ci possono essere più PCB in attesa
 * di seguito alcune domande che mi pongo da almeno 3 giorni
 * 1) la dimensione della hash table influenza direttamente il numero di bit della chiave, ma se la chiave(semAdd) che usiamo
 *    è l'indirizzo di un intero, non dovrei creare una hash con chiavi di bit pari a quelli dell' indirizzo di un intero?
 *    Ma visto che il massimo di SEMD su una singola hash è 20, allora non ha senso avere una dimensione così grande solo per
 *    far coincidere n_bit_addres == n_bit_key, mi servirebbe un afunzione hash per mappare il tutto
 * 2) capire come dovrebbe essere lo schema della hash table
 * 3) operazione di ricerca, come si fa?
 */
DEFINE_HASHTABLE(semd_h, 5);

int insertBlocked(int *semAdd, pcb_t *p)
{
    if (semAdd != NULL && p != NULL)
    {
        // semd associato a semAdd
        semd_t *tmp = NULL;
        hash_for_each_possible(semd_h, tmp, s_link, (int)semAdd);
        // semd_t *tmp = semd_h[(int)semAdd].first; // container_of(semd_h[*semAdd].first, semd_t, s_link);
        // caso: il semd non è presente nella ASH -> alloco un nuovo semd e aggiungo alla ASH
        if (tmp == NULL)
        {
            if (semdFree_h == NULL)
                return true;
            else
            {
                // eliminazione in testa dalla lista dei semdFree
                tmp = semdFree_h;
                semdFree_h = semdFree_h->s_freelink.next; // container_of(semdFree_h->s_freelink.next, semd_t, s_freelink);
                // setto i campi del nuovo semd
                tmp->s_freelink.next = NULL;
                // tmp->s_freelink.prev = &(semd_h[*semAdd]);
                tmp->s_key = semAdd;
                // s_procq è la sentinella della lista di pcb associati al semd
                mkEmptyProcQ(&(tmp->s_procq));
                // aggiorno il campo semAdd del pcb e lo aggiungo alla lista
                p->p_semAdd = semAdd;
                insertProcQ(&(tmp->s_procq), p);
                hash_add(semd_h, &(tmp->s_link), (int)semAdd);
                return false;
            }
        }
        // caso: il semd è nella ASH -> aggiungo p alla pcb queue del SEMD
        else
        {
            p->p_semAdd = semAdd;
            insertProcQ(&(tmp->s_procq), p);
            return false;
        }
    }
    return false;
}
/*
Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata al SEMD della ASH con chiave semAdd.
Se tale descrittore non esiste nella ASH, restituisce NULL. Altrimenti, restituisce l’elemento rimosso. Se la coda
dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASH e lo inserisce nella
coda dei descrittori liberi (semdFree_h).
*/
pcb_t *removeBlocked(int *semAdd)
{
    if (semAdd == NULL)
        return NULL;
    semd_t *tmp = NULL; //= semd_h[(int)semAdd].first;
    hash_for_each_possible(semd_h, tmp, s_link, (int)semAdd);
    // caso: semd non è nella ASH
    if (tmp == NULL)
        return NULL;
    // caso: semd è nella ASH
    else
    {
        // dopo la rimozione controllo se la lista procQ è vuota
        pcb_t *p = removeProcQ(&(tmp->s_procq));
        if (list_empty(&(tmp->s_procq)))
        {
            // rimuovo il semd dalla ASH
            hash_del(&(tmp->s_link));
            // inserisco nella testa di semdFree_h
            tmp->s_freelink.next = semdFree_h;
            semdFree_h = tmp;
        }
        return p;
    }
}

/*
 * Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da p->p_semAdd). Se il PCB non compare
 * in tale coda, allora restituisce NULL (condizione di errore). Altrimenti, restituisce p. Se la coda dei processi bloccati
 * per il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASH e lo inserisce nella coda dei descrittori liberi
 */
pcb_t *outBlocked(pcb_t *p) {}

// Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa alla coda dei processi associata al SEMD con chiave
// semAdd. Ritorna NULL se il SEMD non compare nella ASH oppure se compare ma la sua coda dei processi è vuota.
pcb_t *headBlocked(int *semAdd) {}

// Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. Questo metodo viene invocato
// una volta sola durante l’inizializzazione della struttura dati.
void initASH()
{
    semdFree_h = &semd_table[0];
    semdFree_h->s_freelink.next = NULL;
    semdFree_h->s_freelink.prev = NULL;
    semd_t *tmp = semdFree_h;
    for (int i = 1; i < MAXPROC; i++)
    {
        tmp->s_freelink.next = &(semd_table[i].s_freelink);
        // lista monodirezionale
        tmp = container_of(tmp->s_freelink.next, semd_t, s_freelink);
        tmp->s_freelink.prev = NULL;
        tmp->s_freelink.next = NULL;
    }
}
