#include "./headers/exceptions.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 * Furthermore, this module will contain the provided skeleton TLB-Refill event
 * handler (e.g. uTLB_RefillHandler).
 */

void uTLB_RefillHandler()
{
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST(PROCSTATE);
}

/**
 * This system call causes the transmission of a message to a specified process. This is an asynchronous operation
 * @param p destination process
 * @param payload payload of the message to send: either a char*, an ssi_payload_t* or 0
 * @returns 0 when successful, -2 when the pcb is not available, default error is -1
 */
int SendMessage(pcb_t *p, unsigned int *payload)
{
    klog_print("dentro a SendMessage");
    if (p == NULL)
    {
        klog_print("destination process passed is null");
        return MSGNOGOOD;
    }
    
    // If the target process is in the pcbFree_h list, set the return register (v0 in μMPS3) to DEST_NOT_EXIST
    if (searchInList(p, NULL) != NULL)
    {
        klog_print("target process is in pcbfree list");
        return DEST_NOT_EXIST;
    }
    
    msg_t message = {
        .m_list.next = NULL,
        .m_list.prev = NULL,
        .m_sender = current_process,
        .m_payload = *payload,
    };
    
    // search in the ready queue and current process
    if (searchInList(p, &ready_queue) == p || p == current_process)
    {
        klog_print("pcb found in ready queue or pcb is current process");
        pushMessage(&p->msg_inbox, &message);
        return 0;
    }
    // search in the blocked lists
    for (int i = 0; i < DEVNUM; i++)
    {
        if (searchInList(p, &blocked_proc[i]) == p)
        {
            klog_print("pcb found in blocked list");
            readyProcess(p, i);
            pushMessage(&p->msg_inbox, &message);
            return 0;
        }
    }
    // if we could not find the receiver for some reason return the default error
    return MSGNOGOOD;
}

/**
 * This syscall is used by a process to extract a message from its inbox matching the required characteristics.
 * This is a synchronous operation since the requesting process will be frozen until a message matching
 * the required characteristics doesn’t arrive.
 * @param sender sender's PCB
 * @param payload the extracted message's payload: either of type char*, an unsigned int*(devregtr), int*, cpu_t*,
 * pcb_t*, support_t*
 * @returns the process which sent the message extracted
 */
pcb_t *ReceiveMessage(pcb_t *sender, unsigned int *payload)
{
    klog_print("dentro a ReceiveMessage");
    if (sender == NULL)
        return NULL;

    msg_t *msg_extracted = NULL;
    // extract the first message from the requesting process inbox
    if (sender == ANYMESSAGE)
    {
        if (emptyMessageQ(&current_process->msg_inbox))
        {
            klog_print("blocca pcb in attesa di un qualsiasi messaggio");
            // wait for any message
            blockProcess(PROCSTATE, SEMDEVLEN);
        }
        klog_print("messaggio qualsiasi estratto");
        msg_extracted = popMessage(&current_process->msg_inbox, NULL);
        return msg_extracted->m_sender;
    }
    // search for the message
    else
    {
        msg_extracted = popMessage(&current_process->msg_inbox, sender);
        klog_print(" messaggio estratto: ");
        klog_print_hex((unsigned int)msg_extracted);
        // wait for the specified message
        if (msg_extracted == NULL)
        {
            klog_print(" blocking ssi process ");
            blockProcess(PROCSTATE, SEMDEVLEN);
        }
        // the payload should be ignored
        if (payload == NULL)
        {
            return msg_extracted->m_sender;
        }
        // match the provided characteristics
        else
        {
            *payload = msg_extracted->m_payload;
            return msg_extracted->m_sender;
        }
    }
    return msg_extracted->m_sender;
}

/**
 * For all other exceptions (e.g. SYSCALL exceptions numbered 1 and above, Program Trap and TLB exceptions)
 * the Nucleus will take one of two actions depending on whether the the Current Process was provided
 * a non-NULL value for its Support Structure pointer when it was created: pass the handling of the exception
 * the suppport level(if it's non-null) or behave as a TerminateProcess
 * @param index Mnemonic code to differentiate exceptions from TLB exceptions and general exceptions
 */
