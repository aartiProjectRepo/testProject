/**
 * @file        hal_gpio.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        28 October 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Interface for platform independent GPIO abstraction layer.
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

/**
 *  This is an opaque handle to private (port dependent) concrete GPIO 'identity'.
*/
typedef struct HalGpioIdentityStruct_t*      HalGpioIdentity_t;

/**
 *  This is an opaque handle to private (port dependent) concrete GPIO 'context'.
*/
typedef struct HalGpioContext_t*            HalGpioHandle_t;

/**
 *  Various return codes that can occur when using this interface.
 */
typedef enum 
{
    HalGpioErrOk,                           /* No error (Success). */
    HalGpioErrParam,                        /* Parameter error (NULL pointer, illegal value, etc). */
    HalGpioErrForbidden,                    /* Interface usage prohibitions. */
    HalGpioErrConfig,                       /* Config error as defined by port. */
    HalGpioErrPort,                         /* Miscellaneous port specific error. */
    HalGpioErrMax                           /* Placeholder for list termination. */
} HalGpioErr_n;

/**
 *  Direction of GPIO can be either INPUT or OUTPUT.
 */
typedef enum
{
    HalGpioDirectionInput,                  /* Defines pin as input. */
    HalGpioDirectionOutput,                 /* Defines pin as output. */
    HalGpioDirectionMax                     /* Placeholder for list termination. */
} HalGpioDirection_n;

/**
 *  Level of GPIO can be either LOW (0) or HIGH (1).
 */
typedef enum
{
    HalGpioLevelLow,                        /* Digital LOW. */
    HalGpioLevelHigh,                       /* Digital HIGH. */
    HalGpioLevelMax                         /* Placeholder for list termination. */
} HalGpioLevel_n;

typedef struct
{
    HalGpioDirection_n direction;           /* Whether the GPIO is INPUT or OUTPUT. */
} HalGpioConfig_t;

/**
 * @brief                                   Creates a handle for the identify.
 * @param       handlePtr                   On success, user provided handle will be updated and can be then be used for further API usage.
 * @param       identity                    An implementation defined identity useful for mapping physical resource to handle abstraction.
 * @return                                  HalGpioErrOk:               Success.
 *                                          HalGpioErrParam:            If any parameter is NULL.
 *                                                                      If handlePtr does not point to a pointer having NULL value.
 *                                          HalGpioErrUnsupported:      The identity is invalid.
 *                                                                      If handle is already created.
 */
HalGpioErr_n hal_gpio_create                (HalGpioHandle_t* handlePtr, HalGpioIdentity_t identity);

/**
 * @brief                                   Initializes a GPIO with user provided configuration.
 * @param       handle                      A valid handle to GPIO.
 * @return                                  HalGpioErrOk:               Success.
 *                                          HalGpioErrParam:            If any parameter is NULL.
 *                                          HalGpioErrConfig:           If any configuration is invalid or isn't supported.
 *                                          HalGpioErrForbidden:        If handle is not created.
 *                                                                      If handle is already initialized.
 */
HalGpioErr_n hal_gpio_init                  (HalGpioHandle_t handle, HalGpioConfig_t config);

/**
 * @brief                                   De-initializes a GPIO.
 * @param      handle                       A valid handle to GPIO.
 * @return                                  HalGpioErrOk:               Success.
 *                                          HalGpioErrParam:            If any parameter is NULL.
 *                                          HalGpioErrForbidden:        If handle is not created.
 */
HalGpioErr_n hal_gpio_deinit                (HalGpioHandle_t handle);

/**
 * @brief                                   Read GPIO for given GPO handle (raw).
 * @param      handle                       A valid handle to GPIO.
 * @param      level                        Either HalGpioLevelHigh or HalGpioLevelLow will be read on the pin.
 * @return                                  HalGpioErrOk:               Success.
 *                                          HalGpioErrParam:            Invalid parameter or direction.
 *                                          HalGpioErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 */
HalGpioErr_n hal_gpio_read_level            (HalGpioHandle_t handle, HalGpioLevel_n* const level);

/**
 * @brief                                   Write GPIO for given GPO handle (raw).
 * @param      handle                       A valid handle to GPIO.
 * @param      level                        Either HalGpioLevelHigh or HalGpioLevelLow will be written on the pin.
 * @return                                  HalGpioErrOk:               Success.
 *                                          HalGpioErrParam:            Invalid parameter or direction.
 *                                          HalGpioErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 */
HalGpioErr_n hal_gpio_write_level           (HalGpioHandle_t handle, HalGpioLevel_n level);

/**
 * @brief                                   Toggle the GPO pin.
 * @param       handle                      The handle for the GPIO is the unique identifier of that particular GPIO.
 * @return                                  HalGpioErrOk:               Success.
 *                                          HalGpioErrParam:            Invalid parameter or direction.
 *                                          HalGpioErrUnsupported:      Invalid direction.
 *                                          HalGpioErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 */
HalGpioErr_n hal_gpio_toggle                (HalGpioHandle_t handle);

/**
 * @brief                                   Convenience function for getting identity from handle.
 * @param      handle                       A valid handle to GPIO.
 * @param      identityPtr                  On success, user provided the identity pointer will be updated to reference identity of handle.
 * @return                                  HalGpioErrOk:               Success.
 *                                          HalGpioErrParam:            If any parameter is NULL.
 *                                                                      If identityPtr does not point to a pointer having NULL value.
 *                                          HalGpioErrForbidden:        If handle is not created.
 */
HalGpioErr_n hal_gpio_get_identity          (HalGpioHandle_t handle, HalGpioIdentity_t* identityPtr);

/**
 *  @brief                                  Convenience function for getting configuration from handle.
 *  @param      handle                      A valid handle to GPIO.
 *  @param      configPtr                   On success, user provided the config pointer will be populated with active configuration of handle.
 *  @return                                 HalGpioErrOk                Success.
 *                                          HalGpioErrParam:            If any parameter is NULL.
 *                                          HalGpioErrForbidden:        If handle is not created.
 *                                                                      If handle is not initialized.
 */
HalGpioErr_n hal_gpio_get_config            (HalGpioHandle_t handle, HalGpioConfig_t* configPtr);

#endif /* HAL_GPIO_H */
