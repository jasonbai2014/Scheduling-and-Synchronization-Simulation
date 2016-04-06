/**
* cpu.c
*
* Programming Team:
* Collin Alligood
* Levi Bingham
* Qing Bai
* Sally Budack
*
* Date: 02/10/16
*
* Description:
* This file simulates how scheduling works in an OS.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pcb.h"
#include "queue.h"
#include "priority_queue.h"
#include "syn.h"

#define CYCLES 1000000 // number of cycles we are going to run
#define MAX_PROC 72 // this includes 4 pairs of PC_PCB, 4 pairs of MR_PCB, and 64 other types of PCBs 
//#define MAX_PROC_PER_RUN 16 // limit number of PCBs being created below 16 every time
#define REFILL_FREQUENCY 3 // the cycle for refilling the ready queue
//#define NEW_PCB_FREQUENCY 1000 // generate new PCBs every 1000 runs
#define TIMER_QUANTUM 300 // time quantum for cpu timer
#define PRI_ZERO 4 // number of priority 0 pcbs
#define PRI_ONE 56 // number of priority 1 pcbs
#define PRI_TWO 8 // number of priority 2 pcbs
#define PRI_THREE 4 // number of priority 3 pcbs
#define IO_PCB 40 // number of IO pcbs
#define COMPUTE_PCB 24 // number of compute pcbs
#define PC_PCB 4 // 4 pairs of producer consumer pcbs
#define MR_PCB 4 // 4 pair of mutual resource pcbs

//define types of interrupts/traps
typedef enum {Timer_interrupt, IO_completion_interrupt, IO_trap, Termination_trap, Lock_trap, Unlock_trap, Wait_trap, Signal_trap} Interrupt_Type;

//define a type for system stack
typedef struct {
    unsigned int pc; // program counter
    int sw; // state work
} SysStack;

typedef SysStack *SysStack_Ptr;

int nextPCB_ID = 1; 
int refillCounter = 0; // a counter for refilling the ready queue
int swRegister = 0; // state work register
unsigned int pcRegister = 0; // program counter register
PCB_Ptr curPCB; // current PCB
PCB_Ptr idleTask; // an idle task
SysStack_Ptr sysStack;
Queue_Ptr newQueue; // a queue holding all newly created PCBs
PriorityQueue_Ptr readyQueue; // a queue holding all PCBs that are in ready state
Queue_Ptr terminationQueue; // a queue holding PCBs that are going to be terminated
Queue_Ptr ioOneWaitQueue; // a wait queue for io device one
Queue_Ptr ioTwoWaitQueue; // a wait queue for io device two
unsigned int cpuTime; // a counter used for system time
int timerCounter; // cpu timer counter
int ioOneCounter; // io device one timer counter
int ioTwoCounter; // io device two timer counter
// 0 and 1 for 1st pair, 2 and 3 for 2nd pair, 4 and 5 for 3rd pair, 6 and 7 for 4th pair, even numbers are for producers
int pcPairID = 0; 
// pair ID for mutual resource users, 0 and 1 for 1st pair, 2 and 3 for 2nd pair, 4 and 5 for 3rd pair, 6 and 7 for 4th pair
// even numbers are for process A type
int mrPairID = 0;
Mutex_Ptr mutexArray[4]; // mutexes at index 0, 1, 2, 3 are for 1st, 2nd, 3rd, and 4th pair respectively, only for producer consumer pair
// only for mutual resource users, mutexes at index 0 and 1 for 1st pair, at index 2 and 3 for 2nd pair, at index 4 and 5 for 3rd pair
// at index 6 and 7 for 4th pair, mutexes at even indexes are resource 1 for each pair
Mutex_Ptr mrMutexArray[8]; 
CondVar_Ptr readCondVars[4]; // conditional variables at index 0, 1, 2, 3 are for 1st, 2nd, 3rd, and 4th pair respectively
CondVar_Ptr writeCondVars[4]; // conditional variables at index 0, 1, 2, 3 are for 1st, 2nd, 3rd, and 4th pair respectively
int shareIntArray[4]; // integers at index 0, 1, 2, 3 are for 1st, 2nd, 3rd, and 4th pair respectively
// flags for the four producer consumer pairs, 1 means the shared integer can be written, 0 means the shared integer can be read
int writableFlags[4]; 
// flags are initially -1, index 0 and 1 are for pair 0, 2 and 3 are for pair 1, 4 and 5 are for pair 2, 6 and 7 are for pair 3
// once deadlock is detected, corresponding indexes will be filled with PIDs.
int deadLockFlags[8];

/**
* Used to initialize a pcb with a given type and priority level
*/
PCB_Ptr initializePCB(PCB_Type type, int priority) {
    srand(time(NULL) + nextPCB_ID);
    PCB_Ptr pcb = PCB_constructor(type);
    PCB_setCreation(pcb, cpuTime);
    PCB_setProcessID(pcb, nextPCB_ID++);
    
    if (type == IO || type == Compute) {
       PCB_setTerminate(pcb, rand() % 15);
       PCB_setIoTraps(pcb);
    }
    
    PCB_setCurPriority(pcb, priority);
    PCB_setOrigPriority(pcb, priority);
    printf("Process created: PID %d at system time %d\n %s\n", nextPCB_ID - 1, cpuTime, PCB_toString(pcb));
    return pcb;
}

