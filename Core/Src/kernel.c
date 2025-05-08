/*
 * kernel.c
 *
 *  Created on: Oct 18, 2023
 *      Author: krishnakalavadia
 *
 */

// Includes
#include "stm32f4xx_hal.h"

// User Includes
#include "kernel.h"
#include <stdio.h>
#include <stdbool.h>

// Global Variables
uint32_t* STACK_PTR;
uint32_t* LAST_STACK_ADDR_PTR;
uint32_t STACK_MEM_USAGE = 0;

uint32_t CURRENT_THREAD = -1;
uint32_t NUM_THREADS_CRREATED = 0;

thread thread_1;
thread thread_2;
thread thread_3;
thread thread_4;
thread thread_5;
thread thread_6;
thread thread_7;

// Note we can only hold 7 threads since the chunk of stack will be MSP
thread threadArray[7];

#define RUN_FIRST_THREAD  0x3
#define THREAD_STACK_SPACE  0x400
#define NULL ((void *)0)
#define YIELD  0x4

#define SHPR2 *(uint32_t*)0xE000ED1C  // Used for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20  // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04  // This lets us trigger PendSV

// Extern
extern void runFirstThread(void);

// System Calls
void print_success(void)
{
	__asm("SVC #0");
}

void print_first_name(void)
{
	__asm("SVC #1");
}

void print_last_name(void)
{
	__asm("SVC #2");
}

void run_thread(void)
{
	__asm("SVC #3");
}

void osYield(void)
{
	__asm("SVC #4");
}

void osKernelInitialize() {
	// We want the MSP value which is stored at address 0x0 thus use a double pointer
	uint32_t* MSP_INIT_VAL = *(uint32_t**)0x0;

	LAST_STACK_ADDR_PTR = MSP_INIT_VAL;

	uint32_t PSP_val = (uint32_t)MSP_INIT_VAL - 0x400;

	// We are modifying our stack space, so update our variables that track our stack space.
	LAST_STACK_ADDR_PTR = PSP_val;
	STACK_MEM_USAGE += 400;

	// Initialize our TCB thread array for our 7 threads
	threadArray[0] = thread_1;
	threadArray[1] = thread_2;
	threadArray[2] = thread_3;
	threadArray[3] = thread_4;
	threadArray[4] = thread_5;
	threadArray[5] = thread_6;
	threadArray[5] = thread_7;

	// Set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; // Shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; // Set the priority of SVC higher than PendSV

	return;
}

void osKernelStart() {
	// Start running our threads.
	CURRENT_THREAD = 0;
	run_thread();
}


int osCreateThread(void *thread_function, void *arg)
{
	// First allocate stack space that we'll use to create a new thread
	uint32_t* new_stack_ptr = allocateThreadStack();

	if (new_stack_ptr)
	{
		// Before creating a new thread, update our variables tracking thread
		// metadata
		CURRENT_THREAD++;
		NUM_THREADS_CRREATED++;

		// We have successfully allocated stack space, now set up
		// the thread's stack and context

		*(--new_stack_ptr) = 1 << 24; // A magic number, this is xPSR
		*(--new_stack_ptr) = (uint32_t)thread_function; // the function name

		/*
		Note in this next portion, we're setting a bunch of registers to some arbitrary value.
		The Link Register (LR) falls within this loop so if the thread ever exited, we'd hard fault.
		What ideally should happen is once we exit our thread, we should go back to using MSP. 		
		*/
		for (int i = 0; i < 5; i++)
		{
		  *(--new_stack_ptr) = 0xA; // An arbitrary number
		}

		// Now our stack pointer is pointing to R0, lets load our thread arguments.
		*(--new_stack_ptr) = (uint32_t)arg;

		// Load rest of the required registers.
		for (int i = 0; i < 8; i++)
		{
		  *(--new_stack_ptr) = 0xA; // An arbitrary number
		}

		// Update our thread struct.
		threadArray[CURRENT_THREAD].sp = new_stack_ptr;
		threadArray[CURRENT_THREAD].thread_function = thread_function;
		threadArray[CURRENT_THREAD].timeslice = 5;
		threadArray[CURRENT_THREAD].runtime = 5;

		return true;
	}
	printf("Error Creating Thread!\r\n");

	return false;
}

