#include "./headers/ssi.h"

// Helper function to send a message to the SSI process
void SSIRequest(pcb_t *sender, int service, void *arg)
{
    msg_t *request_msg = allocMsg();
    request_msg->m_sender = sender;
    request_msg->m_payload = (unsigned int)arg;
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)request_msg, 0);
}

// As the SSI, receive a message from other processes
msg_t *receive_request()
{
    msg_t *received_msg;
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&received_msg, 0);
    return received_msg;
}

// As the SSI, send a response back after executing a service
void send_response(pcb_t *sender, void *response)
{
    SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)response, 0);
}

// create a new process, progeny of the sender
void create_process_service(pcb_t *sender, ssi_create_process_t *args)
{
    // Allocate a new PCB
    pcb_t *new_process = allocPcb();

    if (new_process == NULL)
    {
        send_response(sender, (int *)NOPROC);
    }

    // Initialize PCB fields
    new_process->p_s = *(args->state);
    new_process->p_supportStruct = args->support;

    // Add the new process to the Ready Queue
    insertProcQ(&ready_queue, new_process);

    process_count++;

    // Add the new process as a child of the current process
    insertChild(current_process, new_process);

    // Initialize p_time to zero
    new_process->p_time = 0;

    send_response(sender, new_process);
}

// Cause the sender or another process to terminate, included all of the
// progeny.
void TerminateProcess(pcb_t *process)
{
    if (process == ssi_pcb) {
        klog_print("SSI PCB KILLED!");
        PANIC();
    }

    process_count--;
    outChild(process);

    // remove process from ready queue (if necessary)
    outProcQ(&ready_queue, process);

    // remove process from blocked list (if necessary)
    for (int i = 0; i < DEVNUM; i++)
    {
        if (searchInList(process, &blocked_proc[i]) == process)
        {
            softBlock_count--;
            outProcQ(&blocked_proc[i], process);
        }
    }

    pcb_t *child;
    while ((child = removeChild(process)) != NULL)
    {
        TerminateProcess(child);
    }

    freePcb(process);
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
}

void DoIO_service(pcb_t *sender, ssi_do_io_t *arg)
{
    int index = getIODeviceIndex(*arg->commandAddr);
    // take the pcb from the general purpose list and put it in the right blocked_proc list
    outProcQ(&blocked_proc[DEVNUM], sender);
    insertProcQ(&blocked_proc[index], sender);
    softBlock_count++;
    *arg->commandAddr = arg->commandValue;
    // the instruction above should rise an interrupt exception which will handle the unblocking
    // of the requesting pcb and send it the device response
}

int get_cpu_time(pcb_t *sender)
{
    return sender->p_time;
}

// blocks the sender on the Interval Timer list
void wait_for_clock_service(pcb_t *sender)
{
    outProcQ(&ready_queue, sender);
    insertProcQ(&blocked_proc[SEMDEVLEN - 1], sender);
    softBlock_count++;
}

// GetSupportData service
support_t *get_support_data(pcb_t *sender)
{
    return sender->p_supportStruct;
}

// GetProcessID service
int get_process_id(pcb_t *sender, int arg)
{
    if (arg == 0)
    {
        // return sender's PID
        return (sender->p_pid);
    }
    else
    {
        // return sender's parent's PID (given its existence)
        return ((sender->p_parent != NULL) ? sender->p_parent->p_pid : 0);
    }
}

// SSI basic server algorithm (implements the RPC)
void SSI_server()
{
    // Loop indefinitely to handle requests
    while (TRUE)
    {
        // Receive a request from the SSI process inbox
        klog_print("SSI receiving a request");
        msg_t *request_msg = receive_request();
        pcb_t *sender = request_msg->m_sender;
        ssi_payload_PTR payload = (ssi_payload_PTR)request_msg->m_payload;
        int service_code = payload->service_code;
        // Satisfy the received request based on the service code
        switch (service_code)
        {
        case CREATEPROCESS:
        {
            klog_print("create");
            create_process_service(sender, payload->arg);
            break;
        }
        case TERMPROCESS:
        {
            klog_print("terminate");
            pcb_t *target_process = (pcb_t *)request_msg->m_payload;
            terminate_process_service(sender, target_process);
            break;
        }
        case DOIO:
        {
            klog_print("DOIO");
            ssi_do_io_t *argument = (ssi_do_io_t *)payload->arg;
            DoIO_service(sender, argument);
            break;
        }
        case GETTIME:
        {
            klog_print("get time");
            // Get the CPU time for the sender process
            int cpu_time = get_cpu_time(sender);
            cpu_time += IntervalTOD();
            // Send back CPU time as a response
            send_response(sender, &cpu_time);
            break;
        }
        case CLOCKWAIT:
        {
            klog_print("clock wait");
            wait_for_clock_service(sender);
            break;
        }
        case GETSUPPORTPTR:
        {
            klog_print("get support");
            // Get the Support Structure for the sender process
            support_t *support_data = get_support_data(sender);
            // Send the Support Structure back as a response
            send_response(sender, support_data);
            break;
        }
        case GETPROCESSID:
        {
            klog_print("get pid");
            // int arg = (int)request_msg->m_payload;
            // Get the pid based on the argument
            int process_id = get_process_id(sender, (int)payload->arg);
            // Send back pid as a response
            send_response(sender, &process_id);
            break;
        }
        // Handle other services if needed
        default:
            klog_print("default: kill process");
            // Invalid service code, terminate the process and its progeny
            TerminateProcess(sender);
        }
    }
}