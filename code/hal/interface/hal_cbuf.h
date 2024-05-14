/**
 * @file        hal_cbuf.h
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
 * @brief       Circular buffer definitions and API.
 * 
 * @note        This circular buffer is meant to be implemented to be producer-consumer thread-safe only.
 *              i.e. only two threads are possible, one of which is producer and another one is consumer.
 */

#ifndef HAL_CBUF_H
#define HAL_CBUF_H

#include <stddef.h>
#include <stdint.h>
#include "hal_buffer.h"

typedef struct 
{
    uint8_t* const mem;                     /* Pointer to an allocated memory area for buffer. */
    const size_t sizeMem;                   /* Size of memory allocated for the buffer. */
    volatile size_t front;                  /* Front (data source for reader, index relative to buffer start). */
    volatile size_t rear;                   /* Rear (data sink for writer, index relative to buffer start). */
} HalCbuf_t;

typedef enum
{
    HalCbufErrOk,                           /* Operation success. */
    HalCbufErrParam,                        /* Parameter error. */
    HalCbufErrForbidden                     /* Operation not permitted as per design. */
} HalCbufErr_n;

/**
 *  @brief                                  Initialize (resets) the circular buffer elements.
 *  @param      ptrCbuf                     Pointer to circular buffer.
 *  @return                                 HalCbufErrOk        - Success.
 *                                          HalCbufErrParam     - NULL pointer or zero sized buffer or other parameter inconsistency.
*/
HalCbufErr_n hal_cbuf_init                  (volatile HalCbuf_t* ptrCbuf);

/**
 *  @brief                                  Enqueues the requested number of elements into circular buffer.
 *  @param      ptrCbuf                     Pointer to circular buffer.
 *  @param      buffer                      A buffer containing data to be enqueued.
 *  @param      reqEnqueue                  The number of elements to be enqueued.
 *  @return                                 HalCbufErrOk        - Success.
 *                                          HalCbufErrParam     - NULL pointer or zero sized buffer or other parameter inconsistency.
 *                                          HalCbufErrForbidden - Not enough free space in circular buffer for requested number of elements. Prevents overflow.
*/
HalCbufErr_n hal_cbuf_enqueue               (volatile HalCbuf_t* ptrCbuf, volatile HalBuffer_t buffer, volatile size_t reqEnqueue);

/**
 *  @brief                                  Dequeues the requested number of elements from circular buffer.
 *  @param      ptrCbuf                     Pointer to circular buffer.
 *  @param      buffer                      A buffer to which data will be dequeued.
 *  @param      reqEnqueue                  The number of elements to be dequeued.
 *  @return                                 HalCbufErrOk        - Success.
 *                                          HalCbufErrParam     - NULL pointer or zero sized buffer or other parameter inconsistency.
 *                                          HalCbufErrForbidden - Requested number of elements not present in circular buffer. Prevents underflow.
*/
HalCbufErr_n hal_cbuf_dequeue               (volatile HalCbuf_t* ptrCbuf, volatile HalBuffer_t buffer, volatile size_t reqDequeue);

/**
 *  @brief                                  Tells how many elements are present and thus can be read in circular buffer.
 *  @param      ptrCbuf                     Pointer to circular buffer.
 *  @param      availableToRead             When params are correct, the number of elements possible to read will be updated into this pointer.
 *  @return                                 HalCbufErrOk        - Success.
 *                                          HalCbufErrParam     - NULL pointer or zero sized buffer or other parameter inconsistency.
*/
HalCbufErr_n hal_cbuf_available_read        (volatile HalCbuf_t* ptrCbuf, volatile size_t* availableToRead);

/**
 *  @brief                                  Tells how many elements can yet be added into the circular buffer.
 *  @param      ptrCbuf                     Pointer to circular buffer.
 *  @param      availableToWrite            When params are correct, the number of elements possible to write will be updated into this pointer.
 *  @return                                 HalCbufErrOk        - Success.
 *                                          HalCbufErrParam     - NULL pointer or zero sized buffer or other parameter inconsistency.
*/
HalCbufErr_n hal_cbuf_available_write       (volatile HalCbuf_t* ptrCbuf, volatile size_t* availableToWrite);

#endif /* HAL_CBUF_H */
