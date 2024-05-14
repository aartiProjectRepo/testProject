/**
 * @file        tcu_2w_util.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        27 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU-2W common utilities - implementation.
 */

// Self include.
#include "hal_util.h"

// Dependencies.
/**
 * FIXME: Shouldn't entangle OSAL with HAL.
 * If a 'smart' delay is to be satisfied by this module, one should inject the 'smart' delay function.
 * In my opinion, there should be no 'smart' delay in HAL, just blocking delays in 'usec' and 'msec'.
 * Individual modules should either use HAL delay or OSAL delay based on their circumstantial requirements.
*/
#include "osal.h"

void hal_util_inc_mem_create                (uint8_t* buf, size_t bufSize, uint8_t incrementingSeed)
{
    while ( bufSize-- )
    {
        *buf = incrementingSeed++;
        buf++;
    }
}

bool hal_util_inc_mem_check                 (uint8_t* buf, size_t bufSize, uint8_t incrementingSeed)
{
    bool success = true;

    while ( bufSize-- )
    {
        if ( *buf != incrementingSeed++ )
        {
            success = false;
            break;
        }
        buf++;
    }

    return success;
}

void hal_util_static_mem_create             (uint8_t* buf, size_t bufSize, uint8_t byte)
{
    while ( bufSize-- )
    {
        *buf = byte;
        buf++;
    }
}

bool hal_util_static_mem_check              (uint8_t* buf, size_t bufSize, uint8_t byte)
{
    bool success = true;

    while ( bufSize-- )
    {
        if ( *buf != byte )
        {
            success = false;
            break;
        }
        buf++;
    }

    return success;
}

void hal_util_delay                         (uint32_t delayMs)
{
    const uint32_t CpuFreq = 15UL * 1000UL * 1000UL;
    const uint32_t Divider = 1;
    uint32_t loopCount;

    if ( OsalErrOk != osal_delay(delayMs) )
    {
        loopCount = ( ( CpuFreq / Divider ) / 1000UL ) * delayMs;
        while ( loopCount-- ) {
            __nop();
        }
    }
}

void hal_util_delay_us                      (uint32_t delayUs)
{
    const uint32_t CpuFreq = 15UL * 1000UL * 1000UL;
    const uint32_t Divider = 1;
    uint32_t loopCount = ( ( CpuFreq / Divider ) / ( 1000UL * 1000UL ) ) * delayUs;
    while ( loopCount-- ) {
        __nop();
    }
}

void* hal_util_memset                       (void* dest, int byte, size_t sizeDest)
{
    size_t index = 0;
    uint8_t* ptr = (uint8_t*) dest;

    while ( index < sizeDest )
    {
        ptr[index++] = byte;
    }

    return dest;
}

void* hal_util_memcpy                       (void *dest, const void *src, size_t sizeDest)
{
    size_t index = 0;
    uint8_t* ptr = (uint8_t*) dest;
    uint8_t* ptrSrc = (uint8_t*) src;

    while ( index < sizeDest )
    {
        ptr[index++] = *(ptrSrc++);
    }

    return dest;
}

size_t hal_util_strncpy                     (char* dest, const char* src, size_t sizeDest)
{
    size_t idx = 0;

    while ( sizeDest-- )
    {
        if ( src[idx] )
        {
            dest[idx] = src[idx];
        }
        else
        {
            break;
        }
        idx++;
    }

    return idx;
}

/**
 *  @brief                                  This function satisfies dependency for assert(..) from standard library header 'assert.h'
*/
void abort(void)
{
    __nop();
    while (1)
    {
        __nop();
    }
}
