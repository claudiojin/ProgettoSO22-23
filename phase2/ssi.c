#include "./headers/ssi.h"
/**
* This module implements the System Service Interface.
*
*
*
*
**/

// Receive request and send response to are currently mockups
while (TRUE) {
	//receive a request
	msg_t* request  = receive_request();
	
	//satisfy said request
	if (request->service ==1){  //CreateProcess service
		ssi_create_process_t* args = (ssi_create_process_t*)request->arg;
		pcb_t* new_process = create_process(args),
		send_response(request->sender, new_process);
	}
	if (request->service ==2){ //TerminateProcess service
		terminate_process(target_process);
	}
	if (request->service ==3){ //DoIO service
		ssi_payload_t* payload = (ssi_payload_t*)request->arg;
		ssi_do_io_t* do_io_args = (ssi_do_io_t*)payload->arg;
	/// da finire
	}
	if (request->service ==4){ //GetCPUTime Service
		
		// Get the CPU time for the sender process
		int cpu_time = get_cpu_time(request->sender);
		
		// Send back CPU time as a response 
		send_response(request->sender, cpu_time);
	}
	if (request->service ==5){ //WaitForClock service
		
	}
	if (request->service ==6){ //GetSupportData service
		//Get the Support Structure for the sender process
		support_t* support_data = get_support_data(request->sender);
		
		//Send The Support Structure back as response
		send_response(request->sender, support_data);
	}
	if (request->service ==7){ //GetProcessID service
		int arg = (int)request->arg;
		
		//Get the pid based on the argument
		int process_id = get_process_id(request->sender, arg);
		
		//Send back pid as a response
		send_response(request->sender, process_id);
	}

	//da inserire caso per codice invalido
}
// standard procedure to be called by processes to make service requests
void SSIRequest(pcb_t* sender, int service, void* arg){
	// Create a message and send it to the SSI process
	msg_t request_msg;
	request_msg.sender = sender;
	request_msg.service = service;
	requeat_msg.arg = arg;

	send_request(ssi_process, &request_msg);
	
	//Wait for the answer, now just a mockup
	wait_for_response();

}

// create a new process, progeny of the sender
pcb_t* create_process(ssi_create_process_t* args) {
	// Allocate a new PCB
	pcb_t* new_process = allocate_pcb();

	// Initialize PCB fields
	new_process->p_s = *(args->state);
	new_process->p_supportStruct = args->support;
	// Initialize other PCB fields as needed

	// Add the new process to the Ready Queue
	insertProcQ(&readyQueue, new_process);

	// Add the new process as a child of the current process
	insertChild(current_process, new_process);
   
	// Initialize p_time to zero
	new_process->p_time = 0;

	// Increment Process Count
	process_count++;

	return new_process;
}

// Cause the sender or another process to terminate, included all of the
// progeny.
void terminate_process(pcb_t* process){
	//remove process from the ready queue
	
	//remove process and progeny from the pcb tree
	
	//free pcb memory (?)
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
support_t* get_support_data(request->sender){
	return sender->p_supportStruct;
}


// GetProcessID service
int get_process_id(pcb_t* sender, int arg){
	if (arg==0){
		//return sender's PID
		return sender->pid;
	} else {
		//return sender's parent's PID (given its existence)
		return (sender->p_parent != NULL) ? sender->p_parent->pid : 0;
	}
}
