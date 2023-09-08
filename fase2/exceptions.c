#include "../headers/exceptions.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 * Furthermore, this module will contain the provided skeleton TLB-Refill event
 * handler (e.g. uTLB_RefillHandler).
 */

static int Create_Process(state_t *statep, support_t *supportp, nsd_t *ns)
{

    pcb_t *new_proc = allocPcb();

    // Non ci sono PCB liberi
    if (new_proc == NULL)
    {
        return -1;
    }

    process_count++;
    if (ns == NULL)
    {
        // ns uguale al processo padre
        addNamespace(new_proc, getNamespace(curr_process, NS_PID));
    }
    else
    {
        addNamespace(new_proc, ns);
    }
    new_proc->p_s = *statep;
    new_proc->p_supportStruct = supportp;
    insertProcQ(&ready_queue, new_proc);
    insertChild(curr_process, new_proc);

    return new_proc->p_pid;
}


int exceptionHandler() { return 0; }
