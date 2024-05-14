/**
 * @file        net_log.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        1 January 2024
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Net logger - implementation.
 */

#include "net_log.h"
#include "printf.h"

static void net_dummy_logger                (char* fmt, ...);

static logger_net_t g_logger = net_dummy_logger;

void net_log_init                           (logger_net_t logger)
{
    if ( logger )
    {
        g_logger = logger;
    }
}

void net_log                                (char* fmt, ...)
{
    char printMem[128UL] = {0};
    va_list args;
    
    va_start(args, fmt);
    (void) vsnprintf(printMem, sizeof(printMem), fmt, args);
    va_end(args);
    g_logger("%s", printMem);
}

static void net_dummy_logger                (char* fmt, ...)
{
    (void) fmt;
}
