#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "pcb.h"

typedef struct node {
  struct node *next;
  PCB_Ptr thisPCB;
} Node;

Node *nodeConstructor(PCB_Ptr pcb);

Node *nodeConstructor(PCB_Ptr pcb) {
  Node *n = malloc(sizeof(Node));
  n->next = NULL;
  n->thisPCB = pcb;
  return n;
}

Queue_Ptr Queue_constructor(void) {
  Queue *q = malloc(sizeof(Queue));
  q->head = NULL;
  q->tail = NULL;
  q->size = 0;
  return q;
}

void Queue_enqueue(Queue_Ptr queue, PCB_Ptr pcb) {
  Node *n = nodeConstructor(pcb);

  if (!queue->size) {
    queue->head = n;
    queue->tail = n;
    
  } else {
    queue->tail->next = n;
    queue->tail = n;
  }
  
  queue->size++;
}

PCB_Ptr Queue_dequeue(Queue_Ptr queue) {
  if (!queue->size) return NULL;

  Node *n = queue->head;

  if (queue->size == 1) {
    queue->head = NULL;
    queue->tail = NULL;
  } else {
    queue->head = n->next;
  }
  
  PCB_Ptr pcb = n->thisPCB;
  free(n);
  queue->size--;

  return pcb;
}

PCB_Ptr Queue_peek(Queue_Ptr queue) {
   return queue->head->thisPCB;
}

int Queue_size(Queue_Ptr queue) {
   return queue->size;
}

int Queue_isEmpty(Queue_Ptr queue) {
  return !queue->size;
}

void Queue_destructor(Queue_Ptr queue) {
  while (Queue_dequeue(queue) != NULL);
  free(queue);
}

char *Queue_toString(const Queue_Ptr queue) {
	Node *current = queue->head;
   int size;
   char *dest = calloc(DEST_LEN, sizeof(char));
   char *src = calloc(SRC_LEN, sizeof(char));
   
   if (current == NULL) {
      return "";
   }
   
	while(current != NULL){
      if (current->next == NULL) {
         size = sprintf(src, "P%d-*", PCB_getProcessID(current->thisPCB));
         strncat(dest, src, size);
      } else {
         size = sprintf(src, "P%d->", PCB_getProcessID(current->thisPCB));
         strncat(dest, src, size);
      }
      
		current = current->next;
	}
   
   return dest;
}