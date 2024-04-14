An implemented real-time operating system (RTOS) for the TI Tiva-C Launchpad microcontroller
in C and ARM assembly for the EEL4745C course at the University of Florida.
Implements a priority scheduler with a doubly linked list.
Since dynamic memory is discouraged in embedded systems, the data structure is stored in an array structure,
which itself is modified in real time without the use of malloc/free.
Inter process communication is supported via FIFOs which transmit/receive data between threads.
Semaphores are used to block threads and prevent race conditions.
Potential improvements:
- Creating a linked list for all the sleeping threads to turn the operation of checking for threads
  that have completed their sleep cycle an O(1) operation.
- Adjusting the stack size of each thread, FIFO size, etc., to assess the maximum capabilities of the RTOS.
- The RTOS works on the assumption that the first thread inserted (the idle thread) will never be deleted,
  else there could be bugs that arise if this condition is not satsified.
- Check in the debugger menu that PendSV triggers a context switch in the proper points in the program
  where such a switch occurs (also check that the ISR flag is set/cleared accordingly).
- KillThread function doesn't work for an ID parameter of the currently running thread
  (can be temporarily fixed by calling KillSelf in this condition).
- Creating an exhaustive test suite to see where the RTOS can be improved upon overall.
