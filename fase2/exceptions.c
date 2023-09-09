#include "../headers/exceptions.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 * Furthermore, this module will contain the provided skeleton TLB-Refill event
 * handler (e.g. uTLB_RefillHandler).
 */

#define PAR1(type) ((type)BLOCKED_PROCESS_STATE->reg_a1)
#define PAR2(type) ((type)BLOCKED_PROCESS_STATE->reg_a2)
#define PAR3(type) ((type)BLOCKED_PROCESS_STATE->reg_a3)
#define EXCEPTION_CODE CAUSE_GET_EXCCODE(BLOCKED_PROCESS_STATE->cause)
#define SYSTEMCALL_CODE ((int)BLOCKED_PROCESS_STATE->reg_a0)
#define V(semaddr) semV(semaddr, curr_process, BLOCKED_PROCESS_STATE)
#define P(semaddr) semP(semaddr, curr_process, BLOCKED_PROCESS_STATE)

static int Create_Process(state_t *statep, support_t *supportp, nsd_t *ns)
{

    pcb_t *new_proc = allocPcb();

    // Non ci sono PCB liberi
    if (new_proc == NULL)
    {
        return -1;
    }

    process_count++;
    if (ns == NULL)
    {
        // ns uguale al processo padre
        addNamespace(new_proc, getNamespace(curr_process, NS_PID));
    }
    else
    {
        addNamespace(new_proc, ns);
    }
    new_proc->p_s = *statep;
    new_proc->p_supportStruct = supportp;
    insertProcQ(&ready_queue, new_proc);
    insertChild(curr_process, new_proc);

    return new_proc->p_pid;
}

static void _killProcess(pcb_t *process)
{
    outChild(process);

    process_count--;

    // Gestione semafori
    if (process->p_semAdd != NULL)
    {
        if (isSoftBlocked(process))
        {
            softBlock_count--;
        }
        outBlocked(process);
    }

    // Rimuove il processo dalla sua coda ready (se necessario)
    outProcQ(&ready_queue, process);

    pcb_t *child;
    while ((child = removeChild(process)) != NULL)
    {
        _killProcess(child);
    }

    freePcb(process);
}

static void Terminate_Process(int pid)
{
    // uccido il padre
    if (pid == 0)
    {
        _killProcess(curr_process);
        scheduler();
    }
    else
    {
        pcb_t *process_to_kill = getProcessByPid(pid);
        if (process_to_kill != NULL)
        {
            _killProcess(process_to_kill);
        }

        // Il pid scelto potrebbe essere un antenato del processo corrente
        if (curr_process->p_pid != -1)
        {
            scheduler();
        }
    }
}

// chiama una P sul semaforo semaddr
static void Passeren(int *semaddr)
{
    P(semaddr);
}

// chiama una V sul semaforo semaddr
static void Verhogen(int *semaddr)
{
    V(semaddr);
}

// Effettua un'operazione di IO
// cmdAddr e' l'indirizzo del dispositivo
// cmdValues e' un vettore di 2 interi(per i terminali) o 4 (per altri devices)
void DO_IO(int *cmdAddr, int *cmdValues)
{
    for (int i = 0; i < (((memaddr)cmdAddr >= TERM0ADDR) ? 2 : 4); i++)
    {
        cmdAddr[i] = cmdValues[i];
    }
    // mette in pausa il processo mancante
    softBlock_count++;
    // metto il semaforo P
    int *semAdrr = getIODeviceSemaphore((memaddr)cmdAddr + 0x4);
    Passeren(semAdrr);
}

// restituisce il tempo di esecuzione (in microsecondi) del
// processo che l’ha chiamata fino a quel momento

int Get_CPU_Time()
{
    curr_process->p_time += getTimeElapsed();
    return curr_process->p_time;
}

void Wait_For_Clock()
{
    softBlock_count++;
    // l'ultimo e' il Pseudo-clock semaphore, usato per Interval Timer
    Passeren(&device_semaphores[DEV_SEMAPHORES]);
}

