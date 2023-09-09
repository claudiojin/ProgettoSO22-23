#include "../headers/pcb.h"

/** non più di venti processi concorrrenti in panda+
 *
 *  processo = sezione di memoria dove il processo lavora, con i suoi dati
 *  e il suo codice sorgente + stato del processore e altre meta-info sul processo stesso -> PCB
 */

// array di PCB con dimensione massima di MAX_PROC, è un pool di memoria
static pcb_t pcbFree_table[MAXPROC];
// testa della lista dei PCB che sono liberi o inutilizzati
static pcb_t *pcbFree_h;

// testa della lista dei pid
struct list_head pidList_h;
int curr_pid;
/**
 * ritorna il processo in base all'id passato
*/
pcb_t *getProcessByPid(int pid)
{
    pcb_t *iter;

    list_for_each_entry(iter, &pidList_h, pid_list)
    {
        if (iter->p_pid == pid)
        {
            return iter;
        }
        if (iter->p_pid > pid)
        {
            break;
        }
    }

    return NULL;
}

int _generatePid()
{
    // Gestione wraparound
    if (curr_pid <= 0)
    {
        curr_pid = 1;
    }

    if (!list_empty(&pidList_h))
    {
        /* Gestione collisioni */
        int curr_greater_pid = container_of(pidList_h.prev, pcb_t, pid_list)->p_pid; // L'ultimo elemento della lista dei pid è il più grande

        if (curr_greater_pid >= curr_pid)
        { // Se minore, sicuramente non ci sono collisioni (la lista dei pid è ordinata)
            while (getProcessByPid(curr_pid) != NULL)
            {
                curr_pid++;
            }
        }
    }

    return curr_pid++;
}

void _addPid(pcb_t *p)
{
    int inserted = FALSE;
    pcb_t *iter;

    // Inserimento per mantenere la lista ordinata in senso crescente per pid
    // Dato che i pid sono (tendenzialmente) crescenti, si inserisce scorrendo la lista al contrario per maggiore ottimizzazione
    list_for_each_entry_reverse(iter, &pidList_h, pid_list)
    {
        if (p->p_pid >= iter->p_pid)
        {
            __list_add(&p->pid_list, &iter->pid_list, iter->pid_list.next);
            inserted = TRUE;
            break;
        }
    }

    // Nel caso in cui si raggiunga l'inizio della lista senza che avvenga l'inserimento o se la lista è vuota
    if (inserted == FALSE)
    {
        list_add(&p->pid_list, &pidList_h);
    }
}

// Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table.
// Questo metodo deve essere chiamato una volta sola in fase di inizializzazione della struttura dati.
void initPcbs()
{
    // inizializzo la testa della lista
    pcbFree_h = &pcbFree_table[0];
    pcbFree_h->p_list.next = NULL;
    pcbFree_h->p_list.prev = NULL;
    pcb_t *tmp = pcbFree_h;

    INIT_LIST_HEAD(&pidList_h);
    curr_pid = 1;

    for (int i = 1; i < MAXPROC; i++)
    {
        tmp->p_list.next = &(pcbFree_table[i].p_list);
        // lista monodirezionale: non mi serve il prev
        tmp = container_of(tmp->p_list.next, pcb_t, p_list);
        tmp->p_pid = NEW_PCB_ID;
        tmp->p_list.prev = NULL;
        tmp->p_list.next = NULL;
    }
}

// Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree_h)
void freePcb(pcb_t *p)
{
    // controllo che non sia null, poi inserisco in testa
    if (p != NULL)
    {
        p->p_list.prev = NULL;
        p->p_list.next = &(pcbFree_h->p_list);
        pcbFree_h = p;
        list_del(&pidList_h);
        p->p_pid = NEW_PCB_ID;
    }
}

void _initPcb(pcb_t *p)
{
    // inizializzo tutti i campi
    p->p_list.next = NULL;
    p->p_list.prev = NULL;
    p->p_parent = NULL;
    p->p_sib.next = NULL;
    p->p_sib.prev = NULL;
    // p_child è la sentinella della lista dei figli
    INIT_LIST_HEAD(&(p->p_child));
    p->p_semAdd = NULL;
    p->p_supportStruct = NULL;
    p->p_time = 0;
    p->p_s.entry_hi = p->p_pid = _generatePid();
}

