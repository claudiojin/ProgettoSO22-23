#ifndef NS_H
#define NS_H
#include "ash.h"
#include "pcb.h"

//  Namespace module

void init_nsFree(nsd_t table[], nsd_t **free_list, nsd_t **active_list);
void initNamespaces();
nsd_t *getNamespace(pcb_t *p, int type);
int addNamespace(pcb_t *p, nsd_t *ns);
nsd_t *allocNamespace(int type);
void freeNamespace(nsd_t *ns);

#endif