#include "./headers/initial.h"

/**
 * This module implements main() and exports the Nucleus’s global variables.
 * (e.g. process count, device semaphores, etc.)
 */

// LEVEL 3 GLOBAL VARIABLES

int process_count;
int softBlock_count;
struct list_head ready_queue;
pcb_t *curr_process;
struct list_head blocked_proc[SEMDEVLEN];

// pointer to process 0 pass up vector memory location
passupvector_t *passupvector;

// function provided by the p2test.c file
extern void test();

// instantiates the SSI process in the Ready Queue
void init_ssi() {
    state_t ssi_state;
    pcb_t *ssi_pcb = allocPcb();
    // bitwise OR on the status register, enabling interrupts and kernel mode
    ssi_state.status = ALLOFF | IEPON | IMON;
    // stack pointer set to ramtop
    RAMTOP(ssi_state.reg_sp);
    // PC set to SSI function address, also update t9 register
    ssi_state.pc_epc = ssi_state.reg_t9 = (memaddr)SSI_function_entry_point;
    // initialize status field
    ssi_pcb->p_s = ssi_state;
    // insert new initialized process in Ready Queue
    insertProcQ(&ready_queue, ssi_pcb); // ready_queue is the sentinel of the queue
    process_count++;
}

// instantiates the test process in the Ready Queue
void init_test() {
    state_t test_state;
    pcb_t *test_pcb = allocPcb();
    // bitwise OR on the status register, enabling interrupts, kernel mode and PLT
    test_state.status = ALLOFF | IEPON | IMON | TEBITON;
    // stack pointer set to ramtop - space_of_last_process
    unsigned int ramTop;
    RAMTOP(ramTop);
    test_state.reg_sp = ramTop - (2 * PAGESIZE);
    // PC set to SSI function address, also update t9 register
    test_state.pc_epc = test_state.reg_t9 = (memaddr)test;
    // initialize status field
    test_pcb->p_s = test_state;
    // insert new initialized process in Ready Queue
    insertProcQ(&ready_queue, test_pcb); // ready_queue is the sentinel of the queue
    process_count++;
}

int main()
{
    // populate the process 0 Pass Up Vector, the BIOS uses this to know where to find Kernel functions
    // when a TLB-refill event or any other exception happens
    passupvector = (passupvector_t *)PASSUPVECTOR; // process 0 pass up vector always stays at his memory location PASSUPVECTOR
    passupvector->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    passupvector->tlb_refill_stackPtr = (memaddr)KERNELSTACK;
    passupvector->exception_handler = (memaddr)exceptionHandler; // our exception handling function TODO: exceptions.c
    passupvector->exception_stackPtr = (memaddr)KERNELSTACK;

    // initialize data structures from Phase 1
    initPcbs();
    initMsg();

    // initialize all global variables
    process_count = 0;
    softBlock_count = 0;
    mkEmptyProcQ(&ready_queue); // ready_queue è la sentinella della coda di PCB
    curr_process = NULL;
    for (int i = 0; i < SEMDEVLEN; i++)
        mkEmptyProcQ(&blocked_proc[i]);

    // initialize Interval Timer device with 100ms
    LDIT(PSECOND);

    init_ssi();
    init_test();

    scheduler();

    // TheSnow legacy lives on
    return 0;
}