/**
 * @file        rh850_gpio.c
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
 *              Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       R8580 port for HAL GPIO interface.
 * 
 * @warning     If pin is configured as output, hal_gpio_init() will make the pin default to low after init done.
 */

// Standard includes.
#include <stdint.h>
#include <stdbool.h>

// HAL related defs.
#include "hal_gpio.h"
#include "hal_util.h"

// Port specific defs.
#include "port_defs.h"
#include "rh850_gpio_defs.h"

typedef struct 
{
    const PortDefsGpio_n portDefsGpio;      // Corresponding port specific define.
} HalGpioIdentityStruct_t;

/**
 *  The GPIO context contains all the information required for GPIO operations and that includes:
 *  - Public members that are required for functionality supported by this interface.
 *  - Private member of type 'HalGpioPrivate_t' which encapsulates any port specific dependencies.
*/
typedef struct
{
    HalGpioConfig_t configActive;                       // Active configuration.
    GpioPort_n gpioPort;                                // Gpio port
    GpioPin_n gpioPin;                                  // Gpio pin
    const HalGpioIdentityStruct_t identityStruct;       // Identity provider.
    uint32_t magicCreate;                               // Whether created.
    uint32_t magicInit;                                 // Whether initialized.
    // Interrupt and callbacks members may be added as they are port specific
} HalGpioContext_t;

// Local.

static HalGpioContext_t g_Context[PortDefsGpioMax] =
{
    { { HalGpioDirectionInput }, Port8, Pin_6,  { PortDefsGpioDebugLed1 }, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE },
    { { HalGpioDirectionInput }, Port8, Pin_7,  { PortDefsGpioDebugLed2 }, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE },
    { { HalGpioDirectionInput }, Port8, Pin_3,  { PortDefsGpioIgnInMcu  }, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE }
};

/**
 * @brief                                   Convenience function for getting identity from gpioContext.
 * @param      ctx                          A pointer to HalGpioContext_t.
 * @param      identity                     An implementation defined identity useful for mapping physical resource to handle abstraction.
 * @return                                  true:           Get identity successfully.
 *                                          false:          Fail to get identity.
*/
static bool context_from_identity           (HalGpioContext_t** ctx, HalGpioIdentity_t identity);

/**
 * @brief                                   Convenience function for validate configuration.
 * @param      config                       A pointer to HalGpioConfig_t.
 * @return                                  true:           Validate configuration successfully.
 *                                          false:          Invalid configuration.
*/
static bool validate_config                 (HalGpioConfig_t* config);

HalGpioErr_n hal_gpio_create                (HalGpioHandle_t* handlePtr, HalGpioIdentity_t identity)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t* ctx = NULL;

    if ( handlePtr && !*handlePtr && identity )
    {
        if ( context_from_identity(&ctx, identity) )
        {
            if ( PORT_DEFS_MAGIC_FALSE == ctx->magicCreate )
            {
                // Give the handle to user.
                *handlePtr = (HalGpioHandle_t) ctx;
                // Creation stamp.
                ctx->magicCreate = PORT_DEFS_MAGIC_TRUE;
                // All good.
                err = HalGpioErrOk;
            }
            else
            {
                err = HalGpioErrForbidden;
            }
        }
        else
        {
            err = HalGpioErrForbidden;
        }
    }

    return err;
}

HalGpioErr_n hal_gpio_init                  (HalGpioHandle_t handle, HalGpioConfig_t config)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t *ctx = (HalGpioContext_t *)handle;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) && ( PORT_DEFS_MAGIC_FALSE == ctx->magicInit ) )
        {
            if ( validate_config(&config) && ( (ctx->gpioPort >= 0) && (ctx->gpioPort <= 10) ) && ( (ctx->gpioPin >= 0) && (ctx->gpioPin <= 15) ) )
            {
                if ( config.direction == HalGpioDirectionInput )
                {
                    *PortList[ctx->gpioPort].PM_Reg |= 1 << ctx->gpioPin;
                    *PortList[ctx->gpioPort].PIBC_Reg |= 1 << ctx->gpioPin;
                    *PortList[ctx->gpioPort].PMC_Reg &= ~(1u << ctx->gpioPin);
                }
                else if ( config.direction == HalGpioDirectionOutput )
                {
                    // Must set a default (setting low), required for PNOT_Reg to function properly.
                    *PortList[ctx->gpioPort].PM_Reg &= ~(1u << ctx->gpioPin);
                }
                // Save the applied configuration.
                ctx->configActive = config;
                // Initialization stamp.
                ctx->magicInit = PORT_DEFS_MAGIC_TRUE;
                // All good and port configured.
                err = HalGpioErrOk;
            }
            else
            {
                err = HalGpioErrConfig;
            }
        }
        else
        {
            err = HalGpioErrForbidden;
        }
    }

    return err;
}

