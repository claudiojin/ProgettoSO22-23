#include "./headers/scheduler.h"

/**
 * Sets the current process to a "blocked" state and calls the scheduler
 * @param state the state of the processor to load
 */
void blockProcess(state_t *state, int index)
{
    insertProcQ(&blocked_proc[index], current_process);
    current_process->p_s = *state;
    updateProcessCPUTime();
    scheduler();
}

/**
 * sets pcb pointed to by p to a "ready" state by putting it into the ready queue
 * @param p the process to awake
 * @param index index for the blocked proc array
 */
void readyProcess(pcb_t *p, int index)
{
    outProcQ(&blocked_proc[index], p);
    insertProcQ(&ready_queue, p);
}

/**
 * By storing off the TOD clock’s value at both the start and end of an interval, one can compute
 * the duration of that interval. The interval in question represents the current process time
 * @returns the interval duration
 */
cpu_t IntervalTOD()
{
    // TOD clock starts from 0 and counts up
    static cpu_t timer_start = 0;
    cpu_t curr_time, diff;

    STCK(curr_time);
    diff = curr_time - timer_start;

    STCK(timer_start); // update the start of the interval to current time for future uses

    return diff;
}

/**
 * Updates the current process p_time
 */
void updateProcessCPUTime()
{
    current_process->p_time += IntervalTOD();
}

/**
 * Returns the index of blocked_proc[] corresponding to the command address provided
 * @param cmdAddr command address for the device
 */
int getIODeviceIndex(memaddr cmdAddr)
{
    int index; // (regAddr - devRegStart) / dimension_of_devReg = total # of devices -> 40
    memaddr register_addr;
    memaddr offset = 0;

    // terminal
    if (cmdAddr >= TERM0ADDR)
    {
        // distance between command fields is 8 bit, so if 4th bit is on then it's receiver sub-device
        if (((cmdAddr >> 3) & 1) == 1)
        {
            register_addr = cmdAddr - 0x4;
        }
        else
        {
            register_addr = cmdAddr - 0xc;
            offset = 8;
        }
    }
    // non-terminal
    else
    {
        register_addr = cmdAddr - 0x4; // command field for non-terminal devices is at (base)+0x4
    }
    index = ((register_addr - DEV_REG_START) / DEV_REG_SIZE) + offset;
    return index;
}

void scheduler()
{
    // get the first process in the ready queue
    current_process = removeProcQ(&ready_queue);
    klog_print(" current process: ");
    klog_print_hex((unsigned int)current_process);
    // if the Process Count is 1 and the SSI is the only process in the system, invoke HALT
    if (process_count == 1 && current_process->p_pid == 1)
    {
        HALT();
    }

    // ready queue is empty
    if (current_process == NULL)
    {
        if (process_count > 0)
        {
            // If the Process Count > 0 and the Soft-block Count > 0 enter a Wait State
            if (softBlock_count > 0)
            {
                // enable interrupts and disable PLT: we are waiting for a device interrupt
                setSTATUS((getSTATUS() | IECON | IMON) & ~TEBITON);
                WAIT();
            }
            // Deadlock!!
            else
            {
                PANIC();
            }
        }
    }
    // ready queue has at least one process
    else
    {
        klog_print("dispatching...");
        // remember to enable PLT for every running process
        current_process->p_s.status = (current_process->p_s.status) | TEBITON;
        // load PLT
        setTIMER((cpu_t)TIMESLICE * (*((cpu_t *)TIMESCALEADDR)));
        // Load processor state
        LDST(&current_process->p_s);
    }
}