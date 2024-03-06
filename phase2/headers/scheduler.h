#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "./initial.h"

/*
Your Nucleus should guarantee finite progress; consequently, every ready process will have an opportu-
nity to execute. The Nucleus should implement a simple preemptive round-robin scheduling algorithm
with a time slice value of 5 milliseconds (constant TIMESLICE)
*/

void freezeProcess(state_t *state);
void awakeProcess(pcb_t *p);
cpu_t IntervalTOD();
void updateProcessCPUTime();
void scheduler();

#endif