/**
 * @file        rh850_uart.h
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
 * @brief       R8580 port required API for HAL UART implementation.
 *              Restriction: Each UART port may be created only once.
 */

#ifndef RH850_UART_H
#define RH850_UART_H

#include "hal_uart.h"
#include "port_defs.h"

/**
 *  @brief                                  Get's identity mapped to specific RH850 physical UART's port define.
 *  @param      identityPtr                 On success, user provided the identity pointer will be populated with identity corresponding to port.
 *  @param      portDefsUart                One of the values in port defines.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                                                      If identityPtr does not point to a pointer having NULL value.
 *                                                                      If portDefsUart is not in valid range of port defines.
*/
HalUartErr_n rh850_uart_get_identity        (HalUartIdentity_t* identityPtr, PortDefsUart_n portDefsUart);

/**
 *  @brief                                  A generic RX handler to be placed in the ISR of corresponding UART.
 *  @param      handle                      A valid handle to UART.
*/
void rh850_uart_isr_handler_rx              (HalUartHandle_t handle);

#endif /* RH850_UART_H */