int osCreateThreadWithDeadline(void *thread_function, void *arg, uint32_t deadline)
{
	// First allocate stack space that we'll use to create a new thread
	uint32_t* new_stack_ptr = allocateThreadStack();

	if (new_stack_ptr)
	{
		// Before creating a new thread, update our variables tracking thread
		// metadata
		NUM_THREADS_CRREATED++;
		CURRENT_THREAD++;

		// We have successfully allocated stack space, now set up
		// the thread's stack and context
		*(--new_stack_ptr) = 1 << 24; // This is xPSR
		*(--new_stack_ptr) = (uint32_t)thread_function; // the function name
		
		/*
		Note in this next portion, we're setting a bunch of registers to some arbitrary value.
		The Link Register (LR) falls within this loop so if the thread ever exited, we'd hard fault.
		What ideally should happen is once we exit our thread, we should go back to using MSP. 		
		*/
		for (int i = 0; i < 5; i++)
		{
		  *(--new_stack_ptr) = 0xA; // An arbitrary number
		}

		// Now our stack pointer is pointing to R0, lets load our thread arguments.
		*(--new_stack_ptr) = (uint32_t)arg;

		// Load rest of the required registers to some arbitrary value.
		for (int i = 0; i < 8; i++)
		{
		  *(--new_stack_ptr) = 0xA; // An arbitrary number
		}

		// Update our thread struct.
		threadArray[CURRENT_THREAD].sp = new_stack_ptr;
		threadArray[CURRENT_THREAD].thread_function = thread_function;
		threadArray[CURRENT_THREAD].timeslice = deadline;
		threadArray[CURRENT_THREAD].runtime = deadline;

		return true;
	}
	printf("Error Creating Thread!\r\n");

	return false;
}

uint32_t* allocateThreadStack()
{
	if(STACK_MEM_USAGE < 3600)
	{
		// Allocate new stack space for our new thread
		uint32_t* new_stack_ptr = LAST_STACK_ADDR_PTR - 0x400;

		// Update our variables tracking our stack space
		LAST_STACK_ADDR_PTR = new_stack_ptr;
		STACK_MEM_USAGE += 400;

		// Return our new stack pointer
		return new_stack_ptr;
	}

	printf("Error Allocating Thread Stack!\r\n");
	return NULL;
}

void osSched() {

	// Save stack pointer of current thread
	threadArray[CURRENT_THREAD].sp = (uint32_t*)(__get_PSP() - 8*4);

	// Employ Round Robin Scheduling
	CURRENT_THREAD = (CURRENT_THREAD + 1) % (NUM_THREADS_CRREATED);

	// Set PSP to the new current thread’s stack pointer.
	// 	Note we are under the assumption that the next thread to run, has already
	// 	be populated with the required data and context to run
	__set_PSP(threadArray[CURRENT_THREAD].sp);

	return;
}

void SVC_Handler_Main( unsigned int *svc_args )
{
	unsigned int svc_number;
	/*
	* Stack contains:
	* r0, r1, r2, r3, r12, r14, the return address and xPSR
	* First argument (r0) is svc_args[0]
	*/

	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
		case 0:
			printf("Success!\r\n");
			break;
		case 1:
			printf("First name: Krishna\r\n");
			break;
		case 2:
			printf("Last name: Kalavadia\r\n");
			break;
		case RUN_FIRST_THREAD:
			__set_PSP(threadArray[CURRENT_THREAD].sp);
			runFirstThread();
			break;
		case YIELD:
			// Pend an interrupt to do the context switch
			_ICSR |= 1<<28;
			__asm("isb");
			break;
		default: /* unknown SVC */
			break;
	}
}