/**
* Generates all priority levels that are needed to create initial pcbs
*/ 
int *generatePriorities() {
    int *priorities = malloc(sizeof(int) * MAX_PROC);
    srand(time(NULL));
    int pri, num = 0, priZeroCounter = 0, priOneCounter = 0, priTwoCounter = 0, priThreeCounter = 0;
    
    while (num < MAX_PROC) {
       pri = rand() % MAX_PROC;
      
       if ((pri >= 0 && pri <= 3) && priZeroCounter < PRI_ZERO) {
          priorities[num++] = 0;
          priZeroCounter++;
       } else if ((pri >= 4 && pri <= 59) && priOneCounter < PRI_ONE) {
          priorities[num++] = 1;
          priOneCounter++;
       } else if ((pri >= 60 && pri <= 67) && priTwoCounter < PRI_TWO) {
          priorities[num++] = 2;
          priTwoCounter++;
       } else if ((pri >= 68 && pri <= 71) && priThreeCounter < PRI_THREE) {
          priorities[num++] = 3;
          priThreeCounter++;
       }
    }
    
    return priorities;
}

/**
* Initializes the new queue with certain amount of each type of pcbs
*/
void initializeNewQueue() {
    srand(time(NULL));
    PCB_Ptr pcb;
    int *priorities = generatePriorities();
    int type, i, success = 0, ioCounter = 0, compCounter = 0, pcPairCounter = 0, mutPairCounter = 0;
    
    for (i = 0; i < MAX_PROC; i++) {
        if (priorities[i] == 0) {
            Queue_enqueue(newQueue, initializePCB(Compute, 0));  
        } else {
            while (!success) {
                type = rand() % 4;
               
                if (type == IO && ioCounter < IO_PCB) {
                    Queue_enqueue(newQueue, initializePCB(IO, priorities[i])); 
                    ioCounter++;
                    success = 1;
                } else if (type == Compute && compCounter < COMPUTE_PCB) {
                    Queue_enqueue(newQueue, initializePCB(Compute, priorities[i])); 
                    compCounter++;
                    success = 1;
                } else if (type == ProducerConsumer && pcPairCounter < PC_PCB && priorities[i] == 1) {
                    pcb = initializePCB(ProducerConsumer, priorities[i]);
                    PCB_setSynData(pcb, 350, 800, 450, 1000, 400, 900);
                    PCB_setIoTraps(pcb);
                    PCB_setPairID(pcb, pcPairID++);
                    Queue_enqueue(newQueue, pcb);
                     
                    pcb = initializePCB(ProducerConsumer, priorities[i]);
                    PCB_setSynData(pcb, 350, 800, 450, 1000, 400, 900);
                    PCB_setIoTraps(pcb);
                    PCB_setPairID(pcb, pcPairID++);
                    Queue_enqueue(newQueue, pcb);

                    pcPairCounter++;
                    success = 1;
                } else if (type == MutualResource && mutPairCounter < MR_PCB && priorities[i] == 1) {
                    pcb = initializePCB(MutualResource, priorities[i]);
                    PCB_setSynData(pcb, 300, 500, 900, 700, -1, -1);
                    PCB_setIoTraps(pcb);
                    PCB_setPairID(pcb, mrPairID++);
                    Queue_enqueue(newQueue, pcb);
                     
                    pcb = initializePCB(MutualResource, priorities[i]);
                    // if want to change to deadlock mode, switch to PCB_setSynData(pcb, 600, 400, 1000, 800, -1, -1);
                    PCB_setSynData(pcb, 400, 600, 1000, 800, -1, -1);
                    PCB_setIoTraps(pcb);
                    PCB_setPairID(pcb, mrPairID++);
                    Queue_enqueue(newQueue, pcb);
                    
                    mutPairCounter++;
                    success = 1;
                }
            }
              
            success = 0;
        }
    }
}


