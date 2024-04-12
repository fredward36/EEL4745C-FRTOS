// G8RTOS_Scheduler.c
// Date Created: 2023-07-25
// Date Updated: 2024-04-08
// Defines for scheduler functions

#include "../G8RTOS_Scheduler.h"

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

#include "../G8RTOS_CriticalSection.h"

#include <inc/hw_memmap.h>
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

/********************************Private Variables**********************************/

// Thread Control Blocks - array to hold information for each thread
static tcb_t threadControlBlocks[MAX_THREADS];

// Thread Stacks - array of arrays for individual stacks of each thread
static uint32_t threadStacks[MAX_THREADS][STACKSIZE];

// Periodic Event Threads - array to hold pertinent information for each thread
static ptcb_t pthreadControlBlocks[MAX_PTHREADS];

// Current Number of Threads currently in the scheduler
static uint32_t NumberOfThreads;

// Current Number of Periodic Threads currently in the scheduler
static uint32_t NumberOfPThreads;

//static uint32_t threadCounter = 0;

/*******************************Private Functions***********************************/

// Occurs every 1 ms.
static void InitSysTick(void) {
    // Set systick period to overflow every 1 ms.
    SysTickPeriodSet((uint32_t) (SysCtlClockGet() / (float) 1000.0));
    // Set systick interrupt handler
    SysTickIntRegister(SysTick_Handler);
    // Set pendsv handler
    IntRegister(FAULT_PENDSV, PendSV_Handler);
    // Enable systick interrupt
    SysTickIntEnable();
    // Enable systick
    SysTickEnable();
    return;
}


/********************************Public Variables***********************************/

uint32_t SystemTime;
tcb_t* CurrentlyRunningThread;
tcb_t* threadHead;
tcb_t* threadTail;

/********************************Public Functions***********************************/

// SysTick_Handler
// Increments system time, sets PendSV flag to start scheduler.
// Return: void
void SysTick_Handler(void) {
    // Traverse the linked-list to find which threads should be awake.
    /* Currently running thread should be put to sleep now, so search linked list for other threads. */
    tcb_t* t_iter = CurrentlyRunningThread->nextTCB;
    while (t_iter != CurrentlyRunningThread) {
        /* If a thread has finished its sleep count, wake it up. */
        if (t_iter->sleepCount == SystemTime) t_iter->asleep = false;
        t_iter = t_iter->nextTCB;
    }
    // Traverse the periodic linked list to run which functions need to be run.
    for (uint32_t i = NULL; i < NumberOfPThreads; i++) {
        if (SystemTime == pthreadControlBlocks[i].currentTime) {
            pthreadControlBlocks[i].handler();
            /* Configure the next time it runs */
            pthreadControlBlocks[i].currentTime = SystemTime + pthreadControlBlocks[i].period;
        }
    }
    SystemTime++;
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    return;
}

// G8RTOS_Init
// Initializes the RTOS by initializing system time.
// Return: void
void G8RTOS_Init(void) {
    uint32_t newVTORTable = 0x20000000;
    uint32_t* newTable = (uint32_t*)newVTORTable;
    uint32_t* oldTable = (uint32_t*) 0;

    for (uint8_t i = 0; i < 155; i++) {
        newTable[i] = oldTable[i];
    }

    HWREG(NVIC_VTABLE) = newVTORTable;

    SystemTime = NULL;
    NumberOfThreads = NULL;
    NumberOfPThreads = NULL;
    threadHead = NULL;
    threadTail = NULL;
    return;
}

// G8RTOS_Launch
// Launches the RTOS.
// Return: error codes, 0 if none
int32_t G8RTOS_Launch(void) {
    // Initialize system tick
    InitSysTick();
    // Set currently running thread to the first control block
    CurrentlyRunningThread = threadHead;
    // Set interrupt priorities
       // Pendsv
    IntPrioritySet(FAULT_PENDSV, 0xE0); /* 0xE0 is lowest priority. */
       // Systick
    IntPrioritySet(FAULT_SYSTICK, 0xE0);
    // Call G8RTOS_Start()
    G8RTOS_Start();
    return NO_ERROR;
}

// G8RTOS_Scheduler
// Chooses next thread in the TCB. This time uses priority scheduling.
// Return: void
void G8RTOS_Scheduler(void) {
    // Using priority, determine the most eligible thread to run that
    // is not blocked or asleep. Set current thread to this thread's TCB.
    uint16_t min_priority = 256;
    tcb_t* iter = CurrentlyRunningThread->nextTCB;
    tcb_t* eligible_thread = CurrentlyRunningThread;
    for (uint32_t i = NULL; i < NumberOfThreads - 1; i++) {
        if (!iter->asleep && !iter->blocked && (iter->priority < min_priority)) {
            min_priority = iter->priority;
            eligible_thread = iter;
        }
        iter = iter->nextTCB;
    }
    CurrentlyRunningThread = eligible_thread;
    return;
}

