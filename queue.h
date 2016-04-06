/***********************************************************************************************
* queue.h
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
* This header file defines the class and methods for the queue implementation
*
************************************************************************************************/


#ifndef QUEUE_H
#define QUEUE_H
#include "pcb.h"
// Both constants are used in Queue_toString() for length of a string
#define DEST_LEN 600
#define SRC_LEN 8

/*
* Node definition for the linked list implementation. Contains a pointer to a PCB object
*/
typedef struct {
  struct node *head;
  struct node *tail;
  unsigned int size;
} Queue;

typedef Queue *Queue_Ptr;

/*
* Constructs an empty queue object
*/
Queue_Ptr Queue_constructor(void);

/*
* Creates a new node and makes that node point to the pcb that is passed in.
* Then that node is appended to the tail of the queue. Queue size is incrimented by 1.
*/
void Queue_enqueue(Queue_Ptr queue, PCB_Ptr pcb);

/*
* If queue is empty, this function will return NULL.
* A node pointer is created and pointed at the head of the queue. 
* If there was only one node in the queue, the head and tail will
* be set to null. Otherwise, the next node is set to be the head of the queue.
* Then a pcb pointer is created to poiint at the pcb that the dequeued node was
* pointing at. The node is freed from memory, and the pointer to the pcb is returned.
*/
PCB_Ptr Queue_dequeue(Queue_Ptr queue);

/*
* Returns true if the Queue has no nodes in it, false otherwise.
*/
int Queue_isEmpty(Queue_Ptr queue);

/*
* Returns the PCB pointed to by the head node in the queue.
*/
PCB_Ptr Queue_peek(Queue_Ptr queue);

/*
* Returns the size of the queue.
*/
int Queue_size(Queue_Ptr queue);

/*
* Loops through each node, and frees the data from memory, destroying the Queue.
*/
void Queue_destructor(Queue_Ptr queue);

/*
* Prints out the processID of the pcb at each node in the queue.
*/
char *Queue_toString(const Queue_Ptr queue);

#endif
