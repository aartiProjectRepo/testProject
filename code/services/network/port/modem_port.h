/**
 * @file        network_port.h
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
 * @brief       Network port headers
 * 
 * @version     v0.1    03 Nov 2023
 * @details     - Created
 * 
 * // FIXME: Should follow public API naming conventions.
 */

#ifndef MODEM_PORT_H
#define MODEM_PORT_H

#include "net_log.h"
#include "osal.h"

#define INFO_EN
#define DEBUG_EN
#define TRACE_EN

// FIXME: This is not network functionality, move to proper library (also semantics require review).
extern uint32_t g_var_sys;
#define IS_TIMER_ELAPSED(tmr)               ( ( g_var_sys ) > ( tmr ) )
#define RESET_TIMER(tmr, time)              ( ( tmr ) = ( ( g_var_sys ) + ( time ) ) )

#ifdef INFO_EN
#define NETWORK_PRINT_INFO(...)             net_log(##__VA_ARGS__)
#else
#define NETWORK_PRINT_INFO(...)
#endif
#ifdef DEBUG_EN
#define NETWORK_PRINT_DEBUG(...)            net_log(##__VA_ARGS__)
#else
#define NETWORK_PRINT_DEBUG(...)
#endif
#ifdef TRACE_EN
#define NETWORK_PRINT_TRACE(...)            net_log(##__VA_ARGS__)
#else
#define NETWORK_PRINT_TRACE(...)
#endif
#define NETWORK_PRINT_ERROR(...)            net_log(##__VA_ARGS__)

/**
 * @brief Initializes the network (UART GSM port).
 */
void ModemInit();

/**
 * @brief Reads characters from the GSM port into the provided buffer.
 * @param rxBuff Pointer to the buffer to store received characters.
 * @param maxbuffsize Maximum size of the buffer.
 * 
 */
uint32_t ModemRead(uint8_t *rxBuff, uint32_t maxbuffsize);

/**
 * @brief Writes characters to the GSM port.
 * @param txBuff Pointer to the buffer containing characters to be transmitted.
 * @param txLen Number of characters to transmit.
 * 
 */
uint32_t ModemWrite(uint8_t *txBuff, uint32_t txLen);

/**
 * @brief Checks if data is ready to be read from the GSM port.
 */
uint32_t ModemDataReady();

/**
 * @brief Sets the power key (P9_1) of EC200 to a high state.
 */
void ModemPwrKeyHigh();

/**
 * @brief Sets the power key (P9_1) of EC200 to a low state.
 */
void ModemPwrKeyLow();

/**
 * @brief Sets the reset key (P9_2) of EC200 to a high state.
 */
void ModemRstKeyHigh();

/**
 * @brief Sets the reset key (P9_2) of EC200 to a low state.
 */
void ModemRstKeyLow();

#endif /* MODEM_PORT_H */
