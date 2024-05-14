/**
 * @file        tcu_test_flash.c
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
 * @brief       TCU test code - SPI flash test.
 */

// Standard includes.
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Board includes.
#include "tcu_board.h"

// Dependency.
#include "hal_util.h"

// Port includes.
#include "nor_flash.h"

// LittleFs includes.
#include "littlefs_adapter.h"
#include "filesystem.h"
#include "dev_file_io.h"

// TCU includes.
#include "tcu_test.h"
#include "tcu_locks.h"
#include "self_test_dev_file_io.h"

// Private data.
#pragma section GRAMB
static uint8_t                              g_tcu_test_flash_mem[2 * NOR_FLASH_SECTOR_SIZE];
#pragma section default

// Private functions.
static void flash_usage_test(void);
static void fs_mount_partitions(void);
static void fs_boot_test(void);
static void fs_seek_test(void);

void tcu_test_flash(void)
{
    // NOR flash init and test.
    flash_usage_test();

    // Filesystem init and test.
    littlefs_adapter_init(tcu_lock_get(TcuLocksModuleLfsAdapter));
    filesystem_inject_lock(tcu_lock_get(TcuLocksModuleFilesystem).lock);
    filesystem_inject_unlock(tcu_lock_get(TcuLocksModuleFilesystem).unlock);
    fs_mount_partitions();
    fs_boot_test();
    fs_seek_test();
}

static void flash_usage_test(void)
{
    const size_t SLICE = 315;
    size_t offset;

    // Initialize flash.
    hal_util_assert ( NorFlashErrOk == nor_flash_init(g_SpiFlash, hal_util_delay, tcu_lock_get(TcuLocksModuleNorFlash)) );
    // Initial erase test.
    hal_util_assert ( true == hal_util_static_mem_check(g_tcu_test_flash_mem, sizeof(g_tcu_test_flash_mem), 0) );
    hal_util_assert ( NorFlashErrOk == nor_flash_erase(4094UL) );
    hal_util_assert ( NorFlashErrOk == nor_flash_erase(4095UL) );
    hal_util_assert ( NorFlashErrOk == nor_flash_read(4094UL * 4096UL, g_tcu_test_flash_mem, sizeof(g_tcu_test_flash_mem)) );
    hal_util_assert ( true == hal_util_static_mem_check(g_tcu_test_flash_mem, sizeof(g_tcu_test_flash_mem), 0xFF) );

    // Write-read test.
    offset = 0;
    hal_util_memset(g_tcu_test_flash_mem, 0, sizeof(g_tcu_test_flash_mem));
    for ( uint8_t seed = 'A' ; seed <= 'Z' ; seed++ )
    {
        hal_util_inc_mem_create(&g_tcu_test_flash_mem[offset], SLICE, seed);
        hal_util_assert ( NorFlashErrOk == nor_flash_write( ( 4094UL * 4096UL ) + offset, &g_tcu_test_flash_mem[offset], SLICE) );
        offset += SLICE;
    }
    offset = 0;
    hal_util_memset(g_tcu_test_flash_mem, 0, sizeof(g_tcu_test_flash_mem));
    for ( uint8_t seed = 'A' ; seed <= 'Z' ; seed++ )
    {
        hal_util_assert ( NorFlashErrOk == nor_flash_read( ( 4094UL * 4096UL ) + offset, &g_tcu_test_flash_mem[offset], SLICE) );
        hal_util_assert ( true == hal_util_inc_mem_check(&g_tcu_test_flash_mem[offset], SLICE, seed) );
        offset += SLICE;
    }
}

static void fs_mount_partitions(void)
{
    for (Filesystem_n fs = FilesystemLfs0 ; fs < FilesystemRaw; ++fs )
    {
        if ( FilesystemErrOk != filesystem_mount(fs) )
        {
            hal_util_assert ( FilesystemErrOk == filesystem_format(fs) );
            hal_util_assert ( FilesystemErrOk == filesystem_mount(fs) );
        }
    }
}

static void fs_boot_test(void)
{
    FilesystemFd_t fd = NULL;
    Filesystem_n fs = FilesystemLfs0;
    const char filePath[] = "boot_count.dat";
    uint32_t boot_count = 0;
    uint32_t count = UINT32_MAX;
    
    // Filesystem must be mounted.
    hal_util_assert ( filesystem_is_mounted(fs) );
    
    // Read current count.
    hal_util_assert ( FilesystemErrOk == filesystem_fopen(fs, &fd, filePath, (FilesystemOpenMode_n) (LFS_O_RDWR | LFS_O_CREAT)) );
    hal_util_assert ( FilesystemErrOk == filesystem_fread(fd, &boot_count, sizeof(boot_count), &count) );
    hal_util_assert ( ( 0 == count ) || ( 4 == count ) );

    // Increment and store boot count.
    boot_count++;
    hal_util_assert ( FilesystemErrOk == filesystem_frewind(fd) );
    hal_util_assert ( FilesystemErrOk == filesystem_fwrite(fd, &boot_count, sizeof(boot_count), &count) );
    hal_util_assert ( 4 == count );
    
    // Close and reopen file, then re-read the count.
    count = UINT32_MAX;
    hal_util_assert ( FilesystemErrOk == filesystem_fclose(fd) );
    fd = NULL;
    hal_util_assert ( FilesystemErrOk == filesystem_fopen(fs, &fd, filePath, LFS_O_RDONLY) );
    hal_util_assert ( FilesystemErrOk == filesystem_fread(fd, &boot_count, sizeof(boot_count), &count) );
    hal_util_assert ( 4 == count );

    // Close file.
    hal_util_assert ( FilesystemErrOk == filesystem_fclose(fd) );
}

