/**
 * @file        hal_buffer.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        30 October 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Buffer definition.
 */

#ifndef HAL_BUFFER_H
#define HAL_BUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t* mem;                                   /* Pointer to an allocated memory area for buffer. */
    size_t sizeMem;                                 /* Size of memory allocated for the buffer. */
} HalBuffer_t;

#endif /* HAL_BUFFER_H */
