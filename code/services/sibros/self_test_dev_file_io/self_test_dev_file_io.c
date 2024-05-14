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

/***********************************************************************************************************************
 *
 *                                                  I N C L U D E S
 *
 **********************************************************************************************************************/
/* Standard Includes */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef POSIX_FILE_DIRECTORY
#include <sys/stat.h>
#endif

/* Mock Includes */

/* External Includes */
#include "printf.h"

/* Module Includes */
#include "dev_file_io.h"
#include "self_test_dev_file_io.h"

/***********************************************************************************************************************
 *
 *                                                   D E F I N E S
 *
 **********************************************************************************************************************/

#ifndef ARRAY_COUNT
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0U]))
#endif

/* Not to be disabled except on Linux-based OS's */
#ifndef FEATURE__NO_FILESYSTEM_SIZE_INFO_TEST
#define FEATURE__NO_FILESYSTEM_SIZE_INFO_TEST (0)
#endif

/* Set 1 if posix, set 0 otherwise */
#ifndef POSIX_FILE_DIRECTORY
#define POSIX_FILE_DIRECTORY (0)
#endif

/***********************************************************************************************************************
 *
 *                                                  T Y P E D E F S
 *
 **********************************************************************************************************************/
typedef bool (*self_test_function_f)(sl_drive__partition_e, size_t);
typedef struct {
  self_test_function_f self_test_function;
  const char *test_description;
} self_test_dev_file_io__test_function_config_s;

typedef struct {
  sl_drive__partition_e partition;
  char test_file_name[32U];
  char test_new_file_name[32U];
  char test_directory_name[32U];
  bool enable_verbose_log;
} self_test_dev_file_io__private_s;

/***********************************************************************************************************************
 *
 *                             P R I V A T E   F U N C T I O N   D E C L A R A T I O N S
 *
 **********************************************************************************************************************/

/***********************************************************************************************************************
 *
 *                                  P R I V A T E   D A T A   D E F I N I T I O N S
 *
 **********************************************************************************************************************/

static self_test_dev_file_io__private_s self_test_dev_file_io__private[SL_DRIVE__PARTITION_COUNT] = {0U};

// Enable verbose printf even for the normal, expected cases (i.e.: 'File opened successfully')
static const char *self_test_dev_file_io__open_mode_write = "w";
static const char *self_test_dev_file_io__open_mode_append = "a";
static const char *self_test_dev_file_io__open_mode_read = "r";
static const char *self_test_dev_file_io__open_mode_write_plus = "w+";
static const char *self_test_dev_file_io__open_mode_append_plus = "a+";
static const char *self_test_dev_file_io__open_mode_read_plus = "r+";

static char self_test_dev_file_io__output_buffer[SL_DRIVE__PARTITION_COUNT][256U];
static uint8_t self_test_dev_file_io__large_source_array[SL_DRIVE__PARTITION_COUNT][2U * 1024U];
static uint8_t self_test_dev_file_io__large_destination_array[SL_DRIVE__PARTITION_COUNT][2U * 1024U];

/***********************************************************************************************************************
 *
 *                                         P R I V A T E   F U N C T I O N S
 *
 **********************************************************************************************************************/
static void self_test_dev_file_io__private_prepare_file_names_by_partition(sl_drive__partition_e partition) {
  snprintf(self_test_dev_file_io__private[partition].test_file_name, sizeof(self_test_dev_file_io__private[partition].test_file_name),
           "%stest_file.txt", sl_drive__get_prefix(partition).cstring);
  snprintf(self_test_dev_file_io__private[partition].test_new_file_name, sizeof(self_test_dev_file_io__private[partition].test_new_file_name),
           "%snew_test_file.txt", sl_drive__get_prefix(partition).cstring);
  snprintf(self_test_dev_file_io__private[partition].test_directory_name,
           sizeof(self_test_dev_file_io__private[partition].test_directory_name), "%stest_folder",
           sl_drive__get_prefix(partition).cstring);
}

static void self_test_dev_file_io__private_set_up(sl_drive__partition_e partition) {
  (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_file_name);
}

static void self_test_dev_file_io__private_log_test_message(sl_drive__partition_e partition, size_t test_id, const char *format, ...) {
  snprintf(self_test_dev_file_io__output_buffer[partition], sizeof(self_test_dev_file_io__output_buffer[partition]),
           "ID# %lu: ", (unsigned long)test_id);

  size_t total_bytes_left_in_output_buffer =
      sizeof(self_test_dev_file_io__output_buffer[partition]) - strlen(self_test_dev_file_io__output_buffer[partition]);

  va_list args;
  va_start(args, format);
  (void)vsnprintf(self_test_dev_file_io__output_buffer[partition] + strlen(self_test_dev_file_io__output_buffer[partition]),
                  total_bytes_left_in_output_buffer, format, args);
  va_end(args);

  total_bytes_left_in_output_buffer =
      sizeof(self_test_dev_file_io__output_buffer[partition]) - strlen(self_test_dev_file_io__output_buffer[partition]);

  (void)strncat(self_test_dev_file_io__output_buffer[partition], "\n", total_bytes_left_in_output_buffer);

  self_test_dev_file_io__message_output(self_test_dev_file_io__output_buffer[partition]);
}

