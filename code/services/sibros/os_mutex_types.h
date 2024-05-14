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
 * Thread Safety Assessment:
 * - Intended to be thread-safe
 */

#ifndef SIBROS__OS_MUTEX_TYPES_H
#define SIBROS__OS_MUTEX_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 *
 *                                                  I N C L U D E S
 *
 **********************************************************************************************************************/
/* Standard Includes */
#include <stddef.h>
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

/*
 * Memory for any mutex implementation must have size less than or equal to this value
 */
typedef struct {
  uint8_t buffer[160U];
} os_mutex__implementation_buffer_s;

/*
 * The user will not be explicitly passing in the implementation to these calls
 * Instead, os_mutex_s.private_data.implementation.buffer will be passed in
 */
typedef bool (*os_mutex__init_f)(void *mutex_implementation);
typedef bool (*os_mutex__lock_f)(void *mutex_implementation);
typedef bool (*os_mutex__unlock_f)(void *mutex_implementation);

typedef struct {
  os_mutex__init_f init;
  os_mutex__lock_f lock;
  os_mutex__unlock_f unlock;
} os_mutex__methods_s;

typedef struct {
  struct {
    os_mutex__implementation_buffer_s implementation;
    bool is_initialized;
  } private_data;
} os_mutex_s;

/***********************************************************************************************************************
 *
 *                                     F U N C T I O N   D E C L A R A T I O N S
 *
 **********************************************************************************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifdef SIBROS__OS_MUTEX_TYPES_H */
