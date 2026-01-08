// G8RTOS_Scheduler.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for scheduler functions

#include "../G8RTOS_Scheduler.h"

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../G8RTOS_CriticalSection.h"

#include <inc/hw_memmap.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

/************************************Includes***************************************/

/********************************Private Variables**********************************/

// Thread Control Blocks - array to hold information for each thread
static tcb_t tcbs[MAX_THREADS];

// Thread Stacks - array of arrays for individual stacks of each thread
static uint32_t threadStacks[MAX_THREADS][STACKSIZE];

// Periodic Event Threads - array to hold pertinent information for each thread
static ptcb_t pthreadControlBlocks[MAX_PTHREADS];

// Current Number of Threads currently in the scheduler
static uint32_t NumberOfThreads;

// Current Number of Periodic Threads currently in the scheduler
static uint32_t NumberOfPThreads;

static uint32_t threadCounter = 0;

ptcb_t *PThreadListHead;

uint32_t idleFlag;




/********************************Private Variables**********************************/

/*******************************Private Functions***********************************/

// Occurs every 1 ms.
static void InitSysTick(void)
{
    SysTickPeriodSet(SysCtlClockGet() / 1000);
    SysTickIntRegister(SysTick_Handler);
    IntRegister(FAULT_PENDSV, PendSV_Handler);
    SysTickIntEnable();
    SysTickEnable();
}

/*******************************Private Functions***********************************/


/********************************Public Variables***********************************/

uint32_t SystemTime;

tcb_t* CurrentlyRunningThread;

/********************************Public Variables***********************************/



/********************************Public Functions***********************************/

