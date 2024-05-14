/**
* @file        tcu_test_ostime.c
*
* @copyright   Accolade Electronics Pvt Ltd, 2023-24
*              All Rights Reserved
*              UNPUBLISHED, LICENSED SOFTWARE.
*              Accolade Electronics, Pune
*              CONFIDENTIAL AND PROPRIETARY INFORMATION
*              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
*
* @date        27 November 2023
* @author      Pranali <pranali.kalekar@accoladeelectronics.com>
*
* @brief       TCU test code - OsTime test.
*/

/* Standard Includes */
#include <stdbool.h>
#include <stdint.h>


// HAL interface includes.
#include "hal_util.h"

// Logger include.
#include "logger.h"
#include "logger_can.h"

// Port includes.
#include "os_time.h"
#include "tcu_time.h"

// TCU 2W includes.
#include "tcu_test.h"

static os_time__epoch_time_s g_epochTime;
static os_time__epoch_time_s g_timeWithOffset;
static os_time__epoch_time_s g_sumEpochs;
static uint32_t g_timeMs;
static uint32_t g_timeUs;
static uint32_t g_elapsedTimeMs;
static uint64_t g_epochInMs;

// Local functions.
static void test_os_time_get_time_ms(void);
static void test_os_time_get_time_us(void);
static void test_os_time_get_time_elapsed_ms(void);
static void test_os_time_get_epoch_time(void);
static void test_os_time_get_epoch_in_ms(void);
static void test_os_time_get_time_with_offset(void);
static void test_os_time_sum_epochs(void);

void tcu_test_ostime(void)
{
    test_os_time_get_time_ms();
    test_os_time_get_time_us();
    test_os_time_get_time_elapsed_ms();
    test_os_time_get_epoch_time();
    test_os_time_get_epoch_in_ms();
    test_os_time_get_time_with_offset();
    test_os_time_sum_epochs();
}

static void test_os_time_get_time_ms(void)
{
    g_timeMs = os_time__get_time_ms();
    logger("ms = %u\r\n", g_timeMs);
}

static void test_os_time_get_time_us(void)
{
    g_timeUs = os_time__get_time_us();
    logger("us = %u\r\n", g_timeUs);
}

static void test_os_time_get_time_elapsed_ms(void)
{
    g_elapsedTimeMs = os_time__get_time_elapsed_ms(1);
    logger("Elapsed ms = %u\r\n", g_elapsedTimeMs);
}

static void test_os_time_get_epoch_time(void)
{
    os_time__get_epoch_time(&g_epochTime);
    logger("Epoch = %llu\r\n", g_epochTime.seconds);
}

static void test_os_time_get_epoch_in_ms(void)
{
    g_epochInMs = os_time__get_epoch_in_ms(g_epochTime);
    logger("Epoch ms = %llu\r\n", g_epochInMs);
}

static void test_os_time_get_time_with_offset(void)
{
    os_time__epoch_time_s offset = {2, 5000};
    g_timeWithOffset = os_time__get_time_with_offset(offset);
    logger("time With Offset %llu s  %llu ns\r\n", g_timeWithOffset.seconds, g_timeWithOffset.nanoseconds);
    //TODO:
}

static void test_os_time_sum_epochs(void)
{
    os_time__epoch_time_s epoch1 = {5, 9000};
    os_time__epoch_time_s epoch2 = {2, 8000};
    g_sumEpochs = os_time__sum_epochs(epoch1, epoch2);
    logger("sum %llu s  %llu ns\r\n", g_timeWithOffset.seconds, g_timeWithOffset.nanoseconds);
    //TODO:
}

void printUpTime()
{
    TcuTime_t uptime = tcu_time_uptime();
    logger("[TCU uptime %lus %lu us]\r\n", uptime.seconds, uptime.fractional);

    LoggerCan_u sysUpTime = {0};
    sysUpTime.sysA.uptimeMs = uptime.seconds; 
    logger_can_set(LoggerCan000_SysA, sysUpTime);
}