static bool self_test_dev_file_io__private_entry_command_callback(void *entry_command_args,
                                                                  dev_file_io__directory_traversal_args_s file_args) {
  (void)entry_command_args;
  bool success = false;

  /**
   * A POSIX filesystem and a littlefs filesystem behave differently.
   * i.e. the size of a folder will be 0U in littlefs. This is not true for POSIX.
   * */
#if (0 == POSIX_FILE_DIRECTORY)
  const uint64_t expected_size = 0U;
#else
  struct stat info = {0};
  int stat_ret = (0 == stat(file_args.info->absolute_path.cstring, &info));
  const uint64_t expected_size = ((stat_ret) ? info.st_size : 0U);
#endif

  if (expected_size == file_args.info->size && DEV_FILE_IO__FILE_TYPE_DIRECTORY == file_args.info->type) {
    success = true;
  }
  return success;
}

static bool self_test_dev_file_io__private_fopen(sl_drive__partition_e partition, size_t test_id, const char *open_mode, void **fd) {
  bool success = false;

  *fd = dev_file_io__fopen(self_test_dev_file_io__private[partition].test_file_name, open_mode);

  if (NULL == *fd) {
    // File does not exists
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s could not open with mode: %s",
                                                    self_test_dev_file_io__private[partition].test_file_name, open_mode);
  } else {
    // Expect file to exists; it either already exists, or it was created on open
    success = dev_file_io__file_exists(self_test_dev_file_io__private[partition].test_file_name);
    if (success) {
      if (self_test_dev_file_io__private[partition].enable_verbose_log) {
        self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s opened with mode: %s",
                                                        self_test_dev_file_io__private[partition].test_file_name, open_mode);
      }
    } else {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s could not be created with mode %s",
                                                      self_test_dev_file_io__private[partition].test_file_name, open_mode);
    }
  }

  return success;
}

static bool self_test_dev_file_io__private_fclose(sl_drive__partition_e partition, size_t test_id, void **fd) {
  bool success = false;

  if (NULL != *fd) {
    const int close_ret = dev_file_io__fclose(*fd);

    if (0 == close_ret) {
      success = true;
      *fd = NULL;
      if (self_test_dev_file_io__private[partition].enable_verbose_log) {
        self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s closed",
                                                        self_test_dev_file_io__private[partition].test_file_name);
      }
    } else {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s could not close",
                                                      self_test_dev_file_io__private[partition].test_file_name);
    }
  }

  return success;
}

static bool self_test_dev_file_io__private_fwrite(sl_drive__partition_e partition, size_t test_id, const void *source_array,
                                                  size_t source_array_size_in_bytes, void *fd) {
  bool success = false;

  const size_t write_ret = dev_file_io__fwrite(source_array, source_array_size_in_bytes, fd);

  if (source_array_size_in_bytes != write_ret) {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s could not be written with %lu bytes",
                                                    self_test_dev_file_io__private[partition].test_file_name,
                                                    source_array_size_in_bytes);
  } else {
    if (self_test_dev_file_io__private[partition].enable_verbose_log) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s written with %lu bytes",
                                                      self_test_dev_file_io__private[partition].test_file_name,
                                                      source_array_size_in_bytes);
    }
    success = true;
  }

  return success;
}

static bool self_test_dev_file_io__private_fseek(sl_drive__partition_e partition, size_t test_id, size_t seek_offset, void *fd) {
  bool success = false;

  if (0 == dev_file_io__fseek(fd, seek_offset)) {
    success = true;
    if (self_test_dev_file_io__private[partition].enable_verbose_log) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s seeked to offset %lu",
                                                      self_test_dev_file_io__private[partition].test_file_name, seek_offset);
    }
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s unable to seek to offset %lu",
                                                    self_test_dev_file_io__private[partition].test_file_name, seek_offset);
  }

  return success;
}

static bool self_test_dev_file_io__private_fread(sl_drive__partition_e partition, size_t test_id, void *destination_array, size_t bytes_to_read,
                                                 void *fd) {
  bool success = false;

  const size_t read_ret = dev_file_io__fread(destination_array, bytes_to_read, fd);

  if (bytes_to_read == read_ret) {
    success = true;
    if (self_test_dev_file_io__private[partition].enable_verbose_log) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s read with %lu bytes",
                                                      self_test_dev_file_io__private[partition].test_file_name, bytes_to_read);
    }
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s unable to read %lu bytes",
                                                    self_test_dev_file_io__private[partition].test_file_name, bytes_to_read);
  }

  return success;
}

static bool self_test_dev_file_io__private_fgets(sl_drive__partition_e partition, size_t test_id, void *destination_array, size_t bytes_to_read,
                                                 void *fd) {
  bool success = false;

  if (dev_file_io__fgets(destination_array, bytes_to_read, fd)) {
    success = true;
    if (self_test_dev_file_io__private[partition].enable_verbose_log) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "Read line %s from file %s", destination_array,
                                                      self_test_dev_file_io__private[partition].test_file_name);
    }
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Read nothing from file %s",
                                                    self_test_dev_file_io__private[partition].test_file_name);
  }

  return success;
}

