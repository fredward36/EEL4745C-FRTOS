// G8RTOS_Scheduler.h
// Date Created: 2023-07-26
// Date Updated: 2024-04-08
// Scheduler / initialization code for G8RTOS

#ifndef G8RTOS_SCHEDULER_H_
#define G8RTOS_SCHEDULER_H_

/************************************Includes***************************************/

#include <stdint.h>

#include "G8RTOS_Structures.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT            0x01000000

#define MAX_THREADS         24 // Adjust accordingly
#define MAX_PTHREADS        6
#define STACKSIZE           275 // Adjust accordingly
#define OSINT_PRIORITY      7
#define NULL                0
#define nullptr             NULL;

/*************************************Defines***************************************/

/******************************Data Type Definitions********************************/

// Scheduler error typedef
typedef enum
{
    NO_ERROR = 0,
    THREAD_LIMIT_REACHED = -1,
    NO_THREADS_SCHEDULED = -2,
    THREADS_INCORRECTLY_ALIVE = -3,
    THREAD_DOES_NOT_EXIST = -4,
    CANNOT_KILL_LAST_THREAD = -5,
    IRQn_INVALID = -6,
    HWI_PRIORITY_INVALID = -7
} sched_ErrCode_t;

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/
/****************************Data Structure Definitions*****************************/

/********************************Public Variables***********************************/

extern tcb_t* CurrentlyRunningThread;

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

extern void G8RTOS_Start(void);
extern void PendSV_Handler(void);

void SysTick_Handler(void);

void G8RTOS_Init(void);
int32_t G8RTOS_Launch(void);
void G8RTOS_Scheduler(void);

void SetInitialStack(unsigned int index);
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name, uint8_t threadID);
sched_ErrCode_t G8RTOS_Add_APeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn);
sched_ErrCode_t G8RTOS_Add_PeriodicEvent(void (*PthreadToAdd)(void), uint32_t period, uint32_t execution);
sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID);
sched_ErrCode_t G8RTOS_KillSelf(void);

void sleep(uint32_t durationMS);

threadID_t G8RTOS_GetThreadID(void);
uint32_t G8RTOS_GetNumberOfThreads(void);
uint32_t G8RTOS_GetSysTime(void);

/********************************Public Functions***********************************/


#endif /* G8RTOS_SCHEDULER_H_ */

