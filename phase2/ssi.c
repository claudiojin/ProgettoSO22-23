#include "./headers/ssi.h"

// As the SSI, receive a message from other processes
pcb_t *receive_request(ssi_payload_t *payload_address)
{
    pcb_PTR sender = NULL;
    sender = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)payload_address, 0);
    return sender;
}

// As the SSI, send a response back after executing a service
void send_response(pcb_t *destination, void *response)
{
    SYSCALL(SENDMESSAGE, (unsigned int)destination, (unsigned int)response, 0);
}

// create a new process, progeny of the sender
void create_process_service(pcb_t *sender, ssi_create_process_t *args)
{
    // Allocate a new PCB
    pcb_t *new_process = allocPcb();

    if (new_process == NULL)
    {
        int ret = NOPROC;
        send_response(sender, (int *)&ret);
        return;
    }

    process_count++;

    // Initialize PCB fields
    new_process->p_s = *(args->state);
    // memcpy(&(new_process->p_s), (args->state), sizeof (state_t));
    new_process->p_supportStruct = args->support;

    // New process has yet to accumulate cpu time
    new_process->p_time = 0;

    // Add the new process as a child of the sender process
    insertChild(sender, new_process);

    klog_print("NEW PROC ADDR: ");
    klog_print_hex((unsigned int)new_process);
    klog_print(", NEW PROC Parent: ");
    klog_print_hex((unsigned int)new_process->p_parent);

    // Add the new process to the Ready Queue
    insertProcQ(&ready_queue, new_process);

    send_response(sender, &new_process);
}

// Cause the sender or another process to terminate, included all of the
// progeny.
void TerminateProcess(pcb_t *process)
{
    if (process == ssi_pcb)
    {
        klog_print("SSI PCB KILLED!");
        PANIC();
    }

    outChild(process);

    process_count--;

    // remove process from ready queue
    outProcQ(&ready_queue, process);

    // remove from general blocked list
    outProcQ(&blocked_proc[SEMDEVLEN], process);

    // remove process from device blocked list
    for (int i = 0; i < SEMDEVLEN; i++)
    {
        if (outProcQ(&blocked_proc[i], process) != NULL)
        {
            softBlock_count--;
            break;
        }
    }

    pcb_t *child;
    while ((child = removeChild(process)) != NULL)
    {
        TerminateProcess(child);
    }
    
    klog_print("KILLING PROCESS: ");
    klog_print_hex((unsigned int)process);
    
    // If the process hasn't already been killed, kill it
    if (searchInList(process, NULL) == NULL){
        freePcb(process);
    }
}

// Expansion of previous function, to distinguish between sender termination
// and other cases. If target is NULL, sender is the process chose for
// termination.
void terminate_process_service(pcb_t *sender, pcb_t *target_process)
{
    if (target_process == NULL)
    {
        // Terminate sender process and its progeny
        TerminateProcess(sender);
    }
    else
    {
        // Terminate target process and its progeny
        TerminateProcess(target_process);
    }
    send_response(sender, NULL);
}

void DOIO_service(pcb_t *sender, ssi_do_io_t *arg)
{
    int index = getIODeviceIndex((memaddr)arg->commandAddr);
    // take the requesting pcb and put it in the right blocked_proc list
    outProcQ(&ready_queue, sender);
    outProcQ(&blocked_proc[SEMDEVLEN], sender);

    insertProcQ(&blocked_proc[index], sender);
    softBlock_count++;

    *arg->commandAddr = arg->commandValue;
    // the instruction above should rise an interrupt exception which will send the device response back to the requesting process
}

void get_cpu_time(pcb_t *sender)
{
    sender->p_time += IntervalTOD();
    // Send back CPU time as a response
    send_response(sender, &sender->p_time);
}

// blocks the sender on the Interval Timer list
void WaitForClock_service(pcb_t *sender)
{
    // ready state
    if (outProcQ(&ready_queue, sender) != NULL)
    {
        insertProcQ(&blocked_proc[SEMDEVLEN - 1], sender);
        softBlock_count++;
    }

    // blocked state
    if (outProcQ(&blocked_proc[SEMDEVLEN], sender) != NULL)
    {
        insertProcQ(&blocked_proc[SEMDEVLEN - 1], sender);
        softBlock_count++;
    }
}

// GetSupportData service
void get_support_data(pcb_t *sender)
{
    // Send the Support Structure back as a response
    send_response(sender, &sender->p_supportStruct);
}

// GetProcessID service
void get_process_id(pcb_t *sender, int arg)
{
    int process_id = 0;
    if (arg == 0)
    {
        // sender's PID
        process_id = sender->p_pid;
    }
    else
    {
        // sender's parent's PID (given its existence)
        process_id = (sender->p_parent != NULL) ? sender->p_parent->p_pid : 0;
    }
    // Send back pid as a response
    send_response(sender, &process_id);
}

// Process the incoming request
void SSIRequest(pcb_t *sender, int service, void *arg)
{
    // Satisfy the received request based on the service code
    switch (service)
    {
    case CREATEPROCESS:
        klog_print("create");
        create_process_service(sender, (ssi_create_process_PTR)arg);
        break;
    case TERMPROCESS:
        klog_print("terminate");
        terminate_process_service(sender, (pcb_PTR)arg);
        break;
    case DOIO:
        klog_print("DOIO");
        DOIO_service(sender, (ssi_do_io_PTR)arg);
        break;
    case GETTIME:
        klog_print("get time");
        // Get the CPU time for the sender process
        get_cpu_time(sender);
        break;
    case CLOCKWAIT:
        klog_print("CLOCKWAIT");
        WaitForClock_service(sender);
        break;
    case GETSUPPORTPTR:
        klog_print("get support");
        // Get the Support Structure for the sender process
        get_support_data(sender);
        break;
    case GETPROCESSID:
        klog_print("get pid");
        // Get the pid based on the argument
        get_process_id(sender, (int)arg);
        break;
    // Handle other services if needed
    default:
        klog_print("default: kill process");
        // Invalid service code, terminate the process and its progeny
        TerminateProcess(sender);
    }
}

// SSI basic server algorithm (implements the RPC)
void SSI_server()
{
    // Loop indefinitely to handle requests
    while (TRUE)
    {
        pcb_t *sender = NULL;
        ssi_payload_t payload;

        // Receive a request from the SSI process inbox
        sender = receive_request(&payload);

        SSIRequest(sender, payload.service_code, payload.arg);
    }
}