static bool self_test_dev_file_io__private_mkdir(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  if (dev_file_io__mkdir(self_test_dev_file_io__private[partition].test_directory_name)) {
    success = true;
    if (self_test_dev_file_io__private[partition].enable_verbose_log) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "Created folder %s",
                                                      self_test_dev_file_io__private[partition].test_directory_name);
    }
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Unable to make the directory %s",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  }
  return success;
}

static bool self_test_dev_file_io__private_match_source_and_destination_array(sl_drive__partition_e partition, size_t test_id, const void *source_array,
                                                                              size_t source_array_size_in_bytes,
                                                                              void *destination_array) {
  bool success = false;

  if (0 == memcmp(source_array, destination_array, source_array_size_in_bytes)) {
    success = true;
    if (self_test_dev_file_io__private[partition].enable_verbose_log) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s read bytes matches input source",
                                                      self_test_dev_file_io__private[partition].test_file_name);
    }
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s read bytes does not match input source",
                                                    self_test_dev_file_io__private[partition].test_file_name);
  }

  return success;
}

static bool self_test_dev_file_io__private_write_then_validate_read(sl_drive__partition_e partition, size_t test_id, const void *source_array,
                                                                    size_t source_array_size_in_bytes,
                                                                    void *destination_array,
                                                                    size_t destination_array_size_in_bytes,
                                                                    size_t seek_offset, void *fd) {
  bool success = false;

  if (source_array_size_in_bytes == destination_array_size_in_bytes) {
    bool write_successful = false;

    // Deliberately break up the write request into two random sizes for additional test scrutiny
    const size_t random_chunk_1 = (rand() % (source_array_size_in_bytes / 2U)) + (source_array_size_in_bytes / 10U);
    const size_t random_chunk_2 = source_array_size_in_bytes - random_chunk_1;
    const void *source_array_chunk2 = (const char *)source_array + random_chunk_1;

    if (!self_test_dev_file_io__private_fwrite(partition, test_id, source_array, random_chunk_1, fd)) {
      write_successful = false;
    } else if (!self_test_dev_file_io__private_fwrite(partition, test_id, source_array_chunk2, random_chunk_2, fd)) {
      write_successful = false;
    } else {
      write_successful = true;
    }

    if (write_successful) {
      if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
        if (self_test_dev_file_io__private_fread(partition, test_id, destination_array, source_array_size_in_bytes, fd)) {
          success = self_test_dev_file_io__private_match_source_and_destination_array(
              partition, test_id, source_array, source_array_size_in_bytes, destination_array);
        }
      }
    }
  }

  return success;
}

static bool self_test_dev_file_io__private_file_remove(sl_drive__partition_e partition, size_t test_id, const char *filename) {
  bool success = false;

  if (dev_file_io__file_remove(filename)) {
    success = true;
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s removed", filename);
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s unable to remove", filename);
  }

  return success;
}

static bool self_test_dev_file_io__private_rename_and_verify_file(sl_drive__partition_e partition, size_t test_id, const char *filename,
                                                                  const char *new_filename) {
  bool success = false;

  if (dev_file_io__rename(filename, new_filename)) {
    if (dev_file_io__file_exists(new_filename) && !dev_file_io__file_exists(filename)) {
      success = true;
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s renamed to %s", filename, new_filename);
    }
  }

  if (!success) {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s unable to rename", filename);
  }

  return success;
}

static bool self_test_dev_file_io__private_dir_traverse(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  const dev_file_io__directory_entry_callback_s entry = {(void *)&test_id,
                                                         self_test_dev_file_io__private_entry_command_callback};

  if (dev_file_io__dir_traverse(self_test_dev_file_io__private[partition].test_directory_name, entry)) {
    success = true;
    if (self_test_dev_file_io__private[partition].enable_verbose_log) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "Traversed directory %s",
                                                      self_test_dev_file_io__private[partition].test_directory_name);
    }
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Failed to traverse directory %s",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  }
  return success;
}

static bool self_test_dev_file_io__private_is_directory_empty(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  if (dev_file_io__is_directory_empty(self_test_dev_file_io__private[partition].test_directory_name)) {
    success = true;
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Directory %s is empty",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Directory %s is not empty",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  }

  return success;
}

static bool self_test_dev_file_io__private_remove_directory_if_empty(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  if (dev_file_io__remove_directory_if_empty(self_test_dev_file_io__private[partition].test_directory_name)) {
    success = true;
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Directory %s was removed",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Failed to remove directory %s",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  }
  return success;
}

static bool self_test_dev_file_io__private_remove_all_content(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  if (dev_file_io__remove_all_content(self_test_dev_file_io__private[partition].test_directory_name)) {
    success = true;
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Deleted all content in directory %s",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  } else {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Failed to delete content in directory %s",
                                                    self_test_dev_file_io__private[partition].test_directory_name);
  }
  return success;
}

static uint64_t self_test_dev_file_io__private_get_accumulated_size_of_directory_in_bytes(sl_drive__partition_e partition, size_t test_id) {
  return dev_file_io__get_accumulated_size_of_directory_in_bytes(self_test_dev_file_io__private[partition].test_directory_name);
}

/***********************************************************************************************************************
 *
 *                                                     T E S T S
 *
 **********************************************************************************************************************/

