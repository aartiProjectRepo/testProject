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

#ifndef SIBROS__OS_TIME_H
#define SIBROS__OS_TIME_H

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
#include <stdint.h>

/* External Includes */

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

typedef struct {
  uint64_t seconds;
  uint32_t nanoseconds;
} os_time__epoch_time_s;

/***********************************************************************************************************************
 *
 *                                     F U N C T I O N   D E C L A R A T I O N S
 *
 **********************************************************************************************************************/
/**
 * Returns the current time in ms.
 * Time is not absolute, it is relative to the boot time.
 */
uint32_t os_time__get_time_ms(void);

/**
 * Time is not absolute, it is relative to the boot time.
 */
uint32_t os_time__get_time_us(void);

/**
 * Returns the time elapsed in ms since the provided input time
 * Useful for calculating if a function has timed out
 */
uint32_t os_time__get_time_elapsed_ms(uint32_t past_time_ms);

/**
 * @param epoch_time the pointer to the epoch time structure to populate with current epoch
 * @returns true upon success
 */
bool os_time__get_epoch_time(os_time__epoch_time_s* epoch_time);

/**
 * @param epoch_time epoch time structure representing the time to convert into milliseconds.
 * @returns the millisecond representation of the specified epoch timestamp.
 */
uint64_t os_time__get_epoch_in_ms(os_time__epoch_time_s epoch);

/**
 * Get current epoch time with the offset
 * @param offset_to_add The offset to add to the current epoch
 * @returns the epoch time with the offset_to_add added
 */
os_time__epoch_time_s os_time__get_time_with_offset(os_time__epoch_time_s offset_to_add);

/**
 * @returns the sum of the two epochs
 */
os_time__epoch_time_s os_time__sum_epochs(os_time__epoch_time_s a, os_time__epoch_time_s b);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* #ifndef SIBROS__OS_TIME_H */
