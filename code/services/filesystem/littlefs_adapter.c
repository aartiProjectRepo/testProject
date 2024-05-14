/**
 * @file        littlefs_adapter.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        28 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *              Pandurang Nagargoje <pandurang.nagargoje@accoladeelectronics.com>
 *
 * @brief       LittleFs adapter - links LittleFs to NorFlash - implementation.
 */

// Standard includes.
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

// Self.
#include "littlefs_adapter.h"

// Dependencies.
#include "nor_flash.h"
#include "hal_util.h"
#include "tcu_locks.h"

// Injectable macros.
#define LITTLEFS_ADAPTER_LOCK()             if ( g_mutex.lock ) { g_mutex.lock(); }
#define LITTLEFS_ADAPTER_UNLOCK()           if ( g_mutex.unlock ) { g_mutex.unlock(); }

// Private defines.
#define LITTLEFS_ADAPTER_CACHE_SIZE         ( NOR_FLASH_PAGE_SIZE )
#define LITTLEFS_ADAPTER_LOOKAHEAD_SIZE     ( NOR_FLASH_PAGE_SIZE )

typedef struct
{
    uint32_t blockStart;                    /* Block start for this partition. */
    uint32_t blockCount;                    /* Block count for this partition. */
    uint32_t byteStart;                     /* Byte start for this partition. */
    uint32_t byteCount;                     /* Byte count for this partition. */
} LittlefsAdapterBounds_t;

typedef struct
{
    Filesystem_n id;
    iface_mutex_t lock;
    const LittlefsAdapterBounds_t* ptrBounds;
    lfs_t lfs;
    struct lfs_config config;
} LittlefsAdapterContext_t;