/**
* This refilles the ready queue using PCBs in the new queue
*/
void refillReadyQueue() {
    PCB_Ptr readyPCB;

    while(!Queue_isEmpty(newQueue)) {
        readyPCB = Queue_dequeue(newQueue);
        PCB_setCurrentState(readyPCB, Ready);
        PriorityQueue_enqueue(readyQueue, readyPCB);
    }
}

/**
* Loads PC and SW values into the SysStack and get next process ready to run
*/
void dispatcher() {

    // if ready queue isn't empty, get a PCB from the head of the queue. Otherwise,
    // get idel task ready to run
    if (!PriorityQueue_isEmpty(readyQueue)) {
        curPCB = PriorityQueue_dequeue(readyQueue);
        PCB_setCurrentState(curPCB, Running);
    } else {
        curPCB = idleTask;
    }

    sysStack->pc = PCB_getPC(curPCB);
    sysStack->sw = PCB_getSW(curPCB);
}

/**
* Processes passed in interrupt/trap 
*/
void scheduler(Interrupt_Type type){
    PCB_Ptr pcb;
    
    if (type == Timer_interrupt && PCB_getCurrentState(curPCB) != Idle) {
        PCB_setCurrentState(curPCB, Ready);
        PriorityQueue_enqueue(readyQueue, curPCB);
    } else if (type == Termination_trap) {
        while (!Queue_isEmpty(terminationQueue)) {
            pcb = Queue_dequeue(terminationQueue);
            Queue_enqueue(newQueue, initializePCB(pcb->type, pcb->origPriority));
            PCB_destructor(pcb);            
        }
    } else if (type == IO_completion_interrupt) {
        return;
    } else if (type == Unlock_trap) {
        int pairID = PCB_getPairID(curPCB);
        PCB_Type type = PCB_getType(curPCB);
        Mutex_Ptr mutex;
   
        if (type == MutualResource) {
           // (pairID / 2) * 2 is not same as pairID here
           int mutexIndex = (pairID / 2) * 2 + locateResource(curPCB->unlockArray);
           mutex = mrMutexArray[mutexIndex];
        } else { // for producer consumer pair
           int mutexIndex = pairID / 2;
           mutex = mutexArray[mutexIndex];
        }

        PCB_setCurrentState(mutex->curPCB, Ready);
        PriorityQueue_enqueue(readyQueue, mutex->curPCB);
        return;
    } else if (type == Wait_trap) {
        Mutex_Ptr mutex = mutexArray[PCB_getPairID(curPCB) / 2];
        
        if (mutex->curPCB != NULL) {
            PCB_setCurrentState(mutex->curPCB, Ready);
            PriorityQueue_enqueue(readyQueue, mutex->curPCB);
        }
    }
    
    // refill the ready queue when interrupted PCB is for idle task or the counter reaches
    // the point that the ready queue needs to be refilled
    if (PCB_getCurrentState(curPCB) == Idle || refillCounter == REFILL_FREQUENCY) {
        refillReadyQueue();
        refillCounter = 0;
    } else if (refillCounter < REFILL_FREQUENCY) {
        refillCounter++;
    }

    dispatcher();
}

/**
* This is interrupt service routine for timer interrupt.
*/
void timerInterruptServiceRoutine(){
    int prePcbID = PCB_getProcessID(curPCB);
    if (PCB_getCurrentState(curPCB) != Idle) {
        PCB_setCurrentState(curPCB, Interrupted);
    }

    PCB_setPC(curPCB, sysStack->pc);
    PCB_setSW(curPCB, sysStack->sw);
    scheduler(Timer_interrupt);
    int curPcbID = PCB_getProcessID(curPCB);
    // simulate IRET here
    pcRegister = sysStack->pc;
    printf("Timer interrupt: PID %d was running, PID %d dispatched\n", prePcbID, curPcbID);
}

