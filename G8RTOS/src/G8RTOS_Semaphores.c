// G8RTOS_Semaphores.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for semaphore functions

#include "../G8RTOS_Semaphores.h"

/************************************Includes***************************************/

#include "../G8RTOS_CriticalSection.h"
#include "../G8RTOS_Scheduler.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
/************************************Includes***************************************/

/******************************Data Type Definitions********************************/
/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/
/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Variables***********************************/


/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

void G8RTOS_InitSemaphore(semaphore_t* s, int32_t value) {
    int32_t IBit_State = StartCriticalSection();

    *s = value;
    EndCriticalSection(IBit_State);
}

void G8RTOS_WaitSemaphore(semaphore_t* s) {
    int32_t IBit_State = StartCriticalSection();
    (*s)--;
    if (*s < 0) {
    CurrentlyRunningThread->blocked = s;
    EndCriticalSection(IBit_State);

    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    }

    else {
        EndCriticalSection(IBit_State);
    }



}



void G8RTOS_SignalSemaphore(semaphore_t* s) {
   int32_t IBit_State = StartCriticalSection();
   (*s)++;

   if ((*s) <= 0) {
       tcb_t *pt = CurrentlyRunningThread->nextTCB;
       while(pt->blocked != s) {
           pt = pt->nextTCB;
       }
       pt->blocked = 0;
   }

   EndCriticalSection(IBit_State);
}

/********************************Public Functions***********************************/
