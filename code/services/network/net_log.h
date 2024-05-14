/**
 * @file        net_log.h
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
 * @brief       Net logger - header.
 */

#ifndef NET_LOG_H
#define NET_LOG_H

#include <stdarg.h>

typedef void (*logger_net_t) (char* fmt, ...);

void net_log_init                           (logger_net_t logger);

void net_log                                (char* fmt, ...);

#endif /* NET_LOG_H */