#include "./headers/exceptions.h"
#include "./headers/klog.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 */

void uTLB_RefillHandler()
{
    // Determine the page number (denoted as p) of the missing TLB entry by inspecting EntryHi in
    // the saved exception state located at the start of the BIOS Data Page.
    unsigned int vpn = (0b1 << 19) + ENTRYHI_GET_VPN(PROCSTATE->entry_hi);

    int page_number;
    // The VPN field will be set to [0x80000..0x8001E] for the first 31 entries. The VPN for the stack
    // page (Page Table entry 31) should be set to 0xBFFFF
    if (vpn == 0xBFFFF)
        page_number = 31;
    else
        page_number = vpn % 0x80000;

    // Get the Page Table entry for page number p for the Current Process. This will be located in
    // the Current Process’s Page Table, which is part of its Support Structure
    pteEntry_t pt_entry = current_process->p_supportStruct->sup_privatePgTbl[page_number];

    // Write this Page Table entry into the TLB. This is a three-set process
    setENTRYHI(pt_entry.pte_entryHI);
    setENTRYLO(pt_entry.pte_entryLO);
    TLBWR();

    // Return control to the Current Process to retry the instruction that caused the TLB-Refill event:
    // LDST on the saved exception state located at the start of the BIOS Data Page
    LDST(PROCSTATE);
}

/**
 * This system call causes the transmission of a message to a specified process. This is an asynchronous operation
 * @param destination destination process
 * @param payload payload of the message to send: either a char*, an ssi_payload_t* or 0
 * @param sender sender process
 * @returns 0 when successful, -2 when the pcb is not available, default error is -1
 */
int SendMessage(pcb_t *destination, unsigned int *payload, pcb_t *sender)
{
    if (destination == NULL)
    {
        return MSGNOGOOD;
    }

    msg_t *message = allocMsg();

    if (message == NULL)
        return MSGNOGOOD;

    // If the target process is in the pcbFree_h list, set the return register (v0 in μMPS3) to DEST_NOT_EXIST
    if (searchInList(destination, NULL) == destination)
    {
        return DEST_NOT_EXIST;
    }

    message->m_sender = sender;

    // payload handling, for now these are the types of messages we deal with, might change in phase 3
    if (destination == ssi_pcb)
    {
        ssi_payload_PTR cast_payload = (ssi_payload_PTR)payload;
        message->ssi_payload.service_code = cast_payload->service_code;
        message->ssi_payload.arg = cast_payload->arg;
    }
    else if (sender == ssi_pcb)
    {
        message->m_payload = *payload;
    }
    else
    {
        message->string_payload = (char *)payload;
    }

    // search in the ready queue or current process
    if (destination == current_process || searchInList(destination, &ready_queue) == destination)
    {
        insertMessage(&destination->msg_inbox, message);
        return 0;
    }
    // search in the blocked list
    if (searchInList(destination, &blocked_proc[SEMDEVLEN]) == destination)
    {
        readyProcess(destination, SEMDEVLEN);
        insertMessage(&destination->msg_inbox, message);
        return 0;
    }

    // if we could not find the receiver for some reason return the default error
    return MSGNOGOOD;
}

/**
 * This syscall is used by a process to extract a message from its inbox matching the required characteristics.
 * This is a synchronous operation since the requesting process will be frozen until a message matching
 * the required characteristics doesn’t arrive.
 * @param sender sender's PCB
 * @param payload a pointer to an area where the nucleus will store the payload of the message (NULL if the payload should be ignored)
 * either of type char*, an unsigned int*(devregtr), int*, cpu_t*, pcb_t*, support_t*
 * @returns the process which sent the message extracted
 */
pcb_t *ReceiveMessage(pcb_t *sender, unsigned int *payload)
{
    msg_t *msg_extracted = NULL;
    // extract the first message from the requesting process inbox
    if (sender == ANYMESSAGE)
    {
        msg_extracted = popMessage(&current_process->msg_inbox, NULL);
    }
    // search for the specified message
    else
    {
        msg_extracted = popMessage(&current_process->msg_inbox, sender);
    }
    // wait for the specified message
    if (msg_extracted == NULL)
    {
        blockProcess(PROCSTATE, SEMDEVLEN);
    }
    // update the payload if needed
    if (payload != NULL)
    {
        if (current_process == ssi_pcb)
        {
            ssi_payload_PTR cast_payload = (ssi_payload_PTR)payload;

            cast_payload->service_code = msg_extracted->ssi_payload.service_code;
            cast_payload->arg = msg_extracted->ssi_payload.arg;
        }
        else if (msg_extracted->string_payload != NULL)
        {
            *payload = (unsigned int)msg_extracted->string_payload;
        }
        else
        {
            *payload = msg_extracted->m_payload;
        }
    }

    // free the message and return the sender
    pcb_PTR extracted_sender = msg_extracted->m_sender;
    freeMsg(msg_extracted);

    return extracted_sender;
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
        klog_print("SYScall code: ");
        klog_print_dec((unsigned int)SYSTEMCALL_CODE);
        syscallProgramTrap();
    }

    // A SYSCALL exception numbered 1 and above occurs when the Current Process executes the SYSCALL
    // instruction and the contents of a0 is greater than or equal to 1. The Nucleus SYSCALL exception
    // handler should perform a standard Pass Up or Die operation using the GENERALEXCEPT index value
    if (SYSTEMCALL_CODE >= 1)
    {
        passUpOrDie(GENERALEXCEPT);
    }

    switch (SYSTEMCALL_CODE)
    {
    case SENDMESSAGE:
        PROCSTATE->reg_v0 = (unsigned int)SendMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int *)PROCSTATE->reg_a2, current_process);
        break;
    case RECEIVEMESSAGE:
        PROCSTATE->reg_v0 = (unsigned int)ReceiveMessage((pcb_t *)PROCSTATE->reg_a1, (unsigned int *)PROCSTATE->reg_a2);
        break;
    default:
        // invalid system call code
        syscallProgramTrap();
        break;
    }

    // In case of SYS1 or non-blocking SYS2, the PC must be incremented by 4
    // (i.e. the μMPS3 wordsize, constant WORDLEN) prior to returning control
    // to the interrupted execution stream
    PROCSTATE->pc_epc += WORDLEN;

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
