#ifndef SSI_H
#define SSI_H

#include "./headers/exceptions.h"
#include "./headers/scheduler.h"
#include "../../phase1/headers/msg.h"

/**
 * This module implements the System Service Interface.
 */

// Global variable to represent SSI process
extern pcb_t* ssi_pcb;

// Standard procedure to be called by processes to make service requests
void SSIRequest(pcb_t* sender, int service, void* arg);

// Receive a message as the SSI
msg_t* receive_request();

// As the SSI, send back a response after a service
void send_response(pcb_t* sender, void* response) ;

// Create a new process, progeny of the sender
void create_process_service(pcb_t* sender, ssi_create_process_t* args);

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

//SSI basic server algorithm (implements the RPC)
void SSI_server();

#endif 