// Restituisce NULL se la pcbFree_h è vuota. Altrimenti rimuove un elemento dalla pcbFree,
// inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso.
pcb_t *allocPcb()
{
    if (&pcbFree_h->p_list == NULL)
        return NULL;
    else
    {
        // rimozione in testa
        pcb_t *p = pcbFree_h;
        pcbFree_h = container_of(pcbFree_h->p_list.next, pcb_t, p_list);
        _initPcb(p);
        // assegno il PID
        _addPid(p);
        return p;
    }
}

// Crea una lista di PCB, inizializzandola come lista vuota
void mkEmptyProcQ(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

// Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti
int emptyProcQ(struct list_head *head)
{
    return list_empty(head);
}

// Inserisce l’elemento puntato da p nella coda dei processi puntata da head.
void insertProcQ(struct list_head *head, pcb_t *p)
{
    if (p != NULL)
    {
        list_add_tail(&(p->p_list), head);
    }
}

// Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO.
// Ritorna NULL se la coda non ha elementi.
pcb_t *headProcQ(struct list_head *head)
{
    if (emptyProcQ(head))
        return NULL;
    else
        // ritorna il puntatore al primo elemento della lista(head->next), castando a *pcb_t
        return container_of(head->next, pcb_t, p_list);
}

// Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda è vuota.
// Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.
pcb_t *removeProcQ(struct list_head *head)
{
    // lista vuota
    if (emptyProcQ(head))
        return NULL;
    // primo elemento della lista
    struct list_head *pos = head->next;
    // lista di un singolo elemento
    if (pos->next == head)
    {
        list_del(pos);
        // reinizializzo la sentinella
        INIT_LIST_HEAD(head);
        return container_of(pos, pcb_t, p_list);
    }
    // più di un elemento
    else
    {
        head->next = pos->next;
        pos->next->prev = head;
        list_del(pos);
        return container_of(pos, pcb_t, p_list);
    }
}

// Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non è presente nella coda,
// restituisce NULL. (NOTA: p può trovarsi in una posizione arbitraria della coda).
pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
    // coda vuota o il puntatore al pcb è NULL
    if (emptyProcQ(head) || p == NULL)
        return NULL;
    // primo elemento della lista
    struct list_head *pos = head->next;
    // caso generale
    if (container_of(pos, pcb_t, p_list) != p)
    {
        while (pos != head)
        {
            if (container_of(pos, pcb_t, p_list) == p)
            {
                pos->prev->next = pos->next;
                pos->next->prev = pos->prev;
                list_del(pos);
                return container_of(pos, pcb_t, p_list);
            }
            pos = pos->next;
        }
        // se non trova nulla ritorno NULL
        return NULL;
    }
    // caso: il pcb da rimuovere è il primo della lista
    else
        return removeProcQ(head);
}

// Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti.
int emptyChild(pcb_t *p)
{
    if (emptyProcQ(&(p->p_child)))
        return true;
    else
        return false;
}

// Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.
void insertChild(pcb_t *prnt, pcb_t *p)
{
    if (prnt != NULL && p != NULL)
    {
        list_add(&p->p_sib, &prnt->p_child);
        p->p_parent = prnt;
    }
}

// Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL
pcb_t *removeChild(pcb_t *p)
{
    if (p == NULL || emptyProcQ(&p->p_child))
        return NULL;
    else
    {
        // primo elemento della lista
        struct list_head *tmp = p->p_child.next;
        // caso: più figli
        if (!list_is_head(tmp, &p->p_child))
        {
            p->p_child.next = tmp->next;
            tmp->next->prev = &p->p_child;
            list_del(tmp);
        }
        // caso: un solo figlio
        else
        {
            list_del(tmp);
            INIT_LIST_HEAD(&p->p_child);
        }
        return container_of(tmp, pcb_t, p_sib);
    }
}

/**  Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB puntato da p non ha
 *   un padre, restituisce NULL, altrimenti restituisce l’elemento rimosso (cioè p). A differenza
 *   della removeChild, p può trovarsi in una posizione arbitraria (ossia non è necessariamente il primo figlio del padre).
 */
pcb_t *outChild(pcb_t *p)
{
    if (p == NULL || p->p_parent == NULL)
        return NULL;
    else
    {
        // fratello sinistro
        struct list_head *tmp = p->p_sib.prev;
        // caso: è il primo fratello
        if (list_is_head(tmp, &p->p_parent->p_child))
            return removeChild(p->p_parent);
        // caso: non è il primo fratello
        else
        {
            tmp->next = p->p_sib.next;
            p->p_sib.next->prev = tmp;
            list_del(&p->p_sib);
            return p;
        }
    }
}