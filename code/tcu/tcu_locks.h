/**
 * @file        tcu_locks.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        18 December 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU locks include.
 */

#ifndef TCU_LOCKS_H
#define TCU_LOCKS_H

#include <stdbool.h>
#include "injectable.h"

typedef enum
{
    // Module locks - make entire file thread-safe.
    TcuLocksModuleNorFlash,
    TcuLocksModuleLfsAdapter,
    TcuLocksModuleFilesystem,
    TcuLocksModuleTcuTime,
    // Filesystem partition lock - required by LFS for each partition.
    TcuLocksFsPartition0,
    TcuLocksFsPartition1,
    TcuLocksFsPartition2,
    TcuLocksFsPartition3,
    TcuLocksFsPartition4,
    TcuLocksFsPartition5,
    TcuLocksFsPartition6,
    TcuLocksMax
} TcuLocks_n;

/**
 *  @brief                                  Initializes this module for usage. 
*/
void tcu_locks_global_init                  (void);

/**
 *  @brief                                  Gets mutex primitive for given predefined enum.
 *  @param      lock                        One of the enumerated values.
 *  @return                                 Mutex primitive (contains lock/unlock procedures).
 *  @warning                                Will assert and halt if illegal argument is provided.
*/
iface_mutex_t tcu_lock_get                  (TcuLocks_n lock);

#endif /* TCU_LOCKS_H*/
