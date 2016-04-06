#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcb.h"

/**
* This is a function for the io trap arrays. It 
* generates eight random numbers for them. io_trap is
* a temporary array holding these numbers, length is 
* the length of the array, seed is the seed used for
* random number generator.
*/
void fillArray(PCB_Ptr pcb, int *io_trap, int length, int seed);

/**
* This can copy numbers from source array to destination
* array. src is source array, dest is destination array,
* start is the index where this starts coping and end
* is the index (exclusive) where this ends coping.
*/
void copyArray(int *src, int *dest, int start, int end);

/**
* This is a helper function for PCB_setPC(). It's a setter
* for the term count.
*/
void PCB_setTermCount(PCB_Ptr pcb);

int max(int *array);

int min(int *array);

PCB_Ptr PCB_constructor(PCB_Type type) {
   PCB_Ptr pcb = malloc(sizeof(PCB));
   pcb->type = type;
   pcb->pairID = -1;
   pcb->origPriority = -1;
   pcb->curPriority = -1;
   pcb->promotedRuns = 0;
   pcb->starvationTime = 0;
   pcb->lockArray = calloc(2, sizeof(int));
   pcb->unlockArray = calloc(2, sizeof(int));
   pcb->lockArray[0] = -1;
   pcb->lockArray[1] = -1;
   pcb->unlockArray[0] = -1;
   pcb->unlockArray[1] = -1;
   pcb->wait = -1;
   pcb->signal = -1;
   pcb->curState = New;
   pcb->PID = 0;
   pcb->pc = 0;
   pcb->sw = 0;
   pcb->creation = -1;
   pcb->termination = -1;
   pcb->terminate = 0;
   pcb->termCount = 0;
   pcb->io_1_trap = calloc(4, sizeof(int));
   pcb->io_2_trap = calloc(4, sizeof(int)); 
   return pcb;
}

void PCB_setSynData(PCB_Ptr pcb, int lock0, int lock1, int unlock0, int unlock1, int wait, int signal) {
   pcb->lockArray[0] = lock0;
   pcb->lockArray[1] = lock1;
   pcb->unlockArray[0] = unlock0;
   pcb->unlockArray[1] = unlock1;
   pcb->wait = wait;
   pcb->signal = signal;
}

