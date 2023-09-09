#include "../headers/initial.h"
/**
 * This module implements main() and exports the Nucleus’s global variables.
 * (e.g. process count, device semaphores, etc.)
 */

// LEVEL 3 GLOBAL VARIABLES

int process_count;
int softBlock_count;
struct list_head ready_queue;
pcb_t *curr_process;
int device_semaphores[DEV_SEMAPHORES];

// pointer to process 0 pass up vector memory location
passupvector_t *passupvector;

state_t process0_state;

// functions provided by the p2test
extern void test();
extern void uTLB_RefillHandler();

// restituire il semaforo associato ad un dispositivo IO ricercato per
// per modificare lo stato del sem
int *getIODeviceSemaphore(memaddr cmdAddr)
{
    int index;
    memaddr register_addr;
    memaddr offset = 0; // un terminale ha 4 sotto-registri

    if (cmdAddr >= TERM0ADDR) // terminale 
    {
        //i due registri di command distano 8 bit, quindi 
        //basta verificare il bit 4 
        if (((cmdAddr >> 3) & 1) == 1) // ricezione
        {
            register_addr = cmdAddr - 0x4;
        }
        else //trasmissione
        {
            register_addr = cmdAddr - 0xc;
            offset = 8;
        }
    }
    else{ // non terminale
        register_addr = cmdAddr - 0x4;
    }
    index = ((register_addr - DEV_REG_START) / DEV_REG_SIZE) + offset;
    return &device_semaphores[index];
}

cpu_t getTimeElapsed()
{
    static cpu_t t_start = 0;
    cpu_t curr_time, t_elapsed;
    
    STCK(curr_time);
    t_elapsed = curr_time - t_start;
    STCK(t_start);

    return t_elapsed;
}
void memcpy(void *dest, const void *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        ((char *)dest)[i] = ((char *)src)[i];
    }
}

int isSoftBlocked(pcb_t *p) {
    if (p->p_semAdd == NULL) { return FALSE; }

    // Il semaforo fa riferimento ad un device di I/O
    if ((p->p_semAdd >= &device_semaphores[0]) && (p->p_semAdd <= &device_semaphores[DEV_SEMAPHORES-1])) { return TRUE; }

    return FALSE;
}

cpu_t timerFlush() {
    static cpu_t timer_start = 0;
    cpu_t curr_time, diff;

    STCK(curr_time);
    diff = curr_time - timer_start;
    
    STCK(timer_start); // Reset tempo di inizio

    return diff;
}

void updateProcessCPUTime() {
    curr_process->p_time += timerFlush();
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
    initASH();
    initNamespaces();

    // initialize global variables
    process_count = 0;
    softBlock_count = 0;
    mkEmptyProcQ(&ready_queue); // ready_queue è la sentinella della coda di PCB
    curr_process = NULL;

    for (int i = 0; i < DEV_SEMAPHORES; i++)
        device_semaphores[i] = 0;

    // initialize Interval Timer device with 100ms
    LDIT(PSECOND);
    // instantiate a single process in the Ready Queue
    pcb_t *proc = allocPcb();
    // STST(&process0_state);  // save processors state in memory
    RAMTOP(process0_state.reg_sp); // stack pointer set to ramtop
    // PC set to test function address, also update t9 register
    process0_state.pc_epc = process0_state.reg_t9 = (memaddr)test;
    process0_state.status = ALLOFF | IEPON | IMON | TEBITON; // bitwise OR on the status register, enabling interrupts, PLT and kernel mode
    // initialize all fields
    proc->p_s = process0_state;
    // insert new initialized process in Ready Queue
    // ready_queue is the sentinel of the queue
    insertProcQ(&ready_queue, proc);
    process_count++;

    scheduler();
    return 0;
}