#include "./headers/sysSupport.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 */

#define PROCESSOR_STATE(support_struct) (support_struct->sup_exceptState[GENERALEXCEPT])
#define SYSTEMCALL_CODE(support_struct) ((int)PROCESSOR_STATE(support_struct).reg_a0)
#define PARAM1(type, name, support_struct) type name = (type)PROCESSOR_STATE(support_struct).reg_a1
#define PARAM2(type, name, support_struct) type name = (type)PROCESSOR_STATE(support_struct).reg_a2
#define PARAM3(type, name, support_struct) type name = (type)PROCESSOR_STATE(support_struct).reg_a3
#define SYSTEMCALL_RETURN(return_value, support_t) PROCESSOR_STATE(support_struct).reg_v0 = (int)return_value

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
 * @brief Gestore delle system call.
 */
static void _systemcallHandler(support_t *support_structure)
{

    switch (SYSTEMCALL_CODE(support_structure))
    {
    case 1: // TODO CLAUDIO: CONTROLLARE SE IL CODICE È GIUSTO
        SendMsg(support_structure);
        break;
    case 2: // TODO CLAUDIO: CONTROLLARE SE IL CODICE È GIUSTO
        ReceiveMsg(support_structure);
        break;
    default:
        trapExceptionHandler();
        break;
    }

    support_structure->sup_exceptState[GENERALEXCEPT].pc_epc += WORD_SIZE;

    LDST(&support_structure->sup_exceptState[GENERALEXCEPT]);
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
        SYSCALL(SENDMESSAGE, swap_mutex_proc, 0, 0);
    }
    terminate(support_struct);
}

/**
 * @brief Gestore delle general exceptions.
 */
void generalExceptionHandler()
{
    support_t *support_structure = GetSupportPtr();

    switch (CAUSE_GET_EXCCODE(support_structure->sup_exceptState[GENERALEXCEPT].cause))
    {
    case 8: // TODO CLAUDIO: CONTROLLARE SE IL CODICE È GIUSTO
        _systemcallHandler(support_structure);
        break;
    default:
        trapExceptionHandler();
        break;
    }
}
