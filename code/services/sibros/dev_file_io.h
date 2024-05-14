/***********************************************************************************************************************
 * SIBROS TECHNOLOGIES, INC. CONFIDENTIAL
 * Copyright (c) 2018 Sibros Technologies, Inc.
 * All Rights Reserved.
 * NOTICE: All information contained herein is, and remains the property of Sibros Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained herein are proprietary to Sibros Technologies, Inc. and its
 * suppliers and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade secret or
 * copyright law. Dissemination of this information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from Sibros Technologies, Inc.
 **********************************************************************************************************************/

/**
 * @file
 *
 * Abstraction needed by Sibros Firmware stack.
 *
 * Certain Sibros firmware components need File I/O to be able to read and write files onto OEM hardware
 * OEM shall provide implementation of these functions externally. Typically, this can be redirected to
 * third party code, such as FATFS or LittleFS which are both Embedded Systems file systems. Alternatively,
 * in POSIX based system, this can just be redirected to <stdio.h> based file system.
 */

#ifndef SIBROS__DEV_FILE_IO_H
#define SIBROS__DEV_FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 *
 *                                                  I N C L U D E S
 *
 **********************************************************************************************************************/
/* Standard Includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* External Includes */
#include "dev_file_io_types.h"
#include "sl_drive.h"

/* Module Includes */

/***********************************************************************************************************************
 *
 *                                                   D E F I N E S
 *
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *
 *                                                  T Y P E D E F S
 *
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *
 *                                     F U N C T I O N   D E C L A R A T I O N S
 *
 **********************************************************************************************************************/

/**
 * Opens a file for reading or writing while returning a file descriptor.
 *
 * @param filepath NULL-terminated string representing the location of a file to be opened.
 * @param mode refers to read or write mode. Acceptable values include "r", "w", or "a", and the "+" variants of them
 * ("r+", "w+", "a+"). The behavior of each mode aligns with the modes for `fopen()` specified by the ISO C standard
 * (see https://man7.org/linux/man-pages/man3/fopen.3.html,
 * https://pubs.opengroup.org/onlinepubs/009695399/functions/fopen.html, or ISO/IEC 9899:1999 TC3 Section 7.19.5.3 on
 * "The fopen function").
 *
 * Supported file open modes:
 * r      Open text file for reading.  The stream is positioned at the
 *        beginning of the file.
 *
 * r+     Open for reading and writing.  The stream is positioned at the
 *        beginning of the file.
 *
 * w      Truncate file to zero length or create text file for writing.
 *        The stream is positioned at the beginning of the file.
 *
 * w+     Open for reading and writing.  The file is created if it does
 *        not exist, otherwise it is truncated.  The stream is
 *        positioned at the beginning of the file.
 *
 * a      Open for appending (writing at end of file).  The file is
 *        created if it does not exist.  The stream is positioned at the
 *        end of the file.
 *
 * a+     Open for reading and appending (writing at end of file).  The
 *        file is created if it does not exist.  The initial file
 *        position for reading is at the beginning of the file, but
 *        output is always appended to the end of the file.
 *
 * @return a file-descriptor upon success, or NULL upon failure.
 */
void* dev_file_io__fopen(const char* filepath, const char* mode);

/**
 * Closes an already-opened file.
 *
 * NOTE: We unconditionally release the file descriptor back to the pool regardless of whether or not the `fclose`
 * function call was successful. Therefore to prevent repeated calling/checking of `dev_file_io__fclose` from higher
 * layers, we unconditionally return success here if a non-NULL parameter is passed.
 *
 * @param file_descriptor a handle to a file opened with dev_file_io_variant__fopen().
 * @return 0 upon success.
 */
int dev_file_io__fclose(void* file_descriptor);

/**
 * Read from a file that has been opened in read-mode.
 *
 * @param buffer_to_read_into a buffer to be populated with data read from the file.
 * @param number_of_bytes maximum number of bytes to read.
 * @param file_descriptor a handle to a file opened with dev_file_io_variant__fopen().
 * @return the number of bytes successfully read from the file.
 */
size_t dev_file_io__fread(void* buffer_to_read_into, size_t number_of_bytes, void* file_descriptor);

/**
 * Write from a file that has been opened in write-mode.
 *
 * @param buffer_to_write_from a buffer used to populated the opened file.
 * @param number_of_bytes maximum number of bytes to write.
 * @param file_descriptor a handle to a file opened with dev_file_io_variant__fopen().
 * @return the number of bytes successfully written into the file.
 */
size_t dev_file_io__fwrite(const void* buffer_to_write_from, size_t number_of_bytes, void* file_descriptor);

/**
 * Seek to a position in a file.
 *
 * @param file_descriptor a handle to a file opened with dev_file_io_variant__fopen().
 * @param absolute_offset the absolute file location to seek the file to
 *
 * @return 0 upon success.
 */
int dev_file_io__fseek(void* file_descriptor, long absolute_offset);

/**
 * Read a line from a open file descriptor until `max_bytes_to_read` are read, a <newline> is read, or EOF is reached.
 *
 * @param buffer_to_read_into a buffer to be populated with data read from the file.
 * @param max_bytes_to_read maximum number of bytes to read. Includes final null-terminator.
 * @param file_descriptor a handle to the opened file.
 *
 * @return true upon success, false if no characters have been read or error occurs.
 */
