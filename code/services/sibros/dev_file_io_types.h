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
 */

#ifndef SIBROS__DEV_FILE_IO_TYPES_H
#define SIBROS__DEV_FILE_IO_TYPES_H

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
#include "sl_file_types.h"

/* Module Includes */

/***********************************************************************************************************************
 *
 *                                                   D E F I N E S
 *
 **********************************************************************************************************************/
#ifndef DEV_FILE_IO__MAX_FILENAME_LENGTH
#define DEV_FILE_IO__MAX_FILENAME_LENGTH 255U
#endif

/***********************************************************************************************************************
 *
 *                                                  T Y P E D E F S
 *
 **********************************************************************************************************************/
typedef enum {
  DEV_FILE_IO__FILE_TYPE_INVALID = 0,
  DEV_FILE_IO__FILE_TYPE_REGULAR_FILE,
  DEV_FILE_IO__FILE_TYPE_DIRECTORY,
} dev_file_io__file_type_e;

typedef struct {
  dev_file_io__file_type_e type;
  size_t size;  // TODO @dwihandi: Gitlab #8693 - SL DTI: use explicit integer types
  char name[DEV_FILE_IO__MAX_FILENAME_LENGTH + 1U];
  long int modify_time;
  sl_file__path_buffer_s absolute_path;
} dev_file_io__file_info_s;

typedef struct {
  /**
   * the number of used bytes in the filesystem.
   * NOTE: Depending on the filesystem, this count may be higher than the actual value, especially when occupied blocks
   * are used to calculate the number of used bytes.
   */
  size_t bytes_used;

  /**
   * the number of free bytes in the filesystem.
   * NOTE: Depending on the filesystem, this count may be less than the actual value, especially when occupied blocks
   * are used to calculate the number of used bytes.
   */
  size_t bytes_free;

  /**
   * the total number of bytes in the filesystem.
   * NOTE: This is total size, not taking into account used and free space.
   */
  size_t total_bytes;
} dev_file_io__filesystem_size_info_s;

typedef struct {
  const dev_file_io__file_info_s *info;

  // the index of the file being read within the traversed directory
  size_t index;
} dev_file_io__directory_traversal_args_s;

/**
 * A function pointer type used during directory traversal to interact with read files.
 *
 * @param entry_callback_args user provided arguments for callback from dev_file_io__directory_entry_callback_s
 * @param file_args information about the file being read
 * @return true upon success
 */
typedef bool (*dev_file_io__directory_entry_callback_f)(void *entry_callback_args,
                                                        dev_file_io__directory_traversal_args_s file_args);

typedef struct {
  // User provided arguments that will be used during the callback. This is an optional value.
  void *args_for_callback;
  dev_file_io__directory_entry_callback_f callback;
} dev_file_io__directory_entry_callback_s;

/***********************************************************************************************************************
 *
 *                                     F U N C T I O N   D E C L A R A T I O N S
 *
 **********************************************************************************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifdef SIBROS__DEV_FILE_IO_TYPES_H */
