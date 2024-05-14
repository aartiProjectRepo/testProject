/**
 * @file        rh850_uart.c
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
 * @brief       R8580 port for HAL UART interface using interrupts and non-blocking API.
 */

#include <stdint.h>
#include <math.h>

// HAL related defs.
#include "hal_uart.h" 
#include "hal_buffer.h" 
#include "hal_cbuf.h"
#include "hal_util.h"

// Port specific defs.
#include "port_defs.h"
#include "rh850_intc2.h"
#include "rh850_uart.h"
#include "rh850_uart_defs.h"
#include "iodefine.h"

#define RH850_UART_SOURCE_CLOCK_HZ                  ( 40UL * 1000UL * 1000UL )
#define RH850_UART_HW_TX_FIFO_SIZE                  ( 9UL )

// Circular buffer backing arrays.
static uint8_t g_bufTxGps[512UL];
static uint8_t g_bufRxGps[512UL];
static uint8_t g_bufTxGsm[16UL * 1024UL];
static uint8_t g_bufRxGsm[16UL * 1024UL];
static uint8_t g_bufTxDbg[4096UL];
static uint8_t g_bufRxDbg[8192UL];

// Identity provider.
typedef struct
{
    const PortDefsUart_n portDefsUart;              // Corresponding port specific define.
} HalUartIdentityStruct_t;

// Context.
typedef struct
{
    HalUartConfig_t configActive;                   // Active configuration.
    const HalUartIdentityStruct_t identityStruct;   // Identification provider.
    HalCbuf_t cbufTx;                               // Transmit internal buffer.
    HalCbuf_t cbufRx;                               // Receive internal buffer.
    volatile struct __tag588* const reg;            // RLIN3 registers.
    uint32_t magicCreate;                           // Whether created.
    uint32_t magicInit;                             // Whether initialized.
    uint32_t magicOpen;                             // Whether opened.
} HalUartContext_t;

typedef struct
{
    uint8_t PSC;
    uint8_t SMP;
    uint16_t LBPR;
    float error;
} RH850UartBaudCalc_t;

// Local.
static HalUartContext_t g_Context[PortDefsUartMax] = 
{
    { {0}, { PortDefsUartGps }, {g_bufTxGps, sizeof(g_bufTxGps), 0, 0}, {g_bufRxGps, sizeof(g_bufRxGps), 0, 0}, &RLN30, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE },
    { {0}, { PortDefsUartGsm }, {g_bufTxGsm, sizeof(g_bufTxGsm), 0, 0}, {g_bufRxGsm, sizeof(g_bufRxGsm), 0, 0}, &RLN31, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE },
    { {0}, { PortDefsUartDbg }, {g_bufTxDbg, sizeof(g_bufTxDbg), 0, 0}, {g_bufRxDbg, sizeof(g_bufRxDbg), 0, 0}, &RLN32, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE, PORT_DEFS_MAGIC_FALSE }
};

// Used for pipeline synchronization.
static volatile uint32_t g_SyncRead = 0;

// Used for ISR.
static volatile HalCbuf_t* const g_isr_gps_ptr_cbuf = &g_Context[PortDefsUartGps].cbufRx;
static volatile uint32_t g_isr_gps_byte;
static volatile uint32_t g_isr_gps_front;
static volatile uint32_t g_isr_gps_rear;
static volatile uint32_t g_isr_gps_new_rear;
static volatile HalCbuf_t* const g_isr_gsm_ptr_cbuf = &g_Context[PortDefsUartGsm].cbufRx;
static volatile uint32_t g_isr_gsm_byte;
static volatile uint32_t g_isr_gsm_front;
static volatile uint32_t g_isr_gsm_rear;
static volatile uint32_t g_isr_gsm_new_rear;
static volatile HalCbuf_t* const g_isr_dbg_ptr_cbuf = &g_Context[PortDefsUartDbg].cbufRx;
static volatile uint32_t g_isr_dbg_byte;
static volatile uint32_t g_isr_dbg_front;
static volatile uint32_t g_isr_dbg_rear;
static volatile uint32_t g_isr_dbg_new_rear;