static void fs_seek_test(void)
{
    FilesystemFd_t fd = NULL;
    Filesystem_n fs = FilesystemLfs0;
    const char kFileName[] = "big_file.dat";
    const int kLoopCount = 10;
    const uint8_t seed = 'A';
    int i = 0;
    uint32_t u32Var;
    FilesystemNodeInfo_t info;

    // Filesystem must be mounted.
    hal_util_assert ( filesystem_is_mounted(fs) );

    // Open the file in R+ and truncate.
    hal_util_assert ( FilesystemErrOk == filesystem_fopen(fs, &fd, kFileName, (FilesystemOpenMode_n) (LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC)) );
    hal_util_assert ( FilesystemErrOk == filesystem_frewind(fd) );

    // Write testing.
    for ( i = 0 ; i < kLoopCount ; ++i )
    {
        // Clear test buffer.
        hal_util_memset(g_tcu_test_flash_mem, 0, sizeof(g_tcu_test_flash_mem));
        // Create incremental data into user buffer.
        hal_util_inc_mem_create(g_tcu_test_flash_mem, sizeof(g_tcu_test_flash_mem), seed + i);
        // Write buffer onto file.
        hal_util_assert ( FilesystemErrOk == filesystem_fwrite(fd, g_tcu_test_flash_mem, sizeof(g_tcu_test_flash_mem), &u32Var) );
        hal_util_assert ( sizeof(g_tcu_test_flash_mem) == u32Var );
        // Confirm position of cursor.
        hal_util_assert ( FilesystemErrOk == filesystem_ftell(fd, &u32Var) );
        hal_util_assert ( sizeof(g_tcu_test_flash_mem) * ( i + 1 ) == u32Var );
    }

    // Close and re-open testing.
    hal_util_assert ( FilesystemErrOk == filesystem_fclose(fd) );
    fd = NULL;
    hal_util_assert ( FilesystemErrOk == filesystem_fopen(fs, &fd, kFileName, LFS_O_RDONLY) );

    // Stat testing.
    hal_util_assert ( FilesystemErrOk == filesystem_stat(fs, kFileName, &info) );
    hal_util_assert ( LFS_TYPE_REG == info.lfs_info.type );
    hal_util_assert ( sizeof(g_tcu_test_flash_mem) * kLoopCount == info.lfs_info.size );
    hal_util_assert ( 0 == memcmp(kFileName, info.lfs_info.name, sizeof(kFileName)) );

    // File size API testing.
    hal_util_assert ( FilesystemErrOk == filesystem_fsize(fd, &u32Var) );
    hal_util_assert ( sizeof(g_tcu_test_flash_mem) * kLoopCount == u32Var );

    // Read validity testing.
    for ( --i ; i >= 0 ; --i )
    {
        // Clear test buffer.
        hal_util_memset(g_tcu_test_flash_mem, 0, sizeof(g_tcu_test_flash_mem));
        // Set cursor.
        hal_util_assert ( FilesystemErrOk == filesystem_fseek(fd, sizeof(g_tcu_test_flash_mem) * i, LFS_SEEK_SET, &u32Var) );
        hal_util_assert ( u32Var == sizeof(g_tcu_test_flash_mem) * i );
        // Read from cursor.
        hal_util_assert ( FilesystemErrOk == filesystem_fread(fd, g_tcu_test_flash_mem, sizeof(g_tcu_test_flash_mem), &u32Var) );
        hal_util_assert ( u32Var == sizeof(g_tcu_test_flash_mem) );
        // Confirm cursor position.
        hal_util_assert ( FilesystemErrOk == filesystem_ftell(fd, &u32Var) );
        hal_util_assert ( u32Var == sizeof(g_tcu_test_flash_mem) * ( i + 1 ) );
        // Check validity of buffer.
        hal_util_assert ( true == hal_util_inc_mem_check(g_tcu_test_flash_mem, sizeof(g_tcu_test_flash_mem), seed + i) );
    }

    // Close file.
    hal_util_assert ( FilesystemErrOk == filesystem_fclose(fd) );
}
