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

/**
 * SendMessage Syscall wrapper for support level
 * TODO: trovare un modo per avere il puntatore al padre senza utilizzare il current proc
 */
int USendMsg(pcb_t *destination, unsigned int payload)
{
    // send message to parent, also called SST
    if (destination == 0)
    {
        destination = current_process->p_parent;
    }

    return SYSCALL(SENDMESSAGE, (unsigned int)destination, (unsigned int)payload, 0);
}

pcb_t *UReceiveMsg(pcb_t *sender, unsigned int payload)
{
    return (pcb_PTR)SYSCALL(RECEIVEMESSAGE, (unsigned int)payload, 0, 0);
}

/**
 * @brief Gestore delle system call.
 */
static void SuppSystemcallHandler(support_t *support_structure)
{

    switch (SUP_SYSCALL_CODE(support_structure))
    {
    case 1:
        SUP_PROC_STATE(support_structure).reg_v0 = (unsigned int)USendMsg((pcb_t *)SUP_PROC_STATE(support_structure).reg_a1, (unsigned int)SUP_PROC_STATE(support_structure).reg_a2);
        break;
    case 2:
        SUP_PROC_STATE(support_structure).reg_v0 = (unsigned int)UReceiveMsg((pcb_t *)SUP_PROC_STATE(support_structure).reg_a1, (unsigned int)SUP_PROC_STATE(support_structure).reg_a2);
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

// 0x2000321C

/**
 * @brief Gestore delle general exceptions.
 */
void generalExceptionHandler()
{
    support_t *support_structure = GetSupportPtr();

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