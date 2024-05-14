/**
 * @file        logger.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        01 January 2024
 * @author      Aditya P <aditya.prajapati@accoladeelectronics.com>
 *              Diksha J <diksha.jadhav@accoladeelectronics.com>
 *              Adwait P <adwait.patil@accoladeelectronics.com>
 * 
 * @brief       Logger to print logs to Debug UART.
 */

// Standard includes.
#include <string.h>
#include <stdarg.h>
#include "printf.h"

// Self.
#include "logger.h"

// Port includes.
#include "tcu_board.h"
#include "hal_uart.h"

static uint8_t g_mem[1024];

void logger                                 (const char *fmt, ...)
{
    HalUartErr_n uartErr;
    HalBuffer_t buffer = {g_mem, sizeof(g_mem)};
    size_t bytesToWrite;
    size_t availableTxCapacity;

    uartErr = hal_uart_transmit_available(g_UartDbg, &availableTxCapacity);
    if ( HalUartErrOk == uartErr )
    {
        va_list args;
        va_start(args, fmt);
        bytesToWrite = vsnprintf((char*)g_mem, sizeof(g_mem), fmt, args);
        va_end(args);
        if ( bytesToWrite > sizeof(g_mem) )
        {
            bytesToWrite = sizeof(g_mem);
        }
        if ( bytesToWrite > availableTxCapacity )
        {
            bytesToWrite = availableTxCapacity;
        }
        if ( bytesToWrite )
        {
            uartErr = hal_uart_write(g_UartDbg, buffer, bytesToWrite);
        }
    }
    
    return;
}

uint32_t logger_read                        (uint8_t *rxBuff, uint32_t maxBuffSize)
{
    HalUartErr_n uartErr;
    size_t readBytes = 0;
    HalBuffer_t buf = {rxBuff, maxBuffSize};

    uartErr = hal_uart_receive_available(g_UartDbg, &readBytes);
    if ( ( HalUartErrOk == uartErr ) && readBytes )
    {
        if ( readBytes > buf.sizeMem )
        {
            readBytes = buf.sizeMem;
        }
        (void) hal_uart_read(g_UartDbg, buf, readBytes);
    }

    return readBytes;
}
