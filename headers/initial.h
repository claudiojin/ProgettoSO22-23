#ifndef INITIAL_H
#define INITIAL_H

#include "pandos_types.h"
#include "pandos_const.h"
#include "exceptions.h"
#include "scheduler.h"
#include "pcb.h"
#include "ash.h"
#include "ns.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/arch.h>

// LEVEL 3 GLOBAL VARIABLES

// integer indicating the number of started, but not yet terminated processes
extern int process_count;
// process can be either in the “ready,” “running,” or “blocked” (also known as “waiting”) state. This integer is
// the number of started, but not terminated processes that in are the “blocked” state due to an I/O or timer request
extern int softBlock_count;
// Tail pointer to a queue of pcbs that are in the “ready” state
extern struct list_head ready_queue;
// pointer to the pcb that is in the “running” state, i.e. the current executing process
extern pcb_t* curr_process;
// The Nucleus maintains one integer semaphore for each external (sub)device in μMPS3, plus one additional semaphore
// to support the Pseudo-clock. Since terminal devices are actually two independent sub-devices, the Nucleus
// maintains two semaphores for each terminal device
extern int device_semaphores[DEV_SEMAPHORES];

int *getIODeviceSemaphore(memaddr cmdAddr);
cpu_t getTimeElapsed();


#endif