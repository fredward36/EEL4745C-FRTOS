// G8RTOS_Semaphores.c
// Date Created: 2023-07-25
// Date Updated: 2024-04-28
// Defines for semaphore functions

#include "../G8RTOS_Semaphores.h"

/************************************Includes***************************************/

#include "../G8RTOS_CriticalSection.h"
#include "../G8RTOS_Scheduler.h"

#include "inc/hw_types.h"
#include "inc/hw_nvic.h"

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/
// G8RTOS_InitSemaphore
// Initializes semaphore to a value.
// Param "s": Pointer to semaphore
// Param "value": Value to initialize semaphore to
// Return: void
void G8RTOS_InitSemaphore(semaphore_t* s, int32_t value) {
    int32_t i_bit = StartCriticalSection();
    *s = value;
    EndCriticalSection(i_bit);
    return;
}

// G8RTOS_WaitSemaphore
// Waits on the semaphore to become available, decrements value by 1.
// If the current resource is not available, block the current thread
// Param "s": Pointer to semaphore
// Return: void
void G8RTOS_WaitSemaphore(semaphore_t* s) {
    int32_t i_bit = StartCriticalSection();
    (*s)--;
    if ((*s) < NULL) {
        CurrentlyRunningThread->blocked = s;
        EndCriticalSection(i_bit);
        /* Run the thread switcher, i.e., no spin-locking! */
        HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    } else {
        EndCriticalSection(i_bit);
    }
    return;
}

// G8RTOS_SignalSemaphore
// Signals that the semaphore has been released by incrementing the value by 1.
// Unblocks all threads currently blocked on the semaphore.
// Param "s": Pointer to semaphore
// Return: void
void G8RTOS_SignalSemaphore(semaphore_t* s) {
    int32_t i_bit = StartCriticalSection();
    (*s)++;
    if ((*s) <= NULL) {
        tcb_t* ptr = (tcb_t*)CurrentlyRunningThread->nextTCB;
        while (ptr->blocked != s) ptr = ptr->nextTCB;
        ptr->blocked = NULL;
    }
    EndCriticalSection(i_bit);
    return;
}
