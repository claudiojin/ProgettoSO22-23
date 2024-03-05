#ifndef SSI_H
#define SSI_H

#include "./headers/exceptions.h"
#include "./headers/scheduler.h"

/**
 * This module implements the System Service Interface.
 */

// Mnemonic constants
const int CREATEPROCESS = 1;
const int TERMINATEPROCESS = 2;
const int DOIO = 3;
const int GETTIME = 4;
const int CLOCKWAIT = 5;
const int GETSUPPORTPTR = 6;
const int GETPROCESSID = 7;

// Global variable to represent SSI process
extern pcb_t* ssi_process;

// Structures defined for the DoIO service
typedef struct {
    unsigned int commandAddr;
    unsigned int commandValue;
} ssi_do_io_t;

typedef struct {
    int service_code;
    void* arg;
} ssi_payload_t;


// Standard procedure to be called by processes to make service requests
void SSIRequest(pcb_t* sender, int service, void* arg);

// Create a new process, progeny of the sender
pcb_t* create_process(ssi_create_process_t* args);

// Cause the sender or another process to terminate, included all of the progeny
void terminate_process(pcb_t* process);

// Expansion of the previous function, to distinguish between sender termination
// and other cases. If target is NULL, sender is the process chosen for termination.
void terminate_process_service(pcb_t* sender, pcb_t* target_process);

// Get the CPU time for the sender process
int get_cpu_time(pcb_t* sender);

// GetSupportData service
support_t* get_support_data(pcb_t* sender);

// GetProcessID service
int get_process_id(pcb_t* sender, int arg);

#endif /* SSI_H */