static bool self_test_dev_file_io__private_test_fopen_then_fclose(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  void *fd = NULL;

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &fd)) {
    success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_test_fwrite_then_fread_small_buffer(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  typedef struct {
    bool test_bool;
    const char test_string[6];
  } self_test_dev_file_io_s;

  const self_test_dev_file_io_s source_array[] = {
      {.test_bool = true, .test_string = "hello"},
      {.test_bool = false, .test_string = "world"},
  };

  self_test_dev_file_io_s destination_array[ARRAY_COUNT(source_array)] = {0};

  const size_t seek_offset = 0U;
  void *fd = NULL;
  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write_plus, &fd)) {
    success = self_test_dev_file_io__private_write_then_validate_read(
        partition, test_id, source_array, sizeof(source_array), &destination_array, sizeof(destination_array), seek_offset, fd);
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_test_fwrite_then_fread_one_megabyte(sl_drive__partition_e partition, size_t test_id) {
  bool success = true;

  const size_t total_write_count = (1000U * 1024U) / sizeof(self_test_dev_file_io__large_source_array[partition]);
  void *fd = NULL;
  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write_plus, &fd)) {
    for (size_t write_count = 0U; write_count < total_write_count; ++write_count) {
      // Populate write blobs of file with random memory
      for (size_t index = 0U; index < sizeof(self_test_dev_file_io__large_source_array[partition]); ++index) {
        self_test_dev_file_io__large_source_array[partition][index] = (char)(rand() & 0xFF);
      }

      const size_t seek_offset = sizeof(self_test_dev_file_io__large_source_array[partition]) * write_count;
      if (!self_test_dev_file_io__private_write_then_validate_read(
              partition, test_id, self_test_dev_file_io__large_source_array[partition], sizeof(self_test_dev_file_io__large_source_array[partition]),
              self_test_dev_file_io__large_destination_array[partition], sizeof(self_test_dev_file_io__large_destination_array[partition]),
              seek_offset, fd)) {
        success = false;
        break;
      }
    }
  } else {
    success = false;
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_test_fwrite_then_fgets_the_line(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  char test_string[30U] = "This is a test string.";
  char destination_array[30U] = {0U};
  const size_t seek_offset = 0U;

  void *fd = NULL;
  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write_plus, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, test_string, sizeof(test_string), fd)) {
      if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
        if (self_test_dev_file_io__private_fgets(partition, test_id, destination_array, sizeof(destination_array), fd)) {
          success = (0 == strcmp(test_string, destination_array));
        } else {
#if (0 == POSIX_FILE_DIRECTORY)
          // littlefs fgets implementation is a stub. The function returns false
          success = true;
#endif
        }
      }
    }
    (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);
  }

  return success;
}

