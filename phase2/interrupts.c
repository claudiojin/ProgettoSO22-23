/**
 * This module implements the device/timer interrupt exception handler.
 * This module will process all the device/timer interrupts, converting
 * device/timer interrupts into V operations on the appropriate semaphores.
 */

#include "./headers/interrupts.h"

/**
 * Handles exit from interrupt handler
 */
static void interruptHandlerExit()
{
    setSTATUS(getSTATUS() | TEBITON); // enable PLT
    IntervalTOD();

    // Return control to the Current Process: Perform a LDST on the saved exception state
    if (current_process != NULL)
    {
        LDST(PROCSTATE);
    }
    else
    {
        // it is also possible that there is no Current Process to return control to. This will be the case
        // when the Scheduler executes the WAIT instruction instead of dispatching a process for execution
        scheduler();
    }
}

/**
 * Processor Local Timer handler
 */
static void PLTHandler()
{
    // Ack PLT
    setTIMER((cpu_t)TIMESLICE * (*((cpu_t *)TIMESCALEADDR)));
    
    updateProcessCPUTime();
    current_process->p_s = *PROCSTATE;

    // Place the Current Process on the Ready Queue; transitioning the Current Process from the
    // “running” state to the “ready” state
    insertProcQ(&ready_queue, current_process);

    current_process = NULL;
    scheduler();
}

/**
 * Interval Timer handler.
 */
static void ITHandler()
{
    // Acknowledge the interrupt by loading the Interval Timer with a new value: 100 milliseconds
    LDIT(PSECOND);

    // Unblock all PCBs blocked waiting a Pseudo-clock tick
    pcb_t *pos = NULL;
    while ((pos = removeProcQ(&blocked_proc[SEMDEVLEN - 1])) != NULL)
    {
        // put the pcb inn the general purpose blocked list
        insertProcQ(&blocked_proc[SEMDEVLEN], pos);
        // send the message directly to the pcb
        SendMessage(pos, NULL, (pcb_PTR)ssi_pcb);

        softBlock_count--;
    }
    // make sure the list is empty
    mkEmptyProcQ(&blocked_proc[SEMDEVLEN - 1]);

    interruptHandlerExit();
}

/**
 * Every device interrupt line (3-7) has 8 devices -> number of bits for the device bitmap.
 * When bit i(device) in word j(interrupt line) is set to one then device i attached to
 * interrupt line j + 3 has a pending interrupt
 * @param line interrupt line.
 * @return the bitmap corresponding to the line of interrupt.
 */
static unsigned int getDevBitmap(int line)
{
    // access the bus register area, located at ram base physical address
    devregarea_t *bus_reg_area = (devregarea_t *)BUS_REG_RAM_BASE;
    return bus_reg_area->interrupt_dev[line - 3];
}

/**
 * Handles the return value of a device.
 * @param status Pointer to device status field.
 * @param command Pointer to device command field.
 */
static void devInterruptReturn(unsigned int status, unsigned int *command)
{
    // save off the status code
    unsigned int status_code = status;
    // acknowledge the outstanding interrupt
    *command = ACK;

    int index = getIODeviceIndex((memaddr)command);
    pcb_t *waiting_pcb = removeProcQ(&blocked_proc[index]);

    /*it is possible that there isn’t any PCB waiting for this device. This can happen if
    while waiting for the initiated I/O operation to complete, an ancestor of this PCB was terminated.
    In this case, simply return control to the Current Process*/
    if (waiting_pcb != NULL)
    {
        // send the status code message to the SSI
        insertProcQ(&blocked_proc[SEMDEVLEN], waiting_pcb);
        softBlock_count--;

        // the device sends to the blocked process a message with the status of the device operation
        SendMessage(waiting_pcb, (unsigned int *)&status_code, (pcb_PTR)ssi_pcb);

        // Place the stored off status code in the newly unblocked PCB’s v0 register
        waiting_pcb->p_s.reg_v0 = status_code;
    }
}

/**
 * Handler of non terminal devices
 * @param line Interrupt line.
 * @param device_number Device number.
 */
static void nonTerminalHandler(int line, int device_number)
{
    dtpreg_t *device_register = (dtpreg_t *)DEV_REG_ADDR(line, device_number);
    devInterruptReturn(device_register->status, &device_register->command);
}

/**
 * Terminal handler.
 * @param device_number Device number.
 */
static void terminalHandler(int device_number)
{
    termreg_t *terminal_register = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, device_number);
    // Receiving
    if (TERMINAL_STATUS(terminal_register->recv_status) == CHARRECV)
    {
        devInterruptReturn(terminal_register->recv_status, &terminal_register->recv_command);
    }
    // Transmitting
    else if (TERMINAL_STATUS(terminal_register->transm_status) == OKCHARTRANS)
    {
        devInterruptReturn(terminal_register->transm_status, &terminal_register->transm_command);
    }
    else
    {
        PANIC();
    }
}

/**
 * Finds the device(s) attached to an interrupt line and handles it accordingly
 * @param line interrupt line
 */
static void deviceHandler(int line)
{
    unsigned int bitmap = getDevBitmap(line);
    unsigned int mask = 1;
    int device_number = 0;

    // find the device which generated the interrupt
    for (int i = 0; i < N_DEV_PER_IL; i++)
    {
        if ((bitmap & mask) != 0)
        {
            if (line != IL_TERMINAL)
                nonTerminalHandler(line, device_number);
            else
                terminalHandler(device_number);

            interruptHandlerExit();
        }
        // cycles from the least significant bit to the most significant by shifting left
        mask = mask << 1;
        device_number++;
    }
}

/**
 * Interrupt handler, called with exception code 0
 */
void interruptHandler()
{
    // Interrupt handling time is not accumulated in the CPU time of current process
    if (current_process != NULL)
    {
        updateProcessCPUTime();
    }
    setSTATUS(getSTATUS() & ~TEBITON); // Disable PLT

    unsigned int ip = PROCSTATE->cause & CAUSE_IP_MASK;

    // Priority: PLT > IT > Disk > Flash drive > Printers > Terminals (writing) > Terminals (reading)

    // Line 1 (PLT)
    if ((ip & LOCALTIMERINT) != 0)
    {
        PLTHandler();
    }
    // Line 2 (IT)
    else if ((ip & TIMERINTERRUPT) != 0)
    {
        ITHandler();
    }
    // Line 3
    else if ((ip & DISKINTERRUPT) != 0)
    {
        deviceHandler(IL_DISK);
    }
    // Line 4
    else if ((ip & FLASHINTERRUPT) != 0)
    {
        deviceHandler(IL_FLASH);
    }
    // Line 5
    else if ((ip & NETINTERRUPT) != 0)
    {
        deviceHandler(IL_ETHERNET);
    }
    // Line 6
    else if ((ip & PRINTINTERRUPT) != 0)
    {
        deviceHandler(IL_PRINTER);
    }
    // Line 7
    else if ((ip & TERMINTERRUPT) != 0)
    {
        deviceHandler(IL_TERMINAL);
    }
    else
    {
        interruptHandlerExit();
    }
}
