/**
 * @file        osal.c
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
 * @brief       OSAL abstraction - implementation.
 */

#include "osal.h"
#include "osal_private.h"

#define OSAL_THREAD_COUNT               ( 7UL )
#define OSAL_LOGGER_DEPTH               ( 16UL )
#define OSAL_LOGGER_LENGTH              ( 128UL )
#define OSAL_THREAD_MAX_STACK_SIZE      ( 0xFFFFUL * sizeof(size_t) )

#define VT100_DEFAULT                   ("\x1B[39m")
#define VT100_WHITE                     ("\x1B[37m")
#define VT100_CYAN                      ("\x1B[36m")
#define VT100_MAGENTA                   ("\x1B[35m")
#define VT100_BLUE                      ("\x1B[34m")
#define VT100_YELLOW                    ("\x1B[33m")
#define VT100_GREEN                     ("\x1B[32m")
#define VT100_RED                       ("\x1B[31m")
#define VT100_BLACK                     ("\x1B[30m")

/* Private defines. */
typedef struct
{
    uint32_t magic;
    TaskHandle_t task;
    QueueHandle_t queue;
    OsalThreadConfig_t config;
    OsalThreadStats_t stats;
} OsalThreadContext_t;

/* Private data. */
static const char* g_vt100[OSAL_THREAD_COUNT] = {VT100_WHITE, VT100_CYAN, VT100_MAGENTA, VT100_BLUE, VT100_YELLOW, VT100_GREEN, VT100_RED};
static OsalThreadContext_t g_ctx[OSAL_THREAD_COUNT];
static OsalGenericLogger_t g_generic_logger;

/* Private functions. */
static bool verify_config                   (const OsalThreadConfig_t* config);
static bool get_free_context                (OsalThreadContext_t** ctx);
static void generic_task                    (void* params);

