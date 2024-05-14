/**
 * @file        osal_types.h
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
 * @brief       OSAL types.
 */

#ifndef OSAL_TYPES_H
#define OSAL_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define OSAL_TRUE                           ( 0x03A1C0CA )
#define OSAL_FALSE                          ( 0x03A1DEAF )

/**
 *  @brief                                  Opaque handle for thread.
*/
typedef struct  OsalThreadContext_t*        OsalThread_t;

/**
 *  @brief                                  Opaque handle for mutex.
*/
typedef struct  OsalMutexContext_t*         OsalMutex_t;

/**
 *  @brief                                  Prototype for user provided functions.
*/
typedef void    (*OsalProcedure_t)          (void);

/**
 *  @brief                                  Prototype for user provided task functions.
*/
typedef void    (*OsalRunnable_t)           (OsalThread_t handle);

/**
 *  @brief                                  Prototype for printf like generic logger.
*/
typedef void    (*OsalGenericLogger_t)      (const char* fmt, ...);

typedef enum
{
    OsalErrOk,                              // Success.
    OsalErrParam,                           // Incorrect parameter.
    OsalErrConfig,                          // Invalid config.
    OsalErrMemory,                          // Not sufficient free memory.
    OsalErrNotStarted,                      // Operation cannot complete as it requires OS running.
    OsalErrTimeout,                         // Timed operation timed out.
    OsalErrForbidden,                       // Incorrect usage.
    OsalErrPort,                            // Lower level error.
    OsalErrUnexpected,                      // Unexpected.
    OsalErrMax
} OsalErr_n;

typedef enum
{
    OsalThreadPriorityLeast = 0,
    OsalThreadPriority_0 = OsalThreadPriorityLeast,
    OsalThreadPriority_1,
    OsalThreadPriority_2,
    OsalThreadPriority_3,
    OsalThreadPriority_4,
    OsalThreadPriority_5,
    OsalThreadPriority_6,
    OsalThreadPriority_7,
    OsalThreadPriority_8,
    OsalThreadPriority_9,
    OsalThreadPriorityMost = OsalThreadPriority_9,
    OsalThreadPriorityMax
} OsalThreadPriority_n;

typedef struct
{
    OsalRunnable_t fnInit;                  // One-time functionality.
    OsalRunnable_t fnPoll;                  // Periodic functionality.
    OsalThreadPriority_n priority;          // Priority.
    uint32_t stackSize;                     // Stack size in bytes.
    uint32_t periodicityMs;                 // Period at which task should execute (0 means continuous).
    const char* name;                       // Task name (optional).
} OsalThreadConfig_t;

typedef struct
{
    uint32_t countLoops;                    // Number of times loop executed.
    uint32_t msExec;                        // Total execute time for poll function till now.
    uint32_t msExecSecond;                  // Average exec time for last second.
    uint32_t msExecMinute;                  // Average exec time for last minute.
    uint32_t msExecHour;                    // Average exec time for last hour.
    uint32_t msExecDay;                     // Average exec time for last day.
} OsalThreadStats_t;

#endif /* OSAL_TYPES_H */