// SysTick_Handler
// Increments system time, sets PendSV flag to start scheduler.
// Return: void
void SysTick_Handler() {
    SystemTime++;

    // --- 1. Run Periodic Events ---
        // Iterate through all registered periodic threads
    if (NumberOfPThreads > 0) {
            ptcb_t *pt = PThreadListHead;
            for (uint32_t i = 0; i < NumberOfPThreads; i++) {
                if (SystemTime >= pt->executeTime) {
                    pt->handler();
                    pt->executeTime += pt->period;
                }
                pt = pt->nextPTCB;
            }
    }

        // --- 2. Wake Up Sleeping Threads ---
        for (uint32_t i = 0; i < MAX_THREADS; i++) {
            // Check if the thread is ALIVE and ASLEEP and its time is up
            if (tcbs[i].isAlive && tcbs[i].asleep && (SystemTime >= tcbs[i].sleepCount)) {
                tcbs[i].asleep = 0; // Wake up the thread
            }
        }

    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

// G8RTOS_Init
// Initializes the RTOS by initializing system time.
// Return: void
void G8RTOS_Init() {
    uint32_t newVTORTable = 0x20000000;
    uint32_t* newTable = (uint32_t*) newVTORTable;
    uint32_t* oldTable = (uint32_t*) 0;

    for (int i = 0; i < 155; i++) {
        newTable[i] = oldTable[i];
    }

    HWREG(NVIC_VTABLE) = newVTORTable;

    SystemTime = 0;
    NumberOfThreads = 0;
    NumberOfPThreads = 0;
}

// G8RTOS_Launch
// Launches the RTOS.
// Return: error codes, 0 if none
int32_t G8RTOS_Launch() {
    InitSysTick();

    CurrentlyRunningThread = &tcbs[0];

    IntPrioritySet(FAULT_SYSTICK, 0xE0);
    IntPrioritySet(FAULT_PENDSV, 0xE0);

    IntMasterEnable();

    G8RTOS_Start(); // call the assembly function
    return 0;
}

// G8RTOS_Scheduler
// Chooses next thread in the TCB. This time uses priority scheduling.
// Return: void
void G8RTOS_Scheduler(void) {
    uint32_t max = 255;
    tcb_t *pt;
    tcb_t *bestPt = NULL;

    idleFlag = 1; // assume idle unless we find a runnable thread

    pt = CurrentlyRunningThread->nextTCB;
    do {

        if ((pt->priority < max) && (pt->isAlive) && (pt->blocked == 0) && (pt->asleep == 0)) {
            max = pt->priority;
            bestPt = pt;
            idleFlag = 0; // found runnable thread
        }

        pt = pt->nextTCB;

    } while (pt != CurrentlyRunningThread->nextTCB);

    if (idleFlag) {
        // No thread ready — stay on current thread only if it’s the idle thread
        if (CurrentlyRunningThread->priority != 255) {
            // Find and switch to the idle thread
            tcb_t *search = CurrentlyRunningThread;
            do {
                search = search->nextTCB;
            } while ((search->priority != 255) && (search != CurrentlyRunningThread));

            CurrentlyRunningThread = search; // switch to idle thread
        }
        return; // remain idle until something wakes
//        CurrentlyRunningThread = &tcbs[0];
//        return;
    }

    // Otherwise, switch to the best thread
    CurrentlyRunningThread = bestPt;
}



// G8RTOS_AddThread
// Adds a thread. This is now in a critical section to support dynamic threads.
// It also now should initalize priority and account for live or dead threads.
// Param void* "threadToAdd": pointer to thread function address
// Param uint8_t "priority": priority from [0, 255].
// Param char* "name": character array containing the thread name.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name) {
    int32_t IBit_State = StartCriticalSection();

       if (NumberOfThreads >= MAX_THREADS) {
           EndCriticalSection(IBit_State);
           return 0; // return error
       }

       int32_t id = -1; // Use -1 as a "not found" flag
           for (int i = 0; i < MAX_THREADS; i++) {
               if (tcbs[i].isAlive == 0) {
                   id = i; // Found a slot!
                   break;
               }
           }

           if (id == -1) {
              EndCriticalSection(IBit_State);
              return 0;
           }



       tcbs[id].stackPointer = &threadStacks[id][STACKSIZE-16]; // thread stack pointer
       threadStacks[id][STACKSIZE-1] = 0x01000000; // Thumb bit
       threadStacks[id][STACKSIZE-2] = (uint32_t)threadToAdd;
       threadStacks[id][STACKSIZE-3] = 0x14141414; // R14
       threadStacks[id][STACKSIZE-4] = 0x12121212; // R12
       threadStacks[id][STACKSIZE-5] = 0x03030303; // R3
       threadStacks[id][STACKSIZE-6] = 0x02020202; // R2
       threadStacks[id][STACKSIZE-7] = 0x01010101; // R1
       threadStacks[id][STACKSIZE-8] = 0x00000000; // R0
       threadStacks[id][STACKSIZE-9] = 0x11111111; // R11
       threadStacks[id][STACKSIZE-10] = 0x10101010; // R10
       threadStacks[id][STACKSIZE-11] = 0x09090909; // R9
       threadStacks[id][STACKSIZE-12] = 0x08080808; // R8
       threadStacks[id][STACKSIZE-13] = 0x07070707; // R7
       threadStacks[id][STACKSIZE-14] = 0x06060606; // R6
       threadStacks[id][STACKSIZE-15] = 0x05050505; // R5
       threadStacks[id][STACKSIZE-16] = 0x04040404; // R4

       strncpy(tcbs[id].threadName, name, MAX_NAME_LENGTH - 1);
       tcbs[id].threadName[MAX_NAME_LENGTH - 1] = '\0';
       tcbs[id].priority = priority;
       tcbs[id].isAlive = true;
       tcbs[id].ThreadID = id;


       if (NumberOfThreads == 0) {
           tcbs[id].nextTCB = &tcbs[id];
           tcbs[id].previousTCB = &tcbs[id];
           CurrentlyRunningThread = &tcbs[id];
       }

       else {
           tcbs[id].nextTCB = CurrentlyRunningThread->nextTCB; // new -> first
           tcbs[id].previousTCB = CurrentlyRunningThread; // previous <- new
           CurrentlyRunningThread->nextTCB->previousTCB = &tcbs[id];
           CurrentlyRunningThread->nextTCB = &tcbs[id];
       }

       NumberOfThreads++;

       // init thread stack to hold a default thread register context

       EndCriticalSection(IBit_State);
       return 1; // successful
        
}

// G8RTOS_Add_APeriodicEvent
// Param void* "AthreadToAdd": pointer to thread function address
// Param uint8_t "priority": Priorit of aperiodic event, [1..6]
// Param int32_t "IRQn": Interrupt request number that references the vector table. [0..155].
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_APeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn) {
    int32_t IBit_State = StartCriticalSection();

    if (IRQn < 16 || IRQn > 155) {
        EndCriticalSection(IBit_State);
        return 0; // FAILED (INVALID_IRQN)
    }

    if (priority < 1 || priority > 6) {
        EndCriticalSection(IBit_State);
        return 0; // FAILED (INVALID_PRIORITY)
    }

    IntRegister(IRQn, AthreadToAdd); // add AthreadToAdd to vector table

    IntPrioritySet(IRQn, (priority << 5)); // set priority of interrupt

    IntEnable(IRQn);  // enable it!

    // 6. Exit Critical Section
    EndCriticalSection(IBit_State);
    return 1;
}

