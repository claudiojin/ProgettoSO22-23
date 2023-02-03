#include "pcb.h"

/** non più di venti processi concorrrenti in panda+
 *
 *  processo = sezione di memoria dove il processo lavora, con i suoi dati
 *  e il suo codice sorgente + stato del processore e altre meta-info sul processo stesso -> PCB
 */

// array di PCB con dimensione massima di MAX_PROC, è un pool di memoria
pcb_t pcbFree_table[MAXPROC];
// testa della lista dei PCB che sono liberi o inutilizzati
pcb_t *pcbFree_h;

// Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table.
// Questo metodo deve essere chiamato una volta sola in fase di inizializzazione della struttura dati.
void initPcbs()
{
    // è la testa della lista e testa deve rimanere
    pcbFree_h = &pcbFree_table[0];
    pcbFree_h->p_list.prev = NULL;
    pcb_t *head = pcbFree_h;
    for (int i = 1; i < MAXPROC; i++)
    {
        head->p_list.next = &pcbFree_table[i];
        // lista monodirezionale: non mi serve il prev
        head->p_list.prev = NULL;
        head = head->p_list.next;
    }
    // chiudo la lista
    head->p_list.next = NULL;
}

// Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree_h)
void freePcb(pcb_t *p)
{
    // controllo che non sia null e poi inserisco in testa
    if (p != NULL)
    {
        p->p_list.prev = NULL;
        p->p_list.next = pcbFree_h;
        pcbFree_h = p;
    }
}

// Restituisce NULL se la pcbFree_h è vuota. Altrimenti rimuove un elemento dalla pcbFree,
// inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso.
pcb_t *allocPcb()
{
    if (pcbFree_h == NULL)
    {
        return NULL;
    }
    else
    {
        // rimozione in testa
        pcb_t *p = pcbFree_h;
        pcbFree_h = pcbFree_h->p_list.next;
        // inizializzo tutti i campi
        p->p_list.next = NULL;
        p->p_list.prev = NULL;
        p->p_parent = NULL;
        p->p_child.next = NULL;
        p->p_child.prev = NULL;
        p->p_sib.next = NULL;
        p->p_sib.prev = NULL;
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
    if (list_empty(head))
        return true;
    else
        return false;
}

// Inserisce l’elemento puntato da p nella coda dei processi puntata da head.
void insertProcQ(struct list_head *head, pcb_t *p)
{
    if (p != NULL)
    {
        // caso: coda vuota
        if (list_empty(head))   
        {
            head->next = &(p->p_list);
            head->prev = &(p->p_list);
            p->p_list.next = head;
            p->p_list.prev = head;
        }
        // caso: inserimento con sentinella
        else
        {
            list_add_tail(&(p->p_list), head);
            /*
            head->prev->next = &(p->p_list);
            p->p_list.prev = head->prev;
            head->prev = &(p->p_list);
            p->p_list.next = head;
            */
        }
    }
}

// Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO.
// Ritorna NULL se la coda non ha elementi.
pcb_t *headProcQ(struct list_head *head)
{
    if (list_empty(head))
    {
        return NULL;
    }
    else
    {
        // ritorna il puntatore al primo elemento della lista(head->next), castando a *pcb_t
        return container_of(head->next, pcb_t, p_list);
    }
}

// Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda è vuota.
// Altrimenti ritorna il puntatore all’elemento rimosso dalla lista.
pcb_t *removeProcQ(struct list_head *head)
{
    // lista vuota
    if (list_empty(head))
        return NULL;
    // un elemento, il nodo dopo head punta di nuovo alla sentinella
    else if (head->next->next == head->prev)
    {
        pcb_t *tmp = container_of(head->next, pcb_t, p_list);
        // container_of(tmp, pcb_t, p_list);
        list_del(head->next);
        // reinizializzo la sentinella
        INIT_LIST_HEAD(head);
        return tmp;
    }
    // più di un elemento
    else
    {
        pcb_t *tmp = container_of(head->next, pcb_t, p_list);
        head->next = head->next->next;
        head->next->prev = head;
        tmp->p_list.next = NULL;
        tmp->p_list.prev = NULL;
        return tmp;
    }
}

// Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non è presente nella coda,
// restituisce NULL. (NOTA: p può trovarsi in una posizione arbitraria della coda).
pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
    // controllo che non sia vuota la coda o il puntatore al pcb sia NULL
    if (list_empty(head) || p == NULL)
        return NULL;
    // faccio puntare pos al primo elemento della lista
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
    // caso in cui il pcb da rimuovere è il primo della lista e la lista è di più elementi
    else if((container_of(pos, pcb_t, p_list) == p) && (pos->next != head)) {
        head = pos->next;
        pos->next->prev = head;
        list_del(pos);
        return container_of(pos, pcb_t, p_list);
    }
    // caso in cui il pcb è in testa ed è l'unico elemento della lista
    else
    {
        // elimino il primo nodo e inizializzo head in modo che list_empty(head) mi ritorni true
        list_del(pos);
        INIT_LIST_HEAD(head);
        return container_of(pos, pcb_t, p_list);
    }
}

// Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti.
int emptyChild(pcb_t *p)
{
    if (p->p_child.next == NULL)
    {
        return true;
    }
    else
        return false;
}

// Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.
void insertChild(pcb_t *prnt, pcb_t *p)
{
    pcb_t *tmp = prnt->p_child.next;
    if (prnt != NULL && p != NULL)
    {
        // caso: prnt ha già almeno un figlio
        if (tmp != NULL)
        {
            prnt->p_child.next = p;
            p->p_sib.next = tmp;
            p->p_sib.prev = NULL;
            p->p_parent = prnt;
        }
        // caso: prnt non ha figli
        else
        {
            p->p_parent = prnt;
            prnt->p_child.next = p;
            p->p_sib.next = NULL;
            p->p_sib.prev = NULL;
        }
    }
}

// Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL
pcb_t *removeChild(pcb_t *p)
{
    if (p == NULL && p->p_child.next == NULL)
        return NULL;
    else
    {
        pcb_t *tmp = p->p_child.next;
        // caso: più figli
        if (tmp->p_sib.next != NULL)
        {
            p->p_child.next = tmp->p_sib.next;
            tmp->p_sib.next->prev = NULL;
        }
        // caso: un solo figlio
        else
        {
            p->p_child.next = NULL;
        }
        return tmp;
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
        // tmp punta al fratello sinistro
        pcb_t *tmp = p->p_sib.prev;
        // caso: è il primo fratello
        if (tmp == NULL)
        {
            return removeChild(p->p_parent);
        }
        // caso: ha dei fratelli, ricollego il fratello a dx con quello a sx
        else
        {
            // è l'ultimo figlio
            if (p->p_sib.next == NULL)
                tmp->p_sib.next = NULL;
            else
            {
                tmp->p_sib.next = p->p_sib.next;
                p->p_sib.next->prev = tmp;
            }
        }
        return p;
    }
}