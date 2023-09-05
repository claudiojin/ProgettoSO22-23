#include "../headers/pandos_types.h"
#include "../headers/pandos_const.h"
#include "../headers/exceptions.h"
#include "../headers/scheduler.h"
#include "../headers/pcb.h"
#include "../headers/ash.h"
#include "../headers/ns.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

// LEVEL 3 GLOBAL VARIABLES

// integer indicating the number of started, but not yet terminated processes
int process_count;
// process can be either in the “ready,” “running,” or “blocked” (also known as “waiting”) state. This integer is
// the number of started, but not terminated processes that in are the “blocked” state due to an I/O or timer request
int softBlock_count;
// Tail pointer to a queue of pcbs that are in the “ready” state
struct list_head ready_queue;
// pointer to the pcb that is in the “running” state, i.e. the current executing process
pcb_t* curr_process;
// The Nucleus maintains one integer semaphore for each external (sub)device in μMPS3, plus one additional semaphore
// to support the Pseudo-clock. Since terminal devices are actually two independent sub-devices, the Nucleus
// maintains two semaphores for each terminal device
int device_semaphores[DEV_SEMAPHORES];
// pointer to process 0 pass up vector memory location
passupvector_t* passupvector;
state_t process0_state;
