#ifndef PANDOS_VMSUPPORT_H_INCLUDED
#define PANDOS_VMSUPPORT_H_INCLUDED

#include "../../headers/listx.h"
#include "../../headers/const.h"
#include "../../headers/types.h"
#include "../../phase2/headers/initial.h"
#include "./initProc.h"

void initSwapStructs();
void TLBExceptionHandler();

void initPageTable(int asid, pteEntry_t *page_table, memaddr tmp_frame);

void freeFrames(int asid);

#endif