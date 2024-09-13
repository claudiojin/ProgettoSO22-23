#ifndef PANDOS_VMSUPPORT_H_INCLUDED
#define PANDOS_VMSUPPORT_H_INCLUDED

#include "./initProc.h"

void initSwapStructs();

void TLBExceptionHandler();

void initPageTable(int asid, pteEntry_t *page_table, memaddr tmp_frame);

void freeFrames(int asid);

#endif