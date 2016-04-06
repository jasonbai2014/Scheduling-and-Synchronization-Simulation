#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pcb.h"
#include "queue.h"
#include "priority_queue.h"

/**
* this promotes pcbs at heads of priority 1, 2, and 3 queue to higher priority levels
*/
void promotePCB(PriorityQueue_Ptr priorityQueue, int origPriority);

PriorityQueue_Ptr PriorityQueue_constructor(void) {
   PriorityQueue_Ptr priorityQueue = malloc(sizeof(PriorityQueue));
   int i;
   
   for (i = 0; i < SIZE; i++) {
      priorityQueue->queueArray[i] = NULL;
   }
   
   return priorityQueue;
}

void promotePCB(PriorityQueue_Ptr priorityQueue, int origPriority) {
   Queue_Ptr origQueue = priorityQueue->queueArray[origPriority];
   PCB_Ptr pcb = Queue_dequeue(origQueue);
   
   if (PCB_getPromotedRuns(pcb) == 0) {
      // promoted pcb can run on cpu 6 times with a higher priority level
      PCB_setPromotedRuns(pcb, 6);
   }
   
   if (Queue_isEmpty(origQueue)) {
      Queue_destructor(origQueue);
      priorityQueue->queueArray[origPriority] = NULL;
   }

   Queue_Ptr curQueue = priorityQueue->queueArray[origPriority - 1];
   
   if (curQueue == NULL) {
      curQueue = Queue_constructor();
      priorityQueue->queueArray[origPriority - 1] = curQueue;
   }
   
   PCB_setCurPriority(pcb, origPriority - 1);
   Queue_enqueue(curQueue, pcb);
}

void PriorityQueue_preventStarvation(PriorityQueue_Ptr priorityQueue) {
   Queue_Ptr queue;
   PCB_Ptr pcb;
   int i;
   
   for (i = 1; i < SIZE; i++) {
      queue = priorityQueue->queueArray[i];
      
      if (queue != NULL) {
         pcb = Queue_peek(queue);
         // promotes a pcb from the head of queue when it reaches the starvation time
         if (pcb->starvationTime == STARVATION_TIME) {
            pcb->starvationTime = 0;
            promotePCB(priorityQueue, i);
         } else {
            pcb->starvationTime++;
         }
      }
   }
}

void PriorityQueue_destructor(PriorityQueue_Ptr priorityQueue) {
   int i;
   
   for (i = 0; i < SIZE; i++) {
      if (priorityQueue->queueArray[i] != NULL) {
         Queue_destructor(priorityQueue->queueArray[i]);
      }
   }
   
   free(priorityQueue);
}

int PriorityQueue_isEmpty(PriorityQueue_Ptr priorityQueue) {
   int i, result = 1;
   
   for (i = 0; i < SIZE; i++) {
      if (priorityQueue->queueArray[i] != NULL) {
         result = 0;
      }
   }
   
   return result;
}

int PriorityQueue_size(PriorityQueue_Ptr priorityQueue) {
   int i, size = 0;
   Queue_Ptr queue;
   
   for(i = 0; i < SIZE; i++) {
      queue = priorityQueue->queueArray[i];
      
      if (queue != NULL) {
         size += Queue_size(queue);
      }
   }
   
   return size;
}

void PriorityQueue_enqueue(PriorityQueue_Ptr priorityQueue, PCB_Ptr pcb) {
   int priority, numOfRuns = PCB_getPromotedRuns(pcb);
   
   if (numOfRuns > 0) {
      priority = PCB_getCurPriority(pcb);
      PCB_setPromotedRuns(pcb, numOfRuns - 1);
   } else {
      // demote the pcb back to its original priority level
      // when number of promoted runs becomes 0
      priority = PCB_getOrigPriority(pcb);
      PCB_setCurPriority(pcb, priority);
   }
   
   Queue_Ptr targetQueue = priorityQueue->queueArray[priority];
   
   if (targetQueue == NULL) {
      targetQueue = Queue_constructor();
      priorityQueue->queueArray[priority] = targetQueue;
   }
   
   Queue_enqueue(targetQueue, pcb);
}

PCB_Ptr PriorityQueue_dequeue(PriorityQueue_Ptr priorityQueue) {
   int i;
   Queue_Ptr q;
   
   for (i = 0; i < SIZE; i++) {
      q = priorityQueue->queueArray[i];
      
      if (q != NULL && !Queue_isEmpty(q)) {
         PCB_Ptr pcb = Queue_dequeue(q);
         
         if (Queue_isEmpty(q)) {
            Queue_destructor(q);
            priorityQueue->queueArray[i] = NULL;
         }
         
         pcb->starvationTime = 0;
         return pcb;
      }
   }
   
   return NULL;
}

char *PriorityQueue_toString(PriorityQueue_Ptr priorityQueue) {
   int i, size;
   Queue_Ptr q;
   char *dest = calloc(QUEUE_DEST_LEN, sizeof(char));
   char *src = calloc(QUEUE_SRC_LEN, sizeof(char));
   char *temp;
   
   for (i = 0; i < SIZE; i++) {
      q = priorityQueue->queueArray[i];
      
      if (q != NULL) {
         size = sprintf(src, "Q%d: ", i);
         strncat(dest, src, size);
         temp = Queue_toString(q);
         strncat(dest, temp, strlen(temp));
         strcat(dest, "\n");
      }
   }
   
   return dest;
}