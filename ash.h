#ifndef ASH_H
#define ASH_H
#include "pandos_types.h"
#include "pandos_const.h"
#include "types.h"
#include "hashtable.h"
#include "pcb.h"
#include <umps3/umps/libumps.h>

// ASH module

int insertBlocked(int *semAdd, pcb_t *p);
pcb_t *removeBlocked(int *semAdd);
pcb_t *outBlocked(pcb_t *p);
pcb_t *headBlocked(int *semAdd);
void initASH();

#endif
