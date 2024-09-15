#include "./headers/deviceMutex.h"

static void **terminal_mutex[UPROCMAX];
static void **printer_mutex[UPROCMAX];

static void terminal_mutex_1()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[0] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[0] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[0], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[0], 0, 0);
            curr_terminal_mutex[0] = NULL;
        }
    }
};

static void terminal_mutex_2()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[1] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[1] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[1], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[1], 0, 0);
            curr_terminal_mutex[1] = NULL;
        }
    }
};

static void terminal_mutex_3()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[2] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[2] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[2], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[2], 0, 0);
            curr_terminal_mutex[2] = NULL;
        }
    }
};

static void terminal_mutex_4()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[3] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[3] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[3], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[3], 0, 0);
            curr_terminal_mutex[3] = NULL;
        }
    }
};

static void terminal_mutex_5()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[4] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[4] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[4], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[4], 0, 0);
            curr_terminal_mutex[4] = NULL;
        }
    }
};

static void terminal_mutex_6()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[5] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[5] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[5], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[5], 0, 0);
            curr_terminal_mutex[5] = NULL;
        }
    }
};

static void terminal_mutex_7()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[6] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[6] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[6], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[6], 0, 0);
            curr_terminal_mutex[6] = NULL;
        }
    }
};

static void terminal_mutex_8()
{
    while (TRUE)
    {
        if (curr_terminal_mutex[7] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_terminal_mutex[7] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_terminal_mutex[7], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_terminal_mutex[7], 0, 0);
            curr_terminal_mutex[7] = NULL;
        }
    }
};

static void printer_mutex_1()
{
    while (TRUE)
    {
        if (curr_printer_mutex[0] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[0] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[0], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[0], 0, 0);
            curr_printer_mutex[0] = NULL;
        }
    }
}

static void printer_mutex_2()
{
    while (TRUE)
    {
        if (curr_printer_mutex[1] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[1] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[1], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[1], 0, 0);
            curr_printer_mutex[1] = NULL;
        }
    }
}

static void printer_mutex_3()
{
    while (TRUE)
    {
        if (curr_printer_mutex[2] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[2] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[2], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[2], 0, 0);
            curr_printer_mutex[2] = NULL;
        }
    }
}

static void printer_mutex_4()
{
    while (TRUE)
    {
        if (curr_printer_mutex[3] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[3] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[3], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[3], 0, 0);
            curr_printer_mutex[3] = NULL;
        }
    }
}

static void printer_mutex_5()
{
    while (TRUE)
    {
        if (curr_printer_mutex[4] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[4] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[4], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[4], 0, 0);
            curr_printer_mutex[4] = NULL;
        }
    }
}

static void printer_mutex_6()
{
    while (TRUE)
    {
        if (curr_printer_mutex[5] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[5] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[5], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[5], 0, 0);
            curr_printer_mutex[5] = NULL;
        }
    }
}

static void printer_mutex_7()
{
    while (TRUE)
    {
        if (curr_printer_mutex[6] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[6] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[6], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[6], 0, 0);
            curr_printer_mutex[6] = NULL;
        }
    }
}

static void printer_mutex_8()
{
    while (TRUE)
    {
        if (curr_printer_mutex[7] == NULL)
        {
            // receive request for mutual exclusion from a process
            curr_printer_mutex[7] = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)curr_printer_mutex[7], 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_printer_mutex[7], 0, 0);
            curr_printer_mutex[7] = NULL;
        }
    }
}

/**
 * Assigns Printer or Terminal Mutex functions for later process initialization
 */
static void assignMutexFunctions(int printer)
{
    if (printer)
    {
        printer_mutex[0] = (void *)&printer_mutex_1;
        printer_mutex[1] = (void *)&printer_mutex_2;
        printer_mutex[2] = (void *)&printer_mutex_3;
        printer_mutex[3] = (void *)&printer_mutex_4;
        printer_mutex[4] = (void *)&printer_mutex_5;
        printer_mutex[5] = (void *)&printer_mutex_6;
        printer_mutex[6] = (void *)&printer_mutex_7;
        printer_mutex[7] = (void *)&printer_mutex_8;
    }
    else
    {
        terminal_mutex[0] = (void *)&terminal_mutex_1;
        terminal_mutex[1] = (void *)&terminal_mutex_2;
        terminal_mutex[2] = (void *)&terminal_mutex_3;
        terminal_mutex[3] = (void *)&terminal_mutex_4;
        terminal_mutex[4] = (void *)&terminal_mutex_5;
        terminal_mutex[5] = (void *)&terminal_mutex_6;
        terminal_mutex[6] = (void *)&terminal_mutex_7;
        terminal_mutex[7] = (void *)&terminal_mutex_8;
    }
}

/**
 * Initializes Printer Mutex processes
 */
void initPrinterMutex(int asid)
{
    assignMutexFunctions(TRUE);

    state_t device_mutex_state;

    STST(&device_mutex_state);
    device_mutex_state.reg_sp = getStackFrame();
    device_mutex_state.pc_epc = (memaddr)printer_mutex[asid - 1];
    device_mutex_state.status |= EALLINTPLT; // all interrupts enabled + PLT enabled

    // create PRINTER mutex process
    printer_mutex_proc[asid - 1] = CreateProcess(&device_mutex_state, NULL);
}

/**
 * Initializes Terminal Mutex processes
 */
void initTerminalMutex(int asid)
{
    assignMutexFunctions(FALSE);

    state_t device_mutex_state;

    STST(&device_mutex_state);
    device_mutex_state.reg_sp = getStackFrame();
    device_mutex_state.pc_epc = (memaddr)terminal_mutex[asid - 1];
    device_mutex_state.status |= EALLINTPLT; // all interrupts enabled + PLT enabled

    // create TERMINAL mutex process
    terminal_mutex_proc[asid - 1] = CreateProcess(&device_mutex_state, NULL);
}