static bool context_from_identity           (HalUartContext_t** ctx, HalUartIdentity_t identity);
static bool validate_config                 (HalUartConfig_t* config);
static void find_best_baud                  (const uint32_t kBaud, const uint32_t kClkSrcHz, RH850UartBaudCalc_t* const ptrBaud);

HalUartErr_n hal_uart_create                (HalUartHandle_t* handlePtr, HalUartIdentity_t identity)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = NULL;

    if ( handlePtr && !*handlePtr && identity )
    {
        if ( context_from_identity(&ctx, identity) )
        {
            if ( PORT_DEFS_MAGIC_FALSE == ctx->magicCreate )
            {
                // Give the handle to user.
                *handlePtr = (HalUartHandle_t) ctx;
                // Creation stamp.
                ctx->magicCreate = PORT_DEFS_MAGIC_TRUE;
                // All good.
                err = HalUartErrOk;
            }
            else
            {
                err = HalUartErrForbidden;
            }
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_init                  (HalUartHandle_t handle, HalUartConfig_t config)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;
    RH850UartBaudCalc_t bestBaud =  {0, 0, 0, 100.0f};

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_FALSE == ctx->magicInit ) \
        && ( PORT_DEFS_MAGIC_FALSE == ctx->magicOpen ) )
        {
            if ( validate_config(&config) )
            {
                // Save the applied configuration.
                ctx->configActive = config;
                // Set the configuration registers.
                find_best_baud(config.baud, RH850_UART_SOURCE_CLOCK_HZ, &bestBaud);
                ctx->reg->LWBR = bestBaud.SMP | bestBaud.PSC;
                ctx->reg->LBRP01.UINT16 = bestBaud.LBPR;
                ctx->reg->LMD = RH850_UART_NOISE_FILTER_ENABLED | RH850_UART_MODE_SELECT;
                ctx->reg->LEDE = RH850_UART_FRAMING_ERROR_DETECTED | RH850_UART_OVERRUN_ERROR_DETECTED;
                ctx->reg->LBFC = RH850_UART_TRANSMISSION_NORMAL | RH850_UART_RECEPTION_NORMAL | RH850_UART_PARITY_PROHIBITED | RH850_UART_STOP_BIT_1 | RH850_UART_LSB | RH850_UART_LENGTH_8;
                //ctx->reg->LUOR1 = RH850_UART_INT_TRANSMISSION_END;
                // Set the overall UART enable register.
                ctx->reg->LCUC = RH850_UART_LIN_RESET_MODE_CANCELED;
                g_SyncRead = RLN32.LCUC;
                __syncp();
                // Initialization stamp.
                ctx->magicInit = PORT_DEFS_MAGIC_TRUE;
                // All good.
                err = HalUartErrOk;
            }
            else
            {
                err = HalUartErrConfig;
            }
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_deinit                (HalUartHandle_t handle)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( ( PORT_DEFS_MAGIC_FALSE == ctx->magicInit ) || ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) ) \
        && ( PORT_DEFS_MAGIC_FALSE == ctx->magicOpen ) )
        {
            // Clear the overall UART enable register.
            ctx->reg->LCUC = RH850_UART_LIN_RESET_MODE_CAUSED;
            // Clear applied configuration.
            hal_util_memset(&ctx->configActive, 0, sizeof(ctx->configActive));
            // Remove initialization stamp.
            ctx->magicInit = PORT_DEFS_MAGIC_FALSE;
            // All good.
            err = HalUartErrOk;
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_open                  (HalUartHandle_t handle)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;
    PortDefsUart_n portDefsUart = PortDefsUartDbg;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) \
        && ( PORT_DEFS_MAGIC_FALSE == ctx->magicOpen ) )
        {
            // Alias (for ease of readability).
            portDefsUart = ctx->identityStruct.portDefsUart;
            // Prepare context-local variables.
            hal_cbuf_init(&ctx->cbufRx);
            hal_cbuf_init(&ctx->cbufTx);
            // Enable Rx and Tx.
            ctx->reg->LUOER |= RH850_UART_RECEPTION_ENABLED | RH850_UART_TRANSMISSION_ENABLED;
            // Clear pending interrupt flag and then enable only Rx interrupt.
            rh850_intc2_uart_clear(portDefsUart, RH850Intc2UartTransmit);
            rh850_intc2_uart_disable(portDefsUart, RH850Intc2UartTransmit);
            rh850_intc2_uart_clear(portDefsUart, RH850Intc2UartReceive);
            rh850_intc2_uart_enable(portDefsUart, RH850Intc2UartReceive);
            rh850_intc2_uart_clear(portDefsUart, RH850Intc2UartStatus);
            rh850_intc2_uart_disable(portDefsUart, RH850Intc2UartStatus);
            // Opened stamp.
            ctx->magicOpen = PORT_DEFS_MAGIC_TRUE;
            // All good.
            err = HalUartErrOk;
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_close                 (HalUartHandle_t handle)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;
    PortDefsUart_n portDefsUart = PortDefsUartDbg;

    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) \
        && ( ( PORT_DEFS_MAGIC_FALSE == ctx->magicOpen ) || (PORT_DEFS_MAGIC_TRUE == ctx->magicOpen) ) )
        {
            // Alias (for ease of readability).
            portDefsUart = ctx->identityStruct.portDefsUart;
            // Disable interrupts.
            rh850_intc2_uart_disable(portDefsUart, RH850Intc2UartTransmit);
            rh850_intc2_uart_disable(portDefsUart, RH850Intc2UartReceive);
            rh850_intc2_uart_disable(portDefsUart, RH850Intc2UartStatus);
            // Disable Rx and Tx.
            ctx->reg->LUOER &= (uint8_t) ~(RH850_UART_RECEPTION_ENABLED | RH850_UART_TRANSMISSION_ENABLED);
            g_SyncRead = RLN32.LCUC;
            __syncp();
            // Clear pending interrupts.
            rh850_intc2_uart_clear(portDefsUart, RH850Intc2UartTransmit);
            rh850_intc2_uart_clear(portDefsUart, RH850Intc2UartReceive);
            rh850_intc2_uart_clear(portDefsUart, RH850Intc2UartStatus);
            g_SyncRead = RLN32.LCUC;
            __syncp();
            // Remove opened stamp.
            ctx->magicOpen = PORT_DEFS_MAGIC_FALSE;
            // All good.
            err = HalUartErrOk;
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_write                 (HalUartHandle_t handle, HalBuffer_t txBuf, size_t txReq)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;

    if ( ctx && txBuf.mem && txBuf.sizeMem && txReq && ( txBuf.sizeMem >= txReq ) )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicOpen ) )
        {
            if ( HalCbufErrOk == hal_cbuf_enqueue(&ctx->cbufTx, txBuf, txReq) )
            {
                err = HalUartErrOk;
            }
            else
            {
                err = HalUartErrTransmit;
            }
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_read                  (HalUartHandle_t handle, HalBuffer_t rxBuf, size_t rxReq)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;

    if ( ctx && rxBuf.mem && rxBuf.sizeMem && rxReq && ( rxBuf.sizeMem >= rxReq ) )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicOpen ) )
        {
            if ( HalCbufErrOk == hal_cbuf_dequeue(&ctx->cbufRx, rxBuf, rxReq) )
            {
                err = HalUartErrOk;
            }
            else
            {
                err = HalUartErrReceive;
            }
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_transmit_available    (HalUartHandle_t handle, size_t* available)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;

    if ( ctx && available )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            if ( HalCbufErrOk == hal_cbuf_available_write(&ctx->cbufTx, available) )
            {
                err = HalUartErrOk;
            }
            else
            {
                err = HalUartErrTransmit;
            }
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_receive_available     (HalUartHandle_t handle, size_t* available)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;

    if ( ctx && available )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            if ( HalCbufErrOk == hal_cbuf_available_read(&ctx->cbufRx, available) )
            {
                err = HalUartErrOk;
            }
            else
            {
                err = HalUartErrReceive;
            }
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_process               (HalUartHandle_t handle)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;
    size_t available = 0;
    uint8_t mem[RH850_UART_HW_TX_FIFO_SIZE];
    uint8_t padding[3];
    HalBuffer_t buf = {mem, sizeof(mem)};
    
    if ( ctx )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) \
        && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            // Unused warning clear.
            padding[0] = 0;
            padding[1] = padding[0];
            
            // Handle error bit.
            if ( ctx->reg->LEST & RH850_UART_CLEAR_ERROR_FLAG )
            {
                // Consume LURDR.
                ctx->reg->LURDR.UINT16;
                // Clear the error flag.
                ctx->reg->LEST &= (uint8_t) ~RH850_UART_CLEAR_ERROR_FLAG;
            }

            // Handle transmission.
            err = HalUartErrTransmit;
            // Check general UART TX busy (UTS bit).
            if ( RH850_UART_TRANSMISSION_ISNOT_OPERATED == ( ctx->reg->LST & RH850_UART_TRANSMISSION_OPERATED ) )
            {
                // Check HW FIFO TX 'transmission start bit' (RTS bit).
                if ( RH850_UART_BUFFER_TRANSMISSION_IS_STOPPED == ( ctx->reg->LTRC & RH850_UART_BUFFER_TRANSMISSION_IS_STARTED ) )
                {
                    // Check HW FIFO TX 'complete bit' (FTC bit).
                    // if ( RH850_UART_TRANSMISSION_COMPLETED == ( ctx->reg->LST & RH850_UART_TRANSMISSION_COMPLETED ) )
                    {
                        // Check if bytes are pending TX.
                        if ( HalCbufErrOk == hal_cbuf_available_read(&ctx->cbufTx, &available) )
                        {
                            if ( available )
                            {
                                // Cap transmission to upto HW TX FIFO size.
                                available = ( available > RH850_UART_HW_TX_FIFO_SIZE ) ? RH850_UART_HW_TX_FIFO_SIZE : available;
                                // Dequeue SW TX FIFO.
                                if ( HalCbufErrOk == hal_cbuf_dequeue(&ctx->cbufTx, buf, available) )
                                {
                                    // Set the 'UART Buffer Data Length Select'.
                                    ctx->reg->LDFC = 0;
                                    ctx->reg->LDFC |= available;
                                    // Write bytes into HW TX FIFO.
                                    if ( 9 == available )
                                    {
                                        ctx->reg->LUDB0 = buf.mem[0];
                                        ctx->reg->LDBR1 = buf.mem[1];
                                        ctx->reg->LDBR2 = buf.mem[2];
                                        ctx->reg->LDBR3 = buf.mem[3];
                                        ctx->reg->LDBR4 = buf.mem[4];
                                        ctx->reg->LDBR5 = buf.mem[5];
                                        ctx->reg->LDBR6 = buf.mem[6];
                                        ctx->reg->LDBR7 = buf.mem[7];
                                        ctx->reg->LDBR8 = buf.mem[8];
                                    }
                                    else
                                    {
                                        ctx->reg->LDBR1 = buf.mem[0];
                                        ctx->reg->LDBR2 = buf.mem[1];
                                        ctx->reg->LDBR3 = buf.mem[2];
                                        ctx->reg->LDBR4 = buf.mem[3];
                                        ctx->reg->LDBR5 = buf.mem[4];
                                        ctx->reg->LDBR6 = buf.mem[5];
                                        ctx->reg->LDBR7 = buf.mem[6];
                                        ctx->reg->LDBR8 = buf.mem[7];
                                    }
                                    // Clear HW FIFO TX 'complete bit' (FTC bit).
                                    //ctx->reg->LST &= ~RH850_UART_TRANSMISSION_COMPLETED;
                                    // Set HW FIFO TX 'transmission start bit' (RTS bit).
                                    ctx->reg->LTRC |= RH850_UART_BUFFER_TRANSMISSION_IS_STARTED;
                                    err = HalUartErrOk;
                                }
                            }
                            else
                            {
                                // There are no bytes to be transmitted.
                                err = HalUartErrOk;
                            }
                        }
                        
                    }
                }
            }
        }
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_get_identity          (HalUartHandle_t handle, HalUartIdentity_t* identity)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;

    if ( ctx && identity && !*identity )
    {
        if ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate )
        {
            *identity = (HalUartIdentity_t) &ctx->identityStruct;
            err = HalUartErrOk;
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n hal_uart_get_config            (HalUartHandle_t handle, HalUartConfig_t* config)
{
    HalUartErr_n err = HalUartErrParam;
    HalUartContext_t* ctx = (HalUartContext_t*) handle;

    if ( ctx && config )
    {
        if ( ( PORT_DEFS_MAGIC_TRUE == ctx->magicCreate ) && ( PORT_DEFS_MAGIC_TRUE == ctx->magicInit ) )
        {
            *config = ctx->configActive;
            err = HalUartErrOk;
        }
        else
        {
            err = HalUartErrForbidden;
        }
    }

    return err;
}

HalUartErr_n rh850_uart_get_identity        (HalUartIdentity_t* identityPtr, PortDefsUart_n portDefsUart)
{
    HalUartErr_n err = HalUartErrParam;

    if ( identityPtr && !*identityPtr && ( portDefsUart < PortDefsUartMax ) )
    {
        *identityPtr = (HalUartIdentity_t) &g_Context[portDefsUart].identityStruct;
        err = HalUartErrOk;
    }

    return err;
}

void rh850_uart_isr_handler_rx              (HalUartHandle_t handle)
{
    HalUartContext_t* ctx = (HalUartContext_t*) handle;
    uint8_t rx[1];
    HalBuffer_t buf = {rx, sizeof(rx)};

    rx[0] = ctx->reg->LURDR.UINT16;
    if ( ctx )
    {
        // We don't check for queue full, bytes will simply be dropped if buffer is full.
        hal_cbuf_enqueue(&ctx->cbufRx, buf, 1);
    }
}

static bool context_from_identity           (HalUartContext_t** ctx, HalUartIdentity_t identity)
{
    HalUartIdentityStruct_t* ptrIdentityStruct = NULL;
    bool ret = false;

    ptrIdentityStruct = (HalUartIdentityStruct_t*) identity;
    if ( ctx && !*ctx && ( ptrIdentityStruct->portDefsUart < PortDefsUartMax) )
    {
        *ctx = &g_Context[ptrIdentityStruct->portDefsUart];
        ret = true;
    }

    return ret;
}

static bool validate_config                 (HalUartConfig_t* config)
{
    bool ret = false;

    if ( config )
    {
        // FIXME: We aren't able to support baud above 57600 right now because of single byte interrupt.
        if ( (config->baud > 0 ) && ( config->baud <= 115200UL ) )
        {
            ret = true;
        }
    }

    return ret;
}

static void find_best_baud(const uint32_t kBaud, const uint32_t kClkSrcHz, RH850UartBaudCalc_t* const ptrBaud)
{
    RH850UartBaudCalc_t currentBest =  {0, 0, 0, 100.0f};
    RH850UartBaudCalc_t loop;
    uint32_t calcBaud;
    uint32_t tmpPSC;
    uint32_t tmpSMP;
    
    loop.PSC = 0x0;
    for ( tmpPSC = 1 ; tmpPSC <= 128 ; tmpPSC *=2, loop.PSC += 0x02 )
    {
        loop.SMP = 0x50;
        for ( tmpSMP = 6 ; tmpSMP <= 16 ; ++tmpSMP, loop.SMP += 0x10 )
        {   
            loop.LBPR = round( ( (float)kClkSrcHz / ( (float)kBaud * (float)tmpPSC * (float)tmpSMP ) ) - 1 );
            if ( loop.LBPR + 1 )
            {
                calcBaud = kClkSrcHz / ( ( loop.LBPR + 1 ) * tmpPSC * tmpSMP );
                loop.error =  ( 100.0 * fabs( (float)calcBaud - (float)kBaud ) ) / (float)kBaud;
                if ( loop.error <= currentBest.error )
                {
                    currentBest = loop;
                }
            }
            else
            {
                // Prevent divide-by-zero error condition.
                break;
            }
        }
    }
    *ptrBaud = currentBest;
}

#pragma interrupt isr_gps(enable=true, fpu=false, callt=false)
void isr_gps(void)
{
    // Read byte from hardware.
    g_isr_gps_byte = (uint32_t) RLN30.LURDR.UINT16;

    // Cache the state of corresponding cbuf.
    g_isr_gps_front = g_isr_gps_ptr_cbuf->front;
    g_isr_gps_rear = g_isr_gps_ptr_cbuf->rear;

    // Enqueue at back if possible.
    g_isr_gps_new_rear = ( g_isr_gps_rear + 1 ) % g_isr_gps_ptr_cbuf->sizeMem;
    if ( g_isr_gps_new_rear != g_isr_gps_front )
    {
        // Queue is not full, so we can enqueue the received byte.
        g_isr_gps_ptr_cbuf->mem[g_isr_gps_rear] = g_isr_gps_byte;
        g_isr_gps_ptr_cbuf->rear = g_isr_gps_new_rear;
    }
}

#pragma interrupt isr_gsm(enable=true, fpu=false, callt=false)
void isr_gsm(void)
{
    // Read byte from hardware.
    g_isr_gsm_byte = (uint32_t) RLN31.LURDR.UINT16;

    // Cache the state of corresponding cbuf.
    g_isr_gsm_front = g_isr_gsm_ptr_cbuf->front;
    g_isr_gsm_rear = g_isr_gsm_ptr_cbuf->rear;

    // Enqueue at back if possible.
    g_isr_gsm_new_rear = ( g_isr_gsm_rear + 1 ) % g_isr_gsm_ptr_cbuf->sizeMem;
    if ( g_isr_gsm_new_rear != g_isr_gsm_front )
    {
        // Queue is not full, so we can enqueue the received byte.
        g_isr_gsm_ptr_cbuf->mem[g_isr_gsm_rear] = g_isr_gsm_byte;
        g_isr_gsm_ptr_cbuf->rear = g_isr_gsm_new_rear;
    }
}

#pragma interrupt isr_dbg(enable=true, fpu=false, callt=false)
void isr_dbg(void)
{
    // Read byte from hardware.
    g_isr_dbg_byte = (uint32_t) RLN32.LURDR.UINT16;

    // Cache the state of corresponding cbuf.
    g_isr_dbg_front = g_isr_dbg_ptr_cbuf->front;
    g_isr_dbg_rear = g_isr_dbg_ptr_cbuf->rear;

    // Enqueue at back if possible.
    g_isr_dbg_new_rear = ( g_isr_dbg_rear + 1 ) % g_isr_dbg_ptr_cbuf->sizeMem;
    if ( g_isr_dbg_new_rear != g_isr_dbg_front )
    {
        // Queue is not full, so we can enqueue the received byte.
        g_isr_dbg_ptr_cbuf->mem[g_isr_dbg_rear] = g_isr_dbg_byte;
        g_isr_dbg_ptr_cbuf->rear = g_isr_dbg_new_rear;
    }
}
