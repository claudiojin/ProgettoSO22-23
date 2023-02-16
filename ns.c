#include "ns.h"
/*
per il momento utilizzeremo un solo namespace: PID

Per aggiungere altri namespace:
    1) Definire il nuovo tipo nel file pandos_types.h, e modificare la variabile:
        #define NS_NEW 1
        NS_TYPE_LAST NS_NEW
    2) Si deve dichiarare l'array del nuovo tipo di namespace qui sotto
*/

// array di namespace tipo PID
static nsd_t pid_nsdTable[MAXPROC];

// Liste dei NSD di tipo type liberi o inutilizzati.
static nsd_t *nsFree_h; //[NS_TYPE_MAX];

// Liste dei namespace di tipo type attivi.
static nsd_t *nsList_h; //[NS_TYPE_MAX];

void init_nsFree(nsd_t table[], nsd_t *free_list, nsd_t *active_list)
{
    active_list = NULL;

    free_list = &table[0];
    free_list->n_link.next = NULL;
    free_list->n_link.prev = NULL;
    nsd_t *tmp = free_list;

    for (int i = 1; i < MAXPROC; i++)
    {
        tmp->n_link.next = &(table[i].n_link);
        // lista monodirezionale: non mi serve il prev
        tmp = container_of(tmp->n_link.next, nsd_t, n_link);
        tmp->n_link.prev = NULL;
        tmp->n_link.next = NULL;
    }
}
// Inizializza tutte le liste dei namespace liberi. Questo metodo viene invocato una volta sola durante l’inizializzazione
// della struttura dati.
void initNamespaces()
{
    // inizializzo la lista di tipo PID
    nsList_h = NULL;
    // init_nsFree(pid_nsdTable, nsFree_h[0], nsList_h[0]);
    nsFree_h = &pid_nsdTable[0];
    nsFree_h->n_link.next = NULL;
    nsFree_h->n_link.prev = NULL;
    nsd_t *tmp = nsFree_h;

    for (int i = 1; i < MAXPROC; i++)
    {
        tmp->n_link.next = &(pid_nsdTable[i].n_link);
        // lista monodirezionale: non mi serve il prev
        tmp = container_of(tmp->n_link.next, nsd_t, n_link);
        tmp->n_link.prev = NULL;
        tmp->n_link.next = NULL;
    }
}

// Ritorna il namespace di tipo type associato al processo p (o NULL).
nsd_t *getNamespace(pcb_t *p, int type)
{
    if (type >= 0 && type < NS_TYPE_MAX)
        return p->namespaces[type];
    else
        return NULL;
}

// Associa al processo p e a tutti I suoi figli il namespace ns. Ritorna FALSE in caso di errore, TRUE altrimenti.
int addNamespace(pcb_t *p, nsd_t *ns)
{
    if (p == NULL || ns == NULL)
        return false;
    // l'indice dell'array namespaces corrisponde al tipo, in questa fase del progetto ho solo PID, quindi ns->n_type sarà 0
    // in futuro potremmo implementare altri namespace, quindi per rendre versatile il codice uso ns->n_type
    p->namespaces[ns->n_type] = ns;
    if (!emptyChild(p))
    {
        struct list_head *pos;
        list_for_each(pos, &p->p_child)
        {
            pcb_t *tmp = container_of(pos, pcb_t, p_sib);
            tmp->namespaces[ns->n_type] = ns;
        }
    }
    return true;
}

// Alloca un namespace di tipo type dalla lista corretta.
nsd_t *allocNamespace(int type)
{
    if (type < 0 || type > NS_TYPE_MAX)
        return NULL;
    // punta alla testa della lista di nsd liberi del tipo corrispondente
    nsd_t *tmp = nsFree_h; //[type];
    // tolgo dalla lista nsFree e aggiungo alla lista nsList
    nsFree_h = container_of(nsFree_h->n_link.next, nsd_t, n_link);
    tmp->n_link.next = &nsList_h->n_link; // questa istruzione fa esplodere tutto
    nsList_h = tmp;
    return tmp;
}

// Libera il namespace ns ri-inserendolo nella lista di namespace corretta.
void freeNamespace(nsd_t *ns) {}
