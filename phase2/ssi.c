#include "./headers/ssi.h"
/**
* This module implements the System Service Interface.
*/

// Helper function to send a message to the SSI process
void SSIRequest(pcb_t* sender, int service, void* arg){
	msg_t* request_msg = allocMsg();
	request_msg->m_sender = sender;
	// request_msg->m_service_code = service;
	request_msg->m_payload = (unsigned int)arg;
	SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb,(unsigned int)request_msg,0);
}

// As the SSI, receive a message from other processes
msg_t* receive_request(){
	msg_t* received_msg;
	SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&received_msg, 0);
	return received_msg;
}

// As the SSI, send a response back after executing a service
void send_response(pcb_t* sender, void* response){
	msg_t* response_msg ;
	SYSCALL(SENDMESSAGE, (unsigned int)sender,(unsigned int)response,0);
}

// create a new process, progeny of the sender
void create_process_service(pcb_t* sender, ssi_create_process_t* args) {
	// Allocate a new PCB
	pcb_t* new_process = allocPcb();

	// Initialize PCB fields
	new_process->p_s = *(args->state);
	new_process->p_supportStruct = args->support;
	// Initialize other PCB fields as needed

	// Add the new process to the Ready Queue
	insertProcQ(&ready_queue, new_process);

	// Add the new process as a child of the current process
	insertChild(current_process, new_process);

	process_count++;
	
	// Initialize p_time to zero
	new_process->p_time = 0;

	send_response(sender, new_process);

}
// Cause the sender or another process to terminate, included all of the
// progeny.
void terminate_process(pcb_t* process){
	//remove process from the ready queue
	
	//remove process and progeny from the pcb tree
	
	
}

// Expansion of previous function, to distinguish between sender termination
// and other cases. If target is NULL, sender is the process chose for
// termination.
void terminate_process_service(pcb_t* sender, pcb_t* target_process){
	if (target_process == NULL){
		// Terminate sender process and its progeny
		terminate_process(sender);
	}else{
		//Terminate target process and its progeny
		terminate_process(target_process);
	}
}

int get_cpu_time(pcb_t* sender){
	return sender->p_time;
}

// GetSupportData service
support_t* get_support_data(pcb_t* sender){
	return sender->p_supportStruct;
}


// GetProcessID service
int get_process_id(pcb_t* sender, int arg){
	if (arg==0){
		//return sender's PID
		return (sender->p_pid);
	} else {
		//return sender's parent's PID (given its existence)
		return ((sender->p_parent != NULL) ? sender->p_parent->p_pid : 0);
	}
}

// SSI basic server algorithm (implements the RPC)
void SSI_server() {
    // Loop indefinitely to handle requests
    while (TRUE) {
        
		// Receive a request from the SSI process inbox
        msg_t* request_msg = receive_request();
        ssi_payload_PTR payload = (ssi_payload_PTR)request_msg->m_payload;
        int service_code = payload->service_code;
        // Satisfy the received request based on the service code
        switch (service_code) {
            case CREATEPROCESS: {
                create_process_service(request_msg->m_sender, payload->arg);
                break;
            }
            case TERMPROCESS: {
                pcb_t* target_process = (pcb_t*)request_msg->m_payload;
                terminate_process_service(request_msg->m_sender, target_process);
                break;
            }
            case DOIO: {
                ssi_payload_t* payload = (ssi_payload_t*)request_msg->m_payload;
                ssi_do_io_t* do_io_args = (ssi_do_io_t*)payload->arg;
                // Implement DoIO service
                // ...
                break;
            }
            case GETTIME: {
                // Get the CPU time for the sender process
                int cpu_time = get_cpu_time(request_msg->m_sender);
				cpu_time += IntervalTOD();
                // Send back CPU time as a response
                send_response(request_msg->m_sender, &cpu_time);
                break;
            }
            case CLOCKWAIT: {
                // Implement WaitForClock service
                // ...
                break;
            }
            case GETSUPPORTPTR: {
                // Get the Support Structure for the sender process
                support_t* support_data = get_support_data(request_msg->m_sender);
                // Send the Support Structure back as a response
                send_response(request_msg->m_sender, support_data);
                break;
            }
            case GETPROCESSID: {
                // int arg = (int)request_msg->m_payload;
                // Get the pid based on the argument
                int process_id = get_process_id(request_msg->m_sender, (int)payload->arg);
                // Send back pid as a response
                send_response(request_msg->m_sender, &process_id);
                break;
            }
            // Handle other services if needed
            default:
                // Invalid service code, terminate the process and its progeny
                terminate_process(request_msg->m_sender);
        }
    }
}