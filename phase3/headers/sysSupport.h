#ifndef PANDOS_SYSSUPPORT_H_INCLUDED
#define PANDOS_SYSSUPPORT_H_INCLUDED

#include "./initProc.h"

/**
 * This module implements the TLB, Program Trap, and SYSCALL exception handlers.
 */

void generalExceptionHandler();

void trapExceptionHandler();

#endif