void passUpOrDie(int index)
{
    if (current_process->p_supportStruct == NULL)
    {
        TerminateProcess(current_process);
        scheduler();
    }
    else
    {
        // Copy the saved exception state from the BIOS Data Page to the correct sup_exceptState field
        // of the Current Process
        current_process->p_supportStruct->sup_exceptState[index] = *PROCSTATE;
        // Perform a Load Context using the fields from the correct sup_exceptContext field of the Current Process
        context_t ctx = current_process->p_supportStruct->sup_exceptContext[index];
        LDCXT(ctx.stackPtr, ctx.status, ctx.pc);
    }
}

/**
 * Simulation of a program trap used when a privileged service is requested in user-mode
 * This is done by setting Cause.ExcCode in the stored exception state to RI
 * (Reserved Instruction), and calling one’s Program Trap exception handler
 */
void syscallProgramTrap()
{
    PROCSTATE->cause = (PROCSTATE->cause & DISBALEExcCode) | RI;
    passUpOrDie(GENERALEXCEPT);
}

/**
 * The return register (reg_v0) is updated here
 */
void systemCallHandler()
{
    // Kernel mode when KUp bit is set to 0
    // KU/IE bit stack is pushed whenever an exception is raised and whenever
    // an interrupted execution stream is restarted, the stack is popped.
    if ((PROCSTATE->status & USERPON) != 0)
    {
        klog_print("syscall chiamata in user mode");
        syscallProgramTrap();
    }

    // A SYSCALL exception numbered 1 and above occurs when the Current Process executes the SYSCALL
    // instruction and the contents of a0 is greater than or equal to 1. The Nucleus SYSCALL exception
    // handler should perform a standard Pass Up or Die operation using the GENERALEXCEPT index value
    if (SYSTEMCALL_CODE >= 1)
    {
        klog_print("syscall code >= 1");
        passUpOrDie(GENERALEXCEPT);
    }

    // In case of SYS1 or non-blocking SYS2, the PC must be incremented by 4
    // (i.e. the μMPS3 wordsize, constant WORDLEN) prior to returning control
    // to the interrupted execution stream
    PROCSTATE->pc_epc += WORDLEN;

    switch (SYSTEMCALL_CODE)
    {
    case SENDMESSAGE:
        PROCSTATE->reg_v0 = (unsigned int)SendMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int *)PROCSTATE->reg_a2);
        break;
    case RECEIVEMESSAGE:
        PROCSTATE->reg_v0 = (unsigned int)ReceiveMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int *)PROCSTATE->reg_a2);
        break;
    default:
        // invalid system call code
        klog_print(" invalid syscall code: ");
        klog_print_dec((unsigned int)SYSTEMCALL_CODE);
        syscallProgramTrap();
        break;
    }

    LDST(PROCSTATE);
}

void exceptionHandler()
{
    switch (EXCEPTION_CODE)
    {
    // Interrupt
    case (0):
        klog_print("interrupt");
        interruptHandler();
        break;

    // TLB exception
    case (1):
    case (2):
    case (3):
        // The Nucleus TLB exception handler should perform a standard Pass Up or Die operation using
        // the PGFAULTEXCEPT index value
        klog_print("tlb exc");
        passUpOrDie(PGFAULTEXCEPT);
        break;

    // Program trap
    case (4):
    case (5):
    case (6):
    case (7):
    case (9):
    case (10):
    case (11):
    case (12):
        klog_print("trap");
        // The Nucleus Program Trap exception handler should perform a standard Pass Up or Die operation
        // using the GENERALEXCEPT index value
        passUpOrDie(GENERALEXCEPT);
        break;

    // System call
    case (8):
        klog_print("syscall");
        systemCallHandler();
        break;
    }
}
