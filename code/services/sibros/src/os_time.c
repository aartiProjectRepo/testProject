/**
 * @file        os_time.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        25 November 2023
 * @author      Pranali <pranali.kalekar@accoladeelectronics.com>
 *
 * @brief       SO time provider functionalities related to time-keeping.
 */

// Standard includes.
#include <stdbool.h>
#include <stdint.h>

// Self include.
#include "os_time.h"

// Platform includes.
#include "tcu_time.h"

uint32_t os_time__get_time_ms(void)
{
    uint32_t osTime;
    TcuTime_t uptime = tcu_time_uptime();
    osTime = (uptime.seconds * 1000 + uptime.fractional / 1000);
    return osTime;
}

uint32_t os_time__get_time_us(void)
{
    TcuTime_t uptime = tcu_time_uptime();
    return uptime.fractional;
}

uint32_t os_time__get_time_elapsed_ms(uint32_t past_time_ms)
{
    return (os_time__get_time_ms() - past_time_ms);
}

bool os_time__get_epoch_time(os_time__epoch_time_s *epoch_time)
{
    epoch_time->seconds = os_time__get_time_ms() / 1000;
    epoch_time->nanoseconds = (os_time__get_time_ms() % 1000) * 1000000;
    return true;
}

uint64_t os_time__get_epoch_in_ms(os_time__epoch_time_s epoch)
{
    return epoch.seconds * 1000 + epoch.nanoseconds / 1000000;
}

os_time__epoch_time_s os_time__get_time_with_offset(os_time__epoch_time_s offset_to_add)
{
    os_time__epoch_time_s current_epoch_time;
    if (!os_time__get_epoch_time(&current_epoch_time))
    {
        // Handle error getting current time
        return current_epoch_time; // Return current time as is in case of error
    }
    // Add the offset to the current time
    current_epoch_time.seconds += offset_to_add.seconds;
    current_epoch_time.nanoseconds += offset_to_add.nanoseconds;
    return current_epoch_time;
}

os_time__epoch_time_s os_time__sum_epochs(os_time__epoch_time_s a, os_time__epoch_time_s b)
{
    os_time__epoch_time_s result_epoch_time;
    result_epoch_time.seconds = a.seconds + b.seconds;
    result_epoch_time.nanoseconds = a.nanoseconds + b.nanoseconds;
    // Adjust for overflow
    if (result_epoch_time.nanoseconds >= 1000000000)
    {
        result_epoch_time.seconds += result_epoch_time.nanoseconds / 1000000000;
        result_epoch_time.nanoseconds %= 1000000000;
    }
    return result_epoch_time;
}
