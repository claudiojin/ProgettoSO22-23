#include "./headers/sysSupport.h"
#include "../phase2/headers/klog.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 */

#define SUP_PROC_STATE(support_struct) (support_struct->sup_exceptState[GENERALEXCEPT])
#define SUP_SYSCALL_CODE(support_struct) ((int)SUP_PROC_STATE(support_struct).reg_a0)

/**
 * SendMessage Syscall wrapper for support level
 * TODO: trovare un modo per avere il puntatore al padre senza utilizzare il current proc
 */
int USendMsg(pcb_t *destination, unsigned int *payload)
{
    // send message to parent, also called SST
    if (destination == 0)
    {
        destination = current_process->p_parent;
    }

    // U-proc always sends an ssi_payload_t struct pointer
    return SYSCALL(SENDMESSAGE, (unsigned int)destination, (unsigned int)payload, 0);
}

pcb_t *UReceiveMsg(pcb_t *sender, unsigned int *payload)
{
    return (pcb_PTR)SYSCALL(RECEIVEMESSAGE, (unsigned int)sender, (unsigned int)payload, 0);
}

/**
 * @brief Gestore delle system call.
 */
static void SuppSystemcallHandler(support_t *support_structure)
{

    switch (SUP_SYSCALL_CODE(support_structure))
    {
    case 1:
        SUP_PROC_STATE(support_structure).reg_v0 = (unsigned int)USendMsg((pcb_t *)SUP_PROC_STATE(support_structure).reg_a1, (unsigned int *)SUP_PROC_STATE(support_structure).reg_a2);
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
 * This function is responsible for handling the trap exception. It releases mutual exclusion if needed and
 * terminates the user process.
 */
void trapExceptionHandler()
{
    // release mutual exclusion
    if (curr_mutex_proc == current_process)
    {
        SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);
    }

    // Terminates U-proc and its SST (its parent)
    UTerminate(current_process);
}

/**
 * @brief Gestore delle general exceptions.
 */
void generalExceptionHandler()
{
    // get current process(User proc that raised the exception) support structure
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