// SetInitialStack
// Creates an initial stack for a particular thread in such a way that it looks like
// it has already been running and previously suspended.
// Based on function of same name in the uP2 textbook / lecture notes.
// Return: void
void SetInitialStack(unsigned int index) {
    threadControlBlocks[index].stackPointer = (void*)&threadStacks[index][STACKSIZE - 16]; // Thread stack pointer
    /* The values chosen below do not matter and are for debugging purposes. */
    threadStacks[index][STACKSIZE - 1]  = THUMBBIT;   // PSR
    threadStacks[index][STACKSIZE - 2]  = 0x15151515; // PC (R15)
    threadStacks[index][STACKSIZE - 3]  = 0x14141414; // LR (R14)
    threadStacks[index][STACKSIZE - 4]  = 0x12121212; // R12
    threadStacks[index][STACKSIZE - 5]  = 0x03030303; // R3
    threadStacks[index][STACKSIZE - 6]  = 0x02020202; // R2
    threadStacks[index][STACKSIZE - 7]  = 0x01010101; // R1
    threadStacks[index][STACKSIZE - 8]  = 0x00000001; // R0
    threadStacks[index][STACKSIZE - 9]  = 0x11111111; // R11
    threadStacks[index][STACKSIZE - 10] = 0x10101010; // R10
    threadStacks[index][STACKSIZE - 11] = 0x09090909; // R9
    threadStacks[index][STACKSIZE - 12] = 0x08080808; // R8
    threadStacks[index][STACKSIZE - 13] = 0x07070707; // R7
    threadStacks[index][STACKSIZE - 14] = 0x06060606; // R6
    threadStacks[index][STACKSIZE - 15] = 0x05050505; // R5
    threadStacks[index][STACKSIZE - 16] = 0x04040404; // R4
    return;
}

// G8RTOS_AddThread
// Adds a thread. This is now in a critical section to support dynamic threads.
// It also now should initialize priority and account for live or dead threads.
// Param void* "threadToAdd": pointer to thread function address
// Param uint8_t "priority": priority from 0, 255.
// Param char* "name": character array containing the thread name.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name, uint8_t threadID) {
    // This should be in a critical section!
    int32_t i_bit = StartCriticalSection();
    // If number of threads is greater than the maximum number of threads, return
    if (NumberOfThreads >= MAX_THREADS) {
        EndCriticalSection(i_bit);
        return THREAD_LIMIT_REACHED;
    }
    /* Used to find a spot in the tcb to place a new thread! */
    uint32_t spotIndex = NULL;
    // if no threads
    if (!NumberOfThreads) {
        threadControlBlocks[spotIndex].nextTCB = &threadControlBlocks[spotIndex];
        threadControlBlocks[spotIndex].previousTCB = &threadControlBlocks[spotIndex];
        threadHead = &threadControlBlocks[spotIndex];

    } else {
        for (uint32_t i = NULL; i < MAX_THREADS; i++) {
            if (!threadControlBlocks[i].isAlive) {
                spotIndex = i;
                break;
            }
        }
        threadControlBlocks[spotIndex].previousTCB = threadTail;
        threadControlBlocks[spotIndex].nextTCB = threadHead;
        threadTail->nextTCB = &threadControlBlocks[spotIndex];
        threadHead->previousTCB = &threadControlBlocks[spotIndex];
    }
    /* Set up thread stack. */
    SetInitialStack(spotIndex);
    /* Thread address saved onto the stack. */
    threadStacks[spotIndex][STACKSIZE - 2] = (uint32_t)(threadToAdd);
    /* Set up thread info. */
    threadControlBlocks[spotIndex].priority = priority;
    threadControlBlocks[spotIndex].ThreadID = threadID;
    uint32_t index = NULL;
    while (name[index] != '\0' && index < MAX_NAME_LENGTH) {
        threadControlBlocks[spotIndex].threadName[index] = name[index];
        index++;
    }
    /* By default, threads should be awake and alive. */
    threadControlBlocks[spotIndex].asleep = false;
    threadControlBlocks[spotIndex].isAlive = true;
    /* Increment thread count, update the tail pointer, and return. */
    NumberOfThreads++;
    threadTail = &threadControlBlocks[spotIndex];
    EndCriticalSection(i_bit);
    return NO_ERROR;
}

