/**
 * @file        tcu_time.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        11 December 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU time provider - implementation based on free running TAUJ @ 1ms counter interval.
 */

#include "tcu_time.h"
#include "iodefine.h"
#include "hal_util.h"

#define TCU_CPU_CLOCK_HZ                    ( 240UL * 1000UL * 1000UL )
#define TCU_TIMER_MAX_COUNTER_VALUE         ( 0xFFFFFFFFUL )
#define TCU_CPU_CLOCK_L_HZ                  ( TCU_CPU_CLOCK_HZ / 4 )

#define TCU_TIME_INVERTED_COUNTER(x)        ( TCU_TIMER_MAX_COUNTER_VALUE - ( ( uint32_t ) ( x ) ) )
#define TCU_TIME_TICK_TO_SECONDS(ticks)     ( ( ( ticks ) / ( TCU_CPU_CLOCK_L_HZ ) ) )
#define TCU_TIME_FRACTION(ticks)            ( ( ( ( ticks ) % ( TCU_CPU_CLOCK_L_HZ )) * 1000000  ) / TCU_CPU_CLOCK_L_HZ )

#define TCU_TIME_LOCK
#define TCU_TIME_UNLOCK

// Last timer counter value - used for handling counter overflow.
static uint32_t g_lastCounterValue = 0;
static uint32_t g_numCounterRollover = 0;
static TcuTime_t g_uptime;
static TcuTime_t g_epochOffset;

static TcuTime_t tcu_time_prv_uptime        (void);
static TcuTime_t tcu_time_prv_epoch         (void);
static void tcu_time_prv_set_epoch          (TcuTime_t epochOffset);
static void tcu_time_update_state           (void);

void tcu_time_init                          (void)
{
    TCU_TIME_LOCK;
    OSTM1.CMP = TCU_TIMER_MAX_COUNTER_VALUE; /* 32bit register, This register stores the start value of the down-counter */
    OSTM1.CTL = 0x00;                        /* 8bit register, This register specifies the operating mode OSTMTINT interrupt requests. */
    OSTM1.TS = 0x01;                         /* 8bit register, This register starts the counter. */
    
    hal_util_memset(&g_uptime, 0x00, sizeof(g_uptime));
    hal_util_memset(&g_epochOffset, 0x00, sizeof(g_epochOffset));
    TCU_TIME_UNLOCK;
    return;
}

TcuTime_t tcu_time_uptime                   (void)
{
    TcuTime_t uptime;

    TCU_TIME_LOCK;
    uptime = tcu_time_prv_uptime();
    TCU_TIME_UNLOCK;
    return uptime;
}

TcuTime_t tcu_time_epoch                    (void)
{
    TcuTime_t epochTime;
    TCU_TIME_LOCK;
    epochTime = tcu_time_prv_epoch();
    TCU_TIME_UNLOCK;
    return epochTime;
}

void tcu_time_set_epoch                     (TcuTime_t epochOffset)
{
    TCU_TIME_LOCK;
    tcu_time_prv_set_epoch(epochOffset);
    TCU_TIME_UNLOCK;
}

static void tcu_time_update_state           (void)
{
    uint32_t counterNow;

    counterNow = TCU_TIME_INVERTED_COUNTER(OSTM1.CNT);     /* 32bit register, This register indicates the counter value of the timer. */
    if (counterNow < g_lastCounterValue )
    {
        // Counter rolled over.
        g_numCounterRollover++;
    }
    g_lastCounterValue = counterNow;
}

static TcuTime_t tcu_time_prv_uptime        (void)
{
    uint64_t totalTick;

    tcu_time_update_state();
    totalTick = (uint64_t)g_numCounterRollover * TCU_TIMER_MAX_COUNTER_VALUE + g_lastCounterValue;
    g_uptime.seconds = (uint32_t)TCU_TIME_TICK_TO_SECONDS(totalTick);
    g_uptime.fractional = (uint32_t)TCU_TIME_FRACTION(totalTick);
    return g_uptime;
}

static TcuTime_t tcu_time_prv_epoch         (void)
{
    TcuTime_t epochTime;

    epochTime.fractional = g_uptime.fractional + g_epochOffset.fractional;
    epochTime.seconds = (epochTime.fractional / 1000) + g_uptime.seconds + g_epochOffset.seconds;

    return epochTime;
}

static void tcu_time_prv_set_epoch(TcuTime_t epochOffset)
{
    g_epochOffset = epochOffset;
}
