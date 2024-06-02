/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
// ----------------------------------------------------------------------------
/****************************************************************************
 **************************  Used Libraries  ********************************
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
// ----------------------------------------------------------------------------
/****************************************************************************
 ****************************  Definitions  *********************************
 ***************************************************************************/
#define CCM_RAM __attribute__((section(".ccmram")))

#define QUEUE_SIZE 10
#define SENDER1_PRIORITY 2
#define SENDER2_PRIORITY 1
#define SENDER3_PRIORITY 1
#define RECEIVER_PRIORITY 3
#define SLEEP_TIME_MS 100
#define TRECEIVER 100
// ----------------------------------------------------------------------------
/****************************************************************************
 *************************  Global variables   ******************************
 ***************************************************************************/
int sendSuccessfulCounter1 = 0;
int sendSuccessfulCounter2 = 0;
int sendSuccessfulCounter3 = 0;

int blockedSendCounter1 = 0;
int blockedSendCounter2 = 0;
int blockedSendCounter3 = 0;

int receivedMessageCounter = 0;

int Iteration = 0;

int arrLowerBoundery[6] = {50, 80, 110, 140, 170, 200};
int arrUpperBoundery[6] = {150, 200, 250, 300, 350, 400};
int* ptrLowerBoundery = arrLowerBoundery; //Put arrLowerBoundery in another pointer so that we can change its address
int* ptrUpperBoundery = arrUpperBoundery; //Put arrLowerBoundery in another pointer so that we can change its address

int TotalTime1 = 0;
int TotalTime2 = 0;
int TotalTime3 = 0;

int CounterTotalTime1 = 0;
int CounterTotalTime2 = 0;
int CounterTotalTime3 = 0;

int Tsender1 = 100; //we can't initialize global with random so we choose random number by our self
int Tsender2 = 50; //we can't initialize global with random so we choose random number by our self
int Tsender3 = 75; //we can't initialize global with random so we choose random number by our self

SemaphoreHandle_t semaphore, semaphore1, semaphore2, semaphore3;
TimerHandle_t Timer, Timer1, Timer2, Timer3;

QueueHandle_t xQueue1; //so that any function can use the shared queue
// ----------------------------------------------------------------------------
/****************************************************************************
 ************************  Function decleration   ***************************
 ***************************************************************************/
void init(void);

static int prvRandom(int value1, int value2);

static void prvUpdateBoundries(void);
static void prvStart(void* Queue);
static void prvReset(void* Queue);

void ReceiveTimerSemaphore(TimerHandle_t xTimer);
void SenderTimer1Semaphore(TimerHandle_t xTimer);
void SenderTimer2Semaphore(TimerHandle_t xTimer);
void SenderTimer3Semaphore(TimerHandle_t xTimer);

static void UpdateAverageTime1(int Totaltime);
static void UpdateAverageTime2(int Totaltime);
static void UpdateAverageTime3(int Totaltime);
// ----------------------------------------------------------------------------
/****************************************************************************
 **************************  Main code   ************************************
 ***************************************************************************/
void vSenderTask1Function(void *pvParameters)
{
	QueueHandle_t xQueue = (QueueHandle_t) pvParameters;

	while(1)
	{
		Tsender1 = prvRandom(*ptrLowerBoundery, *ptrUpperBoundery);
		xTimerChangePeriod(Timer1, Tsender1, 0);
		xSemaphoreTake(semaphore1, portMAX_DELAY);

		UpdateAverageTime1(Tsender2);

		// Create the send string with task number and random value
		char sendString[50];
		sprintf(sendString, "Time1 is %d RandomePriod1 = %d", (int)xTaskGetTickCount(), Tsender1);
		// Send the send string to the queue
		if (xQueueSend(xQueue, sendString, 0) == pdPASS)
		{
			sendSuccessfulCounter1++;
		}
		else
		{
			blockedSendCounter1++;
		}

	}
}


