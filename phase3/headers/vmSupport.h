#ifndef PANDOS_VMSUPPORT_H_INCLUDED
#define PANDOS_VMSUPPORT_H_INCLUDED

#include "../../headers/listx.h"
#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../phase2/headers/initial.h"
#include "./initProc.h"

/**
 * Swap pool process to guarantee mutual exclusion.
 * To access the Swap Pool table, a process must first perform a message to this process and waiting
 * for a response message. When access to the Swap Pool table is concluded, a process will then send a
 * message to this process to release the mutual exclusion
 */
extern pcb_PTR swap_mutex_proc;
/**
 * Process currently accessing the mutually exclusive swap pool
 */
extern pcb_PTR curr_mutex_proc;

void initSwapStructs();

void TLBExceptionHandler();

void initPageTable(int asid, pteEntry_t *page_table, memaddr tmp_frame);

void freeFrames(int asid);

#endif