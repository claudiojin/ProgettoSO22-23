#include "./headers/scheduler.h"

/**
 * Sets the current process to a "blocked" state and calls the scheduler
 * @param state the state of the processor to load
 */
void blockProcess(state_t *state)
{
    insertProcQ(&frozen_list, current_process);
    current_process->p_s = *state;
    updateProcessCPUTime();
    scheduler();
}

/**
 * sets pcb pointed to by p to a "ready" state by putting it into the ready queue
 * @param p the process to awake
 */
void readyProcess(pcb_t *p)
{
    outProcQ(&frozen_list, p);
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

void scheduler()
{
    // get the first process in the ready queue
    current_process = removeProcQ(&ready_queue);

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
        // remember to enable PLT for every running process
        current_process->p_s.status = (current_process->p_s.status) | TEBITON;
        // load PLT
        setTIMER((cpu_t)TIMESLICE * (*((cpu_t *)TIMESCALEADDR)));
        // Load processor state
        LDST(&current_process->p_s);
    }
}