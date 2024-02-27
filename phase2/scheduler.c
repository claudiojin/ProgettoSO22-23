#include "./headers/scheduler.h"

void scheduler()
{
    // get the first process in the ready queue
    curr_process = removeProcQ(&ready_queue);

    // if the Process Count is 1 and the SSI is the only process in the system, invoke HALT
    if (process_count == 1 && curr_process->p_pid == 1)
    {
        HALT();
    }

    // ready queue is empty
    if (curr_process == NULL)
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
        curr_process->p_s.status = (curr_process->p_s.status) | TEBITON;
        // load PLT
        setTIMER((cpu_t) TIMESLICE * (*((cpu_t *)TIMESCALEADDR)));
        // Load processor state
        LDST(&curr_process->p_s);
    }
}