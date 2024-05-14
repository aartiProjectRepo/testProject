/**
 * @file        os_mutex.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        04 January 2024
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Implements 'os_mutex.h' interface using 'osal_mutex.h' API.
 */

// Self.
#include "os_mutex.h"
#include "os_mutex_types.h"

// Dependency.
#include "osal_mutex.h"

// Private defines.
#define OS_MUTEX_COUNT_MAX                  ( 12 )

#define OS_MUTEX_MAGIC_PAINT32              ( 0x10C710C7UL )

typedef struct
{
    uint32_t magicBegin;
    OsalMutex_t mtx;
    uint32_t magicEnd;
} OsMutexSchema_t;

// Public API implementation.
void os_mutex__set_methods(os_mutex__methods_s methods)
{
    // We don't care about any global methods presently.
    (void) methods.init;
    (void) methods.lock;
    (void) methods.unlock;
}

bool os_mutex__init(os_mutex_s *mutex)
{
    bool success = false;
    OsMutexSchema_t* ptrMtxSchema = NULL;

    if ( mutex )
    {
        ptrMtxSchema = (OsMutexSchema_t*) &mutex->private_data.implementation;
        if ( OsalErrOk == osal_mutex_create( &(ptrMtxSchema->mtx) ) )
        {
            ptrMtxSchema->magicBegin = OS_MUTEX_MAGIC_PAINT32;
            ptrMtxSchema->magicEnd = ~OS_MUTEX_MAGIC_PAINT32;
            mutex->private_data.is_initialized = true;
            success = true;
        }
    }

    return success;
}

bool os_mutex__is_valid(const os_mutex_s *mutex)
{
    bool success = false;
    OsMutexSchema_t* ptrMtxSchema = NULL;

    if ( mutex )
    {
        ptrMtxSchema = (OsMutexSchema_t*) &mutex->private_data.implementation;
        if (    ( OS_MUTEX_MAGIC_PAINT32 == ptrMtxSchema->magicBegin ) && \
                ( ~OS_MUTEX_MAGIC_PAINT32 == ptrMtxSchema->magicEnd ) && \
                ( true == mutex->private_data.is_initialized ) )
        {
            success = true;
        }
    }

    return success;
}

bool os_mutex__lock(os_mutex_s *mutex)
{
    bool success = false;
    OsMutexSchema_t* ptrMtxSchema = NULL;

    if ( mutex )
    {
        ptrMtxSchema = (OsMutexSchema_t*) &mutex->private_data.implementation;
        if ( os_mutex__is_valid(mutex) )
        {
            if ( OsalErrOk == osal_mutex_take(ptrMtxSchema->mtx) )
            {
                success = true;
            }
        }
    }

    return success;
}

bool os_mutex__unlock(os_mutex_s *mutex)
{
    bool success = false;
    OsMutexSchema_t* ptrMtxSchema = NULL;

    if ( mutex )
    {
        ptrMtxSchema = (OsMutexSchema_t*) &mutex->private_data.implementation;
        if ( os_mutex__is_valid(mutex) )
        {
            if ( OsalErrOk == osal_mutex_give(ptrMtxSchema->mtx) )
            {
                success = true;
            }
        }
    }

    return success;
}
