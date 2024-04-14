An implemented real-time operating system (RTOS) for the TI Tiva-C Launchpad microcontroller.
Implements a priority scheduler with a doubly linked list.
Since dynamic memory is discouraged in embedded systems, the data structure is stored in an array structure,
which itself is modified in real time without the use of malloc/free.
Inter process communication is supported via FIFOs which transmit/receive data between threads.
Semaphores are used to block threads and prevent race conditions.
Potential improvements:
- Creating a linked list for all the sleeping threads to turn the operation of checking for threads
  that have completed their sleep cycle an O(1) operation.
- Adjusting the stack size of each thread, FIFO size, etc., to assess the maximum capabilities of the RTOS.
- Creating an exhaustive test suite to see where the RTOS can be improved upon overall.
