#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "queue.h"
#include "bestfit.h"

#define SJF "SJF"
#define RR "RR"
#define BF "best-fit"
#define MAX_MEMORY 2048
#define NOT_ALLOCATED -1
#define STRLEN 5
#define BYTESIZE 32
#define STRSIZE 64

#define IMPLEMENTS_REAL_PROCESS

// Run arrived tasks by shortest job first
void ShortestJobFirst(queue_t *inputQueue, char **argv);
// Run processes by auantum
void RoundRobin(queue_t *inputQueue, char **argv);
// Calculate turnaround and oevrhead
void calculations(int turnaroundTime[], double overheadTime[], int numProcesses, int time);
// Convert time to Big Endian
void convertToBEBO(uint32_t time, uint8_t* arr);
// Get index from process name
int getIndex(char *processName);

int main(int argc, char *argv[]) {
    queue_t *inputQueue = createQueue();

    FILE *fp = fopen(argv[2], "r");

    // Read file and insert into inputQueue
    int timeArrived, serviceTime, memoryRequirement;
    char *processName = (char *)malloc(STRLEN * sizeof(char));
    while (fscanf(fp, "%d %s %d %d\n", &timeArrived, processName, &serviceTime, &memoryRequirement) == 4) {
        // Insert process node into queue
        char *nameCopy = strdup(processName);
        process_t* process = createProcessNode(nameCopy, timeArrived, serviceTime, memoryRequirement, 0, 0, NOT_ALLOCATED);
        push(inputQueue, process);
    }
    fclose(fp);

    if (strcmp(argv[4], SJF) == 0) {
        ShortestJobFirst(inputQueue, argv);
    } else if (strcmp(argv[4], RR) == 0) {
        RoundRobin(inputQueue, argv);
    }

    free(processName);
    freeQueue(inputQueue);
}

