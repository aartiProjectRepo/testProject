/**
 * @file        filesystem.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        29 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Filesystem provider - implementation.
 */

// Standard includes.
#include <stdbool.h>
#include <stdint.h>

// Self.
#include "filesystem.h"

// Injectable macros.
INJECTABLE_DEFN_PROC(filesystem, lock);
INJECTABLE_DEFN_PROC(filesystem, unlock);
INJECTABLE_DEFN_LOGGER(filesystem, logger);
#define FILESYSTEM_LOCK()                   if ( g_fn_lock ) { g_fn_lock(); }
#define FILESYSTEM_UNLOCK()                 if ( g_fn_unlock ) { g_fn_unlock(); }
#define FILESYSTEM_PRINT(x, ...)            if ( NULL != g_fn_logger ) { g_fn_logger((x), ##__VA_ARGS__); }

// Dependency.
#include "littlefs_adapter.h"
#include "hal_util.h"

// Private defines.
#define FILESYSTEM_MAX_DESCRIPTORS    ( 16 )

typedef union
{
    lfs_file_t file;
    lfs_dir_t dir;
} LfsNode_u;

typedef struct
{
    uint8_t mem_file[1024UL];
    char path[512];
    struct lfs_file_config file_config;
    LfsNode_u node;
    uint8_t self_index;
    Filesystem_n fs;
    bool is_directory;
    bool is_used;
} FilesystemContext_t;

// Private data
#pragma section GRAMB
static FilesystemContext_t g_fd[FILESYSTEM_MAX_DESCRIPTORS];
static uint32_t g_bits_opened_fd;
static uint32_t g_bits_mounted;
#pragma section default

// Private functions.
static bool filesystem_fd_alloc(FilesystemContext_t** fdPtrPtr);
static void filesystem_fd_free(FilesystemContext_t* fdPtr);
static FilesystemErr_n filesystem_fclose_priv (FilesystemFd_t fd);
static FilesystemErr_n filesystem_dopen_priv (Filesystem_n fs, FilesystemFd_t* fdPtr, const char *path);
static FilesystemErr_n filesystem_dclose_priv (FilesystemFd_t fd);
static FilesystemErr_n filesystem_remove_priv (Filesystem_n fs, const char *path);
static FilesystemErr_n filesystem_drecursive_priv (FilesystemFd_t fd, FilesystemRecursiveArgs_t* args, FilesystemRecursiveOutputs_t* outputs, bool firstFlag);

