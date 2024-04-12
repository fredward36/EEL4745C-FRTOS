// G8RTOS_IPC.h
// Date Created: 2023-07-26
// Date Updated: 2024-04-08
// Interprocess communication code for G8RTOS

#ifndef G8RTOS_IPC_H_
#define G8RTOS_IPC_H_

/************************************Includes***************************************/

#include <stdint.h>

#include "./G8RTOS_Semaphores.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/

#define FIFO_SIZE               16
#define MAX_NUMBER_OF_FIFOS     5

/*************************************Defines***************************************/

/******************************Data Type Definitions********************************/

// IPC error typedef
typedef enum
{
    SUCCESS = 0,
    INDEX_OUT_OF_BOUNDS = -1,
    FIFO_EMPTY = -2,
    FIFO_FULL = -3
} IPC_ErrCode_t;

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/
/****************************Data Structure Definitions*****************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

int32_t G8RTOS_InitFIFO(uint32_t FIFO_index);
int32_t G8RTOS_ReadFIFO(uint32_t FIFO_index);
int32_t G8RTOS_WriteFIFO(uint32_t FIFO_index, int32_t data);

/********************************Public Functions***********************************/

#endif /* G8RTOS_IPC_H_ */

