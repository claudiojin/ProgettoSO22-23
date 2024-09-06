#include "./headers/vmSupport.h"
#include "../phase2/headers/klog.h"

/**  This module implements The Pager.
 * Since reading and writing to each U-proc’s flash device is limited to supporting paging, this module should
 * also contain the function(s) for reading and writing flash devices.
 * Additionally, the Swap Pool table is local to this module.
 * Instead of declaring them globally in initProc.c they can be declared module-wide in vmSupport.c.
 * The test function will now invoke a new “public” function initSwapStructs which will do the work of initializing
 * the Swap Pool table.
 */

/**
 * Address (virtual) for the start of the .data area. The .data area is placed immediately after the .text area at the
 * start of a 4KB block, i.e. .text Start Addr. + .text File Size
 */
#define DATA_START_ADDR_OFFSET 0x18
/**
 * Size of .data area in the .aout file. Different from the .data Mem. Size since it doesn’t include the .bss area
 * but is padded to the nearest 4KB block boundary
 */
#define DATA_FILE_SIZE_OFFSET 0x24
/**
 * Calculate the starting frame of the swap pool
 */
#define SWAP_POOL_START (*((int *)(KERNELSTACK + DATA_START_ADDR_OFFSET)) + *((int *)(KERNELSTACK + DATA_FILE_SIZE_OFFSET)))

#define IS_FREE_FRAME(frame) ((frame)->sw_asid == NOPROC)
#define FRAME_NUMBER(frame_addr) ((frame_addr - SWAP_POOL_START) / PAGESIZE)                      // Returns frame index given its address
#define FRAME_ADDRESS(index) (SWAP_POOL_START + (index) * PAGESIZE)                               // Returns frame address given its index
#define INDEX_OF_POOL_TABLE(frame) (((memaddr)frame - (memaddr)swap_pool_table) / sizeof(swap_t)) // Returns frame index given the swap pool table entry
#define GET_FRAME_ADDRESS(frame) (FRAME_ADDRESS(INDEX_OF_POOL_TABLE(frame)))                      // Returns frame address given swap pool table entry

#define DISABLE_INTERRUPTS setSTATUS(getSTATUS() & ~IECON)
#define ENABLE_INTERRUPTS setSTATUS(getSTATUS() | IECON)

// p bit (page number) mask for page table
#define INDEX_P_BIT_MASK 0x80000000

/**
 * A Swap Pool is a set of RAM frames set aside to support virtual memory. To ensure the proper exercise
 * of μPandOS’s paging functionality, the size of the Swap Pool should be set to two times UPROCMAX
 */
static swap_t swap_pool_table[POOLSIZE];

/**
 * Mutex function for the swap pool
 */
static void swap_mutex()
{
    while (TRUE)
    {
        if (curr_mutex_proc == NULL)
        {
            // receive request for mutual exclusion from a process
            pcb_PTR sender = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0);
            // send a message back to sender to notify it
            SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
        }
        else
        {
            // receive a message from current mutex process to release mutual exclusion
            SYSCALL(RECEIVEMESSAGE, (unsigned int)curr_mutex_proc, 0, 0);
            curr_mutex_proc = NULL;
        }
    }
};

/**
 * @param entry_hi EntryHI CP0 register.
 * @return page table entry index of the process.
 */
static int getPageIndex(unsigned int entry_hi)
{
    memaddr vpn = ((0b1 << 19) + ENTRYHI_GET_VPN(entry_hi));

    if (vpn == 0xBFFFF)
        return 31;

    return vpn % 0x80000;
}

/**
 * Executes a READ/WRITE operation on the backing store
 * @param operation_command Type of operation (FLASHWRITE/FLASHREAD)
 * @param asid              Process ASID
 * @param page_num          Number of the page to execute operation onto
 * @param frame_address     Starting physical address of the 4k block to be read (or written);
 */
static void backingStoreOperation(int operation_command, int asid, int page_num, memaddr frame_address)
{
    // since asid starts from 1 device number must be decremented
    dtpreg_t *flash_dev_reg = (dtpreg_t *)DEV_REG_ADDR(IL_FLASH, asid - 1);

    // Write the flash device’s DATA0 field with the appropriate starting physical address of the 4k
    // block to be read (or written); the particular frame’s starting address
    flash_dev_reg->data0 = frame_address;

    // Initialize command value
    int command = (page_num << 8) + operation_command;

    int status = DoIO(&flash_dev_reg->command, command);

    if (status == FLASH_WRITE_ERROR || status == FLASH_READ_ERROR)
    {
        trapExceptionHandler();
    }
}

