#include "./headers/exceptions.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 * Furthermore, this module will contain the provided skeleton TLB-Refill event
 * handler (e.g. uTLB_RefillHandler).
 */

// The processor state at the time of the exception (the saved exception state) will
// have been stored (for Processor 0) at the start of the BIOS Data Page. Don't use current process state
// since it's obsolete
#define PROCSTATE (((state_t *)BIOSDATAPAGE))

#define SYSTEMCALL_CODE ((int)PROCSTATE->reg_a0)
#define EXCEPTION_CODE CAUSE_GET_EXCCODE(PROCSTATE->cause)

void uTLB_RefillHandler()
{
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t *)BIOSDATAPAGE);
}

int SendMessage(pcb_t *p, unsigned int payload) {
    // If the target process is in the pcbFree_h list, set the return register (v0 in μMPS3) to DEST_NOT_EXIST
    if (isFree(p)) {
        return DEST_NOT_EXIST;
    }

    msg_t message = {
        .m_list = NULL,
        .m_sender = curr_process,
        .m_payload = payload
    };

    if (contains(p, &ready_queue)) {
        pushMessage(&p->msg_inbox, &message);
        return 0;
    }
    else {
        pcb_t *found_pcb = NULL;
        for (int i = 0; i < SEMDEVLEN; i++) {
            found_pcb = contains(p, &blocked_proc[i]);
            if (found_pcb != NULL) {
                outProcQ(&blocked_proc[i], found_pcb);
                return 0;
            }
        }
    }
    return MSGNOGOOD;
}

/**
 * @param p sender's PCB
 * @param payload message payload
 * @returns the sender's PCB
*/
pcb_t *ReceiveMessage(pcb_t *p, unsigned int payload) {
    if (p == ANYMESSAGE) {
        if (list_empty(&curr_process->msg_inbox)) {
            return headMessage(&curr_process->msg_inbox)->m_sender;
        }
        else {
            // wait for any message
        }
    }
    // wait for the right message
}

void systemCallHandler()
{
    // Kernel mode when KUp bit is set to 0
    // KU/IE bit stack is pushed whenever an exception is raised and whenever
    // an interrupted execution stream is restarted, the stack is popped.
    if ((PROCSTATE->status & USERPON) != 0)
    {
        ProgramTrapHandler();
    }

    // In case of SYS1 or non-blocking SYS2, the PC must be incremented by 4
    // (i.e. the μMPS3 wordsize, constant WORDLEN) prior to returning control
    // to the interrupted execution stream
    PROCSTATE->pc_epc += WORDLEN;

    switch (SYSTEMCALL_CODE)
    {
    case SENDMESSAGE:
        PROCSTATE->reg_v0 = SendMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int)PROCSTATE->reg_a2);
        break;
    case RECEIVEMESSAGE:
        ReceiveMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int *)PROCSTATE->reg_a2);
        break;
    default:
        ProgramTrapHandler();
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
        passUpOrDieHandler(PGFAULTEXCEPT);
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
        passUpOrDieHandler(GENERALEXCEPT);
        break;

    // System call
    case (8):
        systemcallHandler();
        break;
    }
}
