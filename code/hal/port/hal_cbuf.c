/**
 * @file        hal_cbuf.c
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
 * @brief       Circular buffer implementation.
 *
 * @note        One element of the circular buffer capacity is 'wasted' for easy disambiguation between queue-full and queue-empty.
 *              Hence, the usable capacity of the queue is always one less than the actual capacity of the backing buffer.
 *              For example, if backing buffer of size 100 is provided, the queue will be able to accommodate only 99 elements at best.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal_buffer.h"
#include "hal_cbuf.h"
#include "hal_util.h"

#define HAL_CBUF_ENABLE_ASSERTS             ( 1 )
#define HAL_CBUF_ENABLE_PARAM_CHECK         ( 1 )

/**
 *  @brief                                  Performs a 'circular next' from current index.
 *  @param      now                         The queue index from where to begin.
 *  @param      step                        The number of steps to jump.
 *  @param      capacity                    The capacity of backing buffer (i.e. size of backing buffer).
 *  @return                                 The index after performing circular next.
*/
static size_t circular_next                 (volatile size_t now, volatile size_t step, volatile size_t capacity);

/**
 *  @brief                                  Tells number of elements that are already present.
 *  @param      front                       Current front (index) of the queue.
 *  @param      rear                        Current rear (index) of the queue.
 *  @param      capacity                    The capacity of backing buffer (i.e. size of backing buffer).
 *  @return                                 The number of elements that are already present.
*/
static size_t available_read                (volatile size_t front, volatile size_t rear, volatile size_t capacity);

/**
 *  @brief                                  Tells max number of elements that can be written.
 *  @param      front                       Current front (index) of the queue.
 *  @param      rear                        Current rear (index) of the queue.
 *  @param      capacity                    The capacity of backing buffer (i.e. size of backing buffer).
 *  @return                                 Max number of elements that can be written.
*/
static size_t available_write               (volatile size_t front, volatile size_t rear, volatile size_t capacity);

HalCbufErr_n hal_cbuf_init                  (volatile HalCbuf_t* ptrCbuf)
{
    volatile size_t i = 0;
    volatile HalCbufErr_n err = HalCbufErrParam;
    
    #if HAL_CBUF_ENABLE_PARAM_CHECK
    if ( ptrCbuf && ptrCbuf->mem && ptrCbuf->sizeMem )
    {
    #endif
        ptrCbuf->front = 0;
        ptrCbuf->rear = 0;
        for ( i = 0 ; i < ptrCbuf->sizeMem ; ++i )
        {
            ptrCbuf->mem[i] = 0;
        }
        err = HalCbufErrOk;
    #if HAL_CBUF_ENABLE_PARAM_CHECK
    }
    #endif

    return err;
}

HalCbufErr_n hal_cbuf_enqueue               (volatile HalCbuf_t* ptrCbuf, volatile HalBuffer_t buffer, volatile size_t reqEnqueue)
{
    volatile HalCbufErr_n err = HalCbufErrParam;
    volatile size_t i = 0;
    volatile size_t rear = 0;
    volatile size_t capacity  = 0;
    volatile size_t availableToWrite = 0;
    volatile size_t newRear = 0;

    #if HAL_CBUF_ENABLE_PARAM_CHECK
    if ( ptrCbuf && ptrCbuf->mem && ptrCbuf->sizeMem && ( ptrCbuf->front < ptrCbuf->sizeMem ) && ( ptrCbuf->rear < ptrCbuf->sizeMem ) )
    {
        if ( buffer.mem && buffer.sizeMem && reqEnqueue && ( reqEnqueue < ptrCbuf->sizeMem ) && ( reqEnqueue <= buffer.sizeMem ) )
        {
    #endif
            rear = ptrCbuf->rear;
            capacity = ptrCbuf->sizeMem;
            availableToWrite = available_write(ptrCbuf->front, rear, capacity);
            if ( reqEnqueue <= availableToWrite )
            {
                newRear = circular_next(rear, reqEnqueue, capacity);
                #if HAL_CBUF_ENABLE_ASSERTS == 1
                hal_util_assert ( newRear < capacity );
                hal_util_assert ( newRear != rear );
                #endif
                do
                {
                    ptrCbuf->mem[rear] = buffer.mem[i++];
                    rear = circular_next(rear, 1, capacity);
                } while ( rear != newRear );
                ptrCbuf->rear = rear;
                err = HalCbufErrOk;
            }
            else
            {
                err = HalCbufErrForbidden;
            }
    #if HAL_CBUF_ENABLE_PARAM_CHECK
        }
    }
    #endif

    return err;
}

