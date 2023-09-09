#ifndef ASH_H
#define ASH_H
#include "hashtable.h"
#include "pcb.h"
#include "scheduler.h"

//  ASH module

int insertBlocked(int *semAdd, pcb_t *p);
pcb_t *removeBlocked(int *semAdd);
pcb_t *outBlocked(pcb_t *p);
pcb_t *headBlocked(int *semAdd);
void initASH();
pcb_t *semP(int *sem, pcb_t *process, state_t *state);
pcb_t *semV(int *sem, pcb_t *process, state_t *state);

#endif
