#ifndef _BESTFIT_H_
#define _BESTFIT_H_

#include "queue.h"

#define MAX_MEMORY 2048

// Allocate processes to next memory block available 
// and returns the start address of the block
int bestFit(process_t *process, int time);

// Free memory of processes
void freeMemory(process_t *process);

#endif