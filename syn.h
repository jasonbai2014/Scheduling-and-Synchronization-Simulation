/**
* syn.h
*
* Programming Team:
* Collin Alligood
* Levi Bingham
* Qing Bai
* Sally Budack
*
* Date: 02/27/16
*
* Description:
* This header file defines the class and methods for the synchronization tools implementation
*
*/
#ifndef SYN_H
#define SYN_H
#include "pcb.h"
#include "queue.h"

/*
* This struct defines a Mutex type
*/
typedef struct {
    PCB_Ptr curPCB;
    Queue_Ptr waitingQueue;
    int inUse; // 1 is in use, 0 is not
} Mutex;

/*
* This defines a Mutex type
*/
typedef Mutex *Mutex_Ptr;

/**
* This defines a type of node in condition variable
*/
typedef struct condvarnode {
    struct condvarnode *next;
    PCB_Ptr thisPCB;
    Mutex_Ptr thisMutex;
} CondVarNode;

typedef CondVarNode *CondVarNode_Ptr;

/*
* This struct defines a Condition Variable type
*/
typedef struct {
    CondVarNode_Ptr head;
    CondVarNode_Ptr tail;
    unsigned int size; 
} CondVar;

/*
* This defines a Condition Variable type
*/
typedef CondVar *CondVar_Ptr;

/*
* This Constructs a Mutex object, and returns a pointer to it.
*/
Mutex_Ptr Mutex_constructor();

/*
* This destroys a Mutex object and frees the memory it was using.
*/
void Mutex_deconstructor(Mutex_Ptr mutex);

/*
* This Constructs a Condition Variable object, and returns a pointer to it.
*/
CondVar_Ptr CondVar_constructor();

/*
* This destroys a Condition Variable object and frees the memory it was using.
*/
void CondVar_deconstructor(CondVar_Ptr condVar);

/*
* This frees the Mutex Lock and puts the PCB into this Condition Variable's 
* waiting queue to be signaled later.
*/
void CondVar_wait(CondVar_Ptr condVar, Mutex_Ptr mutexLock);

/*
* This removes the PCB at the head of this Condition Variable's waiting queue.
*/
void CondVar_signal(CondVar_Ptr condVar);


/*
* This puts a lock on the mutex and sets the inUse variable to 1.
* If the mutex is already in use, the PCB will be sent to the mutex's
* waiting queue. 
*/
int Mutex_lock(Mutex_Ptr mutex, PCB_Ptr pcb);


/*
* This removes the PCB at the head of this Mutex's waiting queue.
* That PCB then becomes the current user of this Mutex, and the PCB
* is returned.
*/
PCB_Ptr Mutex_unlock(Mutex_Ptr mutex);

#endif