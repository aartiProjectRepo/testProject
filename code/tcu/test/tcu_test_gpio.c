/**
 * @file        gpio_test.c
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
 *
 * @brief       GPIO test code implementation with DIET PCB. This test will toggle the debug LEDS and read the IGN input.
 */

// Standard includes.
#include <stdbool.h>
#include <stdint.h>

// Dependency.
#include "hal_util.h"

// HAL interface includes.
#include "hal_gpio.h"

// Board includes.
#include "tcu_board.h"

// Extra includes.
#include "logger.h"

void tcu_test_gpio(void)
{
    HalGpioLevel_n level = HalGpioLevelMax;
    static HalGpioLevel_n stateIgnition = HalGpioLevelMax;
    static bool toggle = false;

    // Toggle LED1 - software method.
    if ( toggle )
    {
        hal_util_assert ( HalGpioErrOk == hal_gpio_write_level(g_GpioLed1, HalGpioLevelLow) );
    }
    else
    {
        hal_util_assert ( HalGpioErrOk == hal_gpio_write_level(g_GpioLed1, HalGpioLevelHigh) );
    }
    toggle = !toggle;

    // Toggle LED2 - hardware method.
    hal_util_assert ( HalGpioErrOk == hal_gpio_toggle(g_GpioLed2) );

    // Read IGN
    hal_util_assert ( HalGpioErrOk == hal_gpio_read_level(g_GpioIgn, &level) );
    if ( level != stateIgnition )
    {
        stateIgnition = level;
        logger("IGN: %d\r\n", level);
    }
}