// Run arrived tasks by shortest job first
void ShortestJobFirst(queue_t *inputQueue, char **argv) {
    int numProcesses = inputQueue->size;
    int quantum = atoi(argv[8]);

    pid_t pid = pid;
    int parentfd[2], childfd[2];

    int turnaroundTime[numProcesses];
    double overheadTime[numProcesses];
    int i = 0;

    // Determine scheduling
    int CPUbusy = 0, requiredTime = 0, startTime = 0, finished = 0;
    int numProcLeft = 0;
    int totalProcLeft = numProcesses;

    process_t *runningProcess = NULL;
    queue_t *readyQueue = createQueue();

    int time = 0; // Simulated time

    while(totalProcLeft > 0) {
        // Calculate how long the process run for
        int runtime = time - startTime;
        // Process finished running it's full duration
        if (runningProcess != NULL && runtime >= requiredTime && requiredTime != 0) {
            CPUbusy = 0; // CPU no longer busy
            numProcLeft = readyQueue->size;
            printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", time, runningProcess->processName, numProcLeft);

            // Free allocated memory
            if (strcmp(argv[6], BF) == 0) {
                freeMemory(runningProcess);
            }

            // Calculate turnaround and overhead
            turnaroundTime[i] = time - runningProcess->timeArrived;
            overheadTime[i] = (double) turnaroundTime[i] / (double) runningProcess->serviceTime;
            totalProcLeft--;
            i++;

            // Terminate process
            // Convert to BEBO
            uint32_t simTime = time;
            u_int8_t arr[4];
            convertToBEBO(simTime, arr);

            kill(pid, SIGTERM);
            
            // Send to standard input
            write(parentfd[1], &arr, 4);

            // Read string
            char hash[STRSIZE];
            read(childfd[0], &hash, STRSIZE);

            printf("%d,FINISHED-PROCESS,process_name=%s,sha=%s\n", time, runningProcess->processName, hash);
            finished = 1;
        } else if (finished == 0){
            // Continue process
            // Convert to BEBO
            uint32_t simTime = time;
            u_int8_t arr[4];

            convertToBEBO(simTime, arr);

            kill(pid, SIGCONT);
            
            // Send to standard input
            write(parentfd[1], &arr, 4);

            // Verify synchronisation
            // Read 1 byte from standard output
            char outputByte;
            read(childfd[0], &outputByte, sizeof(outputByte));

            // Verify read and sent
            uint8_t leastSig = arr[3];
            if (leastSig != (outputByte & 0xff)){
                perror("Not synced CONTINUE");

            }
        }

        
        
        // End cycle early if no processes left
        if (totalProcLeft == 0) {
            break;
        }

        // Move arrived processes to a READY queue
        if (inputQueue->size != 0) {
            process_t *currProcess = inputQueue->head;
            if (currProcess->timeArrived <= time){
                // Allocate memory by best fit
                if (strcmp(argv[6], BF) == 0) {
                    int startAddress = bestFit(currProcess, time);
                    // If allocated, push into ready queue
                    if (startAddress != NOT_ALLOCATED){
                        process_t *arrivedProcess = pop(inputQueue);
                        arrivedProcess->startAddress = startAddress;

                        // Sort queue by SJF
                        SJFpush(readyQueue, arrivedProcess);
                    } 
                    
                } else {
                    process_t *arrivedProcess = pop(inputQueue);
                    // Sort queue by SJF
                    SJFpush(readyQueue, arrivedProcess);
                }
            }
                
        }

        // Choose and run the READY process with the shortest job
        if (CPUbusy == 0) {
            // Choose from READY processes
            startTime = time;

            // Choose next process with shortest job
            if (readyQueue->size != 0) {
                runningProcess = pop(readyQueue);
                requiredTime = runningProcess->serviceTime;
                printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", time, runningProcess->processName, requiredTime);
                CPUbusy = 1;
                finished = 0;

                // Create new process
                pipe(childfd);
                pipe(parentfd);
                if ((pid = fork()) == -1){
                    perror("fork");
                    exit(1);
                }
                // Create processes
                if (pid == 0){
                    close(childfd[0]);
                    close(parentfd[1]);

                    // Redirect output to stdout and input to STDIN
                    dup2(parentfd[0], STDIN_FILENO);
                    dup2(childfd[1], STDOUT_FILENO);

                    // Execute process with arguments
                    char* args[] = {"./process", runningProcess->processName, NULL};
                    execv(args[0], args);

                // Parent process
                } else {
                    // Convert to BEBO
                    uint32_t simTime = time;
                    u_int8_t arr[4];
                    convertToBEBO(simTime, arr);
                    
                    // Send to standard input
                    write(parentfd[1], &arr, 4);

                    // Verify read and sent
                    // Read 1 byte from standard output
                    char outputByte;
                    read(childfd[0], &outputByte, sizeof(outputByte));

                    // Verify read and sent
                    uint8_t leastSig = arr[3];
                    if (leastSig != (outputByte & 0xff)){
                        perror("Not synced CREATE");
                    }
                }
            }
        }
        runningProcess->cyclesRun++;
        time += quantum;
    }

    calculations(turnaroundTime, overheadTime, numProcesses, time);
}