HalGpioErr_n hal_gpio_deinit                (HalGpioHandle_t handle)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t *ctx = (HalGpioContext_t *)handle;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( ( PORT_DEFS_MAGIC_FALSE == ctx->magicInit ) || ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) ) )
        {
            // Set Port_Pin to GPIO High impedance input (Port Input Buffer disabled) irrespective of direction.
            *PortList[ctx->gpioPort].PIBC_Reg &= ~(1 << ctx->gpioPin);
            *PortList[ctx->gpioPort].PM_Reg |= 1 << ctx->gpioPin;

            // Clear applied configuration.
            hal_util_memset(&ctx->configActive, 0, sizeof(ctx->configActive));
            // Remove initialization stamp.
            ctx->magicInit = PORT_DEFS_MAGIC_FALSE;
            // All good.
            err = HalGpioErrOk;
        }
        else
        {
            err = HalGpioErrForbidden;
        }
    }

    return err;
}

HalGpioErr_n hal_gpio_read_level            (HalGpioHandle_t handle, HalGpioLevel_n* const level)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t* ctx = (HalGpioContext_t*) handle;
    uint16_t portLevel = 0;

    if ( ctx && level )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            if ( HalGpioDirectionInput == ctx->configActive.direction )
            {
                portLevel = *PortList[ctx->gpioPort].PPR_Reg;
                portLevel &= 1 << ctx->gpioPin;
                if ( portLevel == 0 )
                {
                    *level = HalGpioLevelLow;
                }
                else
                {
                    *level = HalGpioLevelHigh;
                }
                err = HalGpioErrOk;
            }
            else
            {
                err = HalGpioErrParam;
            }
        }
        else
        {
            err = HalGpioErrForbidden;
        }
    }

    return err;
}

HalGpioErr_n hal_gpio_write_level           (HalGpioHandle_t handle, HalGpioLevel_n level)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t* ctx = (HalGpioContext_t*) handle;

    if ( ctx && ( level < HalGpioLevelMax ) )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            if ( HalGpioDirectionOutput == ctx->configActive.direction )
            {
                if ( level == HalGpioLevelLow )
                {
                    *PortList[ctx->gpioPort].P_Reg &= ~(1u << ctx->gpioPin);
                }
                else
                {
                    *PortList[ctx->gpioPort].P_Reg |= (1u << ctx->gpioPin);
                }
                *PortList[ctx->gpioPort].PM_Reg &= ~(1u << ctx->gpioPin);
                *PortList[ctx->gpioPort].PMC_Reg &= ~(1u << ctx->gpioPin);
                err = HalGpioErrOk;
            }
            else
            {
                err = HalGpioErrParam;
            }
        }
        else
        {
            err = HalGpioErrForbidden;
        }
    }

    return err;
}

HalGpioErr_n hal_gpio_toggle                (HalGpioHandle_t handle)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t* ctx = (HalGpioContext_t*) handle;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            if ( HalGpioDirectionOutput == ctx->configActive.direction )
            {
                *PortList[ctx->gpioPort].PNOT_Reg |= 1 << ctx-> gpioPin;
                err = HalGpioErrOk;
            }
            else
            {
                err = HalGpioErrParam;
            }
        }
        else
        {
            err = HalGpioErrForbidden;
        }
    }

    return err;
}

HalGpioErr_n rh850_gpio_get_identity        (HalGpioIdentity_t* identityPtr, PortDefsGpio_n portDefsGpio)
{
    HalGpioErr_n err = HalGpioErrParam;
    
    if ( identityPtr && !*identityPtr && ( portDefsGpio < PortDefsGpioMax ) )
    {
        *identityPtr = (HalGpioIdentity_t) &g_Context[portDefsGpio].identityStruct;
        err = HalGpioErrOk;
    }

    return err;
}

static bool context_from_identity           (HalGpioContext_t** ctx, HalGpioIdentity_t identity)
{
    HalGpioIdentityStruct_t* ptrIdentityStruct = NULL;
    bool ret = false;

    ptrIdentityStruct = (HalGpioIdentityStruct_t*) identity;
    if ( ctx && !*ctx && ( ptrIdentityStruct->portDefsGpio < PortDefsGpioMax) )
    {
        *ctx = &g_Context[ptrIdentityStruct->portDefsGpio];
        ret = true;
    }

    return ret;
}

static bool validate_config                 (HalGpioConfig_t* config)
{
    bool ret = false;

    if ( config )
    {
        if ( ( config->direction == HalGpioDirectionInput ) || ( config->direction == HalGpioDirectionOutput ) ) 
        {
            ret = true;
        }
    }

    return ret;
}

HalGpioErr_n hal_gpio_get_identity          (HalGpioHandle_t handle, HalGpioIdentity_t* identity)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t* ctx = (HalGpioContext_t*) handle;

    if ( ctx && identity && !*identity )
    {
        if ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate )
        {
            *identity = (HalGpioIdentity_t) &ctx->identityStruct;
            err = HalGpioErrOk;
        }
        else
        {
            err = HalGpioErrForbidden;
        }
    }

    return err;
}

HalGpioErr_n hal_gpio_get_config            (HalGpioHandle_t handle, HalGpioConfig_t* config)
{
    HalGpioErr_n err = HalGpioErrParam;
    HalGpioContext_t* ctx = (HalGpioContext_t*) handle;

    if ( ctx && config )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            *config = ctx->configActive;
            err = HalGpioErrOk;
        }
        else
        {
            err = HalGpioErrParam;
        }
    }

    return err;
}
