#include "ash.h"
// array di semd con dimensione massima 20
semd_t semd_table[MAXPROC];

// testa della lista dei semd liberi o inutilizzati(come PCBFree_h)
// ogni semd ha accesso alla lista dei semd liberi semdFree_h tramite il campo s_freelink
static semd_t *semdFree_h;

int l = 0;

/* Hash dei semafori attivi (ASH), su un singolo semaforo ci possono essere più PCB in attesa
 * di seguito alcune domande che mi pongo da almeno 3 giorni
 * 1) come gestisco le collisioni?
 *  - inserisco sempre se la semdFree è libera, quindi anche se trovo il semd associato alla chiave e la semdFree != NULL, allora
 *    aggiungo alla hash un nuovo semaforo nello stesso bucket
 *  - aggiungo solo se hlist_head->first == NULL, quindi se il bucket associato alla chiave è vuoto
 */
DEFINE_HASHTABLE(semd_h, 5);

/** Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave semAdd. Se il semaforo
 *   corrispondente non è presente nella ASH, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella
 *   ASH, settando I campi in maniera opportuna (i.e. key e s_procQ). Se non è possibile allocare un nuovo SEMD perché la lista di
 *   quelli liberi è vuota, restituisce TRUE. In tutti gli altri casi, restituisce FALSE.
 */
int insertBlocked(int *semAdd, pcb_t *p)
{
    if (semAdd != NULL && p != NULL)
    {
        l++;
        int key = hash_min((int)semAdd, HASH_BITS(semd_h));
        int empty = hlist_empty(&semd_h[key]);
        // semd associato a semAdd
        // semd_t *tmp; // = NULL;
        // faccio una sola scansione, il for_each assegna a tmp il primo elemento della lista puntato da hlist_head->first
        // struct hlist_node *tmp = semd_h[hash_min((int)semAdd, HASH_BITS(semd_h))].first;
        // caso: il semd non è presente nella ASH -> alloco un nuovo semd e aggiungo alla ASH
        if (empty && semdFree_h != NULL)
        {
            // rimozione in testa dalla lista dei semdFree
            semd_t *tmp = semdFree_h;
            semdFree_h = container_of(semdFree_h->s_freelink.next, semd_t, s_freelink);
            // setto i campi del nuovo semd
            tmp->s_freelink.next = NULL;
            tmp->s_key = semAdd;
            // s_procq è la sentinella della lista di pcb associati al semd
            mkEmptyProcQ(&(tmp->s_procq));
            p->p_semAdd = semAdd;
            insertProcQ(&(tmp->s_procq), p);
            // inserisce il nodo attuale in testa alla lista dei semd associata alla chiave semAdd
            hash_add(semd_h, &(tmp->s_link), (int)semAdd);
            return false;
        }
        // caso: il semd è nella lista -> aggiungo il pcb alla lista dei bloccati di semd
        else if (!empty && semdFree_h != NULL)
        {
            semd_t *tmp;
            // semd_t *tmp = container_of(semd_h[hash_min((int)semAdd, HASH_BITS(semd_h))].first, semd_t, s_link);
            hash_for_each_possible(semd_h, tmp, s_link, (int)semAdd)
            {
                p->p_semAdd = semAdd;
                insertProcQ(&(tmp->s_procq), p);
                return false;
            }
        }
        // caso: bisogna aggiungere un nuovo semd ma la semdFree è vuota
        else
            return true;
    }
    return false;
}

int counter()
{
    int i = 0;
    int buck = 0;
    //struct list_head *pos;
    semd_t *pos;
    hash_for_each(semd_h, buck, pos, s_link)
    //for (pos = &semdFree_h->s_freelink; pos != NULL ; pos = pos->next)
    {
        i++;
    }
    return i;
}

semd_t *line()
{
    return semdFree_h;
}

/*
 *   Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata al SEMD della ASH con chiave semAdd.
 *   Se tale descrittore non esiste nella ASH, restituisce NULL. Altrimenti, restituisce l’elemento rimosso. Se la coda
 *   dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASH e lo inserisce nella
 *   coda dei descrittori liberi (semdFree_h).
 */
pcb_t *removeBlocked(int *semAdd)
{
    if (semAdd == NULL)
        return NULL;
    semd_t *tmp; // = NULL;
    // scansiono la lista associata al semaforo con chiave semAdd, tmp è il cursore
    hash_for_each_possible(semd_h, tmp, s_link, (int)semAdd)
    {
        // visto che sono già nella lista giusta, controllo che ci siano dei semd
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
                tmp->s_freelink.next = &(semdFree_h->s_freelink);
                semdFree_h = tmp;
            }
            return p;
        }
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
