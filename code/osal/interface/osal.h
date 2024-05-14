/**
 * @file        osal.h
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
 * @brief       OSAL abstraction - header.
 */

#ifndef OSAL_H
#define OSAL_H

#include "osal_types.h"

/**
 *  @brief                                  A global init for OSAL.
 *                                          Sets up the necessary environment for correct functioning of OSAL.
 *                                          This function must be called before using any OSAL features!
 *  @param  system_logger                   A system wide logger function.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_global_init                  (OsalGenericLogger_t system_logger);

/**
 *  @brief                                  Starts the OSAL scheduler.
 *                                          Threads 'created' before this will actually begin execution after this call.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_start                        (void);

/**
 *  @brief                                  Does service related stuff (normally placed in the service thread)
 *                                          Thread Logger feature requires this function.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_service_execute              (void);

/**
 *  @brief                                  Creates a thread with given configuration.
 *  @param  ptrHandle                       The created thread handle will be updated into this upon success.
 *  @param  config                          Configuration.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_thread_create                (OsalThread_t* ptrHandle, const OsalThreadConfig_t* config);

/**
 *  @brief                                  Destroys a previously created thread.
 *  @param  handle                          Handle to previously created thread.
 *  @return                                 OsalErrOk on success, else one of the defined error codes. 
*/
OsalErr_n osal_thread_destroy               (OsalThread_t handle);

/**
 *  @brief                                  Gets stat for a previously created thread.
 *  @param  handle                          Handle to previously created thread.
 *  @param  stats                           Latest stats will be set into this.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_thread_get_stats             (OsalThread_t handle, OsalThreadStats_t* stats);

/**
 *  @brief                                  Gets logger for a previously created thread.
 *  @param  handle                          Handle to previously created thread.
 *  @param  fmt                             Variadic arguments (printf like function).
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_logger                       (OsalThread_t handle, const char* fmt, ...);

/**
 *  @brief                                  Sets state variable for periodic execute timer.
 *  @param  currentTick                     Variable to initialize.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_tmr_init                     (uint32_t* currentTick);

/**
 *  @brief                                  Periodically executes a function.
 *  @param  execute                         Procedural function to execute.
 *  @param  periodMs                        Periodicity in miliseconds.
 *  @param  lastTick                        Pointer to tick value (state variable whose memory is managed by caller).
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_tmr_exec                     (OsalProcedure_t execute, uint32_t periodMs, uint32_t* lastTick);

/**
 *  @brief                                  OSAL delay in miliseconds.
 *  @param  delayMs                         Delay time in miliseconds.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_delay                        (uint32_t delayMs);

/**
 *  @brief                                  OSAL tick getter.
 *  @param  tick                            Latest tick will be updated into this.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_tick_get                     (uint32_t* tick);

#endif /* OSAL_H */
