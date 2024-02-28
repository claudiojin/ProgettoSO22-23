#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "./initial.h"

void uTLB_RefillHandler();
void exceptionHandler();
cpu_t IntervalTOD();
void updateProcessCPUTime();

#endif