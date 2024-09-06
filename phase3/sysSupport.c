#include "./headers/sysSupport.h"
#include "../phase2/headers/klog.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 */

#define SUP_PROC_STATE(support_struct) (support_struct->sup_exceptState[GENERALEXCEPT])
#define SUP_SYSCALL_CODE(support_struct) ((int)SUP_PROC_STATE(support_struct).reg_a0)

/**
 * @brief Terminate the process.
 *
 * This function frees the frames associated with the given ASID,
 * signals the termination of the process, and terminates the process.
 *
 * @param support_struct Pointer to the support structure.
 */
static void terminate(support_t *support_struct)
{
    freeFrames(support_struct->sup_asid);
    signalProcessTermination(support_struct->sup_asid);
    TerminateProc(NULL);
}

int USendMsg(pcb_t *destination, unsigned int *payload, pcb_t *sender)
{
    // send message to parent, also called SST
    if (destination == 0)
    {
        destination = sender->p_parent;
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
    if (destination == swap_mutex_proc)
    {
        ssi_payload_PTR cast_payload = (ssi_payload_PTR)payload;
        message->ssi_payload.service_code = cast_payload->service_code;
        message->ssi_payload.arg = cast_payload->arg;
    }
    else if (sender == swap_mutex_proc)
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

pcb_t *UReceiveMsg(pcb_t *sender, unsigned int *payload)
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
        if (current_process == swap_mutex_proc)
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
 * @brief Gestore delle system call.
 */
static void SuppSystemcallHandler(support_t *support_structure)
{

    switch (SUP_SYSCALL_CODE(support_structure))
    {
    case 1:
        SUP_PROC_STATE(support_structure).reg_v0 = (unsigned int)USendMsg((pcb_t *)SUP_PROC_STATE(support_structure).reg_a1, (unsigned int *)SUP_PROC_STATE(support_structure).reg_a2, current_process);
        break;
    case 2:
        SUP_PROC_STATE(support_structure).reg_v0 = (unsigned int)UReceiveMsg((pcb_t *)SUP_PROC_STATE(support_structure).reg_a1, (unsigned int *)SUP_PROC_STATE(support_structure).reg_a2);
        break;
    default:
        trapExceptionHandler();
        break;
    }

    SUP_PROC_STATE(support_structure).pc_epc += WORD_SIZE;

    LDST(&SUP_PROC_STATE(support_structure));
}

/**
 * @brief Handles the trap exception.
 *
 * This function is responsible for handling the trap exception. It retrieves the support structure pointer
 * and checks if the current mutex process is the same as the current process. If they are the same, it calls
 * the SYSCALL function to send a message to the swap mutex process. Finally, it calls the terminate function
 * passing the support structure as an argument.
 */
void trapExceptionHandler()
{
    support_t *support_struct = GetSupportPtr();
    if (curr_mutex_proc == current_process)
    {
        SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);
    }
    terminate(support_struct);
}

/**
 * @brief Gestore delle general exceptions.
 */
void generalExceptionHandler()
{
    support_t *support_structure = GetSupportPtr();

    klog_print("Cause: ");
    klog_print_dec(CAUSE_GET_EXCCODE(support_structure->sup_exceptState[GENERALEXCEPT].cause));

    switch (CAUSE_GET_EXCCODE(support_structure->sup_exceptState[GENERALEXCEPT].cause))
    {
    case 8:
        SuppSystemcallHandler(support_structure);
        break;
    default:
        trapExceptionHandler();
        break;
    }
}