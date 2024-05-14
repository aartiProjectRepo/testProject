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
 * Self Test module for `dev_file_io.h` to test the ability to robustly read and write files
 */

#ifndef SIBROS__SELF_TEST_DEV_FILE_IO_H
#define SIBROS__SELF_TEST_DEV_FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 *
 *                                                  I N C L U D E S
 *
 **********************************************************************************************************************/
/* Standard Includes */

/* External Includes */
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
 * Logs test message
 *
 * @param message is sent to customer implemented output
 */
void self_test_dev_file_io__message_output(const char *message);

/**
 * Executes all tests for dev_file_io
 *
 * @return 0 on success, 1 on failure
 */
int self_test_dev_file_io__run(sl_drive__partition_e partition, bool enable_verbose_log);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifdef SIBROS__SELF_TEST_DEV_FILE_IO_H */
