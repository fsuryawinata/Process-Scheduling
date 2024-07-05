#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bestfit.h"
#include "queue.h"

#define MAX_MEMORY 2048
#define NOT_ALLOCATED -1
#define FREE 0
#define ALLOCATED 1

// Initialise memBlocks
static int memBlocks[MAX_MEMORY];

// Allocate processes to next memory block available 
// and returns the start address of the block
int bestFit(process_t *process, int time) {
    int freeBlocks = MAX_MEMORY, startBlock, endBlock = 0;
    int startAddress = NOT_ALLOCATED;
    int minGap = MAX_MEMORY;
    int j = 0;

    // Find free block set with the least memory waste (best fit)
    for (int i = 0; i < MAX_MEMORY; i++) {

        while (j<10) {
            //printf("MEMBLOCKS %d\n", memBlocks[j]);
            j++;
        }

        // Found a not yet allocated block
        if (memBlocks[i] == FREE){
            startBlock = i;
            endBlock = i;
            freeBlocks = 0;

            // Find a continuous block of free space from startAddress
            while ((endBlock < MAX_MEMORY) && (memBlocks[endBlock] == FREE)) {
                endBlock++;
                freeBlocks++;
            }

            // Check if process is able to be allocated
            if (freeBlocks >= process->memoryRequirement){
                // Find the block set with minimal waste
                int gap = freeBlocks - process->memoryRequirement;

                if (gap < minGap){
                    minGap = gap;
                    startAddress = startBlock;
                }
            }
            // Go to next free block
            i = endBlock;
        }
    }
    if (startAddress == NOT_ALLOCATED){
        return NOT_ALLOCATED;
    }

    // Allocate block
    for (int i = startAddress; i < startAddress + process->memoryRequirement; i++){
        memBlocks[i] = ALLOCATED;
    }
    printf("%d,READY,process_name=%s,assigned_at=%d\n", time, process->processName, startAddress);
    return startAddress;
}

// Free memory of processes
void freeMemory(process_t *process){
    int startAddress = process->startAddress;
    for (int i = startAddress; i < startAddress + process->memoryRequirement; i++){
        memBlocks[i] = FREE;
    }
    process->startAddress = NOT_ALLOCATED;
}