#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "./initial.h"

/*
Your Nucleus should guarantee finite progress; consequently, every ready process will have an opportu-
nity to execute. The Nucleus should implement a simple preemptive round-robin scheduling algorithm
with a time slice value of 5 milliseconds (constant TIMESLICE)
*/

void scheduler();
void setBlockedProcess(state_t *state);
void setReadyProcess(pcb_t *p);

#endif