/**
 * Writes on correct flash drive the data of a selected process page.
 * @param asid              Process ASID
 * @param page_num          Number of the page to write
 * @param frame_address     Starting physical address of the 4k block to be written
 */
static void writePageToFlash(int asid, int page_num, memaddr frame_address)
{
    backingStoreOperation(FLASHWRITE, asid, page_num, frame_address);
}

/**
 * Reads from correct flash drive, the data of a selected process page.
 * @param asid              Process ASID
 * @param page_num          Number of the page to read
 * @param frame_address     Starting physical address of the 4k block to be read
 */
static void readPageFromFlash(int asid, int page_num, memaddr frame_address)
{
    backingStoreOperation(FLASHREAD, asid, page_num, frame_address);
}

/**
 * Updates (if it exists) an entry of the page table in the TLB
 * @param entry Page table entry to update
 */
static void updateTLB(pteEntry_t *entry)
{
    // Probing TLB in search of frame
    setENTRYHI(entry->pte_entryHI);
    TLBP();

    // Update TLB
    if ((getINDEX() & INDEX_P_BIT_MASK) == 0)
    {
        setENTRYHI(entry->pte_entryHI);
        setENTRYLO(entry->pte_entryLO);
        TLBWI();
    }
}

/**
 * If frame i is currently occupied, assume it is occupied by logical page number k belonging to
 * process x (ASID) and that it is “dirty” (i.e. been modified)
 * @param frame Pointer to invalid swap pool frame
 */
static void storePage(swap_t *frame)
{
    // The follwing steps must be accomplished atomically
    DISABLE_INTERRUPTS;
    // Update process x's Page Table. Mark Page Table entry k as not valid
    frame->sw_pte->pte_entryLO = frame->sw_pte->pte_entryLO & ~VALIDON;
    // Update the TLB, if needed.
    updateTLB(frame->sw_pte);
    ENABLE_INTERRUPTS;

    // Update process x’s backing store
    writePageToFlash(frame->sw_asid, frame->sw_pageNo, GET_FRAME_ADDRESS(frame));
}

/**
 * @brief Handles the loading of a page into memory
 * @param pt_entry  Pointer to page table entry to load
 * @param frame     Pointer to swap pool frame that will hold the page
 */
static void loadPage(pteEntry_t *pt_entry, swap_t *frame)
{
    int asid = ENTRYHI_GET_ASID(pt_entry->pte_entryHI);
    int page_num = getPageIndex(pt_entry->pte_entryHI);

    // Caricamento della nuova pagina in memoria
    readPageFromFlash(asid, page_num, GET_FRAME_ADDRESS(frame));

    // Update the Swap Pool table’s entry i to reflect frame i’s new contents: page p belonging to the
    // Current Process’s ASID, and a pointer to the Current Process’s Page Table entry for page p
    frame->sw_asid = asid;
    frame->sw_pageNo = page_num;
    frame->sw_pte = pt_entry;

    // The follwing steps must be accomplished atomically
    DISABLE_INTERRUPTS;
    pt_entry->pte_entryLO = (pt_entry->pte_entryLO & ~ENTRYLO_PFN_MASK) | GET_FRAME_ADDRESS(frame) | VALIDON;
    updateTLB(pt_entry);
    ENABLE_INTERRUPTS;
}

/**
 * Implements the Page Replacement Algorithm (FIFO)
 * @returns swap pool table entry of selected frame.
 */
static swap_t *getFrame()
{
    static int frame_index = 0;
    swap_t *frame_entry = NULL;

    // moves counter to a free frame if it exists, does a full circle otherwise
    for (int i = 0; i < POOLSIZE; i++)
    {
        if (IS_FREE_FRAME(&swap_pool_table[frame_index]))
        {
            break;
        }
        frame_index = (frame_index + 1) % POOLSIZE;
    }

    // extract frame from swap pool table
    frame_entry = &swap_pool_table[frame_index];

    // increment counter, which is possibly the next free frame
    frame_index = (frame_index + 1) % POOLSIZE;

    return frame_entry;
}

