/**
 * @file        osal_mutex.c
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
 * @brief       OSAL Mutex abstraction - implementation using FreeRTOS.
 */

#include "osal_mutex.h"
#include "osal_private.h"

/* Private defines. */
#define OSAL_MUTEX_COUNT                    ( 32UL )

typedef struct
{
    uint32_t magic;
    SemaphoreHandle_t sem;
} OsalMutexContext_t;

/* Private data. */
static OsalMutexContext_t g_ctx[OSAL_MUTEX_COUNT];

/* Private functions. */
static bool get_free_context                (OsalMutexContext_t** ctx);

OsalErr_n osal_mutex_global_init            (void)
{
    OsalErr_n err = OsalErrUnexpected;
    SemaphoreHandle_t localSem = NULL;
    size_t i;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_FALSE == g_osal_initialized )
        {
            err = OsalErrOk;
            for ( i = 0 ; i < sizeof(g_ctx)/sizeof(g_ctx[0]) ; ++i )
            {
                localSem = xSemaphoreCreateMutex();
                if ( localSem )
                {
                    g_ctx[i].sem = localSem;
                    g_ctx[i].magic = OSAL_FALSE;
                }
                else
                {
                    err = OsalErrMemory;
                    break;
                }
            }
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_mutex_create                 (OsalMutex_t* ptrMutex)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalMutexContext_t* ctx;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            osal_private_lock();
            if ( ptrMutex && !*ptrMutex )
            {
                if ( get_free_context(&ctx) )
                {
                    hal_util_assert ( NULL != ctx->sem );
                    ctx->magic = OSAL_TRUE;
                    *ptrMutex = (OsalMutex_t) ctx;
                    err = OsalErrOk;
                }
                else
                {
                    err = OsalErrMemory;
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

OsalErr_n osal_mutex_destroy                (OsalMutex_t mutex)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalMutexContext_t* ctx;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            osal_private_lock();
            if ( mutex )
            {
                ctx = (OsalMutexContext_t*) mutex;
                if ( OSAL_TRUE == ctx->magic )
                {
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

OsalErr_n osal_mutex_take_timed             (OsalMutex_t mutex, uint32_t timeoutMs)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalMutexContext_t* ctx;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( mutex )
            {
                ctx = (OsalMutexContext_t*) mutex;
                if ( OSAL_TRUE == ctx->magic )
                {
                    if ( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
                    {
                        if ( pdTRUE == xSemaphoreTake(ctx->sem, pdMS_TO_TICKS(timeoutMs)) )
                        {
                            err = OsalErrOk;    // Success.
                        }
                        else
                        {
                            err = OsalErrTimeout;
                        }
                    }
                    else
                    {
                        err = OsalErrOk;        // Success - no OS.
                    }
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
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_mutex_take                   (OsalMutex_t mutex)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalMutexContext_t* ctx;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( mutex )
            {
                ctx = (OsalMutexContext_t*) mutex;
                if ( OSAL_TRUE == ctx->magic )
                {
                    if ( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
                    {
                        if ( pdTRUE == xSemaphoreTake(ctx->sem, portMAX_DELAY) )
                        {
                            err = OsalErrOk;    // Success.
                        }
                        else
                        {
                            err = OsalErrTimeout;
                        }
                    }
                    else
                    {
                        err = OsalErrOk;        // Success - no OS.
                    }
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
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

OsalErr_n osal_mutex_give                   (OsalMutex_t mutex)
{
    OsalErr_n err = OsalErrUnexpected;
    OsalMutexContext_t* ctx;

    if ( ( OSAL_FALSE == g_osal_initialized ) || ( OSAL_TRUE == g_osal_initialized ) )
    {
        if ( OSAL_TRUE == g_osal_initialized )
        {
            if ( mutex )
            {
                ctx = (OsalMutexContext_t*) mutex;
                if ( OSAL_TRUE == ctx->magic )
                {
                    if ( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
                    {
                        if ( pdTRUE == xSemaphoreGive(ctx->sem) )
                        {
                            err = OsalErrOk;    // Success.
                        }
                        else
                        {
                            err = OsalErrPort;
                        }
                    }
                    else
                    {
                        err = OsalErrOk;        // Success - no OS.
                    }
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
        }
        else
        {
            err = OsalErrForbidden;
        }
    }

    return err;
}

static bool get_free_context                (OsalMutexContext_t** ctx)
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
