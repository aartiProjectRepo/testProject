/**
 * @file        nor_flash.c
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

// Standard
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
// Self
#include "nor_flash.h"
// Dependencies.
#include "hal_util.h"
#include "SPI_Driver.h" //FIXME: Entangles GPIO and SPI....

// Private defines.
#define SPI_Ch                              ( 3 )
#define TIMEOUT_MS_WRITE                    ( 3 )
#define TIMEOUT_MS_ERASE                    ( 400 )
#define REG_CHIP_ID                         ( 0x90 )
#define REG_READ                            ( 0x03 )
#define REG_PROGRAM                         ( 0x02 )
#define REG_ERASE                           ( 0x20 )
#define REG_STATUS                          ( 0x05 )
#define REG_WREN                            ( 0x06 )
#define NOR_FLASH_LOCK()                    if ( g_ctx.mutex.lock )     { g_ctx.mutex.lock();   }
#define NOR_FLASH_UNLOCK()                  if ( g_ctx.mutex.unlock )   { g_ctx.mutex.unlock(); }

// Private globals.
typedef struct
{
    HalSpiHandle_t handle;
    iface_v_oaf_32_t delayMs;
    iface_mutex_t mutex;
    const HalSpiConfig_t config;
    bool isInit;
} NorFlashCtx_t;

static NorFlashCtx_t g_ctx;

// Private functions.
static void chip_select_activate            (void);
static void chip_select_deactivate          (void);
static NorFlashErr_n read_chip_id           (uint8_t* manufacturerId, uint8_t* deviceId);
static NorFlashErr_n read_status_register   (uint8_t* status);
static NorFlashErr_n write_enable           (void);
static NorFlashErr_n wait_complete          (uint32_t timeoutMs);
static NorFlashErr_n page_write             (const uint32_t address, const uint8_t* const writeBuf, const size_t writeLen);

NorFlashErr_n nor_flash_init                (HalSpiHandle_t halSpiHandle, iface_v_oaf_32_t fnDelayMs, iface_mutex_t mutex)
{
    NorFlashErr_n err = NorFlashErrParam;
    NorFlashCtx_t* ctx = &g_ctx;
    uint8_t manufacturerId = 0;
    uint8_t deviceId = 0;
    
    if ( halSpiHandle && fnDelayMs && mutex.lock && mutex.unlock )
    {
        g_ctx.handle = halSpiHandle;
        g_ctx.delayMs = fnDelayMs;
        g_ctx.mutex = mutex;
        if ( false == ctx->isInit )
        {
            err = read_chip_id(&manufacturerId, &deviceId);
            if ( NorFlashErrOk == err )
            {
                // The IDs should be as per data-sheet.
                hal_util_assert ( 0xEF == manufacturerId );
                hal_util_assert ( 0x17 == deviceId );
                ctx->isInit = true;
            }
        }
        else
        {
            err = NorFlashErrForbidden;
        }
    }

    return err;
}

NorFlashErr_n nor_flash_read                (const uint32_t address, uint8_t* const readBuf, const size_t readLen)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    NorFlashCtx_t* ctx = &g_ctx;
    uint8_t cmdRead[4] = {REG_READ, 0, 0, 0};
    HalBuffer_t buffer = {0};
    
    NOR_FLASH_LOCK();
    if ( true == ctx->isInit )
    {
        if ( readBuf && readLen && ( ( address + readLen ) <= NOR_FLASH_CAPACITY_BYTES ) )
        {
            chip_select_activate();
            cmdRead[1] = (uint8_t) ( address >> 16 );
            cmdRead[2] = (uint8_t) ( address >> 8 );
            cmdRead[3] = (uint8_t) ( address >> 0 );
            buffer.mem = cmdRead;
            buffer.sizeMem = sizeof(cmdRead);
            if ( HalSpiErrOk == hal_spi_write(ctx->handle, buffer, buffer.sizeMem) )
            {
                buffer.mem = readBuf;
                buffer.sizeMem = readLen;
                if ( HalSpiErrOk == hal_spi_read(ctx->handle, buffer, buffer.sizeMem) )
                {
                    err = NorFlashErrOk;
                }
            }
            chip_select_deactivate();
        }
        else
        {
            err = NorFlashErrParam;
        }
    }
    else
    {
        err = NorFlashErrForbidden;
    }
    NOR_FLASH_UNLOCK();

    return err;
}

NorFlashErr_n nor_flash_write               (const uint32_t address, const uint8_t* const writeBuf, const size_t writeLen)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    NorFlashCtx_t* ctx = &g_ctx;
    size_t nowAddress = address;
    size_t nowLen = 0;
    size_t remLen = writeLen;

    NOR_FLASH_LOCK();
    if ( true == ctx->isInit )
    {
        if ( writeBuf && writeLen && ( ( address + writeLen ) <= NOR_FLASH_CAPACITY_BYTES ) )
        {
            while ( remLen )
            {
                nowLen = NOR_FLASH_PAGE_SIZE - ( nowAddress % NOR_FLASH_PAGE_SIZE );
                if ( nowLen > remLen )
                {
                    nowLen = remLen;
                }
                if ( NorFlashErrOk == page_write(nowAddress, &writeBuf[nowAddress - address], nowLen) )
                {
                    nowAddress += nowLen;
                    remLen -= nowLen;
                }
                else
                {
                    break;
                }
            }
            if ( 0 == remLen )
            {
                err = NorFlashErrOk;
            }
        }
        else
        {
            err = NorFlashErrParam;
        }
    }
    else
    {
        err = NorFlashErrForbidden;
    }
    NOR_FLASH_UNLOCK();

    return err;
}

NorFlashErr_n nor_flash_erase               (const uint32_t sector)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    NorFlashCtx_t* ctx = &g_ctx;
    uint32_t address = 0;
    uint8_t cmdErase[4] = {REG_ERASE, 0, 0, 0};
    HalBuffer_t buffer = {0};
    
    NOR_FLASH_LOCK();
    if ( true == ctx->isInit )
    {
        if ( sector < NOR_FLASH_SECTOR_COUNT )
        {
            if ( NorFlashErrOk == write_enable() )
            {
                chip_select_activate();
                address = sector << 12;
                cmdErase[1] = (uint8_t) ( address >> 16 );
                cmdErase[2] = (uint8_t) ( address >> 8  );
                cmdErase[3] = (uint8_t) ( address >> 0  );
                buffer.mem = cmdErase;
                buffer.sizeMem = sizeof(cmdErase);
                if ( HalSpiErrOk == hal_spi_write(ctx->handle, buffer, buffer.sizeMem) )
                {
                    err = NorFlashErrOk;
                }
                chip_select_deactivate();
            }
            if ( NorFlashErrOk == err )
            {
                err = wait_complete(TIMEOUT_MS_ERASE);
            }
        }
        else
        {
            err = NorFlashErrParam;
        }
    }
    else
    {
        err = NorFlashErrForbidden;
    }
    NOR_FLASH_UNLOCK();

    return err;
}

static void chip_select_activate            (void)
{
    R_PORT_SetGpioOutput(SPITblList[SPI_Ch].CsPort, SPITblList[SPI_Ch].CsPin, SPITblList[SPI_Ch].CsLvlLow);
}

static void chip_select_deactivate          (void)
{
    R_PORT_SetGpioOutput(SPITblList[SPI_Ch].CsPort, SPITblList[SPI_Ch].CsPin, SPITblList[SPI_Ch].CsLvlHigh);
}

static NorFlashErr_n read_chip_id           (uint8_t* manufacturerId, uint8_t* deviceId)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    NorFlashCtx_t* ctx = &g_ctx;
    uint8_t cmdChipId[4] = {REG_CHIP_ID, 0x0, 0x0, 0x0};
    uint8_t datChipId[2] = {0};
    HalBuffer_t buffer = {0};
    
    chip_select_activate();
    buffer.mem = cmdChipId;
    buffer.sizeMem = sizeof(cmdChipId);
    if ( HalSpiErrOk == hal_spi_write(ctx->handle, buffer, buffer.sizeMem) )
    {
        buffer.mem = datChipId;
        buffer.sizeMem = sizeof(datChipId);
        if ( HalSpiErrOk == hal_spi_read(ctx->handle, buffer, buffer.sizeMem) )
        {
            *manufacturerId = buffer.mem[0];
            *deviceId = buffer.mem[1];
            err = NorFlashErrOk;
        }
    }
    chip_select_deactivate();

    return err;
}

static NorFlashErr_n read_status_register   (uint8_t* status)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    NorFlashCtx_t* ctx = &g_ctx;
    uint8_t cmdStatus[1] = {REG_STATUS};
    uint8_t datStatus[1] = {0};
    HalBuffer_t buffer = {0};

    chip_select_activate();
    buffer.mem = cmdStatus;
    buffer.sizeMem = sizeof(cmdStatus);
    if ( HalSpiErrOk == hal_spi_write(ctx->handle, buffer, buffer.sizeMem) )
    {
        buffer.mem = datStatus;
        buffer.sizeMem = sizeof(datStatus);
        if ( HalSpiErrOk == hal_spi_read(ctx->handle, buffer, buffer.sizeMem) )
        {
            *status = buffer.mem[0];
            err = NorFlashErrOk;
        }
    }
    chip_select_deactivate();

    return err;
}

static NorFlashErr_n write_enable           (void)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    NorFlashCtx_t* ctx = &g_ctx;
    uint8_t cmdWriteEnable[1] = {REG_WREN};
    HalBuffer_t buffer = {cmdWriteEnable, sizeof(cmdWriteEnable)};

    chip_select_activate();
    if ( HalSpiErrOk == hal_spi_write(ctx->handle, buffer, buffer.sizeMem) )
    {
        err = NorFlashErrOk;
    }
    chip_select_deactivate();

    return err;
}

static NorFlashErr_n wait_complete          (uint32_t timeoutMs)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    uint8_t status = {0};

    timeoutMs += 1;
    while ( timeoutMs-- )
    {
        g_ctx.delayMs(1);
        if ( NorFlashErrOk == read_status_register(&status) )
        {
            if ( 0 == ( status & 0x03 ) )
            {
                err = NorFlashErrOk;
                break;
            }
        }
        else
        {
            break;
        }
    }

    return err;
}

static NorFlashErr_n page_write             (const uint32_t address, const uint8_t* const writeBuf, const size_t writeLen)
{
    NorFlashErr_n err = NorFlashErrLowLevel;
    NorFlashCtx_t* ctx = &g_ctx;
    uint8_t cmdWrite[4] = {REG_PROGRAM, 0, 0, 0};
    HalBuffer_t buffer = {0};

    if ( NorFlashErrOk == write_enable() )
    {
        chip_select_activate();
        cmdWrite[1] = (uint8_t) ( address >> 16 );
        cmdWrite[2] = (uint8_t) ( address >> 8  );
        cmdWrite[3] = (uint8_t) ( address >> 0  );
        buffer.mem = cmdWrite;
        buffer.sizeMem = sizeof(cmdWrite);
        if ( HalSpiErrOk == hal_spi_write(ctx->handle, buffer, buffer.sizeMem) )
        {
            buffer.mem = (uint8_t*) writeBuf;
            buffer.sizeMem = writeLen;
            if ( HalSpiErrOk == hal_spi_write(ctx->handle, buffer, buffer.sizeMem) )
            {
                err = NorFlashErrOk;
            }
        }
        chip_select_deactivate();
    }
    if ( NorFlashErrOk == err )
    {
        err = wait_complete(TIMEOUT_MS_WRITE);
    }

    return err;
}
