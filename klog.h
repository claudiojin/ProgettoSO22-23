#ifndef DEBUG_H
#define DEBUG_H

#include "phase1/headers/pcb.h"

void klog_print(char *str);
void klog_print_dec(unsigned int num);
void klog_print_hex(unsigned int num);
int child_counter(pcb_t *p);

#endif