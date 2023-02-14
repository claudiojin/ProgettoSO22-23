#ifndef ASH_H
#define ASH_H
#include "pandos_types.h"
#include "pandos_const.h"
#include "types.h"
#include "hashtable.h"
#include "pcb.h"

//  ASH module

int insertBlocked(int *semAdd, pcb_t *p);
semd_t* line();
int counter();
pcb_t *removeBlocked(int *semAdd);
pcb_t *outBlocked(pcb_t *p);
pcb_t *headBlocked(int *semAdd);
void initASH();

#endif
