/**
 * @file        hal_uart.h
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
 * @brief       Interface for platform independent UART abstraction layer.
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal_buffer.h"

/**
 *  This is an opaque handle to private (port dependent) concrete UART 'identity'.
*/
typedef struct HalUartIdentityStruct_t*     HalUartIdentity_t;

/**
 *  This is an opaque handle to private (port dependent) concrete UART 'context'.
*/
typedef struct HalUartContext_t*            HalUartHandle_t;

/**
 *  Various return codes that can occur when using this interface.
*/
typedef enum 
{
    HalUartErrOk,                           /* Success. */
    HalUartErrParam,                        /* Parameter error. */
    HalUartErrForbidden,                    /* Interface usage prohibitions. */
    HalUartErrTransmit,                     /* Transmit fail. */
    HalUartErrReceive,                      /* Receive fail. */
    HalUartErrConfig,                       /* Config error as defined by port. */
    HalUartErrMax                           /* Placeholder for list termination. */
} HalUartErr_n;

typedef struct
{
    uint32_t baud;
} HalUartConfig_t;

/**
 *  @brief                                  Creates a handle for the identify.
 *  @param      handlePtr                   On success, user provided handle will be updated and can be then be used for further API usage.
 *  @param      identity                    An implementation defined identity useful for mapping physical resource to handle abstraction.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                                                      If handlePtr does not point to a pointer having NULL value.
 *                                          HalUartErrForbidden:        The identity is invalid.
 *                                                                      If handle is already created.
*/
HalUartErr_n hal_uart_create                (HalUartHandle_t* handlePtr, HalUartIdentity_t identity);

/**
 *  @brief                                  Initializes a UART with user provided configuration.
 *  @param      handle                      A valid handle to UART.
 *  @param      config                      Overall user-configuration for this UART.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrConfig:           If any configuration is invalid or isn't supported.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is already initialized.
 *                                                                      If handle is open.
*/
HalUartErr_n hal_uart_init                  (HalUartHandle_t handle, HalUartConfig_t config);

/**
 *  @brief                                  De-initializes a UART.
 *  @param      handle                      A valid handle to UART.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is open.
*/
HalUartErr_n hal_uart_deinit                (HalUartHandle_t handle);

/**
 *  @brief                                  Opens (equivalent to enable) a UART.
 *  @param      handle                      A valid handle to UART.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is already open.
*/
HalUartErr_n hal_uart_open                  (HalUartHandle_t handle);

/**
 *  @brief                                  Closes (equivalent to enable) a UART.
 *  @param      handle                      A valid handle to UART.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
*/
HalUartErr_n hal_uart_close                 (HalUartHandle_t handle);

/**
 *  @brief                                  Writes data to implementation managed transmit buffer.
 *  @param      handle                      A valid handle to UART.
 *  @param      txBuf                       A user-buffer containing the data to be sent.
 *  @param      txNum                       The number of bytes requested to send.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                                                      If request to write zero bytes or provided user-buffer is having zero size.
 *                                                                      Provided user-buffer does not have sufficient capacity for requested number of bytes.
 *                                          HalUartErrTransmit:         The implementation is unable to perform requested transmit operation.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalUartErr_n hal_uart_write                 (HalUartHandle_t handle, HalBuffer_t txBuf, size_t txReq);

/**
 *  @brief                                  Reads data from implementation managed receive buffer.
 *  @param      handle                      A valid handle to UART.
 *  @param      rxBuf                       A user-buffer to store the read data.
 *  @param      rxReq                       The number of bytes requested to receive.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                                                      Request to read zero bytes or provided user-buffer is having zero size.
 *                                                                      Provided user-buffer does not have sufficient capacity for requested number of bytes.
 *                                          HalUartErrReceive:          The implementation is unable to perform requested receive operation.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalUartErr_n hal_uart_read                  (HalUartHandle_t handle, HalBuffer_t rxBuf, size_t rxReq);

/**
 *  @brief                                  Tells how many bytes are available in the implementation managed transmit buffer.
 *  @param      handle                      A valid handle to UART.
 *  @param      available                   The number of bytes available will be updated into this variable.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrTransmit          No bytes available for transmit.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalUartErr_n hal_uart_transmit_available    (HalUartHandle_t handle, size_t* available);

/**
 *  @brief                                  Tells how many bytes are available in the implementation managed receive buffer.
 *  @param      handle                      A valid handle to UART.
 *  @param      available                   The number of bytes available will be updated into this variable.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrReceive           No bytes available for read.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalUartErr_n hal_uart_receive_available     (HalUartHandle_t handle, size_t* available);

/**
 *  @brief                                  Function needs to be called for background operations.
 *  @param      handle                      A valid handle to UART.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 *                                                                      If handle is not open.
*/
HalUartErr_n hal_uart_process               (HalUartHandle_t handle);

/**
 *  @brief                                  Convenience function for getting identity from handle.
 *  @param      handle                      A valid handle to UART.
 *  @param      identityPtr                 On success, user provided the identity pointer will be updated to reference identity of handle.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                                                      If identityPtr does not point to a pointer having NULL value.
 *                                          HalUartErrForbidden:        If handle is not created.
*/
HalUartErr_n hal_uart_get_identity          (HalUartHandle_t handle, HalUartIdentity_t* identityPtr);

/**
 *  @brief                                  Convenience function for getting configuration from handle.
 *  @param      handle                      A valid handle to UART.
 *  @param      configPtr                   On success, user provided the config pointer will be populated with active configuration of handle.
 *  @return                                 HalUartErrOk:               Success.
 *                                          HalUartErrParam:            If any parameter is NULL.
 *                                          HalUartErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
*/
HalUartErr_n hal_uart_get_config            (HalUartHandle_t handle, HalUartConfig_t* configPtr);

#endif /* HAL_UART_H */