// Run processes by auantum
void RoundRobin(queue_t *inputQueue, char **argv){
    int numProcesses = inputQueue->size;
    int quantum = atoi(argv[8]);

    pid_t pid[numProcesses + 1];
    int parentfd[numProcesses + 1][2], childfd[numProcesses + 1][2];

    int turnaroundTime[numProcesses];
    double overheadTime[numProcesses];
    int i = 0;

    // Determine scheduling
    int CPUbusy = 0, requiredTime = 0;
    int numProcLeft = 0;
    int totalProcLeft = numProcesses;

    process_t *runningProcess = NULL, *prevProcess = NULL;
    queue_t *readyQueue = createQueue();

    int time = 0; // Simulated time

    while(totalProcLeft > 0) {
        // Identify if process finished running
        // Process finished running fully
        if (runningProcess != NULL) {
            runningProcess->remainingTime = requiredTime - quantum * runningProcess->cyclesRun;

            // Process has no more remaining time
            if (runningProcess->remainingTime <= 0) {
                prevProcess = runningProcess;

                // Free allocated memory
                if (strcmp(argv[6], BF) == 0) {

                    // Find number of processes arrived left
                    numProcLeft = readyQueue->size + inputQueue -> size;
                    printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", time, runningProcess->processName, numProcLeft);
                
                    freeMemory(runningProcess);
                } else {
                    numProcLeft = readyQueue->size;
                    printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", time, runningProcess->processName, numProcLeft);
                }


                turnaroundTime[i] = time - runningProcess->timeArrived;
                overheadTime[i] = (double) turnaroundTime[i] / runningProcess->serviceTime;
                totalProcLeft--;
                i++;

                // Terminate process
                int index = getIndex(runningProcess->processName);
                // Convert to BEBO
                uint32_t simTime = time;
                u_int8_t arr[4];
                convertToBEBO(simTime, arr);

                kill(pid[index], SIGTERM);
                
                // Send to standard input
                write(parentfd[index][1], &arr, 4);

                // Read string
                char hash[STRSIZE];
                read(childfd[index][0], &hash, STRSIZE);

                printf("%d,FINISHED-PROCESS,process_name=%s,sha=%s\n", time, runningProcess->processName, hash);

                runningProcess = NULL;
            }
        }
        
        // End cycle early if no processes left
        if (totalProcLeft == 0) {
            break;
        }

        // Move arrived processes to a READY queue
        if (inputQueue->size != 0) {
            process_t *currProcess = inputQueue->head;
            if (currProcess->timeArrived <= time){  
                if (strcmp(argv[6], BF) == 0) {
                    // Allocate memory
                    int startAddress = bestFit(currProcess, time);

                    // If allocated, push into ready queue
                    if (startAddress != NOT_ALLOCATED){
                        process_t *arrivedProcess = pop(inputQueue);
                        arrivedProcess->startAddress = startAddress;
                        // Insert by arrival into READY queue
                        push(readyQueue, arrivedProcess);
                    }
                } else {
                    process_t *arrivedProcess = pop(inputQueue);
                    // Insert by arrival into READY queue
                    push(readyQueue, arrivedProcess);
                }
            }
        }

        // Choose and run the READY process with the shortest job
        if (CPUbusy == 0) {
            // Choose from READY processes
            // Process finished quantum
            if (runningProcess != NULL) {
                CPUbusy = 0;
                // Add to the back of the READY queue
                runningProcess->remainingTime = requiredTime - quantum * runningProcess->cyclesRun;
                push(readyQueue, runningProcess);
                prevProcess = runningProcess;

                // Suspend process if there are other processes
                if (readyQueue->size > 1){
                    int index = getIndex(runningProcess->processName);
                    // Send time to process
                    uint32_t simTime = time;
                    u_int8_t arr[4];
                    convertToBEBO(simTime, arr);
        
                    // Send to input
                    write(parentfd[index][1], arr, sizeof(arr));

                    kill(pid[index], SIGTSTP);

                    // Wait
                    int wstatus;
                    do {
                        pid_t w = waitpid(pid[index], &wstatus, WUNTRACED);
                        if (w == -1) {
                            perror("waitpid");
                            exit(EXIT_FAILURE);
                        }

                    } while (!WIFSTOPPED(wstatus));
                }
            }

            // Pick next ready process
            if (readyQueue->size != 0) {
                runningProcess = pop(readyQueue);
                requiredTime = runningProcess->serviceTime;
                
                // Calculate remaining time
                if (prevProcess != runningProcess) {
                    runningProcess->remainingTime = requiredTime - quantum * runningProcess->cyclesRun;
                    printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", time, runningProcess->processName, 
                    runningProcess->remainingTime);
                }

                // Create new process when it first is run
                if (runningProcess->cyclesRun == 0){
                    int index = getIndex(runningProcess->processName);;

                    pipe(childfd[index]);
                    pipe(parentfd[index]);
                    if ((pid[index] = fork()) == -1){
                        perror("fork");
                        exit(1);
                    }
                    // Create processes
                    if (pid[index] == 0){
                        close(childfd[index][0]);
                        close(parentfd[index][1]);

                        // Redirect output to stdout and input to STDIN
                        dup2(parentfd[index][0], STDIN_FILENO);
                        dup2(childfd[index][1], STDOUT_FILENO);

                        // Execute process with arguments
                        char* args[] = {"./process", runningProcess->processName, NULL};
                        execv(args[0], args);

                    // Parent process
                    } else {
                        // Convert to BEBO
                        uint32_t simTime = time;
                        u_int8_t arr[4];
                        convertToBEBO(simTime, arr);

                        // Send to standard input
                        write(parentfd[index][1], &arr, 4);

                        // Verify read and sent
                        // Read 1 byte from standard output
                        char outputByte;
                        read(childfd[index][0], &outputByte, sizeof(outputByte));

                        // Verify read and sent
                        uint8_t leastSig = arr[3];
                        if (leastSig != (outputByte & 0xff)){
                            perror("Not synced CREATE");
                        }
                    }
                } else {
                    // Continue process
                    int index = getIndex(runningProcess->processName);
                    // Convert to BEBO
                    uint32_t simTime = time;
                    u_int8_t arr[4];

                    convertToBEBO(simTime, arr);

                    kill(pid[index], SIGCONT);
                    
                    // Send to standard input
                    write(parentfd[index][1], &arr, 4);

                    // Verify synchronisation
                    // Read 1 byte from standard output
                    unsigned char outputByte;
                    read(childfd[index][0], &outputByte, sizeof(outputByte));

                    // Verify read and sent
                    uint8_t leastSig = arr[3];
                    if (leastSig != (outputByte & 0xff)){
                        perror("Not synced CONTINUE");
                    }
                }
            }
            
        }
        runningProcess->cyclesRun++;
        time += quantum;
    }
    calculations(turnaroundTime, overheadTime, numProcesses, time);
}

