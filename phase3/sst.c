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
    char *string = print_payload->string;
    dtpreg_t *dev_reg = (dtpreg_t *)DEV_REG_ADDR(6, sender->p_supportStruct->sup_asid - 1);

    klog_print(print_payload->string);
    klog_print_dec(print_payload->length);

    for (int i = 0; i < print_payload->length; i++)
    {
        dev_reg->data0 = *string;
        int status = DoIO(&dev_reg->command, PRINTCHR);

        if (status != OK)
        {
            klog_print("Char not transmitted correctly");
            break;
        }
        string++;
    }
}

// Function to handle the WriteTerminal request
void handle_WriteTerminal(pcb_t *sender, sst_print_t *print_payload)
{
    char *string = print_payload->string;
    termreg_t *terminal = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, sender->p_supportStruct->sup_asid - 1);

    klog_print(print_payload->string);
    klog_print_dec(print_payload->length);

    // Scrittura carattere per carattere
    for (int i = 0; i < print_payload->length; i++)
    {
        unsigned int value = PRINTCHR + ((*string) << 8);
        int status = DoIO(&terminal->transm_command, value);
        if (TERMINAL_STATUS(status) != OKCHARTRANS)
        {
            klog_print("Char not transmitted correctly");
            break;
        }
        string++;
    }

    send_res(NULL);
}

// Process the incoming request from the child process
void SSTRequest(pcb_t *sender, int service, sst_print_t *arg)
{
    klog_print(" service code SST: ");
    klog_print_dec(service);

    switch (service)
    {
    case GET_TOD:
        handle_GetTOD(sender);
        break;
    case TERMINATE:
        handle_Terminate(sender);
        break;
    case WRITEPRINTER:
        handle_WritePrinter(sender, arg);
        break;
    case WRITETERMINAL:
        handle_WriteTerminal(sender, arg);
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

        if (sender == u_proc)
            // Process the request based on the service code
            SSTRequest(sender, payload.service_code, (sst_print_t *)payload.arg);
    }
}
