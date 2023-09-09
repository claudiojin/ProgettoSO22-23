#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "initial.h"

void scheduler();
void setProcessBlocked(pcb_t *p, state_t *state);
void setProcessReady(pcb_t *p);

#endif