/**
 * @file        rh850_gpio.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        24 November 2023
 * @author      Aarti B <aarti.bhelke@accoladeelectronics.com>
 *              Eshwar J <eshwar.jorvekar@accoladeelectronics.com>
 *
 * @brief       R8580 port required API for HAL GPIO implementation.
 *              Restriction: Each GPIO port may be created only once.
 */

#ifndef RH850_GPIO_H
#define RH850_GPIO_H

#include "hal_gpio.h"
#include "port_defs.h"

/**
 * @brief                                       Get's identity mapped to specific RH850 physical GPIO's port define.
 * @param       identityPtr                     On success, user provided the identity pointer will be populated with identity corresponding to port.
 * @param       portDefsGpio                    One of the values in port defines.
 * @return                                      HalGpioErrOk:               Success.
 *                                              HalGpioErrParam:            If any parameter is NULL.
 *                                                                          If identityPtr does not point to a pointer having NULL value.
 *                                                                          If portDefsUart is not in valid range of port defines.
*/
HalGpioErr_n rh850_gpio_get_identity            (HalGpioIdentity_t* identityPtr, PortDefsGpio_n portDefsGpio);

#endif /* RH850_GPIO_H */
