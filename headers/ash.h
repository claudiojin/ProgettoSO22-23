#ifndef ASH_H
#define ASH_H
#include "hashtable.h"
#include "pcb.h"

//  ASH module

int insertBlocked(int *semAdd, pcb_t *p);
pcb_t *removeBlocked(int *semAdd);
pcb_t *outBlocked(pcb_t *p);
pcb_t *headBlocked(int *semAdd);
void initASH();

#endif
