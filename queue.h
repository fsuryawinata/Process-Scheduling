#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>
#include <stdio.h>
#include "data.h"

typedef struct {
    process_t *head;
    process_t *tail;
    int size;
} queue_t;

// Creates new queue
queue_t* createQueue();

// Add process node into queue
void push(queue_t *q, process_t *node);

// Adds process into priority queue sorted by SJF
void SJFpush(queue_t *q, process_t *node);

// Remove first element from queue
process_t* pop(queue_t *q);

// Print queue
void printQueue(queue_t *q);

// Free queue memory
void freeQueue(queue_t *q);

#endif