/**
* decrements a timer counter when it's greater than 0 and return 0 
* or reset the timer counter to cpu qutanum value and return 1
* and 1 means there is a timer interrupt.
*/
int timer() {
    if (timerCounter > 0) {
        timerCounter--;
        return 0;
    } else {
        timerCounter = TIMER_QUANTUM;
        return 1;
    }
}

/**
* This is interrupt service routine for I/O completion interrupt.
*/
void ioInterruptServiceRoutine(int deviceNum) {
    PCB_Ptr blockedPCB;
    
    if (deviceNum == 1) {
        blockedPCB = Queue_dequeue(ioOneWaitQueue);
        if (!Queue_isEmpty(ioOneWaitQueue)) {
            srand(time(NULL) + cpuTime);
            ioOneCounter = (rand() % 3 + 3) * TIMER_QUANTUM;
        }
    } else {
        blockedPCB = Queue_dequeue(ioTwoWaitQueue);
        if (!Queue_isEmpty(ioTwoWaitQueue)) {
            srand(time(NULL) + cpuTime);
            ioTwoCounter = (rand() % 3 + 3) * TIMER_QUANTUM;
        }
    }
    
    printf("I/O completion interrupt: PID %d is running, PID %d put in ready queue\n",
        PCB_getProcessID(curPCB), PCB_getProcessID(blockedPCB));
    PCB_setCurrentState(blockedPCB, Ready);
    PriorityQueue_enqueue(readyQueue, blockedPCB);
    scheduler(IO_completion_interrupt);
}

/**
* This is timer for I/O device 1, returns 1 when I/O completion 
* interrupt occurs. Otherwise, returns 0.
*/
int ioOneTimer() {
    
    if (ioOneCounter > 0) {
        ioOneCounter--;
        return 0;
    } else {
        if (!Queue_isEmpty(ioOneWaitQueue)) {
            return 1;
        } else {
            return 0;
        }
    }
}

/**
* This is timer for I/O device 2, returns 1 when I/O completion 
* interrupt occurs. Otherwise, returns 0.
*/
int ioTwoTimer() {

    if (ioTwoCounter > 0) {
        ioTwoCounter--;
        return 0;
    } else {
        if (!Queue_isEmpty(ioTwoWaitQueue)) {
            return 1;
        } else {
            return 0;
        }
    }
}

/**
* Compares pc with values in the trap array and return 1 if it needs
* io device 1, or 2 if it needs io device 2. otherwise, 0.
*/
int checkIOTrapDevice(int pc) {
    int i;
    int *io_1_trap = PCB_getIo_1_trap(curPCB);
    int *io_2_trap = PCB_getIo_2_trap(curPCB);
    
    for (i = 0; i < 4; i++) {
        if (io_1_trap[i] == pc) {
            return 1;
        }
    }
    
    for (i = 0; i < 4; i++) {
        if (io_2_trap[i] == pc) {
            return 2;
        }
    }

    return 0;
}

/*
* This processes I/O request trap for an I/O device with given device number.
*/ 
void ioTrapHandler(int deviceNum) {
    PCB_setCurrentState(curPCB, Blocked);
    PCB_setPC(curPCB, sysStack->pc);
    PCB_setSW(curPCB, sysStack->sw);
    int prePcbID = PCB_getProcessID(curPCB);
    if (deviceNum == 1) {
        Queue_enqueue(ioOneWaitQueue, curPCB);
        
        if (ioOneCounter == 0) {
            srand(time(NULL) + cpuTime);
            ioOneCounter = (rand() % 3 + 3) * TIMER_QUANTUM;
        }
    } else {
        Queue_enqueue(ioTwoWaitQueue, curPCB);
        
        if (ioTwoCounter == 0) {
            srand(time(NULL) + cpuTime);
            ioTwoCounter = (rand() % 3 + 3) * TIMER_QUANTUM;
        }
    }
    
    timerCounter = TIMER_QUANTUM;
    scheduler(IO_trap);
    int curPcbID = PCB_getProcessID(curPCB);
    pcRegister = sysStack->pc;
    printf("I/O trap request: I/O device %d, PID %d put into waiting queue, PID %d dispatched\n",
        deviceNum, prePcbID, curPcbID);
}