/**
 * TLB invalid handler
 * @param support_structure support structure of the process that raised the exception
 */
static void TLBInvalidHandler(support_t *support_structure)
{
    // Gain mutual exclusion
    SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);

    // Determine the missing page number (denoted as p): found in the saved exception state’s EntryHi
    int missing_page_index = getPageIndex(support_structure->sup_exceptState[PGFAULTEXCEPT].entry_hi);
    pteEntry_t *page_pt_entry = &support_structure->sup_privatePgTbl[missing_page_index];

    // Pick a frame, i, from the Swap Pool. Which frame is selected is determined by the μPandOS page replacement algorithm
    swap_t *new_frame = getFrame();

    // Determine if frame i is occupied; examine entry i in the Swap Pool table
    if (!IS_FREE_FRAME(new_frame))
    {
        storePage(new_frame);
    }
    loadPage(page_pt_entry, new_frame);

    // release mutual exclusion
    SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);

    LDST(&support_structure->sup_exceptState[PGFAULTEXCEPT]);
}

/**
 * @brief Gestore delle eccezioni TLB.
 */
void TLBExceptionHandler()
{
    support_t *support_structure = GetSupportPtr();

    switch (CAUSE_GET_EXCCODE(support_structure->sup_exceptState[PGFAULTEXCEPT].cause))
    {
    case TLBMOD:
        trapExceptionHandler();
        break;

    case TLBINVLDL:
    case TLBINVLDS:
        TLBInvalidHandler(support_structure);
        break;

    default:
        PANIC();
        break;
    }
}

/**
 * Frees the frames linked to a process
 * @param asid Process ASID.
 */
void freeFrames(int asid)
{
    // Gain mutual exclusion
    SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);

    for (int i = 0; i < POOLSIZE; i++)
    {
        if (swap_pool_table[i].sw_asid == asid)
        {
            swap_pool_table[i].sw_asid = NOPROC;
            swap_pool_table[i].sw_pageNo = 0;
            swap_pool_table[i].sw_pte = NULL;
        }
    }

    // Release mutual exclusion
    SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex_proc, 0, 0);
}

/**
 * Initializes a process page table.
 * @param asid          Process ASID.
 * @param page_table    Pointer to process page table.
 * @param tmp_frame     Address of a temporary frame.
 */
void initPageTable(int asid, pteEntry_t *page_table, memaddr tmp_frame)
{
    // Extraction of upper part
    readPageFromFlash(asid, 0, tmp_frame);
    int *aout_header = (int *)tmp_frame;

    // Calculate number of .text pages
    int text_file_size = aout_header[5];
    int num_text_file = text_file_size / PAGESIZE;

    // Initialize this process page table
    for (int i = 0; i < 31; i++)
    {
        page_table[i].pte_entryHI = ((0x80000 + i) << ENTRYHI_VPN_BIT) + (asid << ENTRYHI_ASID_BIT);
        if (i < num_text_file)
        {
            page_table[i].pte_entryLO = 0;
        }
        else
        {
            page_table[i].pte_entryLO = 0 | ENTRYLO_DIRTY;
        }
    }
    // Last page for the stack
    page_table[31].pte_entryHI = ((0xBFFFF) << ENTRYHI_VPN_BIT) + (asid << ENTRYHI_ASID_BIT);
    page_table[31].pte_entryLO = 0 | ENTRYLO_DIRTY;
}

/**
 * Initializes data structures for virtual memory support.
 */
void initSwapStructs()
{
    state_t swap_mutex_state;

    STST(&swap_mutex_state);
    swap_mutex_state.reg_sp = getStackFrame();
    swap_mutex_state.pc_epc = (memaddr)swap_mutex;
    swap_mutex_state.status |= EALLINTPLT; // all interrupts enabled + PLT enabled

    // create swap mutex process
    swap_mutex_proc = CreateProcess(&swap_mutex_state, NULL);

    // initialize swap pool table
    for (int i = 0; i < POOLSIZE; i++)
    {
        swap_pool_table[i].sw_asid = NOPROC;
        swap_pool_table[i].sw_pageNo = 0;
        swap_pool_table[i].sw_pte = NULL;
    }
}
