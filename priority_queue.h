/***********************************************************************************************
* priority_queue.h
*
* Programming Team:
* Qing Bai
* Levi Bingham
* Collin Alligood
* Sally Budack
*
* Date: 1/27/16
*
* Description:
* This header file defines the class and methods for the priority queue implementation
*
************************************************************************************************/

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include "pcb.h"
#include "queue.h"

#define QUEUE_DEST_LEN 1500
#define QUEUE_SRC_LEN 6
#define SIZE 4 // number of queues in this priority queue
#define STARVATION_TIME 1200 // a pcb will stay at head of a queue no longer than 1200 preventStarvation() calls

typedef struct {
   Queue_Ptr queueArray[SIZE];
} PriorityQueue;

typedef PriorityQueue *PriorityQueue_Ptr;

/**
* creates a priority queue and returns pointer of the queue
*/
PriorityQueue_Ptr PriorityQueue_constructor(void);

/**
* destructs the passed in priority queue
*/
void PriorityQueue_destructor(PriorityQueue_Ptr priorityQueue);

/**
* adds one pcb into this priority queue
*/
void PriorityQueue_enqueue(PriorityQueue_Ptr priorityQueue, PCB_Ptr pcb);

/**
* gets one pcb from this priority queue
*/
PCB_Ptr PriorityQueue_dequeue(PriorityQueue_Ptr priorityQueue);

/**
* checks if this priority queue is empty
*/
int PriorityQueue_isEmpty(PriorityQueue_Ptr priorityQueue);

/**
* returns size of this priority queue
*/ 
int PriorityQueue_size(PriorityQueue_Ptr priorityQueue);

/**
* returns contents in this priority queue as a string
*/
char *PriorityQueue_toString(PriorityQueue_Ptr priorityQueue);

/**
* used to prevent pcbs in this priority from starvation
*/
void PriorityQueue_preventStarvation(PriorityQueue_Ptr priorityQueue);

#endif