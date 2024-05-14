/**
 * @file        rh850_spi.h
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
 * @brief       R8580 port required API for HAL SPI implementation.
 *              Restriction: Each SPI port may be created only once.
 */

#ifndef RH850_SPI_H
#define RH850_SPI_H

#include "hal_spi.h"
#include "port_defs.h"

/**
 * @brief                                   Get's identity mapped to specific RH850 physical SPI's port define.
 * @param       identityPtr                 On success, user provided the identity pointer will be populated with identity corresponding to port.
 * @param       portDefsUart                One of the values in port defines.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                                                      If identityPtr does not point to a pointer having NULL value.
 *                                                                      If portDefsUart is not in valid range of port defines.
*/
HalSpiErr_n rh850_spi_get_identity          (HalSpiIdentity_t* identityPtr, PortDefsSpi_n portDefsSpi);

#endif /* RH850_SPI_H */