void PCB_setIoTraps(PCB_Ptr pcb) {
    int temp[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    
    // io trap arrays contain valid values only when
    // this is a IO, ProducerConsumer, or MutualResource type process
    if (pcb->type == IO || pcb->type == ProducerConsumer || pcb->type == MutualResource) {
        fillArray(pcb, temp, 8, pcb->PID);
    }
    
    copyArray(temp, pcb->io_1_trap, 0, 4);
    copyArray(temp, pcb->io_2_trap, 4, 8);
}

/**
* finds maxmium value in either lock or unlock array
*/
int max(int *array) {
   if (array[0] > array[1]) {
      return array[0];
   } else {
      return array[1];
   }
}

/**
* finds minimum value in either lock or unlock array
*/
int min(int *array) {
   if (array[0] < array[1]) {
      return array[0];
   } else {
      return array[1];
   }
}

void fillArray(PCB_Ptr pcb, int *io_trap, int length, int seed) {

    srand(time(NULL) + seed);
    int i, j, found, num;
    
    for (i = 0; i < length; i++) {
        // keep looping until find an unique number
        do {
            num = rand() % 1500 + 100;
            found = 0;
            
            if (pcb->type == MutualResource && (min(pcb->lockArray) <= num && num <= max(pcb->unlockArray))) {
               found = 1;
               continue;
            } else if (pcb->type != MutualResource && ((pcb->lockArray[0] <= num && num <= pcb->unlockArray[0]) 
               || (pcb->lockArray[1] <= num && num <= pcb->unlockArray[1]))) {
               found = 1;
               continue;
            }
            
            for (j = 0; j < i; j++) {
                if (io_trap[j] == num) {
                    found = 1;
                    break;
                }
            }
         } while (found == 1);
         
         io_trap[i] = num;
    }          
}

void copyArray(int *src, int *dest, int start, int end) {
   int i = 0;
   
   while (start < end) {
      dest[i] = src[start];
      start++;
      i++;
   }
}

void PCB_destructor(PCB_Ptr pcb) {
   free(pcb);
}

void PCB_setProcessID(PCB_Ptr pcb, int PID) {
   pcb->PID = PID;
}

int PCB_getProcessID(PCB_Ptr pcb) {
   return pcb->PID;
}

// void PCB_setPriority(PCB_Ptr pcb, int priority) {
//    pcb->priority = priority;
// }
// 
// int PCB_getPriority(PCB_Ptr pcb) {
//    return pcb->priority;
// }

void PCB_setCurrentState(PCB_Ptr pcb, State curState) {
   pcb->curState = curState;
}

State PCB_getCurrentState(PCB_Ptr pcb) {
   return pcb->curState;
}

void PCB_setPC(PCB_Ptr pcb, unsigned int pc) {
    if (pc < MAX_PC) {
        pcb->pc = pc;
    } else {
        pcb->pc = pc % MAX_PC;
        PCB_setTermCount(pcb);
    }  
}

unsigned int PCB_getPC(PCB_Ptr pcb) {
    return pcb->pc;
}

void PCB_setSW(PCB_Ptr pcb, int sw) {
    pcb->sw = sw;
}

int PCB_getSW(PCB_Ptr pcb) {
    return pcb->sw;
}

void PCB_setCreation(PCB_Ptr pcb, int creation) {
   pcb->creation = creation;
}

int PCB_getCreation(PCB_Ptr pcb) {
   return pcb->creation;
}

void PCB_setTermination(PCB_Ptr pcb, int termination) {
   pcb->termination = termination;
}

int PCB_getTermination(PCB_Ptr pcb) {
   return pcb->termination;
}

void PCB_setTerminate(PCB_Ptr pcb, int terminate) {
   pcb->terminate = terminate;
}

int PCB_getTerminate(PCB_Ptr pcb) {
   return pcb->terminate;
}

void PCB_setTermCount(PCB_Ptr pcb) {
   pcb->termCount++;
}

int PCB_getTermCount(PCB_Ptr pcb) {
   return pcb->termCount;
}

int *PCB_getIo_1_trap(PCB_Ptr pcb) {
   return pcb->io_1_trap;
}

int *PCB_getIo_2_trap(PCB_Ptr pcb) {
   return pcb->io_2_trap;
}

void PCB_setOrigPriority(PCB_Ptr pcb, int priority) {
   pcb->origPriority = priority;
}

int PCB_getOrigPriority(PCB_Ptr pcb) {
   return pcb->origPriority;
}

void PCB_setCurPriority(PCB_Ptr pcb, int priority) {
   pcb->curPriority = priority;
}

int PCB_getCurPriority(PCB_Ptr pcb) {
   return pcb->curPriority;
}

void PCB_setPairID(PCB_Ptr pcb, int pairID) {
   pcb->pairID = pairID;
}

int PCB_getPairID(PCB_Ptr pcb) {
   return pcb->pairID;
}

void PCB_setPromotedRuns(PCB_Ptr pcb, int promotedRuns) {
   pcb->promotedRuns = promotedRuns;
}

int PCB_getPromotedRuns(PCB_Ptr pcb) {
   return pcb->promotedRuns;
}

PCB_Type PCB_getType(PCB_Ptr pcb) {
   return pcb->type;
}

char *PCB_toString(const PCB_Ptr pcb) {
   char *str = calloc(PCB_STR_LEN, sizeof(char));
   const char *states[] = {"New", "Ready", "Running", "Blocked", "Halted", "Interrupted", "Idle", "Terminated"};
   const char *types[] = {"IO", "Compute", "ProducerConsumer", "MutualResource"};
   sprintf(str, "PID: %d, Priority: %d, State: %s, PC: %d, SW: %d, Terminatate: %d, Type: %s", pcb->PID, pcb->curPriority, 
   states[pcb->curState], pcb->pc, pcb->sw, pcb->terminate, types[pcb->type]);
   return str;
}