// G8RTOS_Add_APeriodicEvent
// Param void* "AthreadToAdd": pointer to thread function address
// Param int32_t "IRQn": Interrupt request number that references the vector table. [0..155].
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_APeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn) {
    // Disable interrupts
    int32_t i_bit = StartCriticalSection();
    // Check if IRQn is valid
    if (IRQn < NULL || IRQn > 155) {
        EndCriticalSection(i_bit);
        return IRQn_INVALID;
    }
    // Check if priority is valid
    if (priority > 6) {
        EndCriticalSection(i_bit);
        return HWI_PRIORITY_INVALID;
    }
    // Set corresponding index in interrupt vector table to handler.
    IntRegister(IRQn, AthreadToAdd);
    // Set priority.
    IntPrioritySet(IRQn, priority);
    // Enable the interrupt.
    IntEnable(IRQn);
    // End the critical section.
    EndCriticalSection(i_bit);
    return NO_ERROR;
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
    // Make sure that the number of PThreads is not greater than max PThreads.
    if (NumberOfPThreads >= MAX_PTHREADS) return THREAD_LIMIT_REACHED;
        // Check if there is no PThread. Initialize and set the first PThread.
    if (!NumberOfPThreads) {
        pthreadControlBlocks[NULL].nextPTCB = &pthreadControlBlocks[NULL];
        pthreadControlBlocks[NULL].previousPTCB = &pthreadControlBlocks[NULL];
    } else {
        // Subsequent PThreads should be added, inserted similarly to a doubly-linked linked list
            // last PTCB should point to first, first PTCB should point to last.
        pthreadControlBlocks[NumberOfPThreads].nextPTCB = &pthreadControlBlocks[NULL];
        pthreadControlBlocks[NumberOfPThreads - 1].nextPTCB = &pthreadControlBlocks[NumberOfPThreads];
        pthreadControlBlocks[NULL].previousPTCB = &pthreadControlBlocks[NumberOfPThreads];
        pthreadControlBlocks[NumberOfPThreads].previousPTCB = &pthreadControlBlocks[NumberOfPThreads - 1];
    }
        // Set function
    pthreadControlBlocks[NumberOfPThreads].handler = PThreadToAdd;
        // Set period
    pthreadControlBlocks[NumberOfPThreads].period = period;
        // Set execute time
    pthreadControlBlocks[NumberOfPThreads].executeTime = execution;
    /* Configure current time */
    pthreadControlBlocks[NumberOfPThreads].currentTime = SystemTime + period;
        // Increment number of PThreads
    NumberOfPThreads++;
    return NO_ERROR;
}

// G8RTOS_KillThread
// Does not work for killing CurrentlyRunningThread
//      see G8RTOS_KillSelf for this instead.
// Param uint32_t "threadID": ID of thread to kill
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID) {
    // Start critical section
    int32_t i_bit = StartCriticalSection();
    // Check if there is only one thread, return if so
    if (NumberOfThreads <= 1) {
        EndCriticalSection(i_bit);
        return CANNOT_KILL_LAST_THREAD;
    }
    // Traverse linked list, find thread to kill
    tcb_t* iter = CurrentlyRunningThread->nextTCB;
    for (uint32_t i = NULL; i < NumberOfThreads - 1; i++) {
        // Update the next tcb and prev tcb pointers if found
        if (iter->ThreadID == threadID) {
            (iter->previousTCB)->nextTCB = iter->nextTCB;
            (iter->nextTCB)->previousTCB = iter->previousTCB;
            // mark as not alive, release the semaphore it is blocked on
            iter->isAlive = false;
            if (*(iter->blocked) < NULL) G8RTOS_SignalSemaphore(iter->blocked);
            (iter->blocked) = NULL;
            NumberOfThreads--;
            /* If the thread is the tail, update tail pointer. */
            if (iter == threadTail) threadTail = threadTail->previousTCB;
            EndCriticalSection(i_bit);
            return NO_ERROR;
        }
        iter = iter->nextTCB;
    }
    // Otherwise, thread does not exist.
    EndCriticalSection(i_bit);
    return THREAD_DOES_NOT_EXIST;
}

// G8RTOS_KillSelf
// Kills currently running thread.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillSelf(void) {
    int32_t i_bit = StartCriticalSection();
    // Check if there is only one thread
    if (NumberOfThreads <= 1) {
        EndCriticalSection(i_bit);
        return CANNOT_KILL_LAST_THREAD;
    }
    (CurrentlyRunningThread->previousTCB)->nextTCB = CurrentlyRunningThread->nextTCB;
    (CurrentlyRunningThread->nextTCB)->previousTCB = CurrentlyRunningThread->previousTCB;
    // Else, mark this thread as not alive.
    CurrentlyRunningThread->isAlive = false;
    if (*(CurrentlyRunningThread->blocked) < NULL) G8RTOS_SignalSemaphore(CurrentlyRunningThread->blocked);
    CurrentlyRunningThread->blocked = NULL;
    NumberOfThreads--;
    /* If the current thread is the tail, update tail pointer. */
    if (CurrentlyRunningThread == threadTail) threadTail = threadTail->previousTCB;
    EndCriticalSection(i_bit);
    /* Perform context switch once thread is killed. */
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    return NO_ERROR;
}

// sleep
// Puts current thread to sleep
// Param uint32_t "durationMS": how many systicks to sleep for
void sleep(uint32_t durationMS) {
    // Update time to sleep to
    // Set thread as asleep
    CurrentlyRunningThread->sleepCount = durationMS + SystemTime;
    CurrentlyRunningThread->asleep = true;
    /* Perform context switch once thread is asleep. */
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;
    return;
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

uint32_t G8RTOS_GetSysTime(void) {
    return SystemTime;
}