static bool self_test_dev_file_io__private_test_verify_and_remove_file(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  void *fd = NULL;

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &fd)) {
    if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
      if (self_test_dev_file_io__private_file_remove(partition, test_id, self_test_dev_file_io__private[partition].test_file_name)) {
        if (!dev_file_io__file_exists(self_test_dev_file_io__private[partition].test_file_name)) {
          success = true;
          self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s does not exist after being removed",
                                                          self_test_dev_file_io__private[partition].test_file_name);
        } else {
          self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s still exists after being removed",
                                                          self_test_dev_file_io__private[partition].test_file_name);
        }
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_test_rename_and_remove_file(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  void *fd = NULL;
  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &fd)) {
    if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
      if (self_test_dev_file_io__private_rename_and_verify_file(partition, test_id, self_test_dev_file_io__private[partition].test_file_name,
                                                                self_test_dev_file_io__private[partition].test_new_file_name)) {
        success =
            self_test_dev_file_io__private_file_remove(partition, test_id, self_test_dev_file_io__private[partition].test_new_file_name);
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_test_mkdir_and_delete(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  if (self_test_dev_file_io__private_mkdir(partition, test_id)) {
    success = dev_file_io__file_exists(self_test_dev_file_io__private[partition].test_directory_name);
    (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_directory_name);
    if (success) {
      success = !(dev_file_io__file_exists(self_test_dev_file_io__private[partition].test_directory_name));
    }
  }

  return success;
}

static bool self_test_dev_file_io__private_test_get_file_info_for_file(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  dev_file_io__file_info_s info = {0};

  memset(self_test_dev_file_io__large_source_array[partition], 0x01, sizeof(self_test_dev_file_io__large_source_array[partition]));
  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_append, &fd)) {
    (void)self_test_dev_file_io__private_write_then_validate_read(
        partition, test_id, self_test_dev_file_io__large_source_array[partition], sizeof(self_test_dev_file_io__large_source_array[partition]),
        self_test_dev_file_io__large_destination_array[partition], sizeof(self_test_dev_file_io__large_destination_array[partition]), 0U, fd);

    (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

    if (!dev_file_io__get_file_info(self_test_dev_file_io__private[partition].test_file_name, &info)) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s unable to get file info",
                                                      self_test_dev_file_io__private[partition].test_file_name);
    } else {
      int name_comparison = -1;
      int configured_path_comparison = -1;
      bool modify_time_comparison = false;
      // littlefs does keep track of modify time, while POSIX does.
#if (0 == POSIX_FILE_DIRECTORY)
      modify_time_comparison = (0 == dev_file_io__last_write_time(&info));
#else
      modify_time_comparison = (dev_file_io__last_write_time(&info) > 0);
#endif
      name_comparison = strcmp("test_file.txt", dev_file_io__get_name(&info));
      configured_path_comparison =
          strcmp(self_test_dev_file_io__private[partition].test_file_name, dev_file_io__get_configured_path(&info));
      if ((dev_file_io__is_regular_file(&info)) &&
          (sizeof(self_test_dev_file_io__large_source_array[partition]) == dev_file_io__get_size(&info)) &&
          (0 == name_comparison) && (0 == configured_path_comparison) && (modify_time_comparison)) {
        success = true;
      } else {
        self_test_dev_file_io__private_log_test_message(partition, test_id, "File %s unexpected info",
                                                        self_test_dev_file_io__private[partition].test_file_name);
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_file_name);

  return success;
}

static bool self_test_dev_file_io__private_test_get_file_info_for_directory(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  dev_file_io__file_info_s info = {0};

  if (self_test_dev_file_io__private_mkdir(partition, test_id)) {
    if (!dev_file_io__get_file_info(self_test_dev_file_io__private[partition].test_directory_name, &info)) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "Failed to retrieve file info for folder: %s",
                                                      self_test_dev_file_io__private[partition].test_directory_name);
    } else {
      int name_comparison = -1;
      int configured_path_comparison = -1;
      bool modify_time_comparison = false;
      uint64_t expected_size = 0U;
      /**
       * littlefs folder size is 0U and it does not keep track of modify time.
       * POSIX folder size is greater than 0 and it keeps track of modify time.
       */
#if (0 == POSIX_FILE_DIRECTORY)
      modify_time_comparison = (0 == dev_file_io__last_write_time(&info));
#else
      modify_time_comparison = (dev_file_io__last_write_time(&info) > 0);

      struct stat stat_info = {0};
      int stat_ret = (0 == stat(self_test_dev_file_io__private[partition].test_directory_name, &stat_info));
      expected_size = ((stat_ret) ? stat_info.st_size : 0U);
#endif
      name_comparison = strcmp("test_folder", dev_file_io__get_name(&info));
      configured_path_comparison =
          strcmp(self_test_dev_file_io__private[partition].test_directory_name, dev_file_io__get_configured_path(&info));
      if ((dev_file_io__is_directory(&info)) && (0 == name_comparison) && (0 == configured_path_comparison) &&
          (modify_time_comparison) && (expected_size == dev_file_io__get_size(&info))) {
        success = true;
      } else {
        self_test_dev_file_io__private_log_test_message(partition, test_id, "Folder %s unexpected info",
                                                        self_test_dev_file_io__private[partition].test_directory_name);
      }
    }
  }

  (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_directory_name);

  return success;
}

static bool self_test_dev_file_io__private_test_dir_traverse(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  if (self_test_dev_file_io__private_mkdir(partition, test_id)) {
    if (self_test_dev_file_io__private_dir_traverse(partition, test_id)) {
      success = true;
    }
  }
  (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_directory_name);

  return success;
}

static bool self_test_dev_file_io__private_test_get_is_directory_empty(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  if (self_test_dev_file_io__private_mkdir(partition, test_id)) {
    if (self_test_dev_file_io__private_is_directory_empty(partition, test_id)) {
      success = true;
    }
  }
  (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_directory_name);

  return success;
}

static bool self_test_dev_file_io__private_test_remove_directory_if_empty(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  if (self_test_dev_file_io__private_mkdir(partition, test_id)) {
    if (self_test_dev_file_io__private_remove_directory_if_empty(partition, test_id)) {
      success = true;
    } else {
      (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_directory_name);
    }
  }

  return success;
}

static bool self_test_dev_file_io__private_test_remove_all_content(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  if (self_test_dev_file_io__private_mkdir(partition, test_id)) {
    if (self_test_dev_file_io__private_remove_all_content(partition, test_id)) {
      success = true;
    }
  }
  (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_directory_name);

  return success;
}

static bool self_test_dev_file_io__private_test_get_accumulated_size_of_directory_in_bytes(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  if (self_test_dev_file_io__private_mkdir(partition, test_id)) {
    // folder size varies between filesystems
#if (0 == POSIX_FILE_DIRECTORY)
    const size_t expected_size = 0U;
#else
    struct stat stat_info = {0};
    int stat_ret = (0 == stat(self_test_dev_file_io__private[partition].test_directory_name, &stat_info));
    const size_t expected_size = ((stat_ret) ? stat_info.st_size : 0U);
#endif
    const size_t expected_size_of_empty_dir = (expected_size * 2U);
    const size_t actual_size = self_test_dev_file_io__private_get_accumulated_size_of_directory_in_bytes(partition, test_id);
    if (expected_size_of_empty_dir == actual_size) {
      success = true;
    }
  }
  (void)dev_file_io__file_remove(self_test_dev_file_io__private[partition].test_directory_name);

  return success;
}

