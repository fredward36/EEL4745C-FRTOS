// G8RTOS_IPC.c
// Date Created: 2023-07-25
// Date Updated: 2024-04-08
// Defines for FIFO functions for interprocess communication

#include "../G8RTOS_IPC.h"

/************************************Includes***************************************/

#include "../G8RTOS_Semaphores.h"

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/

typedef struct G8RTOS_FIFO_t {
    int32_t buffer[FIFO_SIZE];
    int32_t *head;
    int32_t *tail;
    uint32_t lostData;
    semaphore_t currentSize;
    semaphore_t roomLeft;
    semaphore_t mutex;
} G8RTOS_FIFO_t;


/***********************************Externs*****************************************/

/********************************Private Variables***********************************/

static G8RTOS_FIFO_t FIFOs[MAX_NUMBER_OF_FIFOS];

/********************************Public Functions***********************************/

// Three semaphore implementation

// G8RTOS_InitFIFO
// Initializes FIFO, points head & tail to relevant buffer
// memory addresses. Returns - 1 if FIFO full, 0 if no error
// Param uint32_t "FIFO_index": Index of FIFO block
// Return: int32_t
int32_t G8RTOS_InitFIFO(uint32_t FIFO_index) {
    if (FIFO_index >= MAX_NUMBER_OF_FIFOS) return INDEX_OUT_OF_BOUNDS;
    // Initialize head, tail pointers
    FIFOs[FIFO_index].head = &FIFOs[FIFO_index].buffer[NULL];
    FIFOs[FIFO_index].tail = &FIFOs[FIFO_index].buffer[NULL];
    /* Clear all the data, if any */
    for (uint32_t i = NULL; i < FIFO_SIZE; i++) FIFOs[FIFO_index].buffer[i] = NULL;
    // Init the mutex, current size
    FIFOs[FIFO_index].currentSize = NULL;
    FIFOs[FIFO_index].roomLeft = FIFO_SIZE;
    G8RTOS_InitSemaphore(&FIFOs[FIFO_index].mutex, 1);
    // Init lost data
    FIFOs[FIFO_index].lostData = NULL;
    return SUCCESS;
}

// G8RTOS_ReadFIFO
// Reads data from head pointer of FIFO.
// Param uint32_t "FIFO_index": Index of FIFO block
// Return: int32_t
int32_t G8RTOS_ReadFIFO(uint32_t FIFO_index) {
    if (FIFO_index >= FIFO_SIZE) return INDEX_OUT_OF_BOUNDS;
    if (!FIFOs[FIFO_index].currentSize) return FIFO_EMPTY;
    /* Read in first in first out fashion. */
    int32_t data = *(FIFOs[FIFO_index].head);
    (FIFOs[FIFO_index].currentSize)--;
    (FIFOs[FIFO_index].roomLeft)++;
    (FIFOs[FIFO_index].head)++;
    if (FIFOs[FIFO_index].head == &FIFOs[FIFO_index].buffer[FIFO_SIZE]) FIFOs[FIFO_index].head = &FIFOs[FIFO_index].buffer[NULL];
    //else (FIFOs[FIFO_index].head)++;
    return data;
}

// G8RTOS_WriteFIFO
// Writes data to tail of buffer.
// 0 if no error, -1 if out of bounds, -2 if full
// Param uint32_t "FIFO_index": Index of FIFO block
// Param int32_t "data": data to be written
// Return: int32_t
int32_t G8RTOS_WriteFIFO(uint32_t FIFO_index, int32_t data) {
    // Your code
    if (FIFO_index >= FIFO_SIZE) return INDEX_OUT_OF_BOUNDS;
    if (!FIFOs[FIFO_index].roomLeft) return FIFO_FULL;
    /* Before we write, we check if there's any data already there. */
    if (*FIFOs[FIFO_index].tail) FIFOs[FIFO_index].lostData++;
    *(FIFOs[FIFO_index].tail) = data;
    (FIFOs[FIFO_index].currentSize)++;
    (FIFOs[FIFO_index].roomLeft)--;
    (FIFOs[FIFO_index].tail)++;
    if (FIFOs[FIFO_index].tail == &FIFOs[FIFO_index].buffer[FIFO_SIZE]) FIFOs[FIFO_index].tail = &FIFOs[FIFO_index].buffer[NULL];
    //else (FIFOs[FIFO_index].tail)++;
    return SUCCESS;
}