support_t *Get_Support_Data()
{
    return curr_process->p_supportStruct;
}

int Get_Process_Id(int parent)
{
    if (parent == 0)
    {
        return curr_process->p_pid;
    }
    else
    {
        if (curr_process->p_parent == NULL || getNamespace(curr_process, NS_PID) != getNamespace(curr_process->p_parent, NS_PID))
        {
            return 0;
        }
        else
        {
            return curr_process->p_parent->p_pid;
        }
    }
}

int Get_Children(int *children, int size)
{
    struct list_head *it_proc;
    int count = 0;
    list_for_each(it_proc, &curr_process->p_child)
    {
        pcb_PTR currentPcb = list_entry(it_proc, pcb_t, p_sib);
        if (getNamespace(currentPcb, NS_PID) == getNamespace(curr_process, NS_PID))
        {
            if (count < size)
            {
                children[count] = currentPcb->p_pid;
                count++;
            }
        }
    }
    return count;
}

static void passUpOrDieHandler(int index)
{
    if (curr_process->p_supportStruct == NULL)
    {
        _killProcess(curr_process);
        scheduler();
    }
    else
    {
        curr_process->p_supportStruct->sup_exceptState[index] = *BLOCKED_PROCESS_STATE;
        context_t ctx = curr_process->p_supportStruct->sup_exceptContext[index];
        LDCXT(ctx.stackPtr, ctx.status, ctx.pc);
    }
}

static void generateInvalidSystemCallTrap()
{
    BLOCKED_PROCESS_STATE->cause = (BLOCKED_PROCESS_STATE->cause & 0xFFFFFF83) | 0x28; // Reserved instruction
    passUpOrDieHandler(GENERALEXCEPT);
}

static void systemcallHandler()
{
    if (SYSTEMCALL_CODE >= 1)
    {
        passUpOrDieHandler(GENERALEXCEPT);
    }

    // Controllo permessi (kernel mode)
    if ((BLOCKED_PROCESS_STATE->status & USERPON) != 0)
    {
        generateInvalidSystemCallTrap();
    }

    // Incremento PC
    BLOCKED_PROCESS_STATE->pc_epc += WORDLEN;

    switch (SYSTEMCALL_CODE)
    {
    case (CREATEPROCESS):
        BLOCKED_PROCESS_STATE->reg_v0 = Create_Process(PAR1(state_t *), PAR2(support_t *), PAR3(nsd_t *));
        break;
    case (TERMPROCESS):
        Terminate_Process(PAR1(int));
        break;
    case (PASSEREN):
        Passeren(PAR1(int *));
        break;
    case (VERHOGEN):
        Verhogen(PAR1(int *));
        break;
    case (DOIO):
        DO_IO(PAR1(int *), PAR2(int *));
        break;
    case (GETTIME):
        BLOCKED_PROCESS_STATE->reg_v0 = Get_CPU_Time();
        break;
    case (CLOCKWAIT):
        Wait_For_Clock();
        break;
    case (GETSUPPORTPTR):
        Get_Support_Data();
        break;
    case (GETPROCESSID):
        Get_Process_Id(PAR1(int));
        break;
    case (GETCHILDREN):
        Get_Children(PAR1(int *), PAR2(int));
        break;
    default: // Codice system call non valida
        generateInvalidSystemCallTrap();
        break;
    }

    LDST(BLOCKED_PROCESS_STATE);
}

void exceptionHandler()
{
    switch (EXCEPTION_CODE)
    {
    // Interrupt
    case (0):
        interruptHandler();
        break;

    // TLB exception
    case (1):
    case (2):
    case (3):
        passUpOrDieHandler(PGFAULTEXCEPT);
        break;

    // Program trap
    case (4):
    case (5):
    case (6):
    case (7):
    case (9):
    case (10):
    case (11):
    case (12):
        passUpOrDieHandler(GENERALEXCEPT);
        break;

    // Systemcall
    case (8):
        systemcallHandler();
        break;
    }
}