// Public functions.
FilesystemErr_n filesystem_format           (Filesystem_n fs)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    const struct lfs_config* pConfig;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax )
    {
        if ( !( g_bits_mounted & ( 0x1UL << fs ) ) )
        {
            pLfs = littlefs_adapter_get_lfs(fs);
            pConfig = littlefs_adapter_get_config(fs);
            if ( LFS_ERR_OK == lfs_format(pLfs, pConfig) )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_mount            (Filesystem_n fs)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    const struct lfs_config* pConfig;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax )
    {
        if ( !( g_bits_mounted & ( 0x1UL << fs ) ) )
        {
            pLfs = littlefs_adapter_get_lfs(fs);
            pConfig = littlefs_adapter_get_config(fs);
            if ( LFS_ERR_OK == lfs_mount(pLfs, pConfig) )
            {
                g_bits_mounted |= ( 0x1UL << fs );
                FILESYSTEM_PRINT("mounted %d\r\n", (int)fs);
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_unmount          (Filesystem_n fs)
{
    FilesystemErr_n err = FilesystemErrParameter;
    FilesystemContext_t* ctx;
    uint32_t idx = 0;
    lfs_t* pLfs;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax )
    {
        // Close all open handles belonging to this filesystem.
        while ( ( g_bits_opened_fd | ( 0x1UL << idx ) ) < g_bits_opened_fd )
        {
            if ( g_bits_opened_fd & ( 0x1UL << idx ) )  // Indicates an open file.
            {
                ctx = &g_fd[idx];
                if ( fs == ctx->fs ) // Indicates open file belongs to fs to be unmounted.
                {
                    if ( ctx->is_directory )
                    {
                        err = filesystem_dclose_priv((FilesystemFd_t) ctx);
                        if ( FilesystemErrOk != err )
                        {
                            break;
                        }
                    }
                    else
                    {
                        err = filesystem_fclose_priv((FilesystemFd_t) ctx);
                        if ( FilesystemErrOk != err )
                        {
                            break;
                        }
                    }
                }
                else
                {
                    // Not related to current fs, hence ignore.
                }
            }
            idx += 1;
        }
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            pLfs = littlefs_adapter_get_lfs(fs);
            if ( LFS_ERR_OK == lfs_unmount(pLfs) )
            {
                g_bits_mounted &= ~( 0x1UL << fs );
                FILESYSTEM_PRINT("un-mounted %d\r\n", (int)fs);
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

bool filesystem_is_mounted                  (Filesystem_n fs)
{
    bool success = false;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax )
    {
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            success = true;
        }
    }
    FILESYSTEM_UNLOCK();
    
    return success;
}

FilesystemErr_n filesystem_remove           (Filesystem_n fs, const char *path)
{
    FilesystemErr_n err = FilesystemErrParameter;

    FILESYSTEM_LOCK();
    err = filesystem_remove_priv(fs, path);
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_rename           (Filesystem_n fs, const char *oldPath, const char *newPath)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax && oldPath && newPath && *oldPath && *newPath )
    {
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            pLfs = littlefs_adapter_get_lfs(fs);
            if ( LFS_ERR_OK == lfs_rename(pLfs, oldPath, newPath) )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_stat             (Filesystem_n fs, const char *path, FilesystemNodeInfo_t* info)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    size_t strncpyIdx;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax && path && info && *path )
    {
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            pLfs = littlefs_adapter_get_lfs(fs);
            if ( LFS_ERR_OK == lfs_stat(pLfs, path, &info->lfs_info ) )
            {
                strncpyIdx = hal_util_strncpy(info->full_path, path, sizeof(info->full_path));
                info->full_path[ ( strncpyIdx >= sizeof(info->full_path) ) ? ( sizeof(info->full_path) - 1 ) : ( strncpyIdx ) ] = '\0';
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fopen            (Filesystem_n fs, FilesystemFd_t* fdPtr, const char *path, FilesystemOpenMode_n flags)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    size_t strncpyIdx;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax && fdPtr && !*fdPtr && path && *path )
    {
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            if ( filesystem_fd_alloc(&pFd) )
            {
                pLfs = littlefs_adapter_get_lfs(fs);
                pFd->fs = fs;
                pFd->file_config.buffer = pFd->mem_file;
                pFd->is_directory = false;
                pFd->is_used = true;
                if ( LFS_ERR_OK == lfs_file_opencfg(pLfs, &pFd->node.file, path, flags, &pFd->file_config) )
                {
                    strncpyIdx = hal_util_strncpy(pFd->path, path, sizeof(pFd->path));
                    pFd->path[ ( strncpyIdx >= sizeof(pFd->path) ) ? ( sizeof(pFd->path) - 1 ) : ( strncpyIdx ) ] = '\0';
                    *fdPtr = (FilesystemFd_t) pFd;
                    err = FilesystemErrOk;
                }
                else
                {
                    filesystem_fd_free(pFd);
                    err = FilesystemErrDriver;
                }
            }
            else
            {
                err = FilesystemErrMemory;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fclose           (FilesystemFd_t fd)
{
    FilesystemErr_n err = FilesystemErrParameter;

    FILESYSTEM_LOCK();
    err = filesystem_fclose_priv(fd);
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fsync            (FilesystemFd_t fd)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;

    FILESYSTEM_LOCK();
    if ( fd )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            if ( LFS_ERR_OK == lfs_file_sync(pLfs, &pFd->node.file) )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fread            (FilesystemFd_t fd, void *buffer, uint32_t size, uint32_t* readCount)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    lfs_ssize_t lfsReadOutput;

    FILESYSTEM_LOCK();
    if ( fd && buffer )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            lfsReadOutput = lfs_file_read(pLfs, &pFd->node.file, buffer, size);
            if ( lfsReadOutput >= 0 )
            {
                *readCount = lfsReadOutput;
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fwrite           (FilesystemFd_t fd, const void *buffer, uint32_t size, uint32_t* writtenCount)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    lfs_ssize_t lfsWriteOutput;

    FILESYSTEM_LOCK();
    if ( fd && buffer )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            lfsWriteOutput = lfs_file_write(pLfs, &pFd->node.file, buffer, size);
            if ( lfsWriteOutput >= 0 )
            {
                *writtenCount = lfsWriteOutput;
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fseek            (FilesystemFd_t fd, uint32_t offset, FilesystemWhence_n whence, uint32_t* newOffset)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    lfs_soff_t seekOffsetTmp;

    FILESYSTEM_LOCK();
    if ( fd && ( whence <= LFS_SEEK_END ) && newOffset )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            seekOffsetTmp = lfs_file_seek(pLfs, &pFd->node.file, offset, whence);
            if ( seekOffsetTmp >= 0 )
            {
                *newOffset = (uint32_t) seekOffsetTmp;
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_ftell            (FilesystemFd_t fd, uint32_t* offset)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    lfs_soff_t tellOffsetTmp;

    FILESYSTEM_LOCK();
    if ( fd && offset )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            tellOffsetTmp = lfs_file_tell(pLfs, &pFd->node.file);
            if ( tellOffsetTmp >= 0 )
            {
                *offset = (uint32_t) tellOffsetTmp;
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_frewind          (FilesystemFd_t fd)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;

    FILESYSTEM_LOCK();
    if ( fd )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            if ( lfs_file_rewind(pLfs, &pFd->node.file) >= 0 )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fsize            (FilesystemFd_t fd, uint32_t* size)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    lfs_soff_t sizeTmp;

    FILESYSTEM_LOCK();
    if ( fd && size )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            sizeTmp = lfs_file_size(pLfs, &pFd->node.file);
            if ( sizeTmp >= 0 )
            {
                *size = (uint32_t) sizeTmp;
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_ftruncate        (FilesystemFd_t fd, uint32_t size)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;

    FILESYSTEM_LOCK();
    if ( fd )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            if ( lfs_file_truncate(pLfs, &pFd->node.file, size) > 0 )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_dcreate          (Filesystem_n fs, const char *path)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;

    FILESYSTEM_LOCK();
    if ( fs < FilesystemMax && path && *path )
    {
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            pLfs = littlefs_adapter_get_lfs(fs);
            if ( LFS_ERR_OK == lfs_mkdir(pLfs, path) )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_dopen            (Filesystem_n fs, FilesystemFd_t* fdPtr, const char *path)
{
    FilesystemErr_n err;

    FILESYSTEM_LOCK();
    err = filesystem_dopen_priv(fs, fdPtr, path);
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_dclose           (FilesystemFd_t fd)
{
    FilesystemErr_n err = FilesystemErrParameter;

    FILESYSTEM_LOCK();
    err = filesystem_dclose_priv(fd);
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_dread            (FilesystemFd_t fd, FilesystemNodeInfo_t* info, uint32_t* newPosition, bool* endOfDir)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    int ret;
    size_t strncpyIdx;

    FILESYSTEM_LOCK();
    if ( fd && info && newPosition && endOfDir )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            while ( true )
            {
                ret = lfs_dir_read(pLfs, &pFd->node.dir, &info->lfs_info);
                // See https://github.com/littlefs-project/littlefs/issues/175
                if ( ret < 0 ) 
                {
                    // Error condition.
                    err = FilesystemErrDriver;
                    break;
                }
                else if ( 0 == ret )
                {
                    // End of file.
                    *endOfDir = true;
                    err = FilesystemErrOk;
                    break;
                }
                else
                {
                    // Valid node (which could potentially be 'special').
                    if ( '.' != info->lfs_info.name[0] )
                    {
                        *newPosition = ret;
                        strncpyIdx = hal_util_strncpy(info->full_path, pFd->path, sizeof(info->full_path));
                        strncpyIdx += hal_util_strncpy(info->full_path+strncpyIdx, "/", sizeof(info->full_path)-strncpyIdx);
                        strncpyIdx += hal_util_strncpy(info->full_path+strncpyIdx, info->lfs_info.name, sizeof(info->full_path)-strncpyIdx);
                        info->full_path[ ( strncpyIdx >= sizeof(info->full_path) ) ? ( sizeof(info->full_path) - 1 ) : ( strncpyIdx ) ] = '\0';
                        err = FilesystemErrOk;
                        break;
                    }
                    else
                    {
                        // In case of special directories, we do nothing.
                        hal_util_assert ( 1 == pFd->node.dir.pos || 2 == pFd->node.dir.pos );
                        continue;
                    }
                }
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_dseek            (FilesystemFd_t fd, uint32_t offset, uint32_t* newOffset)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    int tmpOffset;

    FILESYSTEM_LOCK();
    if ( fd && newOffset )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            tmpOffset = lfs_dir_seek(pLfs, &pFd->node.dir, offset);
            if ( tmpOffset >= 0 )
            {
                *newOffset = tmpOffset;
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_dtell            (FilesystemFd_t fd, uint32_t* offset)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;
    lfs_soff_t tmpOffset;

    FILESYSTEM_LOCK();
    if ( fd && offset )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            tmpOffset = lfs_dir_tell(pLfs, &pFd->node.dir);
            if ( tmpOffset >= 0 )
            {
                *offset = tmpOffset;
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_drewind          (FilesystemFd_t fd)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;

    FILESYSTEM_LOCK();
    if ( fd )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            if ( lfs_dir_rewind(pLfs, &pFd->node.dir) >= 0 )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_drecursive       (FilesystemFd_t fd, FilesystemRecursiveArgs_t* args, FilesystemRecursiveOutputs_t* outputs)
{
    FilesystemErr_n err = FilesystemErrParameter;

    FILESYSTEM_LOCK();
    if ( fd && outputs && args )
    {
        outputs->countDirs = 0;
        outputs->countFiles = 0;
        outputs->countOthers = 0;
        outputs->totalBytes = 0;
        err = filesystem_drecursive_priv(fd, args, outputs, true);
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fs_stat          (Filesystem_n fs, FilesystemInfo_t *info)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;

    FILESYSTEM_LOCK();
    if ( fs && info )
    {
        pLfs = littlefs_adapter_get_lfs(fs);
        if ( lfs_fs_stat(pLfs, info) >= 0 )
        {
            err = FilesystemErrOk;
        }
        else
        {
            err = FilesystemErrDriver;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

FilesystemErr_n filesystem_fs_size          (Filesystem_n fs, uint32_t* outSize)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    lfs_ssize_t tmpSize;

    FILESYSTEM_LOCK();
    if ( fs && outSize )
    {
        pLfs = littlefs_adapter_get_lfs(fs);
        tmpSize = lfs_fs_size(pLfs);
        if ( tmpSize >= 0 )
        {
            *outSize = tmpSize;
            err = FilesystemErrOk;
        }
        else
        {
            err = FilesystemErrDriver;
        }
    }
    FILESYSTEM_UNLOCK();

    return err;
}

static bool filesystem_fd_alloc             (FilesystemContext_t** fdPtrPtr)
{
    bool found = false;
    uint8_t count = 0;

    while ( ( count < FILESYSTEM_MAX_DESCRIPTORS ) && ( g_bits_opened_fd & ( 1UL << count ) ) )
    {
        count++;
    }
    if ( count < FILESYSTEM_MAX_DESCRIPTORS )
    {
        found = true;
        g_bits_opened_fd |= ( 1UL << count );
        g_fd[count].self_index = count;
        *fdPtrPtr = &g_fd[count];
    }

    return found;
}

static void filesystem_fd_free              (FilesystemContext_t* fdPtr)
{
    hal_util_assert ( &g_fd[fdPtr->self_index] == fdPtr );
    g_bits_opened_fd &= ~( 1UL << fdPtr->self_index );
}

static FilesystemErr_n filesystem_fclose_priv (FilesystemFd_t fd)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;

    if ( fd )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( !pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            if ( g_bits_opened_fd & ( 0x1UL << pFd->self_index ) )
            {
                if ( LFS_ERR_OK == lfs_file_close(pLfs, &pFd->node.file) )
                {
                    filesystem_fd_free(pFd);
                    pFd->is_used = false;
                    err = FilesystemErrOk;
                }
                else
                {
                    err = FilesystemErrDriver;
                }
            }
            else
            {
                err = FilesystemErrForbidden;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }

    return err;
}

static FilesystemErr_n filesystem_dopen_priv (Filesystem_n fs, FilesystemFd_t* fdPtr, const char *path)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;

    if ( fs < FilesystemMax && path && *path )
    {
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            if ( filesystem_fd_alloc(&pFd) )
            {
                pLfs = littlefs_adapter_get_lfs(fs);
                pFd->file_config.buffer = pFd->mem_file;
                pFd->fs = fs;
                pFd->is_directory = true;
                pFd->is_used = true;
                if ( LFS_ERR_OK == lfs_dir_open(pLfs,  &pFd->node.dir, path) )
                {
                    hal_util_strncpy(pFd->path, path, sizeof(pFd->path));
                    *fdPtr = (FilesystemFd_t) pFd;
                    err = FilesystemErrOk;
                }
                else
                {
                    filesystem_fd_free(pFd);
                    err = FilesystemErrDriver;
                }
            }
            else
            {
                err = FilesystemErrMemory;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }

    return err;
}

static FilesystemErr_n filesystem_dclose_priv (FilesystemFd_t fd)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;
    FilesystemContext_t* pFd;

    if ( fd )
    {
        pFd = (FilesystemContext_t*) fd;
        if ( pFd->is_directory && pFd->is_used )
        {
            pLfs = littlefs_adapter_get_lfs(pFd->fs);
            if ( g_bits_opened_fd & ( 0x1UL << pFd->self_index ) )
            {
                if ( LFS_ERR_OK == lfs_dir_close(pLfs, &pFd->node.dir) )
                {
                    filesystem_fd_free(pFd);
                    pFd->is_used = false;
                    err = FilesystemErrOk;
                }
                else
                {
                    err = FilesystemErrDriver;
                }
            }
            else
            {
                err = FilesystemErrForbidden;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }

    return err;
}

static FilesystemErr_n filesystem_remove_priv (Filesystem_n fs, const char *path)
{
    FilesystemErr_n err = FilesystemErrParameter;
    lfs_t* pLfs;

    if ( fs < FilesystemMax && path && *path )
    {
        if ( g_bits_mounted & ( 0x1UL << fs ) )
        {
            pLfs = littlefs_adapter_get_lfs(fs);
            if ( LFS_ERR_OK == lfs_remove(pLfs, path) )
            {
                err = FilesystemErrOk;
            }
            else
            {
                err = FilesystemErrDriver;
            }
        }
        else
        {
            err = FilesystemErrForbidden;
        }
    }

    return err;
}

static FilesystemErr_n filesystem_drecursive_priv (FilesystemFd_t fd, FilesystemRecursiveArgs_t* args, FilesystemRecursiveOutputs_t* outputs, bool firstFlag)
{
    FilesystemErr_n err = FilesystemErrOk;
    FilesystemRecursiveArgs_t downArgs;
    FilesystemNodeInfo_t newInfo;
    FilesystemContext_t* ctx = (FilesystemContext_t*) fd;
    FilesystemFd_t newDirFd;
    lfs_t* pLfs = littlefs_adapter_get_lfs(ctx->fs);
    char newPath[512];
    size_t strncpyIdx;
    int dreadIdx = 1;

    if ( ctx->is_directory )
    {
        outputs->countDirs += 1;
        if ( firstFlag ? ( LFS_ERR_OK == lfs_dir_rewind(pLfs, &ctx->node.dir) ) : (1) )
        {
            // Read next node.
            while ( dreadIdx )
            {
                dreadIdx = lfs_dir_read(pLfs, &ctx->node.dir, &newInfo.lfs_info);
                if ( dreadIdx >= 0 )
                {
                    if ( dreadIdx )
                    {
                        if ( '.' == newInfo.lfs_info.name[0] )
                        {
                            // Skip special directories.
                            continue;
                        }
                        strncpyIdx = hal_util_strncpy(newPath, ctx->path, sizeof(newPath));
                        strncpyIdx += hal_util_strncpy(newPath+strncpyIdx, "/", sizeof(newPath)-strncpyIdx);
                        strncpyIdx += hal_util_strncpy(newPath+strncpyIdx, newInfo.lfs_info.name, sizeof(newPath)-strncpyIdx);
                        newPath[ ( strncpyIdx >= sizeof(newPath) ) ? ( sizeof(newPath) - 1 ) : ( strncpyIdx ) ] = '\0';
                        if ( LFS_TYPE_DIR == newInfo.lfs_info.type )
                        {
                            outputs->countDirs += 1;
                            if ( args->maxRecursionLevel )
                            {
                                downArgs.deleteFlag = args->deleteFlag;
                                downArgs.maxRecursionLevel = args->maxRecursionLevel - 1;
                                err = filesystem_dopen_priv(ctx->fs, &newDirFd, newPath);
                                if ( FilesystemErrOk == err )
                                {
                                    err = filesystem_drecursive_priv(newDirFd, &downArgs, outputs, false);
                                    if ( FilesystemErrOk == err )
                                    {
                                        // We don't check for success of dclose() and hope for the best.
                                        filesystem_dclose_priv(newDirFd);
                                        newDirFd = NULL;
                                    }
                                    else
                                    {
                                        // Recursed call failure.
                                        break;
                                    }
                                }
                                else
                                {
                                    // Open directory for recursion failed.
                                    break;
                                }
                            }
                        }
                        else if ( LFS_TYPE_REG == newInfo.lfs_info.type )
                        {
                            outputs->countFiles += 1;
                            outputs->totalBytes += newInfo.lfs_info.size;
                        }
                        else
                        {
                            outputs->countOthers += 1;
                        }
                        // Code for handling delete option.
                        if ( args->deleteFlag )
                        {
                            err = filesystem_remove_priv(ctx->fs, newPath);
                            if ( FilesystemErrOk != err )
                            {
                                // Delete failed.
                                break;
                            }
                        }
                    }
                    else
                    {
                        // End of directory.
                        err = FilesystemErrOk;
                        break;
                    }
                }
                else
                {
                    // Directory read failed.
                    err =  FilesystemErrDriver;
                    break;
                }
            }
        }
        else
        {
            // Rewind fail.
            err =  FilesystemErrDriver;
        }
    }
    else
    {
        // Don't call this function on a file.
        err =  FilesystemErrForbidden;
    }

   return  err;
}
