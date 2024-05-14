/**
 * @file        osal_mutex.h
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
 * @brief       OSAL Mutex abstraction - header.
 */

#ifndef OSAL_MUTEX_H
#define OSAL_MUTEX_H

#include "osal_types.h"

/**
 *  @brief                                  A global init for OSAL Mutex.
 *                                          All available mutex are internally allocated and initialized here.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_mutex_global_init            (void);

/**
 *  @brief                                  Creates a mutex.
 *  @param  ptrMutex                        Pointer to handle where created mutex will be updated into.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_mutex_create                 (OsalMutex_t* ptrMutex);

/**
 *  @brief                                  Destroys a mutex.
 *  @param  mutex                           Handle to a previously created mutex.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_mutex_destroy                (OsalMutex_t mutex);

/**
 *  @brief                                  Takes mutex.
 *  @param  mutex                           Handle to a previously created mutex.
 *  @param  timeoutMs                       Maximum time to wait for taking lock.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_mutex_take_timed             (OsalMutex_t mutex, uint32_t timeoutMs);

/**
 *  @brief                                  Takes mutex.
 *  @param  mutex                           Handle to a previously created mutex.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
 *  @note                                   This version of API blocks on the semaphore forever until lock is taken.
*/
OsalErr_n osal_mutex_take                   (OsalMutex_t mutex);

/**
 *  @brief                                  Gives previously taken mutex.
 *  @param  mutex                           Handle to a previously created and currently taken mutex.
 *  @return                                 OsalErrOk on success, else one of the defined error codes.
*/
OsalErr_n osal_mutex_give                   (OsalMutex_t mutex);

#endif /* OSAL_MUTEX_H */
