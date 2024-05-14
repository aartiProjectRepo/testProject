/**
 * @file        tcu_board.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        09 December 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU board global include.
 */

#ifndef TCU_BOARD_H
#define TCU_BOARD_H

#include "hal_util.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_spi.h"

// HAL interfaces for board.
extern volatile HalGpioHandle_t g_GpioLed1;
extern volatile HalGpioHandle_t g_GpioLed2;
extern volatile HalGpioHandle_t g_GpioIgn;
extern volatile HalUartHandle_t g_UartGps;
extern volatile HalUartHandle_t g_UartGsm;
extern volatile HalUartHandle_t g_UartDbg;
extern volatile HalUartHandle_t g_UartDbg;
extern volatile HalSpiHandle_t g_SpiEeprom;
extern volatile HalSpiHandle_t g_SpiAccel;
extern volatile HalSpiHandle_t g_SpiFlash;

/**
 * @brief   Initializes the board.
*/
void tcu_board_init(void);

#endif /* TCU_BOARD_H */