#if (0 == FEATURE__NO_FILESYSTEM_SIZE_INFO_TEST)
static bool self_test_dev_file_io__private_write_data_to_file(sl_drive__partition_e partition, size_t test_id) {
  void *fd = NULL;
  bool success = self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &fd);
  memset(self_test_dev_file_io__large_source_array[partition], 'a', sizeof(self_test_dev_file_io__large_source_array[partition]));
  if (success) {
    const size_t max_iterations = (512U * 1024U) / sizeof(self_test_dev_file_io__large_source_array[partition]);
    for (size_t iteration_count = 0U; iteration_count < max_iterations; ++iteration_count) {
      success = self_test_dev_file_io__private_fwrite(partition, test_id, self_test_dev_file_io__large_source_array[partition],
                                                      sizeof(self_test_dev_file_io__large_source_array[partition]), fd);
      if (!success) {
        break;
      }
    }
  }
  success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
  return success;
}
#endif

#if (0 == FEATURE__NO_FILESYSTEM_SIZE_INFO_TEST)
static bool self_test_dev_file_io__private_test_get_filesystem_size_info(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;

  dev_file_io__filesystem_size_info_s size_info_before_writing_files = {0};

  success =
      dev_file_io__get_filesystem_size_info(self_test_dev_file_io__private[partition].partition, &size_info_before_writing_files);
  if (!success) {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Unable to retrieve filesystem size information");
  }

  if (success) {
    success = ((size_info_before_writing_files.bytes_free + size_info_before_writing_files.bytes_used) ==
               size_info_before_writing_files.total_bytes);
    if (!success) {
      self_test_dev_file_io__private_log_test_message(partition, test_id,
                                                      "Filesystem used and free space did not add up to total space");
    }
  }

  if (success) {
    success = self_test_dev_file_io__private_write_data_to_file(partition, test_id);
    if (!success) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "Unable to write 500 kB of test data");
    }
  }

  dev_file_io__filesystem_size_info_s size_info_after_writing_files = {0};
  if (success) {
    success =
        dev_file_io__get_filesystem_size_info(self_test_dev_file_io__private[partition].partition, &size_info_after_writing_files);
    if (!success) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "Unable to retrieve filesystem size information");
    }
  }

  if (success) {
    success =
        (size_info_after_writing_files.bytes_used > size_info_before_writing_files.bytes_used) &&
        ((size_info_after_writing_files.bytes_used - size_info_before_writing_files.bytes_used) >= (512U * 1024U));
    if (!success) {
      self_test_dev_file_io__private_log_test_message(partition, test_id,
                                                      "Filesystem used space did not increase by at least 512 kB.");
    }
  }

  if (success) {
    success = (size_info_after_writing_files.total_bytes == size_info_before_writing_files.total_bytes);
    if (!success) {
      self_test_dev_file_io__private_log_test_message(
          partition, test_id, "Filesystem total size changed when data was written when it should not have.");
    }
  }

  if (success) {
    success = ((size_info_after_writing_files.bytes_free + size_info_after_writing_files.bytes_used) ==
               size_info_after_writing_files.total_bytes);
    if (!success) {
      self_test_dev_file_io__private_log_test_message(
          partition, test_id, "Filesystem used and free space after writing a file did not add up to the total filesystem space");
    }
  }

  if (success) {
    success = self_test_dev_file_io__private_file_remove(partition, test_id, self_test_dev_file_io__private[partition].test_file_name);
    if (!success) {
      self_test_dev_file_io__private_log_test_message(partition, test_id, "Failed to remove test file %s.",
                                                      self_test_dev_file_io__private[partition].test_file_name);
    }
  }

  return success;
}
#endif

