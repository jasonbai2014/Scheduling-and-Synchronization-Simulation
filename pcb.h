/**
* pcb.h
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
* This header file defines the class and methods for the process control block implementation
*
*/

#ifndef PCB_H
#define PCB_H
#define PCB_STR_LEN 50 // number of chars that a string can hold
#define MAX_PC 2345 // max value of a pc can be

// This defines an enum type for all conditions that a PCB can have
// Idle is only used for the PCB of Idle task
typedef enum {New, Ready, Running, Blocked, Halted, Interrupted, Idle, Terminated} State;
typedef enum {IO, Compute, ProducerConsumer, MutualResource} PCB_Type;

// This defines a PCB type
typedef struct {
   PCB_Type type; // this is for type of this pcb
   int pairID;  // this is for producer consumer or mutual resource users pair
   int origPriority; // original priority of this pcb, used for starvation prevention
   int curPriority; // current priority of this pcb, used for starvation prevention
   int promotedRuns; // number of runs this pcb can be promoted
   int starvationTime; // keep track of time this pcb stays in the head of a queue in the priority queue
   int *lockArray; // an array holding lock values
   int *unlockArray; // an array holding unlock values
   int wait; // a value for wait()
   int signal; // a value for signal()
   State curState; // shows current state of PCB
   int PID; // a process ID given to this PCB
   unsigned int pc; // a program counter of a process related to this PCB
   int sw; // state work of a process related to this PCB
   int creation; // creation time of a process
   int termination; // termination time of a process 
   int terminate; // a control field deciding when a process is terminated
   int termCount; // a counter keeping track of number of times that passes the MAX_PC value
   int *io_1_trap; // an array for io trap values
   int *io_2_trap; // another array for io trap values
} PCB;

typedef PCB *PCB_Ptr; // This defines a PCB pointer type

/**
 * This is a constructor of this PCB type
 * return a pointer of newly created PCB
 */
PCB_Ptr PCB_constructor(PCB_Type type);

/**
 * This is a deconstructor of this PCB type which frees memory used by a PCB
 * PCB_Ptr pcb is the PCB instance freed in this deconstructor
 */
void PCB_destructor(PCB_Ptr pcb);

/**
 * This is a setter for process ID
 * PCB_Ptr pcb is the PCB where you set new PID
 * int PID is a new PID
 */
void PCB_setProcessID(PCB_Ptr pcb, int PID);

/**
 * This is a getter for process ID
 * PCB_Ptr pcb is the PCB where you get a PID
 * return a PID of the PCB you pass in
 */
int PCB_getProcessID(PCB_Ptr pcb);

/**
 * This is a setter for priority
 * PCB_Ptr pcb is the PCB where you set new priority
 * int priority is a new priority
 */
// void PCB_setPriority(PCB_Ptr pcb, int priority);
// 
// /**
//  * This is a getter for priority
//  * PCB_Ptr pcb is the PCB where you get a priority
//  * return priority of the PCB you pass in
//  */
// int PCB_getPriority(PCB_Ptr pcb);

/**
 * This is a setter for current state
 * PCB_Ptr pcb is the PCB where you set new current state
 * State curState is a new curState
 */
void PCB_setCurrentState(PCB_Ptr pcb, State curState);

/**
 * This is a getter for current state
 * PCB_Ptr pcb is the PCB where you get current state
 * return current state of the PCB you pass in
 */
State PCB_getCurrentState(PCB_Ptr pcb);

/**
 * This is a setter for PC
 * PCB_Ptr pcb is the PCB where you set new PC value
 * unsigned int is a new PC value
 */
void PCB_setPC(PCB_Ptr pcb, unsigned int pc);

/**
 * This is a getter for PC
 * PCB_Ptr pcb is the PCB where you get current PC
 * return PC value stored in the PCB you pass in
 */
unsigned int PCB_getPC(PCB_Ptr pcb);

/**
 * This is a setter for state work
 * PCB_Ptr pcb is the PCB where you set new state work
 * int sw is a new state work value
 */
void PCB_setSW(PCB_Ptr pcb, int sw);

/**
 * This is a getter for state work
 * PCB_Ptr pcb is the PCB where you get current state work
 * return state work stored in the PCB you pass in
 */
int PCB_getSW(PCB_Ptr pcb);

/**
 * This returns a char pointer holding a string that shows contents in
 * the PCB you pass in
 */
char *PCB_toString(const PCB_Ptr pcb);

/*
* This is a setter for creation
* PCB_Ptr pcb is the PCB where you set creation time
* int creation is the creation time
*/
void PCB_setCreation(PCB_Ptr pcb, int creation);

/**
* This is a getter for creation
* PCB_Ptr pcb is the PCB where you get creation time
* return creation time stored in the PCB
*/
int PCB_getCreation(PCB_Ptr pcb);

/**
* This is a setter for termination 
* PCB_Ptr pcb is the PCB where you set termination time
* int termination is the termination time
*/
void PCB_setTermination(PCB_Ptr pcb, int termination);

/**
* This is a getter for termination
* PCB_Ptr pcb is the PCB where you get termination time
* return termination time stored in the PCB
*/
int PCB_getTermination(PCB_Ptr pcb);

/**
* This is a setter for terminate
* PCB_Ptr pcb is the PCB where you set terminate 
* int terminate is the value you want to set
*/
void PCB_setTerminate(PCB_Ptr pcb, int terminate);

/**
* This is a getter for terminate
* PCB_Ptr pcb is the PCB where you get terminate
* return terminate value stored in the PCB
*/
int PCB_getTerminate(PCB_Ptr pcb);

/**
* This is a getter for term count
* PCB_Ptr pcb is the PCB where you get term count
* return term count stored in the PCB
*/
int PCB_getTermCount(PCB_Ptr pcb);

/**
* This initializes the io_trap arrays in the given PCB.
*/
void PCB_setIoTraps(PCB_Ptr pcb);

/**
* This is a getter for io_trap 1 array
* PCB_Ptr pcb is the PCB where you get this array
* return the array in the PCB
*/
int *PCB_getIo_1_trap(PCB_Ptr pcb);

/**
* This is a getter for io_trap 2 array
* PCB_Ptr pcb is the PCB where you get this array
* return the array in the PCB
*/
int *PCB_getIo_2_trap(PCB_Ptr pcb);

/**
* a setter for original priority of this pcb
*/
void PCB_setOrigPriority(PCB_Ptr pcb, int priority);

/**
* returns original priority of this pcb
*/
int PCB_getOrigPriority(PCB_Ptr pcb);

/**
* a setter for current priority of this pcb
*/
void PCB_setCurPriority(PCB_Ptr pcb, int priority);

/**
* returns current priority of this pcb
*/
int PCB_getCurPriority(PCB_Ptr pcb);

/**
* a setter for pairID of this pcb
*/ 
void PCB_setPairID(PCB_Ptr pcb, int pairID);

/**
* returns pairID of this pcb
*/
int PCB_getPairID(PCB_Ptr pcb);

/**
* a setter for promotedRuns of this pcb
*/
void PCB_setPromotedRuns(PCB_Ptr pcb, int promotedRuns);

/**
* returns promotedRuns of this pcb
*/
int PCB_getPromotedRuns(PCB_Ptr pcb);

/**
* returns type of this pcb
*/
PCB_Type PCB_getType(PCB_Ptr pcb);

/**
* set values for locks, unlocks, wait, and signal
*/
void PCB_setSynData(PCB_Ptr pcb, int lock0, int lock1, int unlock0, int unlock1, int wait, int signal);
#endif