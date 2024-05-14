/**
 * @file        osal_private.h
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
 * @brief       OSAL private data shared across sub-modules.
 */

#ifndef OSAL_PRIVATE_H
#define OSAL_PRIVATE_H

#include "osal_types.h"
#include "osal_mutex.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "hal_util.h"
#include "printf.h"

extern volatile uint32_t g_osal_initialized;
extern volatile uint32_t g_osal_running;
extern volatile SemaphoreHandle_t g_osal_semaphore;

/**
 *  @brief                                  Used internally for OSAL overall thread safety - takes lock.
*/
void osal_private_lock                      (void);

/**
 *  @brief                                  Used internally for OSAL overall thread safety - releases lock.
*/
void osal_private_unlock                    (void);

#endif /* OSAL_PRIVATE_H */
