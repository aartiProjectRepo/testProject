/**
 * @file        rh850_spi.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved.
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune.
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION.
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        30 October 2023
 * @author      Pandurang Nagargoje <pandurang@accoladeelectronics.com>     
 *
 * @brief       
 */

#include "hal_spi.h"
#include "rh850_spi.h"
#include "SPI_Driver.h"

typedef void (*spiCss_f) (uint8_t SPI_Ch);
typedef void (*spiXfer_f)(uint8_t SPI_Ch, uint8_t* buf, uint16_t count);

typedef struct
{
    const PortDefsSpi_n portDefsSpi;                // Corresponding port specific define.
} HalSpiIdentityStruct_t;

typedef struct
{
    uint32_t magicCreate;                           // Whether created.
    uint32_t magicInit;                             // Whether initialized.
    uint32_t magicOpen;                             // Whether open.
    HalSpiConfig_t configActive;                    // Active configuration.
    const HalSpiIdentityStruct_t identityStruct;    // Identification provider.
    const uint8_t spiCn;                            // Spi channel number.
    const spiCss_f createFn;                        // Spi init function.
    const spiCss_f startFn;                         // Spi start function.
    const spiCss_f stopFn;                          // Spi stop function.
    const spiXfer_f rxFn;                           // example: 
    const spiXfer_f txFn;                           // example: 
    volatile struct __tag595* const reg;            // CSIH registers.
} HalSpiContext_t;

// Local.
static HalSpiContext_t g_Context[PortDefsSpiMax] = 
{
    {PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, {0}, { PortDefsSpiFlash},         3, SPI_Create, SPI_Start, SPI_Stop,SPI_Receive,SPI_Transmit, (volatile struct __tag595 *)&CSIH2},
    {PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, {0}, { PortDefsSpiEeprom},        2, SPI_Create, SPI_Start, SPI_Stop,SPI_Receive,SPI_Transmit, (volatile struct __tag595 *)&CSIH2},
    {PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, {0}, { PortDefsSpiAccelerometer}, 1, SPI_Create, SPI_Start, SPI_Stop,SPI_Receive,SPI_Transmit, (volatile struct __tag595 *)&CSIH2}
};

static bool context_from_identity (HalSpiContext_t** ctx, HalSpiIdentity_t identity);
static bool validate_config (HalSpiConfig_t* config);

HalSpiErr_n hal_spi_create (HalSpiHandle_t* handlePtr, HalSpiIdentity_t identity)
{
    HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = NULL;

    if ( handlePtr && !*handlePtr && identity )
    {
        if ( context_from_identity(&ctx, identity) )
        {
            if ( PORT_DEFS_MAGIC_FALSE == (*ctx).magicCreate )
            {
                // Give the handle to user.
                *handlePtr = (HalSpiHandle_t)ctx;
                // Creation stamp.
                (*ctx).magicCreate = PORT_DEFS_MAGIC_TRUE;
                // All good.
                err = HalSpiErrOk;
            }
            else
            {
                err = HalSpiErrForbidden;
            }
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }

    return err;
}

HalSpiErr_n hal_spi_init (HalSpiHandle_t handle, HalSpiConfig_t config)
{
    HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = (HalSpiContext_t*) handle;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicCreate ) \
        && ( PORT_DEFS_MAGIC_FALSE == (*ctx).magicInit ) \
        && ( PORT_DEFS_MAGIC_FALSE == (*ctx).magicOpen ) )
        {
            if ( validate_config(&config) )
            {
                // TODO ************ Apply the actual configuration. ************
                // Save the applied configuration.
                ctx->configActive = config;
                // Set the configuration registers.
                
                ctx->createFn(ctx->spiCn);

                // Initialization stamp.
                ctx->magicInit = PORT_DEFS_MAGIC_TRUE;
                // All good.
                err = HalSpiErrOk;
            }
            else
            {
                err = HalSpiErrConfig;
            }
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }

    return err;
}

