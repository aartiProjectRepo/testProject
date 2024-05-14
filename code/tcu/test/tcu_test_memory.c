/**
 * @file        tcu_test_memory.c
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
 * @brief       TCU-2W test code - memory test.
 */

// Standard includes.
#include <stdbool.h>
#include <stdint.h>

// Dependency.
#include "hal_util.h"

// HAL interface includes.
#include "hal_util.h"

// Private defines.
#define TCU_TEST_BUF_SIZE                   ( 10 )

// Uninitialized bss section.
#pragma section GRAMA
static uint8_t g_tcu_test_memory_mem_A_bss[TCU_TEST_BUF_SIZE];
#pragma section GRAMB
static uint8_t g_tcu_test_memory_mem_B_bss[TCU_TEST_BUF_SIZE];
#pragma section RRAM
static uint8_t g_tcu_test_memory_mem_R_bss[TCU_TEST_BUF_SIZE];
#pragma section default
static uint8_t g_tcu_test_memory_mem_L_bss[TCU_TEST_BUF_SIZE];

// Initialized data section.
#pragma section GRAMA
static uint8_t g_tcu_test_memory_mem_A_data[TCU_TEST_BUF_SIZE] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9};
#pragma section GRAMB
static uint8_t g_tcu_test_memory_mem_B_data[TCU_TEST_BUF_SIZE] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9};
#pragma section RRAM
static uint8_t g_tcu_test_memory_mem_R_data[TCU_TEST_BUF_SIZE] = {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9};
#pragma section
static uint8_t g_tcu_test_memory_mem_L_data[TCU_TEST_BUF_SIZE] = {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9};

// Restore default section.
#pragma section default

void tcu_test_memory(void)
{
    hal_util_assert ( true == hal_util_static_mem_check(g_tcu_test_memory_mem_A_bss, sizeof(g_tcu_test_memory_mem_A_bss), 0x0) );
    hal_util_assert ( true == hal_util_static_mem_check(g_tcu_test_memory_mem_B_bss, sizeof(g_tcu_test_memory_mem_B_bss), 0x0) );
    hal_util_assert ( true == hal_util_static_mem_check(g_tcu_test_memory_mem_R_bss, sizeof(g_tcu_test_memory_mem_R_bss), 0x0) );
    hal_util_assert ( true == hal_util_static_mem_check(g_tcu_test_memory_mem_L_bss, sizeof(g_tcu_test_memory_mem_L_bss), 0x0) );
    hal_util_assert ( true == hal_util_inc_mem_check(g_tcu_test_memory_mem_A_data, sizeof(g_tcu_test_memory_mem_A_data), 0xA0) );
    hal_util_assert ( true == hal_util_inc_mem_check(g_tcu_test_memory_mem_B_data, sizeof(g_tcu_test_memory_mem_B_data), 0xB0) );
    hal_util_assert ( true == hal_util_inc_mem_check(g_tcu_test_memory_mem_R_data, sizeof(g_tcu_test_memory_mem_R_data), 0xC0) );
    hal_util_assert ( true == hal_util_inc_mem_check(g_tcu_test_memory_mem_L_data, sizeof(g_tcu_test_memory_mem_L_data), 0xD0) );
    g_tcu_test_memory_mem_A_data[0] = 0xAA;
    g_tcu_test_memory_mem_B_data[0] = 0xBA;
    g_tcu_test_memory_mem_R_data[0] = 0xCA;
    g_tcu_test_memory_mem_L_data[0] = 0xDA;
    hal_util_assert ( 0xAA == g_tcu_test_memory_mem_A_data[0] );
    hal_util_assert ( 0xBA == g_tcu_test_memory_mem_B_data[0] );
    hal_util_assert ( 0xCA == g_tcu_test_memory_mem_R_data[0] );
    hal_util_assert ( 0xDA == g_tcu_test_memory_mem_L_data[0] );
}
