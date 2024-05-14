/**
 * @file        osal_private.c
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
 * @brief       OSAL private data shared across sub-modules - data.
 */

#include "osal_private.h"

volatile uint32_t g_osal_initialized = OSAL_FALSE;
volatile uint32_t g_osal_running = OSAL_FALSE;
volatile SemaphoreHandle_t g_osal_semaphore = NULL;

void osal_private_lock                      (void)
{
    if ( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
    {
        if ( g_osal_semaphore )
        {
            xSemaphoreTake ( g_osal_semaphore, portMAX_DELAY );
        }
    }
}

void osal_private_unlock                    (void)
{
    if ( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
    {
        if ( g_osal_semaphore )
        {
            xSemaphoreGive ( g_osal_semaphore );
        }
    }
}
