/*
 * kernel.h
 *
 *  Created on: Oct 18, 2023
 *      Author: krishnakalavadia
 */

/*
 * TCB struct
 */
typedef struct k_thread{
	uint32_t* sp; // stack pointer
	void (*thread_function)(void*); // function pointer
	uint32_t timeslice; // time slice of our thread
	uint32_t runtime; // runtime of our thread
} thread;

/*
 *  SVC System Calls
 */
void print_success(void);

void print_first_name(void);

void print_last_name(void);

void run_thread(void);

/*
 * User written SVC Handler, takes in integer and runs it based on the code
 */
void SVC_Handler_Main( unsigned int *svc_args );

/*
 * Allocates stack space for new thread if space is available
 */
uint32_t* allocateThreadStack();

/*
 *  Set up new thread's stack and context
 */
int osCreateThread(void *thread_function, void *arg);

/*
 *  Set up new thread's stack and context
 */
int osCreateThreadWithDeadline(void *thread_function, void *arg, uint32_t deadline);

/*
 *  Initialize our kernel and its data
 */
void osKernelInitialize();

/*
 *  Start our kernel and began running our threads
 */
void osKernelStart();
