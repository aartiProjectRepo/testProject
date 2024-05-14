/**
 * @file        logger.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        01 January 2024
 * @author      Aditya P <aditya.prajapati@accoladeelectronics.com>
 *              Diksha J <diksha.jadhav@accoladeelectronics.com>
 *              Adwait P <adwait.patil@accoladeelectronics.com>
 * 
 * @brief       Logger headers
 * 
 * @warning     Logger is the unprotected system logger.
 *              It should not be used directly by application writers.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdarg.h>

/**
 * @brief                                   Writes formatted characters to the debug port.
 * @param   fmt                             Format string specifying the format of the additional arguments.
 * @param   ...                             Additional arguments to be formatted and written.
 */
void logger                                 (const char *fmt, ...);

/**
 * @brief                                   Reads characters from the debug port into the provided buffer.
 * @param   rxBuff                          Pointer to the buffer to store received characters.
 * @param   maxBuffSize                     Maximum size of the buffer.
 * @return                                  Number of bytes actually read.
 */
uint32_t logger_read                        (uint8_t *rxBuff, uint32_t maxBuffSize);

#endif /* LOGGER_H */
