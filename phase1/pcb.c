#include "./headers/pcb.h"
#include "../klog.c"

static pcb_t pcbTable[MAXPROC];
LIST_HEAD(pcbFree_h);
static int next_pid = 1;

/*
    Initialize the pcbFree list to contain all the elements of the static array of MAXPROC PCBs.
    This method will be called only once during data structure initialization.
*/
void initPcbs()
{
    INIT_LIST_HEAD(&pcbFree_h);

    // using the "p_list" field of pcb_t to link the PCBs
    for (int i = 0; i < MAXPROC; i++)
    {
        list_add(&pcbTable[i].p_list, &pcbFree_h);
    }
}

/*
    Insert the element pointed to by p onto the pcbFree list.
*/
void freePcb(pcb_t *p)
{
    // usual NULL check, then add to the pbcFree list head
    if (p != NULL)
    {
        list_add(&p->p_list, &pcbFree_h);
    }
}

/*
    Resets all the fields of the pcb provided
*/
void resetPcb(pcb_t *p)
{
    INIT_LIST_HEAD(&(p->p_list));
    p->p_parent = NULL;
    INIT_LIST_HEAD(&(p->p_child));
    // INIT_LIST_HEAD(&(p->p_sib));
    p->p_sib.next = NULL;
    p->p_sib.prev = NULL;
    // subject to change in phase 2
    p->p_time = 0;
    INIT_LIST_HEAD(&p->msg_inbox);
    p->p_supportStruct = NULL;
    // subject to change in phase 2
    p->p_pid = 0;
}

/*
    Return NULL if the pcbFree list is empty. Otherwise, remove an element from the pcbFree
    list, provide initial values for ALL of the PCBs fields (i.e. NULL and/or 0) and then return
    a pointer to the removed element. PCBs get reused, so it is important that no previous
    value persist in a PCB when it gets reallocated.
*/
pcb_t *allocPcb()
{
    if (list_empty(&pcbFree_h))
        return NULL;
    else
    {
        // head removal
        pcb_t *p = container_of(pcbFree_h.next, pcb_t, p_list);
        list_del(&p->p_list);
        // reset all fields
        resetPcb(p);
        return p;
    }
}

/*
    This method is used to initialize a variable to be head pointer to a process queue.
*/
void mkEmptyProcQ(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

/*
    Return TRUE if the queue whose head is pointed to by head is empty. Return FALSE otherwise.
*/
int emptyProcQ(struct list_head *head)
{
    return list_empty(head);
}

/*
    Insert the PCB pointed by p into the process queue whose head pointer is pointed to by head.
*/
void insertProcQ(struct list_head *head, pcb_t *p)
{
    if (p != NULL)
    {
        list_add_tail(&(p->p_list), head);
    }
}

/*
    Return a pointer to the first PCB from the process queue whose head is pointed to by head. Do
    not remove this PCB from the process queue. Return NULL if the process queue is empty.
*/
pcb_t *headProcQ(struct list_head *head)
{
    if (emptyProcQ(head))
        return NULL;
    else
        // returns the pointer to the first element (head->next), casting it to *pcb_t
        return container_of(head->next, pcb_t, p_list);
}

/*
    Remove the first (i.e. head) element from the process queue whose head pointer is pointed to
    by head. Return NULL if the process queue was initially empty; otherwise return the pointer
    to the removed element.
*/
pcb_t *removeProcQ(struct list_head *head)
{
    // check for empty list
    if (emptyProcQ(head))
        return NULL;
    // first element of the list
    struct list_head *pos = head->next;
    // single element list
    if (pos->next == head)
    {
        list_del(pos);
        // reinizializzo la sentinella
        INIT_LIST_HEAD(head);
        return container_of(pos, pcb_t, p_list);
    }
    // multiple elements
    else
    {
        head->next = pos->next;
        pos->next->prev = head;
        list_del(pos);
        return container_of(pos, pcb_t, p_list);
    }
}

/*
    Remove the PCB pointed to by p from the process queue whose head pointer is pointed to by
    head. If the desired entry is not in the indicated queue (an error condition), return NULL;
    otherwise, return p. Note that p can point to any element of the process queue.
*/
pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
    // usual null check
    if (emptyProcQ(head) || p == NULL)
        return NULL;
    // first element of the list
    struct list_head *pos = head->next;
    // general case
    if (container_of(pos, pcb_t, p_list) != p)
    {
        while (pos != head)
        {
            if (container_of(pos, pcb_t, p_list) == p)
            {
                pos->prev->next = pos->next;
                pos->next->prev = pos->prev;
                list_del(pos);
                return container_of(pos, pcb_t, p_list);
            }
            pos = pos->next;
        }
        // if nothing's found return NULL
        return NULL;
    }
    // will go here if the pcb to remove is first in the list
    else
        return removeProcQ(head);
}

/*
    Return TRUE if the PCB pointed to by p has no children. Return FALSE otherwise.
*/
int emptyChild(pcb_t *p)
{
    if (emptyProcQ(&(p->p_child)))
        return TRUE;
    else
        return FALSE;
}

/*
    make the PCB pointed to by p a child of the PCB pointed to by prnt.
*/
void insertChild(pcb_t *prnt, pcb_t *p)
{
    if (prnt != NULL && p != NULL)
    {
        // add to the tail so the order of the children is kept when inserting
        list_add_tail(&p->p_sib, &prnt->p_child);
        p->p_parent = prnt;
    }
}

/*
    Make the first child of the PCB pointed to by p no longer a child of p. Return NULL if initially
    there were no children of p. Otherwise, return a pointer to this removed first child PCB.
*/
pcb_t *removeChild(pcb_t *p)
{
    if (p == NULL || emptyProcQ(&p->p_child))
        return NULL;
    else
    {
        // first element of the list
        struct list_head *tmp = p->p_child.next;
        // multiple children
        if (tmp != &p->p_child)
        {
            p->p_child.next = tmp->next;
            tmp->next->prev = &p->p_child;
            list_del(tmp);
        }
        // only child
        else
        {
            list_del(tmp);
            INIT_LIST_HEAD(&p->p_child);
        }
        return container_of(tmp, pcb_t, p_sib);
    }
}

/*
    Make the PCB pointed to by p no longer the child of its parent. If the PCB pointed to by p has
    no parent, return NULL; otherwise, return p. Note that the element pointed to by p could be
    in an arbitrary position (i.e. not be the first child of its parent).
*/
pcb_t *outChild(pcb_t *p)
{
    if (p == NULL || p->p_parent == NULL)
        return NULL;
    else
    {
        // left brother
        struct list_head *tmp = p->p_sib.prev;
        // pcb to remove is the first brother
        if (tmp == &p->p_parent->p_child)
            return removeChild(p->p_parent);
        // general case
        else
        {
            tmp->next = p->p_sib.next;
            p->p_sib.next->prev = tmp;
            list_del(&p->p_sib);
            return p;
        }
    }
}