void vSenderTask2Function(void *pvParameters)
{
	QueueHandle_t xQueue = (QueueHandle_t) pvParameters;

	while(1)
	{
		Tsender2 = prvRandom(*ptrLowerBoundery, *ptrUpperBoundery);
		xTimerChangePeriod(Timer2, Tsender2, 0);
		xSemaphoreTake(semaphore2, portMAX_DELAY);

		UpdateAverageTime2(Tsender2);

		// Create the send string with task number and random value
		char sendString[50];
		sprintf(sendString, "Time2 is %d RandomePriod2 = %d", (int)xTaskGetTickCount(), Tsender2);
		// Send the send string to the queue
		if (xQueueSend(xQueue, sendString, 0) == pdPASS)
		{
			sendSuccessfulCounter2++;
		}
		else
		{
			blockedSendCounter2++;
		}

	}
}


void vSenderTask3Function(void *pvParameters)
{
	QueueHandle_t xQueue = (QueueHandle_t) pvParameters;

	while(1)
	{
		Tsender3 = prvRandom(*ptrLowerBoundery, *ptrUpperBoundery);
		xTimerChangePeriod(Timer3, Tsender3, 0);
		xSemaphoreTake(semaphore3, portMAX_DELAY);

		UpdateAverageTime3(Tsender3);

		// Create the send string with task number and random value
		char sendString[50];
		sprintf(sendString, "Time3 is %d RandomePriod3 = %d", (int)xTaskGetTickCount(), Tsender3);
		// Send the send string to the queue
		if (xQueueSend(xQueue, sendString, 0) == pdPASS)
		{
			sendSuccessfulCounter3++;
		}
		else
		{
			blockedSendCounter3++;
		}

	}
}


void vReceiverTaskFunction(void *pvParameters)
{
	QueueHandle_t xQueue = (QueueHandle_t) pvParameters;
	while(1)
	{
		xSemaphoreTake(semaphore, portMAX_DELAY);

		char receivedString[50];
		if (xQueueReceive(xQueue, receivedString, 0) == pdPASS)
		{
			// Print the received string
			printf("%s\n", receivedString);
			receivedMessageCounter++;
		}

	}
}


/*-----------------------------------------------------------*/
// ----------------------------------------------------------------------------
//
// Semihosting STM32F4 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace-impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int main(int argc, char* argv[])
{
	//initialize and create timers, semaphores and the queue that is used for task communication
	init();

	// Create sender tasks and receiver task
	xTaskCreate(vSenderTask1Function, "Sender 1", configMINIMAL_STACK_SIZE, (void *)xQueue1, SENDER1_PRIORITY, NULL);
	xTaskCreate(vSenderTask2Function, "Sender 2", configMINIMAL_STACK_SIZE, (void *)xQueue1, SENDER2_PRIORITY, NULL);
	xTaskCreate(vSenderTask3Function, "Sender 3", configMINIMAL_STACK_SIZE, (void *)xQueue1, SENDER3_PRIORITY, NULL);
	xTaskCreate(vReceiverTaskFunction, "Receiver", configMINIMAL_STACK_SIZE, (void *)xQueue1, RECEIVER_PRIORITY, NULL);

	//start program
	prvStart((void *)xQueue1);

	// Start the scheduler
	vTaskStartScheduler();
	return 0;
}

#pragma GCC diagnostic pop
// ----------------------------------------------------------------------------
/****************************************************************************
 ************************  Function definition   ****************************
 ***************************************************************************/
void init(void)
{
	//create semaphores
	semaphore = xSemaphoreCreateBinary();
	semaphore1 = xSemaphoreCreateBinary();
	semaphore2 = xSemaphoreCreateBinary();
	semaphore3 = xSemaphoreCreateBinary();

	//create timers
	Timer = xTimerCreate( "Timer", pdMS_TO_TICKS(TRECEIVER), pdTRUE, ( void * )0, ReceiveTimerSemaphore);
	Timer1 = xTimerCreate( "Timer1", pdMS_TO_TICKS(Tsender1), pdTRUE, ( void * )1, SenderTimer1Semaphore);
	Timer2 = xTimerCreate( "Timer2", pdMS_TO_TICKS(Tsender2), pdTRUE, ( void * )2, SenderTimer2Semaphore);
	Timer3 = xTimerCreate( "Timer3", pdMS_TO_TICKS(Tsender3), pdTRUE, ( void * )3, SenderTimer3Semaphore);

	//start timers
	xTimerStart(Timer, 0 );
	xTimerStart(Timer1, 0 );
	xTimerStart(Timer2, 0 );
	xTimerStart(Timer3, 0 );

	// Create the queue
	QueueHandle_t xQueue;
	xQueue = xQueueCreate(QUEUE_SIZE, sizeof(char[50]));
	xQueue1 = xQueue; //so that we can use queue in any function

}



