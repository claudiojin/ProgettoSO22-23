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
 * @param destination destination process
 * @param payload payload of the message to send: either a char*, an ssi_payload_t* or 0
 * @param sender sender process
 * @returns 0 when successful, -2 when the pcb is not available, default error is -1
 */
int SendMessage(pcb_t *destination, unsigned int *payload, pcb_t *sender)
{
    // In case of SYS1 or non-blocking SYS2, the PC must be incremented by 4
    // (i.e. the μMPS3 wordsize, constant WORDLEN) prior to returning control
    // to the interrupted execution stream
    PROCSTATE->pc_epc += WORDLEN;

    klog_print(" SENDMESSAGE ");
    if (destination == NULL)
    {
        klog_print("destination process passed is null");
        return MSGNOGOOD;
    }

    msg_t *message = allocMsg();

    if (message == NULL)
        return MSGNOGOOD;

    klog_print(" Dest Adrr: ");
    klog_print_hex((unsigned int)destination);
    klog_print(" Msg Addr: ");
    klog_print_hex((unsigned int)message);

    message->m_sender = sender;

    // payload handling
    if (destination == ssi_pcb) {
        ssi_payload_PTR cast_payload = (ssi_payload_PTR)payload; 
        message->ssi_payload.service_code = cast_payload->service_code;
        message->ssi_payload.arg = cast_payload->arg;
        
        klog_print(" ssi service code: ");
        klog_print_hex(message->ssi_payload.service_code);
    }
    else if (sender == ssi_pcb) {
        message->m_payload = *payload;
    }
    else {
        message->string_payload = (char *)payload;
    }

    // If the target process is in the pcbFree_h list, set the return register (v0 in μMPS3) to DEST_NOT_EXIST
    if (searchInList(destination, NULL) == destination)
    {
        klog_print("target process is in pcbfree list");
        return DEST_NOT_EXIST;
    }
    
    // search in the ready queue or current process
    if (destination == current_process || searchInList(destination, &ready_queue) == destination)
    {
        klog_print("Pcb running or ready");
        pushMessage(&destination->msg_inbox, message);
        return 0;
    }
    // search in the blocked list
    if (searchInList(destination, &blocked_proc[SEMDEVLEN]) == destination)
    {
        klog_print("Pcb in blocked list");
        readyProcess(destination, SEMDEVLEN);
        pushMessage(&destination->msg_inbox, message);
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
    klog_print(" RECEIVEMESSAGE ");
    
    msg_t *msg_extracted = NULL;
    // extract the first message from the requesting process inbox
    if (sender == ANYMESSAGE)
    {
        msg_extracted = popMessage(&current_process->msg_inbox, NULL);
        klog_print(" any message extracted: ");
        klog_print_hex((unsigned int)msg_extracted);
    }
    // search for the specified message
    else
    {
        msg_extracted = popMessage(&current_process->msg_inbox, sender);
        klog_print(" message extracted: ");
        klog_print_hex((unsigned int)msg_extracted);
    }
    // wait for the specified message
    if (msg_extracted == NULL)
    {
        klog_print(" blocking process ");
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
            klog_print(" ssi service code: ");
            klog_print_hex(cast_payload->service_code);
        }
        else if (msg_extracted->string_payload != NULL)
        {
            *payload = (unsigned int)msg_extracted->string_payload;
        }
        else {
            *payload = msg_extracted->m_payload;
        }
    }

    // In case of SYS1 or non-blocking SYS2, the PC must be incremented by 4
    // (i.e. the μMPS3 wordsize, constant WORDLEN) prior to returning control
    // to the interrupted execution stream
    PROCSTATE->pc_epc += WORDLEN;

    pcb_PTR extracted_sender = msg_extracted->m_sender;
    freeMsg(msg_extracted);
    klog_print("Msg Sender: ");
    klog_print_hex((unsigned int)extracted_sender);
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
        klog_print("Die portion");
        TerminateProcess(current_process);
        scheduler();
    }
    else
    {
        klog_print("Pass Up portion");
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
