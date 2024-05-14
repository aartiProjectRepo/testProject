/**
 * @file        nor_flash.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2021-22
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        9 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       NOR flash API.
 */

#ifndef NOR_FLASH_H
#define NOR_FLASH_H

// Dependencies.
#include <stdint.h>
#include <stddef.h>
#include "hal_spi.h"
#include "injectable.h"

// Page is the 'program' unit.
#define NOR_FLASH_PAGE_SIZE                 ( 256UL )
#define NOR_FLASH_PAGE_COUNT                ( 65536UL )

// Sector is the 'erase' unit.
#define NOR_FLASH_SECTOR_SIZE               ( 4096UL )
#define NOR_FLASH_SECTOR_COUNT              ( 4096UL )

// Capacity.
#define NOR_FLASH_CAPACITY_BYTES            ( NOR_FLASH_SECTOR_SIZE * NOR_FLASH_SECTOR_COUNT )

typedef enum
{
    NorFlashErrOk,                          /* Success. */
    NorFlashErrParam,                       /* Parameter error. */
    NorFlashErrForbidden,                   /* Interface usage prohibitions. */
    NorFlashErrLowLevel,                    /* Lower level module error (SPI error or Chip error). */
    NorFlashErrMax
} NorFlashErr_n;

/**
 * @brief                                   Initializes flash and opens underlying SPI interface.
 * @param       spiHandle                   An initialized HAL SPI handle.
 * @param       fnDelayMs                   A delay function with millisecond resolution.
 * @param       mutex                       A mutex.
 * @return                                  HalSpiErrOk:                Success.
 *                                          NorFlashErrLowLevel:        If any SPI low-level error.
 *                                          NorFlashErrForbidden:       If trying to initialize when already initialized.
*/
NorFlashErr_n nor_flash_init                (HalSpiHandle_t halSpiHandle, iface_v_oaf_32_t fnDelayMs, iface_mutex_t mutex);

/**
 * @brief                                   Reads arbitrary number of bytes.
 * @param       address                     Address to read from.
 * @param       readBuf                     Buffer into which data will be read.
 * @param       readLen                     Number of bytes to read.
 * @return                                  HalSpiErrOk:                Success.
 *                                          NorFlashErrLowLevel:        If any SPI low-level error.
 *                                          NorFlashErrParam            Invalid parameter value.
 *                                          NorFlashErrForbidden:       If trying to use without initialization.
*/
NorFlashErr_n nor_flash_read                (const uint32_t address, uint8_t* const readBuf, const size_t readLen);

/**
 * @brief                                   Writes arbitrary number of bytes.
 * @param       address                     Address to write to.
 * @param       writeBuf                    Buffer containing data to be written.
 * @param       writeLen                    Number of bytes to write.
 * @return                                  HalSpiErrOk:                Success.
 *                                          NorFlashErrLowLevel:        If any SPI low-level error.
 *                                          NorFlashErrParam            Invalid parameter value.
 *                                          NorFlashErrForbidden:       If trying to use without initialization.
*/
NorFlashErr_n nor_flash_write               (const uint32_t address, const uint8_t* const writeBuf, const size_t writeLen);

/**
 * @brief                                   Erases a sector.
 * @param       sector                      A valid sector number.
 * @return                                  HalSpiErrOk:                Success.
 *                                          NorFlashErrLowLevel:        If any SPI low-level error.
 *                                          NorFlashErrParam            Invalid parameter value.
 *                                          NorFlashErrForbidden:       If trying to use without initialization.
*/
NorFlashErr_n nor_flash_erase               (const uint32_t sector);

#endif /* NOR_FLASH_H */