bool dev_file_io__fgets(void* buffer_to_read_into, int max_bytes_to_read, void* file_descriptor);

/**
 * Determine whether or not a file exists.
 *
 * @param filepath NULL-terminated string representing the location of a file to be checked.
 * @return true if the file exists; false otherwise.
 */
bool dev_file_io__file_exists(const char* filepath);

/**
 * Remove a file.
 *
 * @param filepath NULL-terminated string representing the location of a file to be removed.
 * @return true if successful; false otherwise.
 */
bool dev_file_io__file_remove(const char* filepath);

/**
 * Rename a file.
 *
 * @param oldpath path of the desired file to rename.
 * @param newpath specified new path name for the file.
 * @return true if successful; false otherwise.
 */
bool dev_file_io__rename(const char* oldpath, const char* newpath);

/**
 * Returns file information.
 *
 * @param filepath NULL-terminated string representing the location of a file.
 * @param info file info such as size, type and name are returned here.
 * @return true if successful; false otherwise.
 */
bool dev_file_io__get_file_info(const char* filepath, dev_file_io__file_info_s* info);

/**
 * Reformats the filesystem, deleting all content. Be very careful with this API.
 * NOTE: If applicable, the filesystem shall be remounted after the reformat takes place.
 * This function may be useful if the filesystem is deemed to be in an irrecoverable state; such as full or corrupted.
 *
 * @param partition the caller needs to specify which partition they would like to format (see sl_drive.h).
 * @param passkey a key unique to each partition that must be provided to successfully reformat the requested
 * partition.
 *
 * @return true if successful; false otherwise.
 */
bool dev_file_io__reformat_filesystem(sl_drive__partition_e partition, uint32_t passkey);

/**
 * Populates filesystem info struct.
 *
 * @param partition the caller needs to specify which partition they would like to format (see sl_drive.h).
 * @param info filesystem info struct to be populated.
 * @return true if successful; false otherwise.
 */
bool dev_file_io__get_filesystem_size_info(sl_drive__partition_e partition, dev_file_io__filesystem_size_info_s* info);

/**
 * Reads and traverses all entries in the root of the current directory. Performs a function on all of the directory
 * entries based on the callback function.
 *
 * @param absolute_path NULL-terminated string that represents the absolute path to a directory
 * @param entry_callback a structure consisting of the function to be called for every directory entry found, and the
 * arguments passed into the function.
 *
 * @return true upon success, false if error occurs
 */
bool dev_file_io__dir_traverse(const char* absolute_path, dev_file_io__directory_entry_callback_s entry_callback);

/**
 * Determines if a directory is empty.
 * A directory is empty if it does not have any files or subdirectories. This does not include entries for "." and "..".
 *
 * @param absolute_path NULL-terminated string that represents the absolute path to the current filesystem object
 *
 * @return true if empty, false if not
 */
bool dev_file_io__is_directory_empty(const char* absolute_path);

/**
 * Delete the directory if it is empty.
 *
 * @param absolute_path NULL-terminated string that represents the absolute path to the current filesystem object
 *
 * @return true upon success, false otherwise
 */
bool dev_file_io__remove_directory_if_empty(const char* absolute_path);

/**
 * Deletes all content within a directory.
 * Note: this function will recurse and delete content within subdirectories.
 *
 * @param absolute_path NULL-terminated string that represents the absolute path to the current directory
 *
 * @return true if directory is already empty or if all content in the directory has been successfully deleted, false
 * otherwise
 */
bool dev_file_io__remove_all_content(const char* absolute_path);

/**
 * Determines the accumulated size (bytes) of all files within the root of the current directory. This function does not
 * traverse into subdirectories.
 *
 * @param absolute_path NULL-terminated string that represents the absolute path to the current directory.
 *
 * @return the accumulated size in bytes; 0 upon failure to calculate size
 */
uint64_t dev_file_io__get_accumulated_size_of_directory_in_bytes(const char* absolute_path);

/**
 * Creates a new directory
 *
 * @param absolute_path NULL-terminated string representing the location of the new directory to be created
 *
 * @return true upon success, false otherwise
 */
bool dev_file_io__mkdir(const char* absolute_path);

/**
 * Determines whether or not the object is a directory
 *
 * @return true if object is a directory, false otherwise
 */
bool dev_file_io__is_directory(const dev_file_io__file_info_s* info);

/**
 * Determines whether or not the object is a regular file
 *
 * @return true if file is regular, false otherwise
 */
bool dev_file_io__is_regular_file(const dev_file_io__file_info_s* info);

/**
 * @return NULL-terminated string representing the name of the filesystem object with path trimmed off on success,
 * empty string upon error
 */
const char* dev_file_io__get_name(const dev_file_io__file_info_s* info);

/**
 * @return a positive integer representing the size of the filesystem object in bytes
 */
uint64_t dev_file_io__get_size(const dev_file_io__file_info_s* info);

/**
 * @return the epoch time of the filesystem object's last modification, 0 upon error
 */
long int dev_file_io__last_write_time(const dev_file_io__file_info_s* info);

/**
 * @return NULL-terminated string representing the filepath that was used to generate the info structure upon success,
 * empty string otherwise
 */
const char* dev_file_io__get_configured_path(const dev_file_io__file_info_s* info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifdef SIBROS__DEV_FILE_IO_H */