/**
* This is a trap handler for process termination trap
*/
void terminationTrapHandler() {
    PCB_setCurrentState(curPCB, Terminated);
    printf("Process terminated: PID %d at system time %d\n", PCB_getProcessID(curPCB), cpuTime);
    Queue_enqueue(terminationQueue, curPCB);
    scheduler(Termination_trap);
    pcRegister = sysStack->pc;
}

/**
* Used by mutual resource users to determine which resource should be used, return 0 when
* resource 1 is going to be used, return 1 when resource 2 is going to be used
*/
int locateResource(int *array) {
   if (pcRegister == array[0]) {
      return 0;
   } else {
      return 1;
   }
}

/**
* Print a message when no deadlock happens and both resources of a mutual resource user pair are used
*/
void printMutualResourceTrace(int *array) {
   int max;
   
   if (array[0] > array[1]) {
      max = array[0];
   } else {
      max = array[1];
   }
   
   if (max == pcRegister) {
      printf("both resources of mutual resource user pair %d are used\n", PCB_getPairID(curPCB) / 2);
   }
}

/**
* This is a handler for lock
*/
void lockTrapHandler() {
   int pairID = PCB_getPairID(curPCB);
   PCB_Type type = PCB_getType(curPCB);
   int mutexIndex;
   Mutex_Ptr mutex;
   
   if (type == MutualResource) {
      // (pairID / 2) * 2 is not same as pairID here
      mutexIndex = (pairID / 2) * 2 + locateResource(curPCB->lockArray);
      mutex = mrMutexArray[mutexIndex];
   } else { // for producer consumer pair
      mutexIndex = pairID / 2;
      mutex = mutexArray[mutexIndex];
   }
   
   int locked = Mutex_lock(mutex, curPCB);
   int processID = PCB_getProcessID(curPCB);

   if (!locked) {
      PCB_setPC(curPCB, pcRegister);
      scheduler(Lock_trap);
      pcRegister = sysStack->pc;
      
      if (type == MutualResource) {
         printf("PID %d: requested lock on mutual resource mutex %d - blocked by PID %d\n", processID, mutexIndex, PCB_getProcessID(mutex->curPCB));
      } else {
         printf("PID %d: requested lock on producer consumer mutex %d - blocked by PID %d\n", processID, mutexIndex, PCB_getProcessID(mutex->curPCB));
      } 
   } else {
      if (type == MutualResource) {
         printf("PID %d: requested lock on mutual resource mutex %d - succeeded\n", processID, mutexIndex);
         printMutualResourceTrace(curPCB->lockArray);
      } else {
         printf("PID %d: requested lock on producer consumer mutex %d - succeeded\n", processID, mutexIndex);
      }
   }
}

/**
* This is a handler for unlock
*/
void unlockTrapHandler() {
   int pairID = PCB_getPairID(curPCB);
   PCB_Type type = PCB_getType(curPCB);
   int mutexIndex;
   Mutex_Ptr mutex;
   
   if (type == MutualResource) {
      // (pairID / 2) * 2 is not same as pairID here
      mutexIndex = (pairID / 2) * 2 + locateResource(curPCB->unlockArray);
      mutex = mrMutexArray[mutexIndex];
   } else { // for producer consumer pair
      mutexIndex = pairID / 2;
      mutex = mutexArray[mutexIndex];
   }
   
   PCB_Ptr waitingPCB = Mutex_unlock(mutex);
   int processID = PCB_getProcessID(curPCB);
   
   if (waitingPCB != NULL) {
      scheduler(Unlock_trap);
   }
   
   if (type == MutualResource) {
      printf("PID %d: requested unlock on mutual resource mutex %d\n", processID, mutexIndex);
   } else {
      printf("PID %d: requested unlock on producer consumer mutex %d\n", processID, mutexIndex);
   }
}

