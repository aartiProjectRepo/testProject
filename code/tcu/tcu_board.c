/**
 * @file        tcu_board.c
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
 * @brief       TCU board.
 */

// Self.
#include "tcu_board.h"

// Startup includes (PinMux and INTC2).
#include "r_cg_cgc.h"
#include "Config_OSTM0.h"
#include "rh850_pinmux.h"
#include "rh850_intc1.h"
#include "rh850_intc2.h"

// Port dependencies.
#include "port_defs.h"
#include "rh850_gpio.h"
#include "rh850_uart.h"
#include "rh850_spi.h"
#include "drv_can.h"
#include "hal_util.h"
#include "tcu_time.h"

// Board interface handles.
volatile HalGpioHandle_t g_GpioLed1;
volatile HalGpioHandle_t g_GpioLed2;
volatile HalGpioHandle_t g_GpioIgn;
volatile HalUartHandle_t g_UartGps;
volatile HalUartHandle_t g_UartGsm;
volatile HalUartHandle_t g_UartDbg;
volatile HalSpiHandle_t g_SpiEeprom;
volatile HalSpiHandle_t g_SpiAccel;
volatile HalSpiHandle_t g_SpiFlash;

// Private functions.
static void tcu_board_init_gpio             (HalGpioHandle_t* hGpio, PortDefsGpio_n nGpio, HalGpioConfig_t cfg);
static void tcu_board_init_uart             (HalUartHandle_t* hUart, PortDefsUart_n nUart, HalUartConfig_t cfg);
static void tcu_board_init_spi              (HalSpiHandle_t* hSpi, PortDefsSpi_n nSpi, HalSpiConfig_t cfg);

// Board initialization.
void tcu_board_init(void)
{
    HalGpioConfig_t cfgGpioInput = { .direction = HalGpioDirectionInput };
    HalGpioConfig_t cfgGpioOutput = { .direction = HalGpioDirectionOutput };
    HalUartConfig_t cfgGsmUart = { .baud = 115200UL };
    HalUartConfig_t cfgGpsUart = { .baud = 115200UL };
    HalUartConfig_t cfgDbgUart = { .baud = 115200UL };
    HalSpiConfig_t cfgSpi = { .someConfig = 123 };
    
    // Disable interrupts.
    __DI();

    // Clock init.
    R_CGC_Create();

    // OS Timer create.
    R_Config_OSTM0_Create();
    tcu_time_init();

    // UART pin-mux and interrupt init.
    rh850_intc2_uart_init(PortDefsUartGps);
    rh850_pinmux_rlin3(PortDefsUartGps);
    rh850_intc2_uart_init(PortDefsUartGsm);
    rh850_pinmux_rlin3(PortDefsUartGsm);
    rh850_intc2_uart_init(PortDefsUartDbg);
    rh850_pinmux_rlin3(PortDefsUartDbg);

    // CAN pin and interrupt init
    rh850_pinmux_rcan();
    rh850_intc1_can_init();
    
    // Init peripherals.
    drv_can_bInit();
    tcu_board_init_gpio((HalGpioHandle_t*) &g_GpioLed1, PortDefsGpioDebugLed1, cfgGpioOutput);
    tcu_board_init_gpio((HalGpioHandle_t*) &g_GpioLed2, PortDefsGpioDebugLed2, cfgGpioOutput);
    tcu_board_init_gpio((HalGpioHandle_t*) &g_GpioIgn, PortDefsGpioIgnInMcu, cfgGpioInput);
    tcu_board_init_uart((HalUartHandle_t*) &g_UartGsm, PortDefsUartGsm, cfgGsmUart);
    tcu_board_init_uart((HalUartHandle_t*) &g_UartGps, PortDefsUartGps, cfgGpsUart);
    tcu_board_init_uart((HalUartHandle_t*) &g_UartDbg, PortDefsUartDbg, cfgDbgUart);
    tcu_board_init_spi((HalSpiHandle_t*) &g_SpiAccel, PortDefsSpiAccelerometer, cfgSpi);
    tcu_board_init_spi((HalSpiHandle_t*) &g_SpiEeprom, PortDefsSpiEeprom, cfgSpi);
    tcu_board_init_spi((HalSpiHandle_t*) &g_SpiFlash, PortDefsSpiFlash, cfgSpi);

    // Enable interrupts.
    __EI();
}

static void tcu_board_init_gpio             (HalGpioHandle_t* hGpio, PortDefsGpio_n nGpio, HalGpioConfig_t cfg)
{
    HalGpioIdentity_t id = NULL;
    HalGpioIdentity_t verifyId = NULL;
    
    hal_util_assert ( hGpio );
    hal_util_assert ( !*hGpio );
    hal_util_assert ( HalGpioErrOk == rh850_gpio_get_identity(&id, nGpio) );
    hal_util_assert ( id );
    hal_util_assert ( HalGpioErrOk == hal_gpio_create(hGpio, id) );
    hal_util_assert ( *hGpio );
    hal_util_assert ( HalGpioErrOk == hal_gpio_init(*hGpio, cfg) );
    hal_util_assert ( HalGpioErrOk == hal_gpio_get_identity(*hGpio, &verifyId) );
    hal_util_assert ( id == verifyId );
}

static void tcu_board_init_uart             (HalUartHandle_t* hUart, PortDefsUart_n nUart, HalUartConfig_t cfg)
{
    HalUartIdentity_t id = NULL;
    HalUartIdentity_t verifyId = NULL;
    
    hal_util_assert ( hUart );
    hal_util_assert ( !*hUart );
    hal_util_assert ( HalUartErrOk == rh850_uart_get_identity(&id, nUart) );
    hal_util_assert ( id );
    hal_util_assert ( HalUartErrOk == hal_uart_create(hUart, id) );
    hal_util_assert ( *hUart );
    hal_util_assert ( HalUartErrOk == hal_uart_init(*hUart, cfg) );
    hal_util_assert ( HalUartErrOk == hal_uart_open(*hUart) );
    hal_util_assert ( HalUartErrOk == hal_uart_get_identity(*hUart, &verifyId) );
    hal_util_assert ( id == verifyId );
}

static void tcu_board_init_spi              (HalSpiHandle_t* hSpi, PortDefsSpi_n nSpi, HalSpiConfig_t cfg)
{
    HalSpiIdentity_t id = NULL;
    HalSpiIdentity_t verifyId = NULL;
    
    hal_util_assert ( hSpi );
    hal_util_assert ( !*hSpi );
    hal_util_assert ( HalSpiErrOk == rh850_spi_get_identity(&id, nSpi) );
    hal_util_assert ( id );
    hal_util_assert ( HalSpiErrOk == hal_spi_create(hSpi, id) );
    hal_util_assert ( *hSpi );
    hal_util_assert ( HalSpiErrOk == hal_spi_init(*hSpi, cfg) );
    hal_util_assert ( HalSpiErrOk == hal_spi_open(*hSpi) );
    hal_util_assert ( HalSpiErrOk == hal_spi_get_identity(*hSpi, &verifyId) );
    hal_util_assert ( id == verifyId );
}
