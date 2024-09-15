#ifndef DEVMUTEX
#define DEVMUTEX

#include "./initProc.h"

void initPrinterMutex(int asid);
void initTerminalMutex(int asid);

#endif