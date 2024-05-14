/**
 * @file        hal_util.h
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
 * @brief       HAL common utilities - header.
 */

#ifndef HAL_UTIL_H
#define HAL_UTIL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define hal_util_assert(e)                  ((e) ? (void)0 : abort())

void hal_util_inc_mem_create                (uint8_t* buf, size_t bufSize, uint8_t incrementingSeed);
bool hal_util_inc_mem_check                 (uint8_t* buf, size_t bufSize, uint8_t incrementingSeed);
void hal_util_static_mem_create             (uint8_t* buf, size_t bufSize, uint8_t byte);
bool hal_util_static_mem_check              (uint8_t* buf, size_t bufSize, uint8_t byte);
void hal_util_delay                         (uint32_t delayMs);
void hal_util_delay_us                      (uint32_t delayUs);
void* hal_util_memset                       (void *dest, int byte, size_t sizeDest);
void* hal_util_memcpy                       (void *dest, const void *src, size_t sizeDest);
size_t hal_util_strncpy                     (char* dest, const char* src, size_t sizeDest);
void abort                                  (void);

#endif /* HAL_UTIL_H */
