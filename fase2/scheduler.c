#include "../headers/scheduler.h"

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

/**
 * @brief Cambia lo stato del processo a bloccato.
 * @param p Puntatore al PCB del processo.
 * @param state Puntatore allo stato da salvare nel processo.
*/
void setProcessBlocked(pcb_t *p, state_t *state) {
    /* Oss: dato che non è prevista la possibilità di bloccare un processo nella ready queue, l'unico che è possibile bloccare è il corrente
            Quindi non è necessario fare nessun controllo sulle ready queue */
    
    curr_process->p_s = *state;
    updateProcessCPUTime();
}


/**
 * @brief Cambia lo stato del processo a ready.
 * @param p Puntatore al PCB del processo.
*/
void setProcessReady(pcb_t *p) {
    if (p->p_pid != -1) {
        insertProcQ(&ready_queue, p);
    }
}