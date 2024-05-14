/**
 * @file        freertos_hooks.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        1 January 2024
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       FreeRTOS hooks and additions.
 */

#include "FreeRTOS.h"
#include "task.h"

static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

/**
 * @brief                   Configures a timer that will be used for FreeRTOS TAD tracing.
 * @see                     configGENERATE_RUN_TIME_STATS, configUSE_TRACE_FACILITY, configUSE_STATS_FORMATTING_FUNCTIONS, portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
 * @note                    May be exclusively invoked by freertos_kernel/tasks.c
 */
void FreeRTOS_AppConfigureTimerForRuntimeStats(void);
void FreeRTOS_AppConfigureTimerForRuntimeStats(void)
{
    return;
}

/**
 * @brief                   Getter function that returns the tick value used for FreeRTOS TAD tracing.
 * @see                     configGENERATE_RUN_TIME_STATS, configUSE_TRACE_FACILITY, configUSE_STATS_FORMATTING_FUNCTIONS, portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
 * @note                    May be exclusively invoked by freertos_kernel/tasks.c
 */
uint32_t FreeRTOS_AppGetRuntimeCounterValueFromISR(void);
uint32_t FreeRTOS_AppGetRuntimeCounterValueFromISR(void)
{
    return xTaskGetTickCount();
}

/**
 * @brief                   This function is called when switching to Idle task, currently it does nothing.
 *                          This can be used for entering low power modes since the CPU is idle. Reference for practical usage:
 *                          https://mcuoneclipse.com/2013/07/06/low-power-with-freertos-tickless-idle-mode/
 * @see                     configUSE_TICKLESS_IDLE, configMINIMAL_STACK_SIZE, configIDLE_SHOULD_YIELD, configUSE_IDLE_HOOK
 */
void vApplicationIdleHook(void);
void vApplicationIdleHook(void)
{
   return;
}

/**
 * @brief                   This function is called at every FreeRTOS tick interrupt.
 *                          You could use this to provide ticks to other application/library - we don't need it.
 * @warn                    This is called from ISR, it should be kept short.
 *                          You should NOT call any FreeRTOS API functions that don't end in "FromISR" or "FROM_ISR"!
 * @see                     configUSE_TICKLESS_IDLE, configMINIMAL_STACK_SIZE, configIDLE_SHOULD_YIELD, configUSE_IDLE_HOOK
 */
void vApplicationTickHook(void);
void vApplicationTickHook(void)
{
   return;
}

/**
 * @brief                   This function is called when a task stack overflow is detected.
 *                          We force an assert in this function since this condition requires intervention by developer.
 * @warn                    Due to the nature of FreeRTOS stack-overflow, the arguments may themselves get corrupted!
 *                          In such a case, one would have to manually inspect memory (specially the variable pxCurrentTCB).
 * @param   xTask           Task handle where stack overflow was detected.
 * @param   pcTaskName      ASCII name of task where stack overflow was detected.
 * @see                     configCHECK_FOR_STACK_OVERFLOW, configRECORD_STACK_HIGH_ADDRESS
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    for ( ; ; )
    {
        __nop();
    }
}

/**
 * @brief                   This function is called when pvPortMalloc(size_t) fails due of lack of requested memory size.
 *                          We force an assert in this function since this condition requires intervention by developer.
 * @param   xwantedSize     The requested amount of memory (which did not succeed in allocation).
 * @see                     configUSE_HEAP_SCHEME, configUSE_MALLOC_FAILED_HOOK
 * @note                    May be exclusively invoked by freertos_kernel/portable/MemMang/heap_N.c
 */
void vApplicationMallocFailedHook(size_t xWantedSize);
void vApplicationMallocFailedHook(size_t xWantedSize)
{
    for ( ; ; )
    {
        __nop();
    }
}

/**
 * @brief                   This function is called during startup of FreeRTOS 'Timer service' task.
 *                          One could use this to call application initialization code that needs FreeRTOS environment.
 * @see                     configUSE_TIMERS, configUSE_DAEMON_TASK_STARTUP_HOOK
 * @note                    May be exclusively invoked by freertos_kernel/timers.c
 */
void vApplicationDaemonTaskStartupHook(void);
void vApplicationDaemonTaskStartupHook(void)
{
    return;
}


/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
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

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
