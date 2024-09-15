#ifndef PANDOS_VMSUPPORT_H_INCLUDED
#define PANDOS_VMSUPPORT_H_INCLUDED

#include "./initProc.h"

/**
 * This module implements The Pager.
 * Since reading and writing to each U-proc’s flash device is limited to supporting paging, this module should
 * also contain the function(s) for reading and writing flash devices.
 * Additionally, the Swap Pool table is local to this module.
 * Instead of declaring them globally in initProc.c they can be declared module-wide in vmSupport.c.
 * The test function will now invoke a new “public” function initSwapStructs which will do the work of initializing
 * the Swap Pool table.
 */

void initSwapStructs();

void TLBExceptionHandler();

void initPageTable(int asid, pteEntry_t *page_table, memaddr tmp_frame);

void freeFrames(int asid);

#endif