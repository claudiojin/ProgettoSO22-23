#include "./headers/exceptions.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 * Furthermore, this module will contain the provided skeleton TLB-Refill event
 * handler (e.g. uTLB_RefillHandler).
 */

// The processor state at the time of the exception (the saved exception state) will
// have been stored (for Processor 0) at the start of the BIOS Data Page. Don't use current process state
// since it's obsolete
#define PROCSTATE ((state_t *)BIOSDATAPAGE)

#define SYSTEMCALL_CODE ((int)PROCSTATE->reg_a0)
#define EXCEPTION_CODE CAUSE_GET_EXCCODE(PROCSTATE->cause)

void uTLB_RefillHandler()
{
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST(PROCSTATE);
}

/**
 * By storing off the TOD clock’s value at both the start and end of an interval, one can compute
 * the duration of that interval. The interval in question represents the time elapsed from the exception
 * raise to the blocking of the process requesting a blocking syscall
 * @returns the interval duration
 */
cpu_t IntervalTOD()
{
    static cpu_t timer_start = 0;
    cpu_t curr_time, diff;

    STCK(curr_time);
    diff = curr_time - timer_start;

    STCK(timer_start); // update the start of the interval to current time for future uses

    return diff;
}

/**
 * Adds to current process cpu time the interval from the exception raise to the current time, using
 * the Time Of Day clock
 */
void updateProcessCPUTime()
{
    current_process->p_time += IntervalTOD();
}

/**
 * @param p target process to send the message to
 * @param payload payload of the message to send
 * @returns 0 when successful, -2 when the pcb is not available, error is -1
 */
int SendMessage(pcb_t *p, unsigned int payload)
{
    if (p == NULL)
    {
        return MSGNOGOOD;
    }

    // If the target process is in the pcbFree_h list, set the return register (v0 in μMPS3) to DEST_NOT_EXIST
    if (searchInList(p, NULL))
    {
        return DEST_NOT_EXIST;
    }

    msg_t message = {
        .m_list.next = NULL,
        .m_list.prev = NULL,
        .m_sender = current_process,
        .m_payload = payload,
    };

    // search in the ready queue
    if (searchInList(p, &ready_queue))
    {
        pushMessage(&p->msg_inbox, &message);
        return 0;
    }
    // search in the blocked_proc lists
    else
    {
        pcb_t *found_pcb = NULL;
        for (int i = 0; i < SEMDEVLEN; i++)
        {
            found_pcb = searchInList(p, &blocked_proc[i]);
            if (found_pcb != NULL)
            {
                setReadyProcess(found_pcb);
                pushMessage(&p->msg_inbox, &message);
                return 0;
            }
        }
    }
    // if we could not find the receiver for some reason return the default error
    return MSGNOGOOD;
}

/**
 * This syscall is used by a process to extract a message from its inbox matching the required characteristics.
 * This is a synchronous operation since the requesting process will be frozen until a message matching
 * the required characteristics doesn’t arrive.
 * @param p sender's PCB
 * @param payload pointer to the expected message payload
 * @returns the process which sent the message extracted
 */
pcb_t *ReceiveMessage(pcb_t *p, unsigned int *payload)
{
    if (p == NULL)
        return NULL;

    msg_t *msg_extracted = NULL;
    // extract the first message from the requesting process inbox
    if (p == ANYMESSAGE)
    {
        if (!list_empty(&current_process->msg_inbox))
        {
            msg_extracted = headMessage(&current_process->msg_inbox);
            return msg_extracted->m_sender;
        }
        else
        {
            // wait for any message
            setBlockedProcess(PROCSTATE);
            scheduler();
        }
    }
    // search for the message with the provided characteristics
    else
    {
        if (payload == NULL)
        {
            msg_extracted = popMessage(&current_process->msg_inbox, p);
            return msg_extracted->m_sender;
        }
        else
        {
            msg_extracted = popMessage(&current_process->msg_inbox, p);
            if (msg_extracted->m_payload == *payload)
            {
                return msg_extracted->m_sender;
            }
        }
        // wait for the specified message
        setBlockedProcess(PROCSTATE);
        scheduler();
    }
    return msg_extracted->m_sender;
}

/**
 * TODO: implement pass up or die function
 */
void passUpOrDie(int index)
{
    return;
}

/**
 * Simulation of a program trap used when a privileged service is requested in user-mode
 * This is done by setting Cause.ExcCode in the stored exception state to RI
 * (Reserved Instruction), and calling one’s Program Trap exception handler
 */
void __syscallProgramTrap()
{
    PROCSTATE->cause = (PROCSTATE->cause & DISBALEExcCode) | RI;
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
        __syscallProgramTrap();
    }

    // A SYSCALL exception numbered 1 and above occurs when the Current Process executes the SYSCALL
    // instruction and the contents of a0 is greater than or equal to 1. The Nucleus SYSCALL exception
    // handler should perform a standard Pass Up or Die operation using the GENERALEXCEPT index value
    if (SYSTEMCALL_CODE >= 1)
    {
        passUpOrDie(GENERALEXCEPT);
    }

    // In case of SYS1 or non-blocking SYS2, the PC must be incremented by 4
    // (i.e. the μMPS3 wordsize, constant WORDLEN) prior to returning control
    // to the interrupted execution stream
    PROCSTATE->pc_epc += WORDLEN;

    switch (SYSTEMCALL_CODE)
    {
    case SENDMESSAGE:
        PROCSTATE->reg_v0 = (unsigned int)SendMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int)PROCSTATE->reg_a2);
        break;
    case RECEIVEMESSAGE:
        PROCSTATE->reg_v0 = (unsigned int)ReceiveMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int *)PROCSTATE->reg_a2);
        break;
    default:
        // invalid system call code
        __syscallProgramTrap();
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
        interruptHandler();
        break;

    // TLB exception
    case (1):
    case (2):
    case (3):
        // The Nucleus TLB exception handler should perform a standard Pass Up or Die operation using
        // the PGFAULTEXCEPT index value
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
        // The Nucleus Program Trap exception handler should perform a standard Pass Up or Die operation
        // using the GENERALEXCEPT index value
        passUpOrDie(GENERALEXCEPT);
        break;

    // System call
    case (8):
        systemCallHandler();
        break;
    }
}
