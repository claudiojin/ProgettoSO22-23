#ifndef INITIAL_H
#define INITIAL_H

#include "../../headers/const.h"
#include "../../headers/types.h"
#include "./scheduler.h"
#include "./exceptions.h"
// #include "./ssi.h"
// #include "./interrupts.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/arch.h>
#include <stdio.h>

// LEVEL 3 GLOBAL VARIABLES

// integer indicating the number of started, but not yet terminated processes
extern int process_count;
// process can be either in the “ready,” “running,” or “blocked” (also known as “waiting”) state. This integer is
// the number of started, but not terminated processes that in are the “blocked” state due to an I/O or timer request
extern int softBlock_count;
// Tail pointer to a queue of pcbs that are in the “ready” state
extern struct list_head ready_queue;
// pointer to the pcb that is in the “running” state, i.e. the current executing process
extern pcb_t *current_process;
// The Nucleus maintains one list of blocked PCBs for each external (sub)device
// (or an array of length SEMDEVLEN - 1 of pcb_t pointers), plus one additional list to support the
// Pseudo-clock. Since terminal devices are actually two independent sub-devices, the Nucleus maintains
// two lists/pointers for each terminal device
extern struct list_head blocked_proc[SEMDEVLEN];
// list of processes waiting for a message
extern struct list_head frozen_list;

void init_ssi();
void init_test();

#endif