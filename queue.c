#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "queue.h"

// Creates new queue
queue_t* createQueue() {
    queue_t *q = (queue_t*) malloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

// Add process node into queue by FIFO
void push(queue_t *q, process_t *node) {
    // If queue is empty, add node to front of queue
    if (q->size == 0) {
        q->head = q->tail = node;
        node->next = NULL;
        q->size++;
    } else {
        // Add node to end of queue and increase size
        q->tail->next = node;
        q->tail = node;
        node->next = NULL;
        q->size++;
    }
}

// Adds process to queue by shortest job first
void SJFpush(queue_t *q, process_t *node) {

    // If first in queue or if service time is shorter than queue head, add to front of queue
    if (q->head == NULL || node->serviceTime < q->head->serviceTime) {
        node->next = q->head;
        q->head = node;

    // If serviceTime is equal
    } else if (node->serviceTime == q->head->serviceTime && node->timeArrived < q->head->timeArrived) {
        node->next = q->head;
        q->head = node;

    // If serviceTime and timeArrived is equal
    } else if (node->serviceTime == q->head->serviceTime && 
            node->timeArrived == q->head->timeArrived && 
            strcmp(node->processName, q->head->processName) < 0) {
        node->next = q->head;
        q->head = node;
        
    } else {
        process_t *currProcess = q->head;

        // Find appropriate place in queue
        while (currProcess->next != NULL && node->serviceTime > currProcess->next->serviceTime) {
            
            if (node->serviceTime == currProcess->next->serviceTime && 
                node->timeArrived > currProcess->next->timeArrived) {
                break;

            } else if (node->serviceTime == currProcess->next->serviceTime &&
                    node->timeArrived == currProcess->next->timeArrived &&
                    strcmp(node->processName, currProcess->next->processName) < 0) {
                break;
            }
            currProcess = currProcess->next;
        }

        // Insert in queue
        node->next = currProcess->next;
        currProcess->next = node;
    }

    q->size++;
}

// Remove first element from queue
process_t* pop(queue_t *queue) {
    if (queue->head == NULL) {
        return NULL;
    }

    // Move queue forward
    process_t *process = queue->head;
    queue->head = queue->head->next;
    
    queue->size--;
    process->next = NULL; 
    return process;
}


// Print queue
void printQueue(queue_t *q) {
    process_t* current = q->head;
    while (current != NULL) {
        printf("%d %s %d %d\n", current->timeArrived, current->processName, current->serviceTime, current->memoryRequirement);
        current = current->next;
    }
}

// Free queue memory
void freeQueue(queue_t *q) {
    process_t* current = q->head;

    while (current != NULL) {
        process_t* next = current->next;
        freeProcess(current);
        current = next;
    }

    free(q);
}