// G8RTOS_Add_PeriodicEvent
// Adds periodic threads to G8RTOS Scheduler
// Function will initialize a periodic event struct to represent event.
// The struct will be added to a linked list of periodic events
// Param void* "PThreadToAdd": void-void function for P thread handler
// Param uint32_t "period": period of P thread to add
// Param uint32_t "execution": When to execute the periodic thread
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_PeriodicEvent(void (*PThreadToAdd)(void), uint32_t period, uint32_t execution) {
    int32_t IBit_State = StartCriticalSection();

    if (NumberOfPThreads >= MAX_PTHREADS) {
            EndCriticalSection(IBit_State);
            return 0; // FAILED (MAX_PTHREADS_REACHED)
        }

    ptcb_t* newNode = &pthreadControlBlocks[NumberOfPThreads];
    newNode->handler = PThreadToAdd;
    newNode->period = period;

    newNode->executeTime = SystemTime + execution;
    newNode->currentTime = 0;

    if (NumberOfPThreads == 0) {
        PThreadListHead = newNode;
        newNode->nextPTCB = newNode;
        newNode->previousPTCB = newNode;

    }

    else {
        newNode->nextPTCB = PThreadListHead;
        newNode->previousPTCB = PThreadListHead->previousPTCB;

        PThreadListHead->previousPTCB->nextPTCB = newNode;
        PThreadListHead->previousPTCB = newNode;
    }

    // Increment the count of periodic threads
    NumberOfPThreads++;


    EndCriticalSection(IBit_State);
    return 1;
    
}

// G8RTOS_KillThread
// Param uint32_t "threadID": ID of thread to kill
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillThread(char* name) {
    int32_t IBit_State = StartCriticalSection();

    // 1. Check if we can kill any threads
    if (NumberOfThreads <= 1) {
        EndCriticalSection(IBit_State);
        return 0; // FAILED (CANNOT_KILL_LAST_THREAD)
    }

    // 2. Search for the thread by name
    tcb_t *threadToKill = NULL;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (tcbs[i].isAlive && strcmp(tcbs[i].threadName, name) == 0) {
            threadToKill = &tcbs[i];
            break;
        }
    }

    // 3. Check if thread was found
    if (threadToKill == NULL) {
        EndCriticalSection(IBit_State);
        return 0; // FAILED (THREAD_DOES_NOT_EXIST)
    }

    // 4. Boundary Condition: Check if thread is already dead
    if (threadToKill->isAlive == false) {
        EndCriticalSection(IBit_State);
        return 0; // FAILED (THREAD_ALREADY_DEAD)
    }

    // 5. Unlink the TCB from the circular linked list
    tcb_t *prevThread = threadToKill->previousTCB;
    tcb_t *nextThread = threadToKill->nextTCB;
    prevThread->nextTCB = nextThread;
    nextThread->previousTCB = prevThread;

    // 6. Update thread state and global count
    threadToKill->isAlive = false;
    NumberOfThreads--;

    // 7. Exit Critical Section
    EndCriticalSection(IBit_State);

    // 8. Trigger context switch if killing the currently running thread
    if (CurrentlyRunningThread == threadToKill) {
        HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    }

    return 1; // SUCCESS
}

// G8RTOS_KillSelf
// Kills currently running thread.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillSelf() {

    int32_t code = G8RTOS_KillThread(CurrentlyRunningThread->threadName);
    return code;
}

// sleep
// Puts current thread to sleep
// Param uint32_t "durationMS": how many systicks to sleep for
void sleep(uint32_t durationMS) {
    CurrentlyRunningThread->sleepCount = durationMS + SystemTime;
    CurrentlyRunningThread->asleep = 1;
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
}

// G8RTOS_GetThreadID
// Gets current thread ID.
// Return: threadID_t
threadID_t G8RTOS_GetThreadID(void) {
    return CurrentlyRunningThread->ThreadID;        //Returns the thread ID
}

// G8RTOS_GetNumberOfThreads
// Gets number of threads.
// Return: uint32_t
uint32_t G8RTOS_GetNumberOfThreads(void) {
    return NumberOfThreads;         //Returns the number of threads
}
/********************************Public Functions***********************************/