HalCbufErr_n hal_cbuf_dequeue               (volatile HalCbuf_t* ptrCbuf, volatile HalBuffer_t buffer, volatile size_t reqDequeue)
{
    volatile HalCbufErr_n err = HalCbufErrParam;
    volatile size_t i = 0;
    volatile size_t front = 0;
    volatile size_t capacity  = 0;
    volatile size_t availableToRead = 0;
    volatile size_t newFront = 0;

    #if HAL_CBUF_ENABLE_PARAM_CHECK
    if ( ptrCbuf && ptrCbuf->mem && ptrCbuf->sizeMem && ( ptrCbuf->front < ptrCbuf->sizeMem ) && ( ptrCbuf->rear < ptrCbuf->sizeMem ) )
    {
        if ( buffer.mem && buffer.sizeMem && reqDequeue && ( reqDequeue < ptrCbuf->sizeMem ) && ( reqDequeue <= buffer.sizeMem ) )
        {
    #endif
            front = ptrCbuf->front;
            capacity = ptrCbuf->sizeMem;
            availableToRead = available_read(front, ptrCbuf->rear, capacity);
            if ( reqDequeue <= availableToRead )
            {
                newFront = circular_next(front, reqDequeue, capacity);
                #if HAL_CBUF_ENABLE_ASSERTS == 1
                hal_util_assert ( newFront < capacity );
                hal_util_assert ( newFront != front );
                #endif
                do
                {
                    buffer.mem[i++] = ptrCbuf->mem[front] ;
                    front = circular_next(front, 1, capacity);
                } while ( front != newFront );
                ptrCbuf->front = front;
                err = HalCbufErrOk;
            }
            else
            {
                err = HalCbufErrForbidden;
            }
    #if HAL_CBUF_ENABLE_PARAM_CHECK
        }
    }
    #endif

    return err;
}

HalCbufErr_n hal_cbuf_available_read        (volatile HalCbuf_t* ptrCbuf, volatile size_t* availableToRead)
{
    volatile HalCbufErr_n err = HalCbufErrParam;

    #if HAL_CBUF_ENABLE_PARAM_CHECK
    if ( ptrCbuf && ptrCbuf->mem && ptrCbuf->sizeMem && ( ptrCbuf->front < ptrCbuf->sizeMem ) && ( ptrCbuf->rear < ptrCbuf->sizeMem ) && availableToRead )
    {
    #endif
        *availableToRead = available_read(ptrCbuf->front, ptrCbuf->rear, ptrCbuf->sizeMem);
        err = HalCbufErrOk;
    #if HAL_CBUF_ENABLE_PARAM_CHECK
    }
    #endif

    return err;
}

HalCbufErr_n hal_cbuf_available_write       (volatile HalCbuf_t* ptrCbuf, volatile size_t* availableToWrite)
{
    volatile HalCbufErr_n err = HalCbufErrParam;

    #if HAL_CBUF_ENABLE_PARAM_CHECK
    if ( ptrCbuf && ptrCbuf->mem && ptrCbuf->sizeMem && ( ptrCbuf->front < ptrCbuf->sizeMem ) && ( ptrCbuf->rear < ptrCbuf->sizeMem ) && availableToWrite )
    {
    #endif
        *availableToWrite = available_write(ptrCbuf->front, ptrCbuf->rear, ptrCbuf->sizeMem);
        err = HalCbufErrOk;
    #if HAL_CBUF_ENABLE_PARAM_CHECK
    }
    #endif

    return err;
}

static size_t circular_next                 (volatile size_t now, volatile size_t step, volatile size_t capacity)
{
    #if HAL_CBUF_ENABLE_ASSERTS
    hal_util_assert ( step < capacity );
    hal_util_assert ( now  < capacity );
    #endif

    return ( ( now + step ) % capacity );
}

static size_t available_read                (volatile size_t front, volatile size_t rear, volatile size_t capacity)
{
    #if HAL_CBUF_ENABLE_ASSERTS
    hal_util_assert ( front < capacity );
    hal_util_assert ( rear  < capacity );
    #endif

    return ( capacity - 1 - ( ( capacity - rear + front - 1 ) % capacity ) );
}

static size_t available_write               (volatile size_t front, volatile size_t rear, volatile size_t capacity)
{
    #if HAL_CBUF_ENABLE_ASSERTS
    hal_util_assert ( front < capacity );
    hal_util_assert ( rear  < capacity );
    #endif

    return ( capacity - rear + front - 1 ) % capacity;
}
