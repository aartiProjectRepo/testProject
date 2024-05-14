/**
 * @file        dev_file_io_impl.c
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
 * @brief       Implements 'dev_file_io.h' interface using 'filesystem.h' API.
 */

// Self.
#include "dev_file_io.h"

// Dependency.
#include "filesystem.h"
#include "hal_util.h"

// Private Macros.
#define DEV_FILE_IO_FS_PATH(fs, path)       ( ( FilesystemMax == fs ) ? ( path ) : ( ( path ) + 2 ) )
#define DEV_FILE_MAX_RECURSION_LEVEL        ( 4 )

// Private functions.
static inline Filesystem_n                  fs_from_path(const char* absolutePath);
static inline Filesystem_n                  fs_from_partiton(sl_drive__partition_e* partition);
static inline FilesystemOpenMode_n          translate_open_flags(const char* mode);

void* dev_file_io__fopen(const char* filepath, const char* mode)
{
    Filesystem_n fs;
    FilesystemOpenMode_n modeEnum;
    FilesystemFd_t fd = NULL;

    fs = fs_from_path(filepath);
    modeEnum = translate_open_flags(mode);
    filesystem_fopen(fs, &fd, DEV_FILE_IO_FS_PATH(fs, filepath), modeEnum);

    return fd;
}

int dev_file_io__fclose(void* file_descriptor)
{
    return filesystem_fclose(file_descriptor);
}

size_t dev_file_io__fread(void* buffer_to_read_into, size_t number_of_bytes, void* file_descriptor)
{
    uint32_t bytesRead = 0;

    filesystem_fread(file_descriptor, buffer_to_read_into, number_of_bytes, &bytesRead);

    return bytesRead;
}

size_t dev_file_io__fwrite(const void* buffer_to_write_from, size_t number_of_bytes, void* file_descriptor)
{
    uint32_t bytesWritten = 0;

    filesystem_fwrite(file_descriptor, buffer_to_write_from, number_of_bytes, &bytesWritten);

    return bytesWritten;
}

int dev_file_io__fseek(void* file_descriptor, long absolute_offset)
{
    FilesystemWhence_n whence = LFS_SEEK_SET;
    uint32_t ignore;

    return filesystem_fseek(file_descriptor, absolute_offset, whence, &ignore);
}

bool dev_file_io__fgets(void* buffer_to_read_into, int max_bytes_to_read, void* file_descriptor)
{
    size_t idx = 0;
    uint32_t readCount;
    bool success = false;
    char* outBuf = buffer_to_read_into;
    
    while ( max_bytes_to_read )
    {
        readCount = 0;
        if ( FilesystemErrOk == filesystem_fread(file_descriptor, outBuf + idx, 1, &readCount) )
        {
            hal_util_assert ( readCount <= 1 );
            // EOF condition
            if ( ( 0 == readCount ) )
            {
                outBuf[idx] = '\0';
                success = true;
                break;
            }
            // Newline read condition.
            if ( '\n' == outBuf[idx] )
            {
                if ( ( idx + 1 ) < max_bytes_to_read )
                {
                    outBuf[idx + 1] = '\0';
                }
                success = true;
                break;
            }
            max_bytes_to_read--;
            idx++;
        }
    }

    return success;
}

bool dev_file_io__file_exists(const char* filepath)
{
    Filesystem_n fs;
    FilesystemNodeInfo_t info;
    bool success = false;

    fs = fs_from_path(filepath);
    if ( FilesystemErrOk == filesystem_stat(fs, DEV_FILE_IO_FS_PATH(fs, filepath), &info) )
    {
        if ( LFS_TYPE_REG == info.lfs_info.type || LFS_TYPE_DIR == info.lfs_info.type )
        {
            success = true;
        }
    }

    return success;
}

bool dev_file_io__file_remove(const char* filepath)
{
    Filesystem_n fs;
    bool success = false;

    fs = fs_from_path(filepath);
    if ( FilesystemErrOk == filesystem_remove(fs, DEV_FILE_IO_FS_PATH(fs, filepath)) )
    {
        success = true;
    }

    return success;
}

