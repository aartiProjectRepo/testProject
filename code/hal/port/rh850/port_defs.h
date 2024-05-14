/**
 * @file        port_defs.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        19 October 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Port definitions.
 */

#ifndef PORT_DEFS_H
#define PORT_DEFS_H

#define PORT_DEFS_MAGIC_TRUE        (0xC0C0CAFE)
#define PORT_DEFS_MAGIC_FALSE       (0xDEAFBABA)

typedef enum
{
    PortDefsUartGps,
    PortDefsUartGsm,
    PortDefsUartDbg,
    PortDefsUartMax             // Enum list terminator.
} PortDefsUart_n;

typedef enum
{
    PortDefsSpiFlash,
    PortDefsSpiEeprom,
    PortDefsSpiAccelerometer,
    PortDefsSpiMax             // Enum list terminator.
}PortDefsSpi_n;

typedef enum
{
    PortDefsGpioDebugLed1,          // GpioPort_8_6
    PortDefsGpioDebugLed2,          // GpioPort_8_7
    PortDefsGpioIgnInMcu,           // GpioPort_8_3
    PortDefsGpioMax                 // Enum list terminator.
}PortDefsGpio_n;

#endif /* PORT_DEFS_H */
