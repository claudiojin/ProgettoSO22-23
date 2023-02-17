#include "namespcae.h"

// non più di venti processi concorrenti in panda+

static nsd_t type_nsd[MAX_PROC];

static nsd_t *type_nsFree_h;

static nsd_t *type_nsList_h;

void initNamespaces()
{
  //Inizializzo la testa della lista, per la fase 1 serve una sola lista per un solo namespace (PID)
  type_nsFree_h = &type_nsd[0];
  type_nsFree_h->n_link.next = NULL;
  type_nsFree_h->n_link.prev = NULL;
  nsd_t *tmp = type_nsFree_h;
  
  for (int i = 1, i < MAX_PROC; i++)
  { 
    tmp->n_link.next = &(type_nsd[i].n_link);
    tmp->n_link.prev = NULL;
    tmp->n_linl.next = NULL;
  }
}

nsd_t *getNamespace(pcb_t *p, int type)
{
}

int addNamespace(pcb_t *p, nsd_t *ns)
{
}

nsd_t *allocNamespace(int type)
{
}

void freeNamespace(nsd_t *ns)
{
}