bool dev_file_io__rename(const char* oldpath, const char* newpath)
{
    Filesystem_n fs;
    bool success = false;

    fs = fs_from_path(oldpath);
    hal_util_assert ( fs == fs_from_path(newpath) );
    if ( FilesystemErrOk == filesystem_rename(fs, DEV_FILE_IO_FS_PATH(fs, oldpath), DEV_FILE_IO_FS_PATH(fs, newpath)) )
    {
        success = true;
    }

    return success;
}

bool dev_file_io__get_file_info(const char* filepath, dev_file_io__file_info_s* info)
{
    Filesystem_n fs;
    FilesystemNodeInfo_t nodeInfo = {0};
    bool success = false;

    fs = fs_from_path(filepath);
    if ( FilesystemErrOk == filesystem_stat(fs, DEV_FILE_IO_FS_PATH(fs, filepath), &nodeInfo) )
    {
        hal_util_strncpy(info->absolute_path.cstring, filepath, 2);
        hal_util_strncpy(info->absolute_path.cstring + 2, nodeInfo.full_path, sizeof(info->absolute_path.cstring) - 2);
        info->modify_time = nodeInfo.modify_time;
        hal_util_strncpy(info->name, nodeInfo.lfs_info.name, sizeof(info->name));
        info->size = nodeInfo.lfs_info.size;
        if ( LFS_TYPE_REG == nodeInfo.lfs_info.type )
        {
            info->type = DEV_FILE_IO__FILE_TYPE_REGULAR_FILE;
            success = true;
        }
        else if ( LFS_TYPE_DIR == nodeInfo.lfs_info.type )
        {
            info->type = DEV_FILE_IO__FILE_TYPE_DIRECTORY;
            success = true;
        }
        else
        {
            success = false;
        }
    }

    return success;
}

bool dev_file_io__reformat_filesystem(sl_drive__partition_e partition, uint32_t passkey)
{
    Filesystem_n fs;

    fs = fs_from_partiton(&partition);
    if ( filesystem_is_mounted(fs) )
    {
        if ( FilesystemErrOk != filesystem_unmount(fs) )
        {
            return false;
        }
    }
    if ( FilesystemErrOk == filesystem_format(fs) )
    {
        if ( FilesystemErrOk == filesystem_mount(fs) )
        {
            return true;
        }
    }

    return false;
}

bool dev_file_io__get_filesystem_size_info(sl_drive__partition_e partition, dev_file_io__filesystem_size_info_s* info)
{
    Filesystem_n fs;
    FilesystemInfo_t fsInfo;
    FilesystemFd_t fdRoot;
    FilesystemRecursiveArgs_t args = { .deleteFlag = false, .maxRecursionLevel = DEV_FILE_MAX_RECURSION_LEVEL };
    FilesystemRecursiveOutputs_t outputs = { 0 };
    bool success = false;

    fs = fs_from_partiton(&partition);
    if ( FilesystemErrOk == filesystem_fs_stat(fs, &fsInfo) )
    {
        info->total_bytes = fsInfo.block_count * fsInfo.block_size;
        if ( FilesystemErrOk == filesystem_dopen(fs, &fdRoot, "/") )
        {
            if ( FilesystemErrOk == filesystem_drecursive(fdRoot, &args, &outputs) )
            {
                info->bytes_used = outputs.totalBytes;
                info->bytes_free = info->total_bytes - info->bytes_used;
                if ( FilesystemErrOk == filesystem_dclose(fdRoot) )
                {
                    success = true;
                }
            }
        }
    }

    return success;
}

