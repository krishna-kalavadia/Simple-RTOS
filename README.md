# Simple RTOS for STM32F401RE

Implemented a simple real-time operating system on an ARM Cortex-M4 (STM32F401RE) to learn and gain a foundational understanding of C development in a real-time embedded environment. Main features implemented include cooperative and pre-emptive multitasking, SVC system calls, context switching, and basic kernel services.


## Main Features

- **Hardware Abstraction & I/O**  
  - Uses STM32 HAL for GPIO, UART, SysTick, PendSV, SVC  

- **Stack Management**  
  - Divides the main stack (MSP) into 0x400-byte PSP regions for up to eight contexts (main + seven threads) 
  - Maintains a simple “stack pool” allocator for up to eight stacks (main + seven threads)

- **System Calls & Context Initialization**  
  - Defines custom SVC numbers and a handler (`SVC_Handler_Main`) for supervisor calls  
  - Builds each thread’s initial context on its PSP: xPSR, PC, LR R0–R3 (arguments), R4–R11

- **Kernel API**  
  - `osKernelInitialize()` — capture initial MSP and reset kernel state  
  - `osCreateThread(void (*fn)(void*), void *arg)` — allocate stack, set up context, register a TCB  
  - `osCreateThreadWithDeadlines(fn, arg, slice_ms, deadline_ms)` — same as above with time-slice and deadline information
  - `osKernelStart()` — start the first thread via SVC  
  - `osYield()` — cooperative context switch trigger  

- **Scheduling (Round-Robin)**  
  - **Cooperative:** threads call `osYield()`, triggering an SVC exception; the PendSV handler then switches to the next thread.  
  - **Preemptive:** a 1 ms SysTick interrupt decrements the current thread’s time slice; when it hits zero, PendSV is pended to rotate to the next thread.  

- **Argument Passing & Exit Handling**  
  - Uses the ARM ABI (R0) to enable uses to pass arguments when creating threads

---

Below is a simplified example usage of the RTOS API

```
int main(void) {
    HAL_Init();

    osKernelInitialize();

    osCreateThread(thread1, arg1);
    osCreateThreadWithDeadlines(thread2, arg2, 20, 50);

    osKernelStart();

    while (1) 
}
```

---

This is a very bare-bones RTOS implementation to learn about real-time systems and related concepts. There are many features omitted that I hope to implement; for example, thread exit handling, dynamic task management, priority-based scheduling, and inter-thread communication, which would make this a more complete implementation.

This RTOS project was developed as a part of MTE-241: Introduction to Computer Structures and Real-Time Systems at the University of Waterloo