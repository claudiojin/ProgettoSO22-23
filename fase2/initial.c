#include "../headers/pandos_types.h"
#include "../headers/pandos_const.h"
#include "../headers/exceptions.h"
#include "../headers/scheduler.h"
#include "../headers/pcb.h"
#include "../headers/ash.h"
#include "../headers/ns.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

/**
 * This module implements main() and exports the Nucleus’s global variables.
 * (e.g. process count, device semaphores, etc.)
 */

// LEVEL 3 GLOBAL VARIABLES

// integer indicating the number of started, but not yet terminated processes
int process_count;
// process can be either in the “ready,” “running,” or “blocked” (also known as “waiting”) state. This integer is
// the number of started, but not terminated processes that in are the “blocked” state due to an I/O or timer request
int softBlock_count;
// Tail pointer to a queue of pcbs that are in the “ready” state
struct list_head ready_queue;
// pointer to the pcb that is in the “running” state, i.e. the current executing process
pcb_t* curr_process;
// The Nucleus maintains one integer semaphore for each external (sub)device in μMPS3, plus one additional semaphore
// to support the Pseudo-clock. Since terminal devices are actually two independent sub-devices, the Nucleus
// maintains two semaphores for each terminal device
int device_semaphores[DEV_SEMAPHORES];
// pointer to process 0 pass up vector memory location
passupvector_t* passupvector;
state_t process0_state;

// functions provided by the p2test
extern void test();
extern void uTLB_RefillHandler();

void memcpy(void *dest, const void *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}

int main()
{
    // populate the process 0 Pass Up Vector, the BIOS uses this to know where to find Kernel functions
    // when a TLB-refill event or any other exception happens
    passupvector = (passupvector_t*)PASSUPVECTOR; // process 0 pass up vector always stays at his memory location PASSUPVECTOR
    passupvector->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    passupvector->tlb_refill_stackPtr = (memaddr)KERNELSTACK;
    passupvector->exception_handler = (memaddr)exceptionHandler; // our exception handling function TODO: exceptions.c
    passupvector->exception_stackPtr = (memaddr)KERNELSTACK;

    // initialize data structures from Phase 1
    initPcbs();
    initASH();
    initNamespaces();

    // initialize global variables
    process_count = 0;
    softBlock_count = 0;
    mkEmptyProcQ(&ready_queue); // ready_queue->p_list è la sentinella della coda di PCB
    curr_process = NULL;

    for (int i = 0; i < DEV_SEMAPHORES; i++)
        device_semaphores[i] = 0;

    // initialize Interval Timer device with 100ms
    LDIT(PSECOND);

    // instantiate a single process in the Ready Queue
    pcb_t* proc = allocPcb();
    //STST(&process0_state);  // save processors state in memory
    RAMTOP(process0_state.gpr[26]); // stack pointer set to ramtop
    // PC set to test function address, update t9 register also
    process0_state.pc_epc = (memaddr)test;
    process0_state.gpr[24] = process0_state.pc_epc;
    process0_state.status = ALLOFF | IEPON | IMON | TEBITON; // bitwise OR on the status register, enabling interrupts, PLT and kernel mode
    // initialize all fields
    proc->p_child.next = NULL;
    proc->p_child.prev = NULL;
    proc->p_parent = NULL;
    proc->p_sib.next = NULL;
    proc->p_sib.prev = NULL;
    proc->p_time = 0;
    proc->p_semAdd = NULL;
    proc->p_supportStruct = NULL;
    //proc->p_s = process0_state;
    // insert new initialized process in Ready Queue
    // p_list field of ready_queue is the sentinel of the struct_list type Queue
    insertProcQ(&ready_queue, proc);
    process_count++;

    scheduler();
    return 0;
}