/**
 * @file        network_port.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        03 November 2023
 * @author      Aditya P <aditya.prajapati@accoladeelectronics.com>
 *              Diksha J <diksha.jadhav@accoladeelectronics.com>
 *
 * @brief       File contains Network Read and Network Write operation that are required for porting
 */

// Standard includes.
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Port includes.
#include "modem_port.h"
#include "Config_PORT.h"    // FIXME: replace with hal_gpio
#include "tcu_board.h"

void ModemInit()
{
    ModemPwrKeyHigh();
    ModemRstKeyHigh();
 }

uint32_t ModemRead(uint8_t *rxBuff, uint32_t maxBuffSize)
{
    size_t readByte = 0;
    HalBuffer_t buf = {rxBuff, maxBuffSize};
    
    // FIXME: Return code not handled.
    hal_uart_receive_available(g_UartGsm, &readByte);
    if (readByte)
    {
        if (readByte > buf.sizeMem)
        {
            readByte = buf.sizeMem;
        }
        // FIXME: Return code not handled.
        hal_uart_read(g_UartGsm, buf, readByte);
        return readByte;
    }

    return 0;
}

uint32_t ModemWrite(uint8_t *txBuff, uint32_t txLen)
{
    size_t writeByte = 0;
    HalBuffer_t buf = {txBuff, txLen};
    
    // FIXME: Return code not handled.
    hal_uart_transmit_available(g_UartGsm, &writeByte);
    if (writeByte)
    {
        if (writeByte > buf.sizeMem)
        {
            writeByte = buf.sizeMem;
        }
        // FIXME: Return code not handled.
        hal_uart_write(g_UartGsm, buf, writeByte);
        return writeByte;
    }

    return 0;
}

uint32_t ModemDataReady(void)
{
    uint32_t readByte = 1;
    
    if (HalUartErrOk == hal_uart_receive_available(g_UartGsm, (size_t *)&readByte) && (0 < readByte))
    {
        return 1;
    }

    return 0;
}

void ModemPwrKeyHigh(void)
{
    R_PORT_SetGpioOutput(Port9,Pin_1,High);
}

void ModemPwrKeyLow(void)
{
    R_PORT_SetGpioOutput(Port9,Pin_1,Low);
}

void ModemRstKeyHigh(void)
{
    R_PORT_SetGpioOutput(Port9,Pin_2,High);
}

void ModemRstKeyLow(void)
{
    R_PORT_SetGpioOutput(Port9,Pin_2,Low);
}
