/**
 * @file        gps.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        30 November 2023
 * @author      Aditya Patil <aditya.patil@accoladeelectronics.com>
 *
 * @brief       File contains APIs for GPS modem
 */

#ifndef GPS_H
#define GPS_H

#include <stdint.h>
#include <stdarg.h>

typedef void (*logger_gps_t)(char* fmt, ...);

typedef struct
{
    uint32_t    utcTime;
    uint32_t    date;
    float       latitude;
    float       longitude;
    uint8_t     fixStatus;
    uint8_t     numOfSatellites;
    float       altitude;
    float       speed;
}GpsInfo_t;

/**
 * @brief                                   This function intentionally kept blank
 * @param   logger                          A printf like logger function.
 */
void GpsInit                                (logger_gps_t logger);

/**
 * @brief                                   Parses the GNGGA or GPGGA string.
 * @return                                  Returns the structure copy
 */
GpsInfo_t GpsGetInfo                        (void);

/**
 * @brief                                   Execute function for monitoring GPS UART and parsing NMEA data
 */
void GpsExe                                 (void);

#endif /* GPS_H */
