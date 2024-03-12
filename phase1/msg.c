#include "./headers/msg.h"

static msg_t msgTable[MAXMESSAGES];
LIST_HEAD(msgFree_h);

/*
    Initialize the msgFree list to contain all the elements of the static array of MAXMESSAGES
    messages. This method will be called only once during data structure initialization.
*/
void initMsgs()
{
    INIT_LIST_HEAD(&msgFree_h);

    // using the "p_list" field of pcb_t to link the PCBs
    for (int i = 0; i < MAXPROC; i++)
    {
        list_add(&msgTable[i].m_list, &msgFree_h);
    }
}

/*
    Insert the element pointed to by m onto the msgFree list.
*/
void freeMsg(msg_t *m)
{
    // usual NULL check, then add to the pbcFree list head
    if (m != NULL)
    {
        list_add(&m->m_list, &msgFree_h);
    }
}

/*
    Resets all the fields of the msg provided
*/
void resetMsg(msg_t *m)
{
    INIT_LIST_HEAD(&m->m_list);
    m->m_payload = 0;
    m->m_sender = NULL;
    // m->m_service_code = -1; // default case, terminated if caught by ssi
}

/*
    Return NULL if the msgFree list is empty. Otherwise, remove an element from the msgFree
    list, provide initial values for ALL of the messages fields (i.e. NULL and/or 0) and then
    return a pointer to the removed element. Messages get reused, so it is important that no
    previous value persist in a message when it gets reallocated.
*/
msg_t *allocMsg()
{
    if (list_empty(&msgFree_h))
        return NULL;
    else
    {
        // head removal
        msg_t *m = container_of(msgFree_h.next, msg_t, m_list);
        list_del(&m->m_list);
        // reset all fields
        resetMsg(m);
        return m;
    }
}

/*
    Used to initialize a variable to be head pointer to a message queue;
    returns a pointer to the head of an empty message queue.
*/
void mkEmptyMessageQ(struct list_head *head)
{
    if (head != NULL)
        INIT_LIST_HEAD(head);
}

/*
    Returns TRUE if the queue whose tail is pointed to by head is empty, FALSE otherwise.
*/
int emptyMessageQ(struct list_head *head)
{
    if (head != NULL)
        return list_empty(head);
    else
        return FALSE;
}

/*
    Insert the message pointed to by m at the END of the queue whose head pointer is pointed to by head.
*/
void insertMessage(struct list_head *head, msg_t *m)
{
    if (head != NULL && m != NULL)
        list_add_tail(&m->m_list, head);
}

/*
    Insert the message pointed to by m at the HEAD of the queue whose head pointer is pointed to by head.
*/
void pushMessage(struct list_head *head, msg_t *m)
{
    if (head != NULL && m != NULL)
        list_add(&m->m_list, head);
}

/*
    Remove the first element (starting by the head) from the message queue accessed via head whose
    sender is p_ptr.
    If p_ptr is NULL, return the first message in the queue. Return NULL if the message queue
    was empty or if no message from p_ptr was found; otherwise return the pointer to the removed
    message.
*/
msg_t *popMessage(struct list_head *head, pcb_t *p_ptr)
{
    if (head == NULL || list_empty(head))
        return NULL;
    if (p_ptr == NULL)
    {
        struct list_head *first = head->next;
        list_del(first);
        return container_of(first, msg_t, m_list);
    }
    else
    {
        msg_t *iter = NULL;
        list_for_each_entry(iter, head, m_list)
        {
            if (iter->m_sender == p_ptr)
            {
                list_del(&iter->m_list);
                return iter;
            }
        }
        // if no message from p_ptr is found
        return NULL;
    }
}

/*
    Return a pointer to the first message from the queue whose head is pointed to by head. Do not
    remove the message from the queue. Return NULL if the queue is empty.
*/
msg_t *headMessage(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return NULL;
    else
        return container_of(head->next, msg_t, m_list);
}

