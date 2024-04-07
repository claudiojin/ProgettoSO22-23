#include "./headers/ssi.h"

// As the SSI, receive a message from other processes
pcb_t *receive_request(ssi_payload_t *payload_address)
{
    pcb_PTR sender = NULL;
    sender = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)payload_address, 0);
    klog_print("arg: ");
    klog_print_hex((unsigned int)payload_address->arg);
    return sender;
}

// As the SSI, send a response back after executing a service
void send_response(pcb_t *destination, void *response)
{
    SYSCALL(SENDMESSAGE, (unsigned int)destination, (unsigned int)response, 0);
}

// create a new process, progeny of the sender
pcb_t *create_process_service(pcb_t *sender, ssi_create_process_t *args)
{
    // Allocate a new PCB
    pcb_t *new_process = allocPcb();

    if (new_process == NULL)
    {
        int ret = NOPROC;
        send_response(sender, (int *)&ret);
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

    return new_process;
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

    process_count--;
    outChild(process);

    // remove process from ready queue
    outProcQ(&ready_queue, process);

    // remove from general blocked list
    outProcQ(&blocked_proc[SEMDEVLEN], process);

    // remove process from blocked list
    for (int i = 0; i < SEMDEVLEN; i++)
    {
        if (outProcQ(&blocked_proc[i], process) == process)
        {
            softBlock_count--;
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

void DOIO_IN(pcb_t *sender, ssi_do_io_t *arg)
{
    int index = getIODeviceIndex((memaddr)arg->commandAddr);
    // take the pcb from the general purpose list and put it in the right blocked_proc list
    outProcQ(&blocked_proc[SEMDEVLEN], sender);
    insertProcQ(&blocked_proc[index], sender);
    softBlock_count++;
    *arg->commandAddr = arg->commandValue;
    // the instruction above should rise an interrupt exception which will send the device response back to the ssi
}

void DOIO_OUT(pcb_t *waiting_pcb) {
    // return the doio request status code to the requesting process
    ssi_pcb->p_s.reg_a3 ^= 0;
    unsigned int status_code = waiting_pcb->p_s.reg_v0;
    send_response(waiting_pcb, &status_code);
}

int get_cpu_time(pcb_t *sender)
{
    sender->p_time += IntervalTOD();
    return sender->p_time;
}

// blocks the sender on the Interval Timer list
void WaitForClock_IN(pcb_t *sender)
{
    // ready state
    if (outProcQ(&ready_queue, sender) != NULL) {
        insertProcQ(&blocked_proc[SEMDEVLEN - 1], sender);
        softBlock_count++;
    }

    // blocked state
    if (outProcQ(&blocked_proc[SEMDEVLEN], sender)) {
        insertProcQ(&blocked_proc[SEMDEVLEN - 1], sender);
        softBlock_count++;
    }
}

void WaitForClock_OUT(struct list_head *clock_list) {
    pcb_t *pos = NULL;
    while ((pos = removeProcQ(clock_list)) != NULL)
    {
        klog_print("awakening IT process");
        insertProcQ(&blocked_proc[SEMDEVLEN], pos);
        send_response(pos, NULL);
    }
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

// Process the incoming request
void SSIRequest(pcb_t *sender, int service, void *arg)
{
    // Satisfy the received request based on the service code
    switch (service)
    {
    case CREATEPROCESS:
        klog_print("create");
        pcb_t *child_process = create_process_service(sender, (ssi_create_process_PTR)arg);

        send_response(sender, &child_process);
        break;
    case TERMPROCESS:
        klog_print("terminate");
        terminate_process_service(sender, (pcb_PTR)arg);

        send_response(sender, NULL);
        break;
    case DOIO:
        klog_print("DOIO");
        if ((unsigned int)sender >= DEV_REG_START && (unsigned int)sender < DEV_REG_END)
            DOIO_OUT((pcb_PTR)arg);
        else 
            DOIO_IN(sender, (ssi_do_io_PTR)arg);
        break;
    case GETTIME:
        klog_print("get time");
        // Get the CPU time for the sender process
        int cpu_time = get_cpu_time(sender);
        // Send back CPU time as a response
        send_response(sender, &cpu_time);
        break;
    case CLOCKWAIT:
        klog_print("clock wait");
        if ((unsigned int)sender == INTERVALTMR)
            WaitForClock_OUT((struct list_head*)arg);
        else
            WaitForClock_IN(sender);
        break;
    case GETSUPPORTPTR:
        klog_print("get support");
        // Get the Support Structure for the sender process
        support_t *support_data = get_support_data(sender);
        // Send the Support Structure back as a response
        send_response(sender, support_data);
        break;
    case GETPROCESSID:
        klog_print("get pid");
        // Get the pid based on the argument
        int process_id = get_process_id(sender, (int)arg);
        // Send back pid as a response
        send_response(sender, &process_id);
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
        sender = (pcb_PTR)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)&payload, 0);

        SSIRequest(sender, payload.service_code, payload.arg);
    }
}