// Private functions.
static int littlefs_adapter_read            (const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
static int littlefs_adapter_write           (const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
static int littlefs_adapter_erase           (const struct lfs_config *c, lfs_block_t block);
static int littlefs_adapter_sync            (const struct lfs_config *c);
static int littlefs_adapter_lock            (const struct lfs_config *c);
static int littlefs_adapter_unlock          (const struct lfs_config *c);

// Private variables.
#pragma section GRAMB
static uint8_t                              g_mem_read[LITTLEFS_ADAPTER_CACHE_SIZE * FilesystemMax];
static uint8_t                              g_mem_prog[LITTLEFS_ADAPTER_CACHE_SIZE * FilesystemMax];
static uint8_t                              g_mem_lookahead[LITTLEFS_ADAPTER_LOOKAHEAD_SIZE * FilesystemMax];
#pragma section default
static LittlefsAdapterContext_t             g_ctx[FilesystemMax];
static const LittlefsAdapterBounds_t        g_bounds[FilesystemMax] = \
{
    { .blockStart = 0UL,    .blockCount = 256UL,    .byteStart = 0UL,                       .byteCount = 1UL * 1024UL * 1024UL },
    { .blockStart = 256UL,  .blockCount = 768UL,    .byteStart = 1UL * 1024UL * 1024UL,     .byteCount = 3UL * 1024UL * 1024UL },
    { .blockStart = 1024UL, .blockCount = 768UL,    .byteStart = 4UL * 1024UL * 1024UL,     .byteCount = 3UL * 1024UL * 1024UL },
    { .blockStart = 1792UL, .blockCount = 768UL,    .byteStart = 7UL * 1024UL * 1024UL,     .byteCount = 3UL * 1024UL * 1024UL },
    { .blockStart = 2560UL, .blockCount = 1024UL,   .byteStart = 10UL * 1024UL * 1024UL,    .byteCount = 4UL * 1024UL * 1024UL },
    { .blockStart = 3584UL, .blockCount = 256UL,    .byteStart = 14UL * 1024UL * 1024UL,    .byteCount = 1UL * 1024UL * 1024UL },
    { .blockStart = 3840UL, .blockCount = 256UL,    .byteStart = 15UL * 1024UL * 1024UL,    .byteCount = 1UL * 1024UL * 1024UL }
};
static iface_mutex_t                        g_mutex;
static bool                                 g_initialized = false;

void littlefs_adapter_init(iface_mutex_t mutex)
{
    uint32_t maxBlock = 0;
    uint32_t maxByte = 0;
    Filesystem_n fs;
    TcuLocks_n lockNum = TcuLocksFsPartition0;

    if ( !g_initialized )
    {
        // Update module mutex.
        g_mutex = mutex;

        // Initialize adapter contexts.
        for ( fs = FilesystemLfs0 ; fs < FilesystemMax ; ++fs )
        {
            // Identification.
            g_ctx[fs].id =                      fs;

            // Locks.
            hal_util_assert ( lockNum < TcuLocksMax );
            g_ctx[fs].lock = tcu_lock_get(lockNum++);

            // Bounds.
            hal_util_assert ( maxBlock == g_bounds[fs].blockStart );
            hal_util_assert ( maxByte == g_bounds[fs].byteStart );
            maxBlock += g_bounds[fs].blockCount;
            maxByte += g_bounds[fs].byteCount;
            hal_util_assert ( maxBlock == g_bounds[fs].blockStart + g_bounds[fs].blockCount );
            hal_util_assert ( maxByte == g_bounds[fs].byteStart + g_bounds[fs].byteCount );
            hal_util_assert ( NOR_FLASH_CAPACITY_BYTES >= NOR_FLASH_SECTOR_SIZE * g_bounds[fs].blockStart );
            hal_util_assert ( NOR_FLASH_CAPACITY_BYTES >= NOR_FLASH_SECTOR_SIZE * g_bounds[fs].blockCount );
            hal_util_assert ( NOR_FLASH_CAPACITY_BYTES >= g_bounds[fs].byteStart );
            hal_util_assert ( NOR_FLASH_CAPACITY_BYTES >= g_bounds[fs].byteCount );
            hal_util_assert ( NOR_FLASH_CAPACITY_BYTES >= NOR_FLASH_SECTOR_SIZE * maxBlock );
            hal_util_assert ( NOR_FLASH_CAPACITY_BYTES >= maxByte );
            g_ctx[fs].ptrBounds = &g_bounds[fs];

            // LFS Config.
            g_ctx[fs].config.context =          &g_ctx[fs];
            g_ctx[fs].config.read =             littlefs_adapter_read;
            g_ctx[fs].config.prog =             littlefs_adapter_write;
            g_ctx[fs].config.erase =            littlefs_adapter_erase;
            g_ctx[fs].config.sync =             littlefs_adapter_sync;
            g_ctx[fs].config.lock =             littlefs_adapter_lock;
            g_ctx[fs].config.unlock =           littlefs_adapter_unlock;
            g_ctx[fs].config.read_size =        NOR_FLASH_PAGE_SIZE;
            g_ctx[fs].config.prog_size =        NOR_FLASH_PAGE_SIZE;
            g_ctx[fs].config.block_size =       NOR_FLASH_SECTOR_SIZE;
            g_ctx[fs].config.block_count =      g_bounds[fs].blockCount;
            g_ctx[fs].config.block_cycles =     100UL;
            g_ctx[fs].config.cache_size =       LITTLEFS_ADAPTER_CACHE_SIZE;
            g_ctx[fs].config.lookahead_size =   LITTLEFS_ADAPTER_LOOKAHEAD_SIZE;
            g_ctx[fs].config.read_buffer =      &g_mem_read[LITTLEFS_ADAPTER_CACHE_SIZE * fs];
            g_ctx[fs].config.prog_buffer =      &g_mem_prog[LITTLEFS_ADAPTER_CACHE_SIZE * fs];
            g_ctx[fs].config.lookahead_buffer = &g_mem_lookahead[LITTLEFS_ADAPTER_LOOKAHEAD_SIZE * fs];
        }
        g_initialized = true;
    }
}

lfs_t* littlefs_adapter_get_lfs(Filesystem_n filesystem)
{
    lfs_t* pLfs;

    // Lock.
    LITTLEFS_ADAPTER_LOCK();

    // Get corresponding LFS type object.
    hal_util_assert ( g_initialized );
    hal_util_assert ( filesystem < FilesystemMax );
    pLfs = &g_ctx[filesystem].lfs;

    // Unlock.
    LITTLEFS_ADAPTER_UNLOCK();

    return pLfs;
}

struct lfs_config* littlefs_adapter_get_config(Filesystem_n filesystem)
{
    struct lfs_config* pConfig;

    // Lock.
    LITTLEFS_ADAPTER_LOCK();

    // Get corresponding LFS type object.
    hal_util_assert ( g_initialized );
    hal_util_assert ( filesystem < FilesystemMax );
    pConfig = &g_ctx[filesystem].config;

    // Unlock.
    LITTLEFS_ADAPTER_UNLOCK();

    return pConfig;

}

static int littlefs_adapter_read            (const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    int err = 0;
    LittlefsAdapterContext_t* ctx = (LittlefsAdapterContext_t*) c->context;
    uint32_t readAddress = ctx->ptrBounds->byteStart + ( block * c->block_size ) + off;

    hal_util_assert ( g_initialized );
    err = nor_flash_read ( readAddress, buffer, size);

    return err;
}

static int littlefs_adapter_write           (const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    int err = 0;
    LittlefsAdapterContext_t* ctx = (LittlefsAdapterContext_t*) c->context;
    uint32_t writeAddress = ctx->ptrBounds->byteStart + ( block * c->block_size ) + off;

    hal_util_assert ( g_initialized );
    err == nor_flash_write ( writeAddress, buffer, size );

    return err;
}

static int littlefs_adapter_erase           (const struct lfs_config *c, lfs_block_t block)
{
    int err = 0;
    LittlefsAdapterContext_t* ctx = (LittlefsAdapterContext_t*) c->context;
    uint32_t eraseBlock = ctx->ptrBounds->blockStart + block;

    hal_util_assert ( g_initialized );
    err = nor_flash_erase ( eraseBlock );

    return err;
}

static int littlefs_adapter_sync            (const struct lfs_config *c)
{
    LittlefsAdapterContext_t* ctx = (LittlefsAdapterContext_t*) c->context;

    hal_util_assert ( g_initialized );
    ctx = ctx->config.context;

    return LFS_ERR_OK;
}

static int littlefs_adapter_lock            (const struct lfs_config *c)
{
    LittlefsAdapterContext_t* ctx = (LittlefsAdapterContext_t*) c->context;

    hal_util_assert ( g_initialized );
    if ( ctx->lock.lock )
    {
        ctx->lock.lock();
    }

    return LFS_ERR_OK;
}

static int littlefs_adapter_unlock          (const struct lfs_config *c)
{
    LittlefsAdapterContext_t* ctx = (LittlefsAdapterContext_t*) c->context;

    hal_util_assert ( g_initialized );
    if ( ctx->lock.unlock )
    {
        ctx->lock.unlock();
    }

    return LFS_ERR_OK;
}
