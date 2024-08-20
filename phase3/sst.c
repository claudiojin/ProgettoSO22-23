#include "./headers/sst.h"

// Function to handle the GetTOD request
void handle_GetTOD(pcb_t *sender)
{
    ssi_payload_t ssi_payload = {
        .service_code = GETTIME,
        .arg = 0};

    // Send request to SSI
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&ssi_payload, 0);

    // Wait for the response from SSI
    unsigned int tod;
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&tod, 0);

    // Send the TOD back to the sender
    send_response(sender, &tod);
}

// Function to handle the Terminate request
void handle_Terminate(pcb_t *sender)
{
    ssi_payload_t ssi_payload = {
        .service_code = TERMPROCESS,
        .arg = (void *)sender};

    // Send terminate request to SSI
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&ssi_payload, 0);

    // Send termination acknowledgment to the sender
    send_response(sender, NULL);

    // Terminate the SST process
    TerminateProcess(sender);
}

// Function to handle the WritePrinter request
void handle_WritePrinter(pcb_t *sender, sst_print_t *print_payload)
{
    ssi_payload_t ssi_payload = {
        .service_code = WRITEPRINTER,
        .arg = (void *)print_payload};

    // Send print request to SSI
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&ssi_payload, 0);

    // Wait for completion acknowledgment from SSI
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);

    // Send completion acknowledgment to the sender
    send_response(sender, NULL);
}

// Function to handle the WriteTerminal request
void handle_WriteTerminal(pcb_t *sender, sst_print_t *print_payload)
{
    ssi_payload_t ssi_payload = {
        .service_code = WRITETERMINAL,
        .arg = (void *)print_payload};

    // Send terminal write request to SSI
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&ssi_payload, 0);

    // Wait for completion acknowledgment from SSI
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);

    // Send completion acknowledgment to the sender
    send_response(sender, NULL);
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
    // start the child U-proc with the current asid
    pcb_PTR u_proc = startProcess(asid);

    while (TRUE)
    {
        pcb_t *sender = NULL;
        ssi_payload_t payload;

        // Receive a request from the SST's inbox
        sender = receive_request(&payload);

        if (sender == u_proc)
            // Process the request based on the service code
            SSTRequest(sender, payload.service_code, payload.arg);
    }
}