OsalErr_n osal_global_init                  (OsalGenericLogger_t system_logger)
{
    OsalErr_n err = OsalErrUnexpected;
    size_t i;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_FALSE == g_osal_initialized )
        {
            g_generic_logger = system_logger;
            g_osal_semaphore = xSemaphoreCreateMutex();
            if ( g_osal_semaphore )
            {
                err = osal_mutex_global_init();
                if ( OsalErrOk == err )
                {
                    for ( i = 0 ; i < sizeof(g_ctx)/sizeof(g_ctx[0]) ; ++i )
                    {
                        g_ctx[i].magic = OSAL_FALSE;
                    }
                    g_osal_initialized = OSAL_TRUE;
                    err = OsalErrOk;
                }
            }
            else
            {
                err = OsalErrPort;
            }
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_start                        (void)
{
    OsalErr_n err = OsalErrUnexpected;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized && OSAL_FALSE == g_osal_running )
        {
            g_osal_running = OSAL_TRUE;
            vTaskStartScheduler();
            // We don't really return from this function, so we never return 'OsalErrOk'.
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_service_execute              (void)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalThreadContext_t* ctx;
    size_t threadNum;
    size_t qIteration;
    char txMem[OSAL_LOGGER_LENGTH] = {0};

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( ( OSAL_TRUE == g_osal_running ) && g_generic_logger )
            {
                for ( threadNum = 0 ; threadNum < sizeof(g_ctx)/sizeof(g_ctx[0]) ; ++threadNum  )
                {
                    ctx = &g_ctx[threadNum];
                    if ( OSAL_TRUE == ctx->magic )
                    {
                        for ( qIteration = 0 ; qIteration < OSAL_LOGGER_DEPTH ; ++qIteration )
                        {
                            if ( pdTRUE == xQueueReceive(ctx->queue, txMem, 0) )
                            {
                                g_generic_logger("%s", g_vt100[threadNum]);
                                g_generic_logger("%s", txMem);
                                g_generic_logger("%s", VT100_DEFAULT);
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }
            err = OsalErrOk;
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_thread_create                (OsalThread_t* ptrHandle, const OsalThreadConfig_t* config)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalThreadContext_t* ctx;
    BaseType_t taskCreateSuccess;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            osal_private_lock();
            if ( ptrHandle && !*ptrHandle && config )
            {
                if ( verify_config(config) )
                {
                    if ( get_free_context(&ctx) )
                    {
                        hal_util_assert ( NULL == ctx->queue );
                        hal_util_assert ( NULL == ctx->task );
                        hal_util_memcpy(&ctx->config, config, sizeof(ctx->config));
                        hal_util_memset(&ctx->stats, 0, sizeof(ctx->stats));
                        taskCreateSuccess = xTaskCreate(generic_task,
                                                        config->name,
                                                        config->stackSize / sizeof(size_t),
                                                        ctx,
                                                        config->priority,
                                                        &(ctx->task));
                        ctx->queue = xQueueCreate(OSAL_LOGGER_DEPTH, OSAL_LOGGER_LENGTH);
                        if ( ctx->queue && ( pdPASS == taskCreateSuccess ) )
                        {
                            ctx->magic = OSAL_TRUE;
                            *ptrHandle = (OsalThread_t) ctx;
                            err = OsalErrOk;
                        }
                        else
                        {
                            err = OsalErrPort;
                        }
                    }
                    else
                    {
                        err = OsalErrMemory;
                    }
                }
                else
                {
                    err = OsalErrConfig;
                }
            }
            else
            {
                err = OsalErrParam;
            }
            osal_private_unlock();
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_thread_destroy               (OsalThread_t handle)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalThreadContext_t* ctx;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            osal_private_lock();
            if ( handle )
            {
                ctx = (OsalThreadContext_t*) handle;
                if ( OSAL_TRUE == ctx->magic )
                {
                    vTaskDelete(ctx->task);
                    vQueueDelete(ctx->queue);
                    ctx->task = NULL;
                    ctx->queue = NULL;
                    ctx->magic = OSAL_FALSE;
                    err = OsalErrOk;
                }
                else
                {
                    err = OsalErrForbidden;
                }
            }
            else
            {
                err = OsalErrParam;
            }
            osal_private_unlock();
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_thread_get_stats             (OsalThread_t handle, OsalThreadStats_t* stats)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalThreadContext_t* ctx;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( handle )
            {
                ctx = (OsalThreadContext_t*) handle;
                if ( OSAL_TRUE == ctx->magic )
                {
                    osal_private_lock();
                    hal_util_memcpy(stats, &ctx->stats, sizeof(*stats));
                    osal_private_unlock();
                    err = OsalErrOk;
                }
                else
                {
                    err = OsalErrUnexpected;
                }
            }
            else
            {
                err = OsalErrParam;
            }
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_logger                       (OsalThread_t handle, const char* fmt, ...)
{
    OsalErr_n err = OsalErrUnexpected;
    char printMem[OSAL_LOGGER_LENGTH] = {0};
    OsalThreadContext_t* ctx;
    va_list args;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( handle )
            {
                ctx = (OsalThreadContext_t*) handle;
                if ( OSAL_TRUE == ctx->magic )
                {
                    va_start(args, fmt);
                    (void) vsnprintf(printMem, sizeof(printMem), fmt, args);
                    va_end(args);
                    printMem[OSAL_LOGGER_LENGTH - 1] = 0;
                    if ( pdTRUE == xQueueSend(ctx->queue, printMem, 1UL) )
                    {
                        err = OsalErrOk;
                    }
                    else
                    {
                        err = OsalErrMemory;
                    }
                }
                else
                {
                    err = OsalErrUnexpected;
                }
            }
            else
            {
                err = OsalErrParam;
            }
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_tmr_init                     (uint32_t* currentTick)
{
    OsalErr_n err = OsalErrParam;

    if ( currentTick )
    {
        err = OsalErrOk;
        *currentTick = xTaskGetTickCount();
    }

    return err;
}

OsalErr_n osal_tmr_exec                     (OsalProcedure_t execute, uint32_t periodMs, uint32_t* lastTick)
{
    OsalErr_n err = OsalErrParam;
    TickType_t now;

    if ( execute && periodMs && lastTick )
    {
        now = xTaskGetTickCount();
        if ( now > *lastTick )
        {
            *lastTick += pdMS_TO_TICKS(periodMs);
            execute();
        }
        err = OsalErrOk;
    }

    return err;
}

OsalErr_n osal_delay                        (uint32_t delayMs)
{
    OsalErr_n err = OsalErrUnexpected;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( OSAL_TRUE == g_osal_running )
            {
                vTaskDelay(pdMS_TO_TICKS(delayMs));
                err = OsalErrOk;
            }
            else
            {
                err = OsalErrNotStarted;
            }
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_tick_get                     (uint32_t* tick)
{
    OsalErr_n err = OsalErrUnexpected;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( OSAL_TRUE == g_osal_running )
            {
                *tick = (uint32_t) xTaskGetTickCount();
                err = OsalErrOk;
            }
            else
            {
                err = OsalErrNotStarted;
            }
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

static bool verify_config                   (const OsalThreadConfig_t* config)
{
    bool valid = false;

    if ( config )
    {
        if ( config->fnPoll )
        {
            if ( ( config->priority <= configMAX_PRIORITIES ) && ( config->stackSize < OSAL_THREAD_MAX_STACK_SIZE ) )
            {
                valid = true;
            }
        }
    }

    return valid;
}

static bool get_free_context                (OsalThreadContext_t** ctx)
{
    size_t i;
    size_t freeIndex = sizeof(g_ctx)/sizeof(g_ctx[0]);
    bool success = false;

    // Find the next free index.
    for ( i = 0 ; i < sizeof(g_ctx)/sizeof(g_ctx[0]) ; ++i )
    {
        if ( OSAL_FALSE == g_ctx[i].magic )
        {
            freeIndex = i;
            break;
        }
    }

    // If found, update the return parameter.
    if ( freeIndex < sizeof(g_ctx)/sizeof(g_ctx[0]) )
    {
        *ctx = &g_ctx[freeIndex];
        success = true;
    }

    return success;
}

static void generic_task                    (void* params)
{
    TickType_t tick;
    TickType_t tickExpected;
    OsalThreadContext_t* ctx = (OsalThreadContext_t*) params;

    // Call the init function if it is attached.
    if ( ctx->config.fnInit )
    {
        ctx->config.fnInit((OsalThread_t)ctx);
    }

    // Set periodicity related variables where applicable.
    if ( ctx->config.periodicityMs )
    {
        tick = xTaskGetTickCount();
        tickExpected = tick;
    }

    for ( ; ; )
    {
        // Call the poll function.
        ctx->config.fnPoll((OsalThread_t)ctx);

        // Update local stats.
        ctx->stats.countLoops++;

        // Handle time-bound threads.
        if ( ctx->config.periodicityMs )
        {
            tickExpected += pdMS_TO_TICKS(ctx->config.periodicityMs);
            hal_util_assert ( xTaskGetTickCount() <= tickExpected );
            vTaskDelayUntil(&tick, pdMS_TO_TICKS(ctx->config.periodicityMs));
        }
    }
}
