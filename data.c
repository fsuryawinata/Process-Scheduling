#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "data.h"

// Create process node
process_t* createProcessNode(char *processName, int timeArrived, int serviceTime, 
int memoryRequirement, int remainingTime, int cyclesRun, int startAddress) {
    process_t* node = (process_t*) malloc(sizeof(process_t));
    node->processName = strdup(processName);
    node->timeArrived = timeArrived;
    node->serviceTime = serviceTime;
    node->memoryRequirement = memoryRequirement;
    node->remainingTime = remainingTime;
    node->cyclesRun = cyclesRun;
    node->startAddress = startAddress;
    node->next = NULL;
    return node;
}

// Free process node memory
void freeProcess(process_t *process) {
    free(process->processName);
    free(process);

}