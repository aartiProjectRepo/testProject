/**
 * @file        TODO.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        25 October 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TODO
 */

#include "port_defs.h"

typedef enum
{
    RH850Intc2UartTransmit,     // 'UR0'
    RH850Intc2UartReceive,      // 'UR1'
    RH850Intc2UartStatus,       // 'UR2'
    RH850Intc2UartMax
} RH850Intc2Uart_n;

void rh850_intc2_uart_init                  (PortDefsUart_n portDefUart);
void rh850_intc2_uart_enable                (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType);
void rh850_intc2_uart_disable               (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType);
void rh850_intc2_uart_set                   (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType);
void rh850_intc2_uart_clear                 (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType);