static bool self_test_dev_file_io__private_read_and_write_data_to_file_using_write_plus(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  const size_t seek_offset = 0U;

  const char test_string[] = "This is a test string.";

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write_plus, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, test_string, sizeof(test_string), fd)) {
      if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
        // Checking w+ allows for read
        if (self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition],
                                                 sizeof(test_string), fd)) {
          if (self_test_dev_file_io__private_match_source_and_destination_array(
                  partition, test_id, test_string, sizeof(test_string), self_test_dev_file_io__large_destination_array[partition])) {
            success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
          }
        }
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_read_and_write_data_to_file_using_read_plus(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  const size_t seek_offset = 0U;

  const char first_test_string[] = "Initializing file.";
  const char second_test_string[] = "Writing again to this file.";

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, first_test_string, sizeof(first_test_string), fd)) {
      if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
        if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_read_plus, &fd)) {
          // Checking r+ allows for write
          if (self_test_dev_file_io__private_fwrite(partition, test_id, second_test_string, sizeof(second_test_string), fd)) {
            if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
              if (self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition],
                                                       sizeof(second_test_string), fd)) {
                if (self_test_dev_file_io__private_match_source_and_destination_array(
                        partition, test_id, second_test_string, sizeof(second_test_string),
                        self_test_dev_file_io__large_destination_array[partition])) {
                  success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
                }
              }
            }
          }
        }
      }
    }
  }

  self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_read_and_write_data_to_file_using_append_plus(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  size_t seek_offset = 0U;

  const char first_test_string[] = "Initializing file.";
  const char second_test_string[] = "Appending this message.";

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_append_plus, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, first_test_string, sizeof(first_test_string), fd)) {
      if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
        if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_append_plus, &fd)) {
          if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
            if (self_test_dev_file_io__private_fwrite(partition, test_id, second_test_string, sizeof(second_test_string), fd)) {
              if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
                if (self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition],
                                                         sizeof(first_test_string), fd)) {
                  if (self_test_dev_file_io__private_match_source_and_destination_array(
                          partition, test_id, first_test_string, sizeof(first_test_string),
                          self_test_dev_file_io__large_destination_array[partition])) {
                    seek_offset += sizeof(first_test_string);
                    if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
                      if (self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition],
                                                               sizeof(second_test_string), fd)) {
                        if (self_test_dev_file_io__private_match_source_and_destination_array(
                                partition, test_id, second_test_string, sizeof(second_test_string),
                                self_test_dev_file_io__large_destination_array[partition])) {
                          success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_read_and_read_plus_does_not_create_new_file(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;

  if (!self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_read_plus, &fd)) {
    if (!self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_read, &fd)) {
      success = true;
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_write_erase_contents_of_existing_file(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  size_t seek_offset = 0U;

  const char first_test_string[] = "Initializing file with a string.";

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, first_test_string, sizeof(first_test_string), fd)) {
      if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
        if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &fd)) {
          if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
            if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_read, &fd)) {
              if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
                // Should not be able to read due to contents being removed
                if (!self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition], 1U,
                                                          fd)) {
                  success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
                }
              }
            }
          }
        }
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_write_plus_erase_contents_of_existing_file(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  size_t seek_offset = 0U;

  const char first_test_string[] = "Initializing file with a string.";

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write_plus, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, first_test_string, sizeof(first_test_string), fd)) {
      if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
        if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write_plus, &fd)) {
          if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
            // Should not be able to read due to contents being removed
            if (!self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition], 1U,
                                                      fd)) {
              success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
            }
          }
        }
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_open_append_does_not_erase_contents(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  size_t seek_offset = 0U;

  const char first_test_string[] = "Initializing file with a string.";

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_append, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, first_test_string, sizeof(first_test_string), fd)) {
      if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
        if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_append, &fd)) {
          if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
            if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_read, &fd)) {
              if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
                if (self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition],
                                                         sizeof(first_test_string), fd)) {
                  if (self_test_dev_file_io__private_match_source_and_destination_array(
                          partition, test_id, first_test_string, sizeof(first_test_string),
                          self_test_dev_file_io__large_destination_array[partition])) {
                    success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_open_append_plus_does_not_erase_contents(sl_drive__partition_e partition, size_t test_id) {
  bool success = false;
  void *fd = NULL;
  size_t seek_offset = 0U;

  const char first_test_string[] = "Initializing file with a string.";

  if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_append_plus, &fd)) {
    if (self_test_dev_file_io__private_fwrite(partition, test_id, first_test_string, sizeof(first_test_string), fd)) {
      if (self_test_dev_file_io__private_fclose(partition, test_id, &fd)) {
        if (self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_append_plus, &fd)) {
          if (self_test_dev_file_io__private_fseek(partition, test_id, seek_offset, fd)) {
            if (self_test_dev_file_io__private_fread(partition, test_id, self_test_dev_file_io__large_destination_array[partition],
                                                     sizeof(first_test_string), fd)) {
              if (self_test_dev_file_io__private_match_source_and_destination_array(
                      partition, test_id, first_test_string, sizeof(first_test_string),
                      self_test_dev_file_io__large_destination_array[partition])) {
                success = self_test_dev_file_io__private_fclose(partition, test_id, &fd);
              }
            }
          }
        }
      }
    }
  }

  (void)self_test_dev_file_io__private_fclose(partition, test_id, &fd);

  return success;
}

static bool self_test_dev_file_io__private_test_format_filesystem(sl_drive__partition_e partition, size_t test_id) {
  const bool success = dev_file_io__reformat_filesystem(partition, 0x289);
  if (!success) {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Unable to format normal partition");
  }
  return success;
}

static bool self_test_dev_file_io__private_seek_position_0_for_file_size_0(sl_drive__partition_e partition, size_t test_id) {
  bool is_success = false;
  void *file_descriptor = NULL;

  // Open file in write mode to create new, truncated fild  (i.e. empty);
  if (!self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_write, &file_descriptor)) {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Failed to create new, empty file by opening in mode %s",
                                                    self_test_dev_file_io__open_mode_write);
  } else if (!self_test_dev_file_io__private_fclose(partition, test_id, &file_descriptor)) {
    self_test_dev_file_io__private_log_test_message(partition, test_id,
                                                    "Failed to close file descriptor after immediately opening it");
  } else if (!self_test_dev_file_io__private_fopen(partition, test_id, self_test_dev_file_io__open_mode_read_plus,
                                                   &file_descriptor)) {
    self_test_dev_file_io__private_log_test_message(partition, test_id, "Failed to open empty file in preparation to test seek");
  } else {
    const size_t seek_offset_beginning = 0U;
    is_success = self_test_dev_file_io__private_fseek(partition, test_id, seek_offset_beginning, file_descriptor);

    // Regardless of success/failure of checks above, the previously-opened file should be closed
    is_success = (self_test_dev_file_io__private_fclose(partition, test_id, &file_descriptor) && is_success);
  }

  return is_success;
}