/**
* This is a handler for wait
*/
void waitTrapHandler() {
   int pairID = PCB_getPairID(curPCB);
   int processID = PCB_getProcessID(curPCB);
   Mutex_Ptr mutex = mutexArray[pairID / 2];
   CondVar_Ptr condVar;
   // producer ID is an even number, consumer ID is an odd number
   if (pairID % 2 == 0) {
      condVar = writeCondVars[pairID / 2];
      printf("PID %d requested condition wait on cond_write %d with mutex %d\n", processID, pairID / 2, pairID / 2);
   } else {
      condVar = readCondVars[pairID / 2];
      printf("PID %d requested condition wait on cond_read %d with mutex %d\n", processID, pairID / 2, pairID / 2);
   }
   
   PCB_setPC(curPCB, pcRegister);
   CondVar_wait(condVar, mutex);
   scheduler(Wait_trap);
   pcRegister = sysStack->pc;
}

/**
* This is a handler for signal
*/
void signalTrapHandler() {
   int pairID = PCB_getPairID(curPCB);
   int processID = PCB_getProcessID(curPCB);
   CondVar_Ptr condVar;
   // producer ID is an even number, consumer ID is an odd number
   if (pairID % 2 == 0) {
      condVar = readCondVars[pairID / 2];
      printf("PID %d sent signal on cond_read %d\n", processID, pairID /2);
   } else {
      condVar = writeCondVars[pairID / 2];
      printf("PID %d sent signal on cond_write %d\n", processID, pairID / 2);
   }
   
   CondVar_signal(condVar);
}

/**
* Used to determine whether or not a synchronizing trap handler should be called
*/
int synchronize(int pc) {
   int i, found = 0;
   
   for (i = 0; i < 2; i++) {
      if (curPCB->lockArray[i] == pc) {
         found = 1;
         lockTrapHandler();
      }  
   }
   
   if (!found) {
      for (i = 0; i < 2; i++) {
         if (curPCB->unlockArray[i] == pc) {
            found = 1;
            unlockTrapHandler();
         }
      }
   }
   
   if (!found && curPCB->wait == pc) {
       found = 1;
       int pairID = PCB_getPairID(curPCB);
       // producer ID is an even number, consumer ID is an odd number
       if ((pairID % 2 == 0 && writableFlags[pairID / 2] == 0) || 
         (pairID % 2 == 1 && writableFlags[pairID / 2] == 1)) {
           waitTrapHandler();
       }   
   }
   
   if (!found && curPCB->signal == pc) {
       found = 1;
       int pairID = PCB_getPairID(curPCB);
       int index = pairID / 2;
       
       if (pairID % 2 == 0) {
           shareIntArray[index]++;
           printf("Producer of pair %d wrote %d to the share space %d\n", index, shareIntArray[index], index);
           writableFlags[index] = 0;
       } else {
           printf("Consumer of pair %d read %d from the share space %d\n", index, shareIntArray[index], index);
           writableFlags[index] = 1;
       }
       
       signalTrapHandler();
   }
   
   return found;
}

/**
* Checks whether or not deadlock has happened
*/
void deadLockMonitor() {
    int i;
    Mutex_Ptr mutexOne, mutexTwo;
    PCB_Ptr pcbOne, pcbTwo;
    
    for (i = 0; i < 4; i++) {
        mutexOne = mrMutexArray[2 * i];
        mutexTwo = mrMutexArray[2 * i + 1];
        pcbOne = mutexOne->curPCB;
        pcbTwo = mutexTwo->curPCB;
        
        if ((pcbOne != NULL && pcbTwo != NULL) && pcbOne != pcbTwo) {
            deadLockFlags[2 * i] = PCB_getProcessID(pcbOne);
            deadLockFlags[2 * i + 1] = PCB_getProcessID(pcbTwo);
        } 
    }
}

/**
* Prints out statistics for this simulation
*/
void stats() {
    printf("\nSimulation summary\n\n");
    int i, flagOne, flagTwo, hasDeadLock = 0;
    
    for (i = 0; i < 4; i++) {
        flagOne = deadLockFlags[2 * i];
        flagTwo = deadLockFlags[2 * i + 1];
        
        if (flagOne != -1) {
            printf("deadlock detected for processes PID %d and PID %d\n", flagOne, flagTwo);
            hasDeadLock = 1;
        }
    }
    
    if (!hasDeadLock) {
        printf("no deadlock detected\n");
    }
    
    printf("Total number of processes run: %d\n", nextPCB_ID);
    printf("%d processes in new queue\n", Queue_size(newQueue));
    printf("%d processes in ready queue\n", PriorityQueue_size(readyQueue));
    printf("%d processes in termination queue\n", Queue_size(terminationQueue));
    printf("%d processes in IO waiting queue #1\n", Queue_size(ioOneWaitQueue));
    printf("%d processes in IO waiting queue #2\n", Queue_size(ioTwoWaitQueue));       
}

