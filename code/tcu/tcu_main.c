/**
 * @file        tcu_main.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        27 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU-2W user main entry point - source.
 */

// Standard includes.
#include <stdint.h>

// TCU 2W includes.
#include "tcu_board.h"
#include "tcu_tasks.h"

// This value may be honoured by the FreeRTOS port in the future.
uint32_t SystemCoreClock = 240UL * 1000UL * 1000UL;

void tcu_main(void)
{
    // Initialize board.
    tcu_board_init();

    // Kick off tasks.
    tcu_tasks();

    // Must never reach here.
    while(1);
}