// Calculate turnaround and oevrhead
void calculations(int turnaroundTime[], double overheadTime[], int numProcesses, int time){
    // Find average turnaround time
    double totalTurnaround = 0;
    for (int j = 0; j < numProcesses; j++){
        totalTurnaround += turnaroundTime[j];
    }

    int avgTurnaround = 0;
    avgTurnaround = ceil(totalTurnaround / numProcesses);
    printf("Turnaround time %d\n", avgTurnaround);

    // Find max and average overhead
    double totalOverhead = 0;
    double maxOverhead = 0;
    maxOverhead = overheadTime[0];
    for (int j = 0; j < numProcesses; j++){
        if (overheadTime[j] > maxOverhead){
            maxOverhead = overheadTime[j];
        }
        totalOverhead += overheadTime[j];
    }

    double avgOverhead = (double) (totalOverhead / numProcesses);
    double roundedAvgOverhead = round(avgOverhead * 100) / 100;
    double roundedMaxOverhead = roundf(maxOverhead * 100.0) / 100.0;
    printf("Time overhead %.2f %.2f\n", roundedMaxOverhead, roundedAvgOverhead);

    printf("Makespan %d\n", time);
}

// Convert time to Big Endian
void convertToBEBO(uint32_t time, uint8_t* arr) {
    arr[0] = (time >> 24) & 0xFF;
    arr[1] = (time >> 16) & 0xFF;
    arr[2] = (time >> 8) & 0xFF;
    arr[3] = time & 0xFF;
}

// Get index from process name
int getIndex(char *processName){
    int index = NOT_ALLOCATED;
    sscanf(processName, "P%d", &index);
    return index;
}