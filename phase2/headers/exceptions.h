#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "./initial.h"

void uTLB_RefillHandler();
void exceptionHandler();
int SendMessage(pcb_t *destination, unsigned int *payload, pcb_t *sender);

#endif