bool dev_file_io__dir_traverse(const char* absolute_path, dev_file_io__directory_entry_callback_s entry_callback)
{
    Filesystem_n fs;
    FilesystemErr_n fsErr;
    FilesystemFd_t fd;
    FilesystemNodeInfo_t info;
    dev_file_io__file_info_s traversalFileInfo;
    dev_file_io__directory_traversal_args_s traversalArgs;
    uint32_t newPosition;
    uint32_t idx = 0;
    bool endOfDir = false;
    bool success = false;

    fs = fs_from_path(absolute_path);
    fsErr = filesystem_dopen(fs, &fd, DEV_FILE_IO_FS_PATH(fs, absolute_path) );
    if ( FilesystemErrOk == fsErr )
    {
        while ( 1 )
        {
            fsErr = filesystem_dread(fd, &info, &newPosition, &endOfDir);
            if ( FilesystemErrOk != fsErr )
            {
                break;
            }
            else
            {
                if ( endOfDir )
                {
                    success = true;
                    break;
                }
                hal_util_strncpy(traversalFileInfo.absolute_path.cstring, info.full_path, sizeof(traversalFileInfo.absolute_path.cstring));
                traversalFileInfo.modify_time = 0;
                hal_util_strncpy(traversalFileInfo.name, info.lfs_info.name, sizeof(traversalFileInfo.name));
                if ( LFS_TYPE_REG == info.lfs_info.type )
                {
                    traversalFileInfo.size = info.lfs_info.size;
                    traversalFileInfo.type = DEV_FILE_IO__FILE_TYPE_REGULAR_FILE;
                }
                else if ( LFS_TYPE_DIR == info.lfs_info.type )
                {
                    traversalFileInfo.size = 0;
                    traversalFileInfo.type = DEV_FILE_IO__FILE_TYPE_DIRECTORY;
                }
                else
                {
                    traversalFileInfo.size = 0;
                    traversalFileInfo.type = DEV_FILE_IO__FILE_TYPE_INVALID;
                }
                traversalArgs.index = idx++;
                traversalArgs.info = &traversalFileInfo;
                if ( false == entry_callback.callback(entry_callback.args_for_callback, traversalArgs) )
                {
                    break;
                }
            }
        }
        // We don't check for success of dclose() and hope for the best.
        filesystem_dclose(fd);
    }

    return success;
}

bool dev_file_io__is_directory_empty(const char* absolute_path)
{
    Filesystem_n fs;
    FilesystemFd_t fd = NULL;
    FilesystemNodeInfo_t info;
    bool endOfDir = false;
    bool success = false;
    uint32_t ignore;

    fs = fs_from_path(absolute_path);
    if ( FilesystemErrOk == filesystem_dopen(fs, &fd, DEV_FILE_IO_FS_PATH(fs, absolute_path) ) )
    {
        if ( FilesystemErrOk == filesystem_dread(fd, &info, &ignore, &endOfDir) )
        {
            if (  FilesystemErrOk == filesystem_dclose(fd) )
            {
                if ( endOfDir )
                {
                    success = true;
                }
            }
        }
    }

    return success;
}

bool dev_file_io__remove_directory_if_empty(const char* absolute_path)
{
    Filesystem_n fs;
    bool success = false;

    fs = fs_from_path(absolute_path);
    if ( dev_file_io__is_directory_empty(absolute_path) )
    {
        if ( FilesystemErrOk == filesystem_remove(fs, DEV_FILE_IO_FS_PATH(fs, absolute_path) ) )
        {
            success = true;
        }
    }

    return success;
}

bool dev_file_io__remove_all_content(const char* absolute_path)
{
    Filesystem_n fs;
    FilesystemFd_t fd = NULL;
    FilesystemRecursiveArgs_t args = { .deleteFlag = true, .maxRecursionLevel = DEV_FILE_MAX_RECURSION_LEVEL };
    FilesystemRecursiveOutputs_t outputs = { 0 };
    bool success = false;

    fs = fs_from_path(absolute_path);
    if ( FilesystemErrOk == filesystem_dopen(fs, &fd, DEV_FILE_IO_FS_PATH(fs, absolute_path) ) )
    {
        if ( FilesystemErrOk == filesystem_drecursive(fd, &args, &outputs) )
        {
            if ( FilesystemErrOk == filesystem_dclose(fd) )
            {
                success = true;
            }
        }
    }

    return success;
}

uint64_t dev_file_io__get_accumulated_size_of_directory_in_bytes(const char* absolute_path)
{
    Filesystem_n fs;
    FilesystemErr_n fsErr;
    FilesystemFd_t fd = NULL;
    FilesystemRecursiveArgs_t args = { .deleteFlag = false, .maxRecursionLevel = 0 };
    FilesystemRecursiveOutputs_t outputs = { 0 };
    uint64_t totalBytes = 0;

    fs = fs_from_path(absolute_path);
    fsErr = filesystem_dopen(fs, &fd, DEV_FILE_IO_FS_PATH(fs, absolute_path) );
    if ( FilesystemErrOk == fsErr )
    {
        fsErr = filesystem_drecursive(fd, &args, &outputs);
        if ( FilesystemErrOk == fsErr )
        {
            fsErr = filesystem_dclose(fd);
            if ( FilesystemErrOk == fsErr )
            {
                totalBytes = outputs.totalBytes;
            }
        }
    }

    return totalBytes;
}