HalSpiErr_n hal_spi_open (HalSpiHandle_t handle)
{
    HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = (HalSpiContext_t*) handle;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) \
        && ( PORT_DEFS_MAGIC_FALSE == ctx->magicOpen ) )
        {
            ctx->startFn(ctx->spiCn);
                
            // Initialization stamp.
            ctx->magicOpen = PORT_DEFS_MAGIC_TRUE;
            // All good.
            err = HalSpiErrOk;
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }
    return err;
}
HalSpiErr_n hal_spi_close (HalSpiHandle_t handle)
{
    HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = (HalSpiContext_t*) handle;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicInit ) \
        && ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicOpen ) )
        {
            ctx->stopFn(ctx->spiCn);
                
            // Initialization stamp.
            ctx->magicOpen = PORT_DEFS_MAGIC_FALSE;
            // All good.
            err = HalSpiErrOk;
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }
    return err;
}

HalSpiErr_n hal_spi_write (HalSpiHandle_t handle, HalBuffer_t txBuf, size_t txReq)
{
    HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = (HalSpiContext_t*) handle;

    if ( ctx && txBuf.mem && txBuf.sizeMem && txReq && ( txBuf.sizeMem >= txReq ) )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicInit ) \
        && ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicOpen ) )
        {
            ctx->txFn(ctx->spiCn, txBuf.mem, txReq);

            // All good.
            err = HalSpiErrOk;
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }
    return err;
}

HalSpiErr_n hal_spi_read (HalSpiHandle_t handle, HalBuffer_t rxBuf, size_t rxReq)
{
    HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = (HalSpiContext_t*) handle;

    if ( ctx && rxBuf.mem && rxBuf.sizeMem && rxReq && ( rxBuf.sizeMem >= rxReq ) )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicInit ) \
        && ( PORT_DEFS_MAGIC_TRUE == (*ctx).magicOpen ) )
        {
                ctx->rxFn(ctx->spiCn,rxBuf.mem,rxReq);
                
                // All good.
                err = HalSpiErrOk;
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }
    return err;
}

HalSpiErr_n hal_spi_get_identity (HalSpiHandle_t handle, HalSpiIdentity_t* identityPtr)
{
    HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = (HalSpiContext_t*) handle;

    if ( ctx && identityPtr && !*identityPtr )
    {
        if ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate )
        {
            *identityPtr = (HalSpiIdentity_t) &ctx->identityStruct;
            err = HalSpiErrOk;
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }

    return err;
}

HalSpiErr_n hal_spi_get_config              (HalSpiHandle_t handle, HalSpiConfig_t* configPtr)
{
     HalSpiErr_n err = HalSpiErrParam;
    HalSpiContext_t* ctx = (HalSpiContext_t*) handle;

    if ( ctx && configPtr )
    {
        if ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate )
        {
            *configPtr = ctx->configActive;
            err = HalSpiErrOk;
        }
        else
        {
            err = HalSpiErrForbidden;
        }
    }

    return err;
}

HalSpiErr_n rh850_spi_get_identity (HalSpiIdentity_t* identityPtr, PortDefsSpi_n portDefsSpi)
{
    HalSpiErr_n err = HalSpiErrParam;

    if ( identityPtr && !*identityPtr && ( portDefsSpi < PortDefsSpiMax ) )
    {
        *identityPtr = (HalSpiIdentity_t) &g_Context[portDefsSpi].identityStruct;
        err = HalSpiErrOk;
    }

    return err;
}

static bool context_from_identity (HalSpiContext_t** ctx, HalSpiIdentity_t identity)
{
    HalSpiIdentityStruct_t* ptrIdentityStruct = NULL;
    bool ret = false;

    ptrIdentityStruct = (HalSpiIdentityStruct_t*) identity;
    if ( ctx && !*ctx && ( ptrIdentityStruct->portDefsSpi < PortDefsSpiMax) )
    {
        *ctx = &g_Context[ptrIdentityStruct->portDefsSpi];
        ret = true;
    }
    return ret;
}

static bool validate_config (HalSpiConfig_t* config)
{
    bool ret = false;

    // TODO: Presently this is just a dummy function.
    if ( config )
    {
        ret = true;
    }

    return ret;
}
