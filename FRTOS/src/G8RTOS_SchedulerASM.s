; G8RTOS_SchedulerASM.s
; Created: 2022-07-26
; Updated: 2024-04-08
; Contains assembly functions for scheduler.

	; Functions Defined
	.def G8RTOS_Start, PendSV_Handler

	; Dependencies
	.ref CurrentlyRunningThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field CurrentlyRunningThread, 32

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc

	; Load the address of RunningPtr
	LDR R0, RunningPtr
	; Load the address of the thread control block of the currently running pointer
	LDR R1, [R0]
	; Load the first thread's stack pointer
	LDR SP, [R1]
	; Load registers with thread register values
	POP {R4-R11}
	POP {R0-R3}
	POP {R12}
	; Load LR with the first thread's PC
	ADD SP, SP , #4
	POP {LR}
	; Enable interrupts at processor level
	CPSIE I
	; Branches to the first thread
	BX LR

	.endasmfunc

; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:

	.asmfunc
	; put your assembly code here!
	CPSID I ; Prevent interrupt during switch
	PUSH {R4-R11} ; save old threads registers
	LDR R0, RunningPtr ; save current stack pointer
	LDR R1, [R0]
	STR SP, [R1]
	PUSH {R0, LR}
	BL G8RTOS_Scheduler ; call function to get new tcb
	POP {R0, LR}
	LDR R1, [R0] ; set stack pointer to new stack pointer from new tcb
	LDR SP, [R1]
	POP {R4-R11} ; pop new threads registers
	CPSIE I ; tasks run with interrupts enabled
	BX LR ; restore R0-R3, R12, LR, PC, PSR

	.endasmfunc

	; end of the asm file
	.align
	.end
