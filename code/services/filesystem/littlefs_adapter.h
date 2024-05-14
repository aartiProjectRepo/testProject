/**
 * @file        littlefs_adapter.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        28 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       LittleFs adapter - links LittleFs to NorFlash - header.
 */

#ifndef LITTLEFS_ADAPTER_H
#define LITTLEFS_ADAPTER_H

#include <stdbool.h>
#include "injectable.h"
#include "filesystem.h"
#include "lfs.h"

void littlefs_adapter_init(iface_mutex_t mutex);
lfs_t* littlefs_adapter_get_lfs(Filesystem_n filesystem);
struct lfs_config* littlefs_adapter_get_config(Filesystem_n filesystem);

#endif /* LITTLEFS_ADAPTER_H */
