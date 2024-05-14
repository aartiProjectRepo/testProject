/**
 * @file        tcu_time.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        11 December 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU time provider - header
 * @note        Atleast one of the API should be called before underlying counter restarts.
 *              For example, for 1us counter, API should be called before 1.19 hours elapses, else overlap will not be handled.
 * 
 * // TODO: Add locks since API touches static globals in a non-threadsafe way.
 * // 
 */

#ifndef TCU_TIME_H
#define TCU_TIME_H

#include <stdint.h>

typedef struct tcu_time
{
    uint32_t seconds;                       // Seconds part of time-point.
    uint32_t fractional;                    // Sub-second factional of time-point (in micro-seconds).
} TcuTime_t;

/**
 * @brief                                   Init the timer 
*/
void tcu_time_init                          (void);

/**
 * @brief                                   Gives the uptime since boot.
 * @return                                  Uptime since boot.
 * @warning                                 Updates 
*/
TcuTime_t tcu_time_uptime                   (void);

/**
 * @brief                                   Gives the epoch time.
 * @return                                  Epoch time.
 * @note                                    Epoch time will be same as uptime time until non-zero epoch offset is provided.
*/
TcuTime_t tcu_time_epoch                    (void);

/**
 * @brief                                   Sets new epoch offset.
 * @param   epochOffset                     An externally received epoch.
 * @note                                    If zero-epoch offset is set, epoch time becomes same as uptime.
 */
void tcu_time_set_epoch                     (TcuTime_t epochOffset);


#endif /* TCU_TIME_H */