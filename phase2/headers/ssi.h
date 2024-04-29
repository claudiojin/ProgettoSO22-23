#ifndef SSI_H
#define SSI_H

#include "./initial.h"

/**
 * This module implements the System Service Interface.
 */

// Cause the sender or another process to terminate, included all of the progeny
void TerminateProcess(pcb_t* process);

//SSI basic server algorithm (implements the RPC)
void SSI_server();

#endif 
