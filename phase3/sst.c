#include "./headers/sst.h"
#include "../phase2/headers/klog.h"

// child User process of this SST thread
static pcb_t *u_proc;

// As the SST, receive a message from child process
pcb_t *receive_req(ssi_payload_t *payload_address)
{
    return (pcb_PTR)SYSCALL(RECEIVEMESSAGE, (unsigned int)u_proc, (unsigned int)payload_address, 0);
}

// As the SST, send a response back to U-proc after executing a service
void send_res(void *payload)
{
    SYSCALL(SENDMESSAGE, (unsigned int)u_proc, (unsigned int)payload, 0);
}

// Function to handle the GetTOD request
void handle_GetTOD(pcb_t *sender)
{
    cpu_t curr_time;
    STCK(curr_time);

    // Send the TOD back to the sender
    send_res(&curr_time);
}

// Function to handle the Terminate request
void handle_Terminate(pcb_t *sender)
{
    // Terminate the requesting user process
    UTerminate(sender);
}

// Function to handle the WritePrinter request
void handle_WritePrinter(pcb_t *sender, sst_print_t *print_payload)
{
}

// Function to handle the WriteTerminal request
void handle_WriteTerminal(pcb_t *sender, sst_print_t *print_payload)
{
}

// Process the incoming request from the child process
void SSTRequest(pcb_t *sender, int service, void *arg)
{
    switch (service)
    {
    case GET_TOD:
        handle_GetTOD(sender);
        break;
    case TERMINATE:
        handle_Terminate(sender);
        break;
    case WRITEPRINTER:
        handle_WritePrinter(sender, (sst_print_t *)arg);
        break;
    case WRITETERMINAL:
        handle_WriteTerminal(sender, (sst_print_t *)arg);
        break;
    default:
        // Invalid service code, terminate the process
        TerminateProcess(sender);
    }
}

// System Service Thread (SST) main loop, also starts the child u-proc before entering the loop
void SST_server()
{
    // SST shares the same ASID and support structure with its child U-proc.
    support_t *sst_support = GetSupportPtr();

    // start the child U-proc with the same SST asid and support structure
    u_proc = startProcess(sst_support->sup_asid, sst_support);

    while (TRUE)
    {
        pcb_t *sender = NULL;
        ssi_payload_t payload;

        // Receive a request from the SST's inbox
        sender = receive_req(&payload);

        klog_print(" service code SST: ");
        klog_print_dec(payload.service_code);

        if (sender == u_proc)
            // Process the request based on the service code
            SSTRequest(sender, payload.service_code, payload.arg);
    }
}
