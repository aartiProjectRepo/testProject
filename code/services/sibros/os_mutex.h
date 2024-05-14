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
 * Interface for custom implementation of OS Mutex interface
 * The methods for initializing, locking and unlocking the mutex must be provided by the user
 * through a call to the os_mutex__set_methods API
 *
 * Memory for the mutex implementation will be allocated inside the os_mutex_s struct instance
 * so the struct must remain in scope as long as the mutex is used
 *
 * Example:
 * @code
 * *** Global configuration ***
 * os_mutex__methods_s mutex_methods = {
 *   .init = mutex_init_implementation,
 *   .lock = mutex_lock_implementation,
 *   .unlock = mutex_unlock_implementation,
 * };
 * os_mutex__set_methods(mutex_methods);
 *
 * *** Mutex usage ***
 * os_mutex_s mutex = {0};
 * (void)os_mutex__init(&mutex);
 *
 * (void)os_mutex__lock(&mutex);
 * ... Critical section ...
 * (void)os_mutex__unlock(&mutex);
 * @endcode
 *
 * Thread Safety Assessment:
 * - It is expected for the implementation to be thread-safe due to the nature of mutual exclusion
 */

#ifndef SIBROS__OS_MUTEX_H
#define SIBROS__OS_MUTEX_H

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
#include "os_mutex_types.h"

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

/*
 * Configure the init, lock, unlock methods for mutexes globally
 * This method must be called before any mutexes are initialized or used
 */
void os_mutex__set_methods(os_mutex__methods_s methods);

/**
 * Initialize the mutex interface with the methods and pointer to an instance of an actual mutex implementation
 */
bool os_mutex__init(os_mutex_s *mutex);

bool os_mutex__is_valid(const os_mutex_s *mutex);

bool os_mutex__lock(os_mutex_s *mutex);

bool os_mutex__unlock(os_mutex_s *mutex);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifdef SIBROS__OS_MUTEX_H */
