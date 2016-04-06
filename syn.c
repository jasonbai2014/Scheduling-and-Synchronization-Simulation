#include <stdlib.h>
#include "pcb.h"
#include "queue.h"
#include "syn.h"


Mutex_Ptr Mutex_constructor() {
    Mutex_Ptr mutex = malloc(sizeof(Mutex));
    mutex->curPCB = NULL;
    mutex->waitingQueue = Queue_constructor();
    mutex->inUse = 0;
    return mutex;
}

void Mutex_deconstructor(Mutex_Ptr mutex) {
   Queue_destructor(mutex->waitingQueue);
   free(mutex);
}

int Mutex_lock(Mutex_Ptr mutex, PCB_Ptr pcb) {
   if (mutex->inUse == 1) {
      PCB_setCurrentState(pcb, Blocked);
      Queue_enqueue(mutex->waitingQueue, pcb);
      return 0;
   } else {
      mutex->inUse = 1;
      mutex->curPCB = pcb;
      return 1;
   }
}

PCB_Ptr Mutex_unlock(Mutex_Ptr mutex) {
   mutex->curPCB = Queue_dequeue(mutex->waitingQueue);
   
   if (mutex->curPCB == NULL) {
      mutex->inUse = 0;
   } else {
      mutex->inUse = 1;
   }
   
   return mutex->curPCB;
}

CondVar_Ptr CondVar_constructor() {
    CondVar_Ptr condVar = malloc(sizeof(CondVar));   
    condVar->head = NULL;
    condVar->tail = NULL;
    condVar->size = 0;
    return condVar;
}

void CondVar_deconstructor(CondVar_Ptr condVar) {
   CondVarNode_Ptr node;
   
   while (condVar->size) {
       node = condVar->head;
       condVar->head = node->next;
       condVar->size--;
       free(node);
   }
   
   free(condVar);
}

CondVarNode_Ptr CondVarNode_constructor(Mutex_Ptr mutex, PCB_Ptr pcb) {
   CondVarNode_Ptr node = malloc(sizeof(CondVarNode));
   node->thisPCB = pcb;
   node->thisMutex = mutex;
   node->next = NULL;
   return node;
}

void CondVar_wait(CondVar_Ptr condVar, Mutex_Ptr mutex) {
    PCB_setCurrentState(mutex->curPCB, Blocked);
    CondVarNode_Ptr condVarNode = CondVarNode_constructor(mutex, mutex->curPCB);
    
    if (condVar->size == 0) {
        condVar->head = condVarNode;
        condVar->tail = condVarNode;
    } else {
        condVar->tail->next = condVarNode;
        condVar->tail = condVarNode;
    }
  
    condVar->size++;
    Mutex_unlock(mutex);
}

void CondVar_signal(CondVar_Ptr condVar) { 
    if (condVar->size > 0) {
        CondVarNode_Ptr node = condVar->head;
        
        if (condVar->size == 1) {
            condVar->head = NULL;
            condVar->tail = NULL;
        } else {
            condVar->head = node->next;
        }
        
        condVar->size--;
        PCB_Ptr pcb = node->thisPCB;
        Mutex_Ptr mutex = node->thisMutex;
        free(node);
        Mutex_lock(mutex, pcb);
    }
}


