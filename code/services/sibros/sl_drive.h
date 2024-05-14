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
#ifndef SIBROS__SL_DRIVE_H
#define SIBROS__SL_DRIVE_H

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

/* External Includes */

/* Module Includes */

/***********************************************************************************************************************
 *
 *                                                   D E F I N E S
 *
 **********************************************************************************************************************/

/**
 * Drive prefixes corresponding to the drives (or partitions)
 * Files need to be opened with respective drive prefixes, such as 'u:updater.hex'
 * The 'normal' partition may have its drive prefix omitted, hence 'n:file.txt' is the same as 'file.txt'
 */

#if (1 == FEATURE__LITTLEFS)
#define SL_DRIVE__UPDATER_PARTITION_PREFIX "u:"
#define SL_DRIVE__SECURE_PARTITION_PREFIX "s:"
#define SL_DRIVE__NORMAL_PARTITION_PREFIX "n:"
#else
#define SL_DRIVE__UPDATER_PARTITION_PREFIX ""
#define SL_DRIVE__SECURE_PARTITION_PREFIX ""
#define SL_DRIVE__NORMAL_PARTITION_PREFIX ""

#endif

/***********************************************************************************************************************
 *
 *                                                  T Y P E D E F S
 *
 **********************************************************************************************************************/
/**
 * Sibros software utilizes multiple partitions for robust storage.
 *
 * The NORMAL drive is the default partition, and other partitions
 * are dedicated for their respective and specific functions.
 */
typedef enum {
  SL_DRIVE__PARTITION_NORMAL,
  SL_DRIVE__PARTITION_UPDATER,
  SL_DRIVE__PARTITION_SECURE,
  SL_DRIVE__PARTITION_COUNT,
} sl_drive__partition_e;

typedef struct {
  char cstring[3U];
} sl_drive__prefix_s;

/***********************************************************************************************************************
 *
 *                                     F U N C T I O N   D E C L A R A T I O N S
 *
 **********************************************************************************************************************/

/* For invalid partitions, the empty prefix is returned */
sl_drive__prefix_s sl_drive__get_prefix(sl_drive__partition_e partition);

bool sl_drive__is_partition_valid(sl_drive__partition_e partition);

#ifdef __cplusplus
} /* extern "c" */
#endif
#endif /* #ifndef SIBROS__SL_DRIVE_H */
