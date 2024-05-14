/**
 * @file        filesystem.h
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
 * @brief       Filesystem provider - interface.
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include "injectable.h"
#include "lfs.h"

INJECTABLE_DECL_PROC(filesystem, lock);
INJECTABLE_DECL_PROC(filesystem, unlock);
INJECTABLE_DECL_LOGGER(filesystem, logger);

typedef struct FilesystemContext_t*         FilesystemFd_t;

typedef enum lfs_open_flags                 FilesystemOpenMode_n;
typedef enum lfs_whence_flags               FilesystemWhence_n;
typedef struct lfs_fsinfo                   FilesystemInfo_t;

typedef enum
{
    FilesystemLfs0,                         // AEPL scratch drive 1MB.
    FilesystemLfs1,                         // Sibros 'Updater' drive 3MB ('u:\').
    FilesystemLfs2,                         // Sibros 'Secure' drive 3MB ('s:\').
    FilesystemLfs3,                         // Sibros 'Normal' drive 3MB ('n:\').
    FilesystemLfs4,                         // AEPL storage drive 4MB.
    FilesystemLfs5,                         // AEPL configuration drive 1MB.
    FilesystemRaw,                          // AEPL raw reserve 1MB.
    FilesystemMax                           // List terminator.
} Filesystem_n;

typedef enum
{
    FilesystemErrOk,                        // Success.
    FilesystemErrParameter,                 // Parameter is incorrect.
    FilesystemErrForbidden,                 // Incorrect usage.
    FilesystemErrMemory,                    // Out of memory error.
    FilesystemErrDriver,                    // Low-level driver error.
    FilesystemErrMax                        // List terminator.
} FilesystemErr_n;

typedef struct
{
    struct lfs_info lfs_info;
    long int modify_time;
    char full_path[512];
} FilesystemNodeInfo_t;

typedef struct
{
    uint8_t maxRecursionLevel;
    bool deleteFlag;
} FilesystemRecursiveArgs_t;

typedef struct
{
    uint64_t totalBytes;
    uint32_t countFiles;
    uint32_t countDirs;
    uint32_t countOthers;
} FilesystemRecursiveOutputs_t;

// Higher order.
FilesystemErr_n filesystem_format           (Filesystem_n fs);
FilesystemErr_n filesystem_mount            (Filesystem_n fs);
FilesystemErr_n filesystem_unmount          (Filesystem_n fs);
bool filesystem_is_mounted                  (Filesystem_n fs);
// Common.
FilesystemErr_n filesystem_remove           (Filesystem_n fs, const char *path);
FilesystemErr_n filesystem_rename           (Filesystem_n fs, const char *oldPath, const char *newPath);
FilesystemErr_n filesystem_stat             (Filesystem_n fs, const char *path, FilesystemNodeInfo_t* info);
// File.
FilesystemErr_n filesystem_fopen            (Filesystem_n fs, FilesystemFd_t* fdPtr, const char *path, FilesystemOpenMode_n flags);
FilesystemErr_n filesystem_fclose           (FilesystemFd_t fd);
FilesystemErr_n filesystem_fsync            (FilesystemFd_t fd);
FilesystemErr_n filesystem_fread            (FilesystemFd_t fd, void *buffer, uint32_t size, uint32_t* readCount);
FilesystemErr_n filesystem_fwrite           (FilesystemFd_t fd, const void *buffer, uint32_t size, uint32_t* writtenCount);
FilesystemErr_n filesystem_fseek            (FilesystemFd_t fd, uint32_t offset, FilesystemWhence_n whence, uint32_t* newOffset);
FilesystemErr_n filesystem_ftell            (FilesystemFd_t fd, uint32_t* offset);
FilesystemErr_n filesystem_frewind          (FilesystemFd_t fd);
FilesystemErr_n filesystem_fsize            (FilesystemFd_t fd, uint32_t* size);
FilesystemErr_n filesystem_ftruncate        (FilesystemFd_t fd, uint32_t size);
// Directory.
FilesystemErr_n filesystem_dcreate          (Filesystem_n fs, const char *path);
FilesystemErr_n filesystem_dopen            (Filesystem_n fs, FilesystemFd_t* fdPtr, const char *path);
FilesystemErr_n filesystem_dclose           (FilesystemFd_t fd);
FilesystemErr_n filesystem_dread            (FilesystemFd_t fd, FilesystemNodeInfo_t* info, uint32_t* newPosition, bool* endOfDir);
FilesystemErr_n filesystem_dseek            (FilesystemFd_t fd, uint32_t offset, uint32_t* newOffset);
FilesystemErr_n filesystem_dtell            (FilesystemFd_t fd, uint32_t* offset);
FilesystemErr_n filesystem_drewind          (FilesystemFd_t fd);
FilesystemErr_n filesystem_drecursive       (FilesystemFd_t fd, FilesystemRecursiveArgs_t* args, FilesystemRecursiveOutputs_t* outputs);
// Filesystem level.
FilesystemErr_n filesystem_fs_stat          (Filesystem_n fs, FilesystemInfo_t *info);
FilesystemErr_n filesystem_fs_size          (Filesystem_n fs, uint32_t* outSize);

#endif /* FILESYSTEM_H */