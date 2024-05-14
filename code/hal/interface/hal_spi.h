/**
 * @file        hal_spi.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        27 October 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Interface for platform independent SPI abstraction layer.
 */

#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "injectable.h"
#include "hal_buffer.h"

/**
 *  This is an opaque handle to private (port dependent) concrete SPI 'identity'.
*/
typedef struct HalSpiIdentityStruct_t*     HalSpiIdentity_t;

/**
 *  This is an opaque handle to private (port dependent) concrete SPI 'context'.
*/
typedef struct HalSpiContext_t*            HalSpiHandle_t;

/**
 *  Various return codes that can occur when using this interface.
*/
typedef enum 
{
    HalSpiErrOk,                                        /* Success. */
    HalSpiErrParam,                                     /* Parameter error. */
    HalSpiErrForbidden,                                 /* Interface usage prohibitions. */
    HalSpiErrTransmit,                                  /* Port defined error during write operation. */
    HalSpiErrReceive,                                   /* Port defined error during read operation.  */
    HalSpiErrConfig,                                    /* Config error as defined by port (DISCOURAGED USE, must be well documented). */
    HalSpiErrPort,                                      /* Miscellaneous error as defined by port (DISCOURAGED USE, must be well documented). */
    HalSpiErrMax                                        /* Placeholder for list termination. */
} HalSpiErr_n;

typedef struct
{
    // *************************************************************************************
    // TODO: add config params, currently they are hardcoded or privately set.
    // *************************************************************************************
    uint32_t someConfig;
} HalSpiConfig_t;

/**
 * @brief                                   Creates a handle for the identify.
 * @param       handlePtr                   On success, user provided handle will be updated and can be then be used for further API usage.
 * @param       identity                    An implementation defined identity useful for mapping physical resource to handle abstraction.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                                                      If handlePtr does not point to a pointer having NULL value.
 *                                          HalSpiErrForbidden:         The identity is invalid.
 *                                                                      If handle is already created.
*/
HalSpiErr_n     hal_spi_create              (HalSpiHandle_t* handlePtr, HalSpiIdentity_t identity);

/**
 * @brief                                   Initializes a SPI with user provided configuration.
 * @param      handle                       A valid handle to SPI.
 * @param      config                       Overall user-configuration for this SPI.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                          HalSpiErrConfig:            If any configuration is invalid or isn't supported.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is already initialized.
 *                                                                      If handle is open.
*/
HalSpiErr_n     hal_spi_init                (HalSpiHandle_t handle, HalSpiConfig_t config);

/**
 * @brief                                   De-initializes a SPI.
 * @param      handle                       A valid handle to SPI.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is open.
*/
HalSpiErr_n     hal_spi_deinit              (HalSpiHandle_t handle);

/**
 * @brief                                   Opens (equivalent to enable) a SPI.
 * @param      handle                       A valid handle to SPI.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is already open.
*/
HalSpiErr_n     hal_spi_open                (HalSpiHandle_t handle);

/**
 * @brief                                   Closes (equivalent to enable) a SPI.
 * @param      handle                       A valid handle to SPI.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is not initialized.
*/
HalSpiErr_n     hal_spi_close               (HalSpiHandle_t handle);

/**
 * @brief                                   Writes data to implementation managed transmit buffer.
 * @param       handle                      A valid handle to SPI.
 * @param       txBuf                       A user-buffer containing the data to be sent.
 * @param       txNum                       The number of bytes requested to send.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                                                      If request to write zero bytes or provided user-buffer is having zero size.
 *                                                                      Provided user-buffer does not have sufficient capacity for requested number of bytes.
 *                                          HalSpiErrTransmit:          The implementation is unable to perform requested transmit operation.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalSpiErr_n     hal_spi_write               (HalSpiHandle_t handle, HalBuffer_t txBuf, size_t txReq);

/**
 * @brief                                   Reads data from implementation managed receive buffer.
 * @param       handle                      A valid handle to SPI.
 * @param       rxBuf                       A user-buffer to store the read data.
 * @param       rxReq                       The number of bytes requested to receive.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                                                      Request to read zero bytes or provided user-buffer is having zero size.
 *                                                                      Provided user-buffer does not have sufficient capacity for requested number of bytes.
 *                                          HalSpiErrReceive:           The implementation is unable to perform requested receive operation.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalSpiErr_n     hal_spi_read                (HalSpiHandle_t handle, HalBuffer_t rxBuf, size_t rxReq);

/**
 * @brief                                   Tells how many bytes are available in the implementation managed transmit buffer.
 * @param       handle                      A valid handle to SPI.
 * @param       available                   The number of bytes available will be updated into this variable.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalSpiErr_n     hal_spi_transmit_available  (HalSpiHandle_t handle, size_t* available);

/**
 * @brief                                   Tells how many bytes are available in the implementation managed receive buffer.
 * @param       handle                      A valid handle to SPI.
 * @param       available                   The number of bytes available will be updated into this variable.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                          HalSpiErrForbidden:         If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalSpiErr_n     hal_spi_receive_available   (HalSpiHandle_t handle, size_t* available);

/**
 * @brief                                   Convenience function for getting identity from handle.
 * @param       handle                      A valid handle to SPI.
 * @param       identityPtr                 On success, user provided the identity pointer will be updated to reference identity of handle.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                                                      If identityPtr does not point to a pointer having NULL value.
 *                                          HalSpiErrForbidden:         If handle is not created.
*/
HalSpiErr_n     hal_spi_get_identity        (HalSpiHandle_t handle, HalSpiIdentity_t* identityPtr);

/**
 * @brief                                   Convenience function for getting configuration from handle.
 * @param       handle                      A valid handle to SPI.
 * @param       configPtr                   On success, user provided the config pointer will be populated with active configuration of handle.
 * @return                                  HalSpiErrOk:                Success.
 *                                          HalSpiErrParam:             If any parameter is NULL.
 *                                          HalSpiErrForbidden:         If handle is not created.
*/
HalSpiErr_n     hal_spi_get_config          (HalSpiHandle_t handle, HalSpiConfig_t* configPtr);

#endif /* HAL_SPI_H */
