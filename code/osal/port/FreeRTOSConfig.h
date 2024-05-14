/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#if defined(__ICCARM__)||defined(__CC_ARM)||defined(__GNUC__)
    /* Clock manager provides in this variable system core clock frequency */
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif
#include "hal_util.h"

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                    1                               // Using pre-emption is NECESSARY for our use-case.
#define configUSE_TICKLESS_IDLE                 0                               // TODO: Enabling this feature requires special handling, but will save some MCU power.
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )             // Make sure this value is matching the actual CPU freq., else you will not get proper timing.
#define configTICK_RATE_HZ                      ( (TickType_t) 10000UL )        // 10000 Hz == 100 us
#define configMAX_PRIORITIES                    10                              // Number of FreeRTOS task priorities.
#define configMINIMAL_STACK_SIZE                ( (size_t) 256UL )              // The stack size in words for the idle task (1 word == 4 bytes on 32-bit system).
#define configMAX_TASK_NAME_LEN                 10                              // Keep the task names shorter than this.
#define configUSE_16_BIT_TICKS                  0                               // Keep this 0, since we have a 32-bit CPU.
#define configIDLE_SHOULD_YIELD                 0
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               20                              // Keep this value greater than number of queues, semaphores and mutexes added to registry.
#define configUSE_QUEUE_SETS                    1                               // Queue sets is an advanced feature that we are not using in our application.
#define configUSE_TIME_SLICING                  1                               // Ready tasks of equally highest priority get executed in an interleaved fashion each tick.

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         1                               // Allows usage of static memory allocation for FreeRTOS primitives - discouraged.
#define configSUPPORT_DYNAMIC_ALLOCATION        1                               // Allows usage of dynamic memory allocation for FreeRTOS primitives (and in general).
#define configTOTAL_HEAP_SIZE                   ( (size_t) ( 48UL * 1024UL ) )  // This is the number of **BYTES** reserved for FreeRTOS heap (Not applicable when using heap_5.c)
#define configAPPLICATION_ALLOCATED_HEAP        1                               // Allows for user to define 'ucHeap' array to be used by FreeRTOS heap (applicable for heap_1.c to heap_4.c only).
#define configUSE_HEAP_SCHEME                   4                               // Tells FreeRTOS which one of the 5 heap implementations (heap_1.c to heap_5.c) to use.
#define configRECORD_STACK_HIGH_ADDRESS         1                               // Adds information (4 bytes) for each that stores where the stack ends. Useful for FreeRTOS TAD.

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     1                               // Callback when switching to Idle task.
#define configUSE_TICK_HOOK                     1                               // Callback at every FreeRTOS tick interrupt.
#define configCHECK_FOR_STACK_OVERFLOW          1                               // Callback when a FreeRTOS stack overflows - detection not guaranteed!
#define configUSE_MALLOC_FAILED_HOOK            1                               // Callback when pvPortMalloc(size_t) fails.
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0                               // Callback during initialization of 'Timer service' task.

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           1                               // This requires defining and implementing portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() and portGET_RUN_TIME_COUNTER_VALUE()
#define configUSE_TRACE_FACILITY                1                               // For better debugging set to 1, for performance set to 0.
#define configUSE_STATS_FORMATTING_FUNCTIONS    1                               // For better debugging set to 1, for performance set to 0.

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         2

/* Software timer related definitions. */
#define configUSE_TIMERS                        0                               // Allows FreeRTOS timer functionality (creates it's own task).
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )    // Timer service should usually have highest or very high priority.
#define configTIMER_QUEUE_LENGTH                10                              // Sets the length of the software timer command queue.
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE )    // Stack size of 'timer service' in words (1 word == 4 bytes on 32-bit system).

/* Define to trap errors during development. */
#define configASSERT(x)                         hal_util_assert(x)

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_xTaskResumeFromISR              1

/* Port configuration. */
#define portTICK_TYPE_IS_ATOMIC                 1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS  FreeRTOS_AppConfigureTimerForRuntimeStats
#define portGET_RUN_TIME_COUNTER_VALUE          FreeRTOS_AppGetRuntimeCounterValueFromISR
extern void FreeRTOS_AppConfigureTimerForRuntimeStats(void);
extern uint32_t FreeRTOS_AppGetRuntimeCounterValueFromISR(void);

#endif /* FREERTOS_CONFIG_H */
