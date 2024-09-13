#ifndef INITPROC
#define INITPROC

#include "../../phase2/headers/initial.h"
#include "./vmSupport.h"
#include "./sst.h"
#include "./sysSupport.h"

/**
 - One Page Table per U-proc. A μPandOS Page Table will be an array of 32 Page Table entries.
Each Page Table entry is a doubleword consisting of an EntryHi and an EntryLo portion [Section
6.3.2-pops]. This array should be added to the Support Structure (support_t) that is pointed
to by a U-proc’s PCB.
Important: TLB entries and Page Table entries are identical in structure: a doubleword con-
sisting of an EntryHi and an EntryLo portion. Which term is used will be dependent on context.
- The Swap Pool; a set of RAM frames reserved for virtual memory. Logical pages will occupy
these frames when present. The size of the Swap Pool should be set to two times UPROCMAX,
where UPROCMAX is defined as the specific degree of multiprogramming to be supported: [1...8].
The Swap Pool is not so much a Support Level data structure, but a set of RAM frames reserved
to support paging.
- The Swap Pool data structure/table. The Support Level will maintain a table, one entry per
Swap Pool frame, recording information about the logical page occupying it. At a minimum,
each entry should record the ASID and logical page number of the occupying page.
- The Swap Mutex process. A process that provides mutual exclusion access to the Swap Pool
table.
- Backing store; secondary storage that contains each U-proc’s complete logical image – which for
μPandOS is limited to 32 pages in size. Associated with each U-proc is a flash device which will
be configured (preloaded) to contain that U-proc’s logical image. While slightly unrealistic, this
basic version of the Support Level will use each U-proc’s flash device as its backing store device.
 */

/**
 * Swap pool process to guarantee mutual exclusion.
 * To access the Swap Pool table, a process must first perform a message to this process and waiting
 * for a response message. When access to the Swap Pool table is concluded, a process will then send a
 * message to this process to release the mutual exclusion
 */
extern pcb_PTR swap_mutex_proc;
/**
 * Process currently accessing the mutually exclusive swap pool
 */
extern pcb_PTR curr_mutex_proc;

// Pcb of device mutex processes
extern pcb_PTR terminal_mutex_proc[UPROCMAX];
extern pcb_PTR printer_mutex_proc[UPROCMAX];

// Current process accessing in mutex the corresponding device
extern pcb_PTR curr_terminal_mutex[UPROCMAX];
extern pcb_PTR curr_printer_mutex[UPROCMAX];

pcb_PTR startProcess(int asid, support_t *sst_support);
memaddr getStackFrame();

pcb_t *CreateProcess(state_t *state, support_t *supp);
void UTerminate(pcb_t *u_proc);
int DoIO(unsigned int *addr, unsigned int value);
support_t *GetSupportPtr();

void test();

#endif