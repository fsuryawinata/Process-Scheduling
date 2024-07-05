#ifndef _DATA_H_
#define _DATA_H_

typedef struct process process_t;

struct process{
    char *processName;
    int timeArrived;
    int serviceTime;
    int memoryRequirement;
    int cyclesRun;
    int remainingTime;
    int startAddress;
    struct process* next;
};

// Create process node
process_t* createProcessNode(char *processName, int timeArrived, int serviceTime,
int memoryRequirement, int remainingTime, int cyclesRun, int startAddress);

// Free process node memory
void freeProcess(process_t *process);

#endif