#include "./headers/initProc.h"

/**
 * Memory pool of support strucures
 */
static support_t support_arr[UPROCMAX];
/**
 * List of free support structs
 */
static struct list_head free_support;

int asid = 1;

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

/**
 * @return address of a free frame that will be used as a stack.
 */
memaddr getStackFrame()
{
    static int curr_offset = 0;
    memaddr ram_top;
    RAMTOP(ram_top);

    // starting from second last frame
    memaddr frame_address = (ram_top - PAGESIZE) - (curr_offset * PAGESIZE);
    curr_offset++;

    return frame_address;
}

int DoIO(unsigned int addr, int value)
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
 * Initializes the processor state of a new SST.
 * @param asid process ASID.
 * @param state points to the new processor state.
 */
static void initSSTState(int asid, state_t *state)
{
    state->reg_t9 = state->pc_epc = (memaddr)SST_server;
    state->reg_sp = getStackFrame();
    state->status = ALLOFF | EALLINTPLT; // all interrupts enabled + PLT enabled
    state->entry_hi = (asid << ENTRYHI_ASID_BIT);
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
    support->sup_asid = asid;

    // Inizializzazione gestori eccezioni
    memaddr page_fault_stack = getStackFrame();
    memaddr general_stack = getStackFrame();
    support->sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)TLBExceptionHandler;
    support->sup_exceptContext[PGFAULTEXCEPT].status = ALLOFF | IMON | IEPON | TEBITON; // Interrupt abilitati + PLT abilitato + Kernel mode
    support->sup_exceptContext[PGFAULTEXCEPT].stackPtr = page_fault_stack;
    support->sup_exceptContext[GENERALEXCEPT].pc = (memaddr)generalExceptionHandler;
    support->sup_exceptContext[GENERALEXCEPT].status = ALLOFF | IMON | IEPON | TEBITON; // Interrupt abilitati + PLT abilitato + Kernel mode
    support->sup_exceptContext[GENERALEXCEPT].stackPtr = general_stack;

    // Initialize page table
    // Process didn't start yet so we can use one designated stack for the exception handlers as a temporary frame.
    memaddr tmp_frame = general_stack - PAGESIZE + WORDLEN;
    initPageTable(asid, support->sup_privatePgTbl, tmp_frame);
}

/**
 * Initializes a SST thread. ASID 0 is reserved for kernel daemons.
 */
static void startSSTs()
{
    state_t state;
    support_t *support_struct = allocate();

    // initialize state
    initSSTState(0, &state);
    // initialize support struct
    initSupportStructure(0, support_struct);
    // request creation to the kernel
    CreateProcess(&state, support_struct);
}

/**
 * Initializes a U-proc.
 * @param asid ASID of the U-proc to initialize.
 * @returns pcb of the User process just created.
 */
pcb_PTR startProcess(int asid)
{
    state_t state;
    support_t *support_struct = allocate();

    // initialize state
    initUProcState(asid, &state);
    // initialize support struct
    initSupportStructure(asid, support_struct);
    // request creation to the kernel
    return CreateProcess(&state, support_struct);
}

/**
 * Whenever a U-proc terminates, a call is made to deallocate to return the Support Structure to the free list.
 * Must be called while terminating a U-proc.
 */
void signalProcessTermination()
{
    deallocate(GetSupportPtr());
}

/**
 * Test Process
 */
void test()
{
    // Initialize the Level 4/Phase 3 data structures.
    initSwapStructs();
    initFreeSupportList();

    // Initialize SST(s)
    for (int i = 1; i <= UPROCMAX; i++)
    {
        startSSTs();
    }

    // Wait for all U-proc termination,
    for (int i = 0; i < UPROCMAX; i++)
    {
        // Test process will wake up UPROCMAX times
        SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
        signalProcessTermination();
    }

    SYSCALL(TERMPROCESS, 0, 0, 0);
}