static int prvRandom(int value1, int value2)
{
	int range = value2 - value1 + 1;
	int rand_num = rand() % range;
	return value1 + rand_num;
}

static void prvUpdateBoundries(void)
{
	if(Iteration > 5)
	{
		printf("\nGame Over!!");
		exit(EXIT_SUCCESS);
	}
	else if(Iteration > 0)
	{
		ptrLowerBoundery++;
		ptrUpperBoundery++;
	}
}


static void prvReset(void* Queue)
{
	printf("\nFor iteration %d of region of lowerBound %d and upperBound %d with Queue length of %d \n\n", Iteration+1,*ptrLowerBoundery, *ptrUpperBoundery, QUEUE_SIZE);

	printf("Number of successfuly received messages = %d\n", receivedMessageCounter);

	printf("Total number of successfuly send messages   = %d \n", sendSuccessfulCounter1+sendSuccessfulCounter2+sendSuccessfulCounter3);
	printf("Total number of blocked send messages = %d\n", blockedSendCounter1+blockedSendCounter2+blockedSendCounter3);

	printf("Number of successfuly send messages of Task1(high priority) = %d\n", sendSuccessfulCounter1);
	printf("Number of blocked send messages of Task1(high priority) = %d\n", blockedSendCounter1);
	printf("Number of successfuly send messages of Task2(low priority) = %d\n", sendSuccessfulCounter2);
	printf("Number of blocked send messages of Task2(low priority) = %d\n", blockedSendCounter2);
	printf("Number of successfuly send messages of Task3(low priority) = %d\n", sendSuccessfulCounter3);
	printf("Number of blocked send messages of Task3(low priority) = %d\n\n", blockedSendCounter3);

	printf("Average total time = %d\n", (TotalTime1+TotalTime2+TotalTime3)/(CounterTotalTime1+CounterTotalTime2+CounterTotalTime3));
	printf("Average Time1 = %d\n", (TotalTime1)/(CounterTotalTime1));
	printf("Average Time2 = %d\n", (TotalTime2)/(CounterTotalTime2));
	printf("Average Time3 = %d\n\n", (TotalTime3)/(CounterTotalTime3));

	Iteration++;//To choose correct boundary for random function

	QueueHandle_t xQueue = (QueueHandle_t) Queue;
	prvStart((void *)xQueue);

}


static void prvStart(void* Queue)
{
	QueueHandle_t xQueue = (QueueHandle_t) Queue;
	// clear the queue
	xQueueReset(xQueue);

	// clear the counters
	sendSuccessfulCounter1 = 0;
	sendSuccessfulCounter2 = 0;
	sendSuccessfulCounter3 = 0;

	blockedSendCounter1 = 0;
	blockedSendCounter2 = 0;
	blockedSendCounter3 = 0;

	receivedMessageCounter = 0;

	TotalTime1 = 0;
	TotalTime2 = 0;
	TotalTime3 = 0;

	CounterTotalTime1 = 0;
	CounterTotalTime2 = 0;
	CounterTotalTime3 = 0;


	prvUpdateBoundries();
}

void ReceiveTimerSemaphore(TimerHandle_t xTimer)
{
	if(receivedMessageCounter >= 1000)
	{
		prvReset((void *)xQueue1);

	}
	xSemaphoreGive(semaphore);
}

void SenderTimer1Semaphore(TimerHandle_t xTimer)
{
	xSemaphoreGive(semaphore1);
}

void SenderTimer2Semaphore(TimerHandle_t xTimer)
{
	xSemaphoreGive(semaphore2);
}

void SenderTimer3Semaphore(TimerHandle_t xTimer)
{
	xSemaphoreGive(semaphore3);
}




static void UpdateAverageTime1(int Totaltime)
{
	TotalTime1 = Totaltime +TotalTime1;
	CounterTotalTime1++;
}

static void UpdateAverageTime2(int Totaltime)
{
	TotalTime2 += Totaltime;
	CounterTotalTime2++;
}

static void UpdateAverageTime3(int Totaltime)
{
	TotalTime3 += Totaltime;
	CounterTotalTime3++;
}


void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