/**
* This initializes some variables that are used in this program.
*/
void initialize() {
    timerCounter = TIMER_QUANTUM;
    // start this program with an idle task
    idleTask = PCB_constructor(Compute);
    PCB_setCurrentState(idleTask, Idle);
    curPCB = idleTask;
    pcRegister = PCB_getPC(curPCB);
    swRegister = PCB_getSW(curPCB);

    sysStack = malloc(sizeof(SysStack));
    newQueue = Queue_constructor();
    initializeNewQueue();
    readyQueue = PriorityQueue_constructor();
    terminationQueue = Queue_constructor();
    ioOneWaitQueue = Queue_constructor();
    ioTwoWaitQueue = Queue_constructor();
    
    int i;
    
    for (i = 0; i < 4; i++) {
        mutexArray[i] = Mutex_constructor();
        readCondVars[i] = CondVar_constructor();
        writeCondVars[i] = CondVar_constructor();
        shareIntArray[i] = 0;
        writableFlags[i] = 1;
    }
    
    for (i = 0; i < 8; i++) {
        mrMutexArray[i] = Mutex_constructor();
        deadLockFlags[i] = -1;
    }
}

/**
* free the memory used by the data structures in this program
*/
void finalize() {
    Queue_destructor(newQueue);
    PriorityQueue_destructor(readyQueue);
    Queue_destructor(terminationQueue);
    Queue_destructor(ioOneWaitQueue);
    Queue_destructor(ioTwoWaitQueue);
    
    // avoid to free the same thing twice
    if (curPCB == idleTask) {
        free(curPCB);
    } else {
        free(curPCB);
        free(idleTask);
    }
    
    free(sysStack);
    int i;
    
    for (i = 0; i < 4; i++) {
        Mutex_deconstructor(mutexArray[i]);
        CondVar_deconstructor(readCondVars[i]);
        CondVar_deconstructor(writeCondVars[i]);
    }
    
    for (i = 0; i < 8; i++) {
        Mutex_deconstructor(mrMutexArray[i]);
    }
}

/**
* This main simulates CPU.
*/
int main(void) {
    int isIOOneCompleted, isIOTwoCompleted, deviceNum;
    initialize();

    for (cpuTime = 0; cpuTime < CYCLES; cpuTime++) {
   
        pcRegister += 1;
        
        PCB_Type type = PCB_getType(curPCB);
        // for synchronization
        if ((type == ProducerConsumer || type == MutualResource) && synchronize(pcRegister)) continue;
        
        // for timer interrupt
        if (timer()) {
            sysStack->pc = pcRegister;
            sysStack->sw = swRegister;
            timerInterruptServiceRoutine();
            continue;
        }
        
        // for I/O completion interrupt
        isIOOneCompleted = ioOneTimer();
        isIOTwoCompleted = ioTwoTimer();
        if (isIOOneCompleted) {
            ioInterruptServiceRoutine(1);
        }
        
        if (isIOTwoCompleted) {
            ioInterruptServiceRoutine(2);
        }

        // for io request trap
        if (PCB_getCurrentState(curPCB) != Idle) {
            deviceNum = checkIOTrapDevice(pcRegister);
            
            if (deviceNum > 0) {
                sysStack->pc = pcRegister;
                sysStack->sw = swRegister;
                ioTrapHandler(deviceNum);
                continue;
            }
        }
        
        // for process termination trap
        if ((PCB_getTermCount(curPCB) + 1 == PCB_getTerminate(curPCB)) && (pcRegister == MAX_PC)) {
            terminationTrapHandler();
        }
        
        PriorityQueue_preventStarvation(readyQueue);
        
        if (cpuTime % 2000 == 0) {
            deadLockMonitor();
        }
    }
    
    stats();
    finalize(); 
    return 0;
}