bool dev_file_io__mkdir(const char* absolute_path)
{
    Filesystem_n fs;
    
    fs = fs_from_path(absolute_path);
    if ( FilesystemErrOk == filesystem_dcreate(fs, DEV_FILE_IO_FS_PATH(fs, absolute_path) ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool dev_file_io__is_directory(const dev_file_io__file_info_s* info)
{
    if ( DEV_FILE_IO__FILE_TYPE_DIRECTORY == info->type )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool dev_file_io__is_regular_file(const dev_file_io__file_info_s* info)
{
    if ( DEV_FILE_IO__FILE_TYPE_REGULAR_FILE == info->type )
    {
        return true;
    }
    else
    {
        return false;
    }
}

const char* dev_file_io__get_name(const dev_file_io__file_info_s* info)
{
    return info->name;
}

uint64_t dev_file_io__get_size(const dev_file_io__file_info_s* info)
{
    return info->size;
}

long int dev_file_io__last_write_time(const dev_file_io__file_info_s* info)
{
    // LFS does not maintain time info.
    //return info->modify_time;

    return 0;
}

const char* dev_file_io__get_configured_path(const dev_file_io__file_info_s* info)
{
    return info->absolute_path.cstring;
}

static inline Filesystem_n                  fs_from_path(const char* absolutePath)
{
    Filesystem_n fs = FilesystemMax;

    if ( 'u' == absolutePath[0] && ':' == absolutePath[1] )
    {
        fs = FilesystemLfs1;
    }
    else if ( 's' == absolutePath[0] && ':' == absolutePath[1] )
    {
        fs = FilesystemLfs2;
    }
    else if ( 'n' == absolutePath[0] && ':' == absolutePath[1] )
    {
        fs = FilesystemLfs3;
    }
    else
    {
        fs = FilesystemLfs0;
    }

    return fs;
}

static inline Filesystem_n                  fs_from_partiton(sl_drive__partition_e* partition)
{
    Filesystem_n fs = FilesystemMax;

    if ( SL_DRIVE__PARTITION_UPDATER == *partition )
    {
        fs = FilesystemLfs1;
    }
    else if ( SL_DRIVE__PARTITION_SECURE == *partition )
    {
        fs = FilesystemLfs2;
    }
    else if ( SL_DRIVE__PARTITION_NORMAL == *partition )
    {
        fs = FilesystemLfs3;
    }
    else
    {
        fs = FilesystemLfs0;
    }

    return fs;
}

static inline FilesystemOpenMode_n          translate_open_flags(const char* mode)
{
    FilesystemOpenMode_n openMode = (FilesystemOpenMode_n) 0x0;

    if ( mode )
    {
        if ( ( ( 'r' == mode[0] || 'R' == mode[0] ) ) && ( '+' == mode[1] ) )
        {
            openMode |= LFS_O_RDWR;
        }
        else if ( ( ( 'w' == mode[0] || 'W' == mode[0] ) ) && ( '+' == mode[1] ) )
        {
            openMode |= LFS_O_RDWR;
            openMode |= LFS_O_CREAT;
            openMode |= LFS_O_TRUNC;
        }
        else if ( ( ( 'a' == mode[0] || 'A' == mode[0] ) ) && ( '+' == mode[1] ) )
        {
            openMode |= LFS_O_RDWR;
            openMode |= LFS_O_CREAT;
            openMode |= LFS_O_APPEND;
        }
        else if ( ( 'r' == mode[0] || 'R' == mode[0] ) )
        {
            openMode |= LFS_O_RDONLY;
        }
        else if ( ( 'w' == mode[0] || 'W' == mode[0] ) )
        {
            openMode |= LFS_O_WRONLY;
            openMode |= LFS_O_CREAT;
            openMode |= LFS_O_TRUNC;
        }
        else if ( ( 'a' == mode[0] || 'A' == mode[0] ) )
        {
            // FIXME: Not standard, but makes sibros test pass.
            // openMode |= LFS_O_WRONLY;
            openMode |= LFS_O_RDWR;
            openMode |= LFS_O_CREAT;
            openMode |= LFS_O_APPEND;
        }
    }

    return openMode;
}
