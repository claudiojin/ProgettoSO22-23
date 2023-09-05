#include "../headers/scheduler.h"
#include "../headers/pcb.h"
#include "../headers/initial.h"
#include <umps3/umps/libumps.h>
#include "../headers/ash.h"

/**
 * This module implements the Scheduler and the deadlock detector.
 */

// Selezionare il prossimo processo da mettere in esecuzione
static pcb_t *getNextProcess()
{
    pcb_t *next_proc = NULL;
    next_proc = removeProcQ(&ready_queue);
    
    return next_proc;
}
void scheduler()
{
    // terminato con successo
    if (process_count == 0)
    {
        HALT();
    }

    // esegue in modalita' FIFO
    curr_process = getNextProcess();

    if (curr_process == NULL){
        // se non la cosa ready_queue e' vuota e softBlock_count > 0,
        //il processore va in attesa
        if (softBlock_count > 0){
            //enable interrupts e termina PLT
            setSTATUS((getSTATUS() | IECON | IMON) & ~TEBITON);
            WAIT();
        }
        else{
            PANIC();
        }
    }
    //esiste almeno un elemento nella ready_queue
    else{
        curr_process->p_s.status =  (curr_process->p_s.status) | TEBITON;
        //load PLT
        setTIMER((cpu_t) TIMESLICE * (*((cpu_t *)TIMESCALEADDR)));
        
        LDST(&curr_process->p_s);

    }
}