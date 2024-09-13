#include "./headers/initProc.h"
#include "../phase2/headers/klog.h"

// TODO: finire di sistemare le print, controllare il payload nella sendmessage quando l'sst invia
// un payload al u-proc.

/**
 * Memory pool of support strucures
 */
static support_t support_arr[UPROCMAX];
/**
 * List of free support structs
 */
static struct list_head free_support;

pcb_PTR swap_mutex_proc;
pcb_PTR curr_mutex_proc;

static void **terminal_mutex[UPROCMAX];
static void **printer_mutex[UPROCMAX];

pcb_PTR terminal_mutex_proc[UPROCMAX];
pcb_PTR printer_mutex_proc[UPROCMAX];

pcb_PTR curr_terminal_mutex[UPROCMAX];
pcb_PTR curr_printer_mutex[UPROCMAX];

// grants mutual exclusion while using terminals and printers
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

// grants mutual exclusion while using terminals and printers
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

// grants mutual exclusion while using terminals and printers
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

// grants mutual exclusion while using terminals and printers
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

// grants mutual exclusion while using terminals and printers
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

// grants mutual exclusion while using terminals and printers
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

// grants mutual exclusion while using terminals and printers
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

// grants mutual exclusion while using terminals and printers
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
 * Whenever a new Support Structure is needed to support a new U-proc, a call to allocate returns a pointer to a
 * Support Structure, allocated from the free list.
 * @return a support structure ready for use.
 */
static support_t *allocate()
{
    // Head removal from free list
    support_t *support_structure = container_of(free_support.next, support_t, s_list);
    list_del(free_support.next);

    return support_structure;
}

/**
 * Upon entry, test iterates over the static array of Support Structures, invoking a new deallocate method
 * to add each Support Structure to the free list.
 * @param support_structure pointer of the support structure to deallocate.
 */
static void deallocate(support_t *support_structure)
{
    list_add(&support_structure->s_list, &free_support);
}

// Signal test process of u-proc termination
void signalUprocTermination()
{
    SYSCALL(SENDMESSAGE, (unsigned int)test_pcb, 0, 0);
}

/**
 * Initializes support structure list.
 */
static void initFreeSupportList()
{
    // Just like phase 1 lists, take the structs from the memory pool and put them in a double-linked list
    INIT_LIST_HEAD(&free_support);

    for (int i = 0; i < UPROCMAX; i++)
    {
        deallocate(&support_arr[i]);
    }
}

static void initPrinterMutex(int asid)
{
    state_t device_mutex_state;

    STST(&device_mutex_state);
    device_mutex_state.reg_sp = getStackFrame();
    device_mutex_state.pc_epc = (memaddr)printer_mutex[asid - 1];
    device_mutex_state.status |= EALLINTPLT; // all interrupts enabled + PLT enabled

    // create PRINTER mutex process
    printer_mutex_proc[asid - 1] = CreateProcess(&device_mutex_state, NULL);
}

static void initTerminalMutex(int asid)
{
    state_t device_mutex_state;

    STST(&device_mutex_state);
    device_mutex_state.reg_sp = getStackFrame();
    device_mutex_state.pc_epc = (memaddr)terminal_mutex[asid - 1];
    device_mutex_state.status |= EALLINTPLT; // all interrupts enabled + PLT enabled

    // create TERMINAL mutex process
    terminal_mutex_proc[asid - 1] = CreateProcess(&device_mutex_state, NULL);
}

/**
 * @return address of a free frame that will be used as a stack.
 */
memaddr getStackFrame()
{
    static int curr_offset = 1;
    memaddr ram_top;
    RAMTOP(ram_top);

    // starting from third last frame (test and ssi stack)
    memaddr frame_address = (ram_top - PAGESIZE) - (curr_offset * PAGESIZE);
    curr_offset++;

    return frame_address;
}

int DoIO(unsigned int *addr, unsigned int value)
{
    int status;
    ssi_do_io_t do_io = {
        .commandAddr = addr,
        .commandValue = value,
    };
    ssi_payload_t payload = {
        .service_code = DOIO,
        .arg = &do_io,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);

    return status;
}

/** Requests a CreateProcess service directly to the SSI
 * @param state Pointer to the initial processor state for the U-proc
 * @param supp Pointer to an initialized Support Structure for the U-proc
 */
pcb_t *CreateProcess(state_t *state, support_t *supp)
{
    pcb_t *p;
    ssi_create_process_t ssi_create_process = {
        .state = state,
        .support = supp,
    };
    ssi_payload_t payload = {
        .service_code = CREATEPROCESS,
        .arg = &ssi_create_process,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&p), 0);
    return p;
}

/**
 * @param arg process to terminate
 */
void TerminateProc(pcb_t *arg)
{
    ssi_payload_t term_process_payload = {
        .service_code = TERMPROCESS,
        .arg = (void *)arg,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&term_process_payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);
}

/**
 * @brief Terminate the User process with its SST.
 *
 * This function frees the frames associated with the given ASID,
 * signals the termination of the process, and terminates the process.
 *
 * @param support_struct Pointer to the support structure.
 */
void UTerminate(pcb_t *u_proc)
{
    if (u_proc->p_supportStruct == NULL)
    {
        // klog_print("USER support struct is null while terminating!");
    }

    freeFrames(u_proc->p_supportStruct->sup_asid);

    // Whenever a U-proc terminates, a call is made to deallocate to return the Support Structure to the free list.
    deallocate(u_proc->p_supportStruct);

    signalUprocTermination();

    // terminates this u-proc sst consequently killing its progeny (the child u-proc)
    TerminateProc(u_proc->p_parent);
}

/** @returns the support structure for the current process */
support_t *GetSupportPtr()
{
    support_t *support;
    ssi_payload_t getsup_payload = {
        .service_code = GETSUPPORTPTR,
        .arg = NULL,
    };

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&getsup_payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&support), 0);
    return support;
}

/**
 * Initializes the processor state of a new U-proc.
 * @param asid process ASID.
 * @param state points to the new processor state.
 */
static void initUProcState(int asid, state_t *state)
{
    state->reg_t9 = state->pc_epc = (memaddr)0x800000B0;
    state->reg_sp = 0xC0000000;
    state->status = ALLOFF | IMON | IEPON | TEBITON | USERPON; // All interrupts enable + PLT enabled + User mode
    state->entry_hi = (asid << ENTRYHI_ASID_BIT);
}

/**
 * Initialize a process support struct.
 * @param asid process ASID.
 * @param support points to the new support structure.
 */
static void initSupportStructure(int asid, support_t *support)
{
    // Initialize asid
    support->sup_asid = asid;

    // Initialize exception handlers
    memaddr page_fault_stack = getStackFrame();
    memaddr general_stack = getStackFrame();
    support->sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)TLBExceptionHandler;
    support->sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | (IMON | IEPON) | TEBITON; // Interrupt abilitati + PLT abilitato + Kernel mode
    support->sup_exceptContext[PGFAULTEXCEPT].stackPtr = page_fault_stack;
    support->sup_exceptContext[GENERALEXCEPT].pc = (memaddr)generalExceptionHandler;
    support->sup_exceptContext[GENERALEXCEPT].status = ALLOFF | (IMON | IEPON) | TEBITON; // Interrupt abilitati + PLT abilitato + Kernel mode
    support->sup_exceptContext[GENERALEXCEPT].stackPtr = general_stack;

    // Initialize page table
    // Process didn't start yet so we can use one designated stack for the exception handlers as a temporary frame.
    memaddr tmp_frame = general_stack - PAGESIZE + WORDLEN;
    initPageTable(asid, support->sup_privatePgTbl, tmp_frame);
}

/**
 * Initializes a SST thread.
 */
static void startSSTs(int asid)
{
    state_t sst_state;
    support_t *support_struct = allocate();

    // initialize state
    STST(&sst_state);
    sst_state.pc_epc = (memaddr)SST_server;
    sst_state.reg_sp = getStackFrame();
    sst_state.status |= EALLINTPLT;                  // all interrupts enabled + PLT enabled
    sst_state.entry_hi = (asid << ENTRYHI_ASID_BIT); // has the same asid as the child U-proc
    // initialize support struct
    initSupportStructure(asid, support_struct);
    // request creation to the kernel
    CreateProcess(&sst_state, support_struct);
}

/**
 * Initializes a U-proc.
 * @param asid ASID of the U-proc to initialize.
 * @returns pcb of the User process just created.
 */
pcb_PTR startProcess(int asid, support_t *sst_support)
{
    state_t state;

    // initialize state
    initUProcState(asid, &state);
    // request creation to the kernel, suuport structure shared with parent sst
    return CreateProcess(&state, sst_support);
}

static void assignMutexFunctions()
{
    printer_mutex[0] = (void *)&printer_mutex_1;
    printer_mutex[1] = (void *)&printer_mutex_2;
    printer_mutex[2] = (void *)&printer_mutex_3;
    printer_mutex[3] = (void *)&printer_mutex_4;
    printer_mutex[4] = (void *)&printer_mutex_5;
    printer_mutex[5] = (void *)&printer_mutex_6;
    printer_mutex[6] = (void *)&printer_mutex_7;
    printer_mutex[7] = (void *)&printer_mutex_8;

    terminal_mutex[0] = (void *)&terminal_mutex_1;
    terminal_mutex[1] = (void *)&terminal_mutex_2;
    terminal_mutex[2] = (void *)&terminal_mutex_3;
    terminal_mutex[3] = (void *)&terminal_mutex_4;
    terminal_mutex[4] = (void *)&terminal_mutex_5;
    terminal_mutex[5] = (void *)&terminal_mutex_6;
    terminal_mutex[6] = (void *)&terminal_mutex_7;
    terminal_mutex[7] = (void *)&terminal_mutex_8;
}

/**
 * Test Process
 */
void test()
{
    // Initialize the Level 4/Phase 3 data structures.
    initFreeSupportList();
    initSwapStructs();
    assignMutexFunctions();

    // Initialize SST(s) with corresponding U-proc + Device mutex processes
    for (int i = 1; i <= UPROCMAX; i++)
    {
        initPrinterMutex(i);
        initTerminalMutex(i);
        startSSTs(i);
    }

    // Wait for all U-proc termination,
    for (int i = 0; i < UPROCMAX; i++)
    {
        // Test process will wake up UPROCMAX times
        SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
    }

    SYSCALL(TERMPROCESS, 0, 0, 0);
}