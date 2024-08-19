#ifndef SST_H
#define SST_H

#include "../headers"
#include "../../phase2/headers/ssi.h"
#include "../../phase2/headers/exceptions.h"


// Service codes for SST
#define GET_TOD         1
#define TERMINATE       2
#define WRITEPRINTER    3
#define WRITETERMINAL   4

// Structure to hold print data for writing to devices
typedef struct {
    int length;
    char *string;
} sst_print_t;

// Function to handle GetTOD request
void handle_GetTOD(pcb_t *sender);

// Function to handle Terminate request
void handle_Terminate(pcb_t *sender);

// Function to handle WritePrinter request
void handle_WritePrinter(pcb_t *sender, sst_print_t *print_payload);

// Function to handle WriteTerminal request
void handle_WriteTerminal(pcb_t *sender, sst_print_t *print_payload);

// Function to process incoming requests
void SSTRequest(pcb_t *sender, int service, void *arg);

// Main server loop for SST
void SST_server();

#endif //SST_H