static void self_test_dev_file_io__private_init_partition(sl_drive__partition_e partition, bool enable_verbose_log) {
  self_test_dev_file_io__private[partition].partition = partition;
  self_test_dev_file_io__private_prepare_file_names_by_partition(partition);
  self_test_dev_file_io__private[partition].enable_verbose_log = enable_verbose_log;
}

static int self_test_dev_file_io__private_run_tests(sl_drive__partition_e partition) {
  bool success = true;
  size_t success_count = 0U;
  size_t fail_count = 0U;

  self_test_dev_file_io__test_function_config_s tests[] = {
    {self_test_dev_file_io__private_test_format_filesystem, "format filesystem"},
    {self_test_dev_file_io__private_test_fopen_then_fclose, "open and close"},
    {self_test_dev_file_io__private_test_fwrite_then_fread_small_buffer, "read and write small buffer"},
    {self_test_dev_file_io__private_test_fwrite_then_fread_one_megabyte, "read and write 1MB"},
    {self_test_dev_file_io__private_test_fwrite_then_fgets_the_line, "write and read a line"},
    {self_test_dev_file_io__private_test_verify_and_remove_file, "remove file"},
    {self_test_dev_file_io__private_test_rename_and_remove_file, "rename file"},
    {self_test_dev_file_io__private_test_mkdir_and_delete, "make a directory and delete"},
    {self_test_dev_file_io__private_test_get_file_info_for_file, "get file information for a file"},
    {self_test_dev_file_io__private_test_get_file_info_for_directory, "get file information for a directory"},
    {self_test_dev_file_io__private_test_dir_traverse, "traverse a directory"},
    {self_test_dev_file_io__private_test_get_is_directory_empty, "check if directory is empty"},
    {self_test_dev_file_io__private_test_remove_directory_if_empty, "delete directory if empty"},
    {self_test_dev_file_io__private_test_remove_all_content, "delete all files in a directory"},
    {self_test_dev_file_io__private_test_get_accumulated_size_of_directory_in_bytes,
     "determine accumulated size of a directory at root"},
    {self_test_dev_file_io__private_read_and_write_data_to_file_using_write_plus, "read and write+"},
    {self_test_dev_file_io__private_read_and_write_data_to_file_using_read_plus, "read and write using read+"},
    {self_test_dev_file_io__private_read_and_write_data_to_file_using_append_plus, "read write and append"},
    {self_test_dev_file_io__private_read_and_read_plus_does_not_create_new_file,
     "read and read+ doesn't create new file "},
    {self_test_dev_file_io__private_write_erase_contents_of_existing_file, "write, erase contents of existing_file"},
    {self_test_dev_file_io__private_write_plus_erase_contents_of_existing_file, "erase contents of existing_file"},
    {self_test_dev_file_io__private_open_append_does_not_erase_contents, "append doesn't erase contents"},
    {self_test_dev_file_io__private_open_append_plus_does_not_erase_contents, "append plus doesn't erase contents"},
    {self_test_dev_file_io__private_seek_position_0_for_file_size_0, "seek 0 for 0 file size"},
#if (0 == FEATURE__NO_FILESYSTEM_SIZE_INFO_TEST)
    {self_test_dev_file_io__private_test_get_filesystem_size_info, "get filesystem size info"}
#endif
  };

  // Ensure random values will be consistent
  const int initial_seed_value = 100;
  srand(initial_seed_value);

  for (size_t self_test_function_count = 0U; self_test_function_count < ARRAY_COUNT(tests);
       ++self_test_function_count) {
    self_test_dev_file_io__private_set_up(partition);
    const bool testcase_success = tests[self_test_function_count].self_test_function(partition, self_test_function_count);
    if (!testcase_success) {
      success = false;  // Latch failure
      ++fail_count;
    } else {
      ++success_count;
    }

    self_test_dev_file_io__private_log_test_message(partition, self_test_function_count, "test: %s : %s---",
                                                    tests[self_test_function_count].test_description,
                                                    testcase_success ? "PASSED" : "FAILED");
  }

  snprintf(self_test_dev_file_io__output_buffer[partition], sizeof(self_test_dev_file_io__output_buffer[partition]),
           "---Complete Dev File IO Self-Test---\n"
           "Successful test cases: %lu\n"
           "Failed test cases: %lu\n"
           "Final test result: %s\n",
           (unsigned long)success_count, (unsigned long)fail_count, success ? "SUCCESS" : "FAILED");

  self_test_dev_file_io__message_output(self_test_dev_file_io__output_buffer[partition]);

  return !success;
}

/***********************************************************************************************************************
 *
 *                                          P U B L I C   F U N C T I O N S
 *
 **********************************************************************************************************************/

void self_test_dev_file_io__message_output(const char *message)
{
  (void) (message);
}

int self_test_dev_file_io__run(sl_drive__partition_e partition, bool enable_verbose_log) {
  self_test_dev_file_io__private_init_partition(partition, enable_verbose_log);
  return self_test_dev_file_io__private_run_tests(partition);
}
