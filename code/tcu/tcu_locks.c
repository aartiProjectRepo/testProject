/**
 * @file        tcu_locks.c
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
 * @brief       TCU locks - lock factory (unclean) implementation.
 */

// Standard includes.
#include <stdbool.h>
#include <stddef.h>

// Self include.
#include "tcu_locks.h"

// Dependencies.
#include "hal_util.h"

// RTOS includes.
#include "osal_mutex.h"

// Private functions.
static void tcu_locks_generic_lock          (OsalMutex_t handle);
static void tcu_locks_generic_unlock        (OsalMutex_t handle);

// Heavy macros.
#define TCU_LOCKS_TRUE                      ( 0xC0C010CC )
#define TCU_LOCKS_FALSE                     ( 0xDEAF10CC )
#define TCU_LOCKS_MACRO(name) \
    static OsalMutex_t g_mtx_##name; \
    static void lock_##name##(void)     { tcu_locks_generic_lock(g_mtx_##name); } \
    static void unlock_##name##(void)   { tcu_locks_generic_unlock(g_mtx_##name); }
TCU_LOCKS_MACRO(0);
TCU_LOCKS_MACRO(1);
TCU_LOCKS_MACRO(2);
TCU_LOCKS_MACRO(3);
TCU_LOCKS_MACRO(4);
TCU_LOCKS_MACRO(5);
TCU_LOCKS_MACRO(6);
TCU_LOCKS_MACRO(7);
TCU_LOCKS_MACRO(8);
TCU_LOCKS_MACRO(9);
TCU_LOCKS_MACRO(10);

// Private defines.
typedef struct
{
    OsalMutex_t* const pHandle;
    iface_mutex_t mutex;
} TcuLocks_t;

// Private data.
static TcuLocks_t g_table[TcuLocksMax] = 
{
    { &g_mtx_0,     { lock_0,   unlock_0    }   },
    { &g_mtx_1,     { lock_1,   unlock_1    }   },
    { &g_mtx_2,     { lock_2,   unlock_2    }   },
    { &g_mtx_3,     { lock_3,   unlock_3    }   },
    { &g_mtx_4,     { lock_4,   unlock_4    }   },
    { &g_mtx_5,     { lock_5,   unlock_5    }   },
    { &g_mtx_6,     { lock_6,   unlock_6    }   },
    { &g_mtx_7,     { lock_7,   unlock_7    }   },
    { &g_mtx_8,     { lock_8,   unlock_8    }   },
    { &g_mtx_9,     { lock_9,   unlock_9    }   },
    { &g_mtx_10,    { lock_10,  unlock_10   }   }
};
static uint32_t g_initDone = 0xDEAF10CC;

void tcu_locks_global_init                  (void)
{
    size_t idx; 

    hal_util_assert ( ( TCU_LOCKS_TRUE == g_initDone ) || ( TCU_LOCKS_FALSE == g_initDone ) );
    if ( TCU_LOCKS_FALSE == g_initDone )
    {
        for ( idx = 0 ; idx < ( sizeof(g_table) / sizeof(g_table[0]) ) ; ++idx )
        {
            hal_util_assert( !*(g_table[idx].pHandle) );
            hal_util_assert( OsalErrOk == osal_mutex_create(g_table[idx].pHandle) );
            hal_util_assert( *(g_table[idx].pHandle) );
        }
        g_initDone = TCU_LOCKS_TRUE;
    }
}

iface_mutex_t tcu_lock_get                  (TcuLocks_n lock)
{
    hal_util_assert ( lock < TcuLocksMax );
    return g_table[lock].mutex;
}

static void tcu_locks_generic_lock          (OsalMutex_t handle)
{
    hal_util_assert ( OsalErrOk == osal_mutex_take(handle) );
}

static void tcu_locks_generic_unlock        (OsalMutex_t handle)
{
    hal_util_assert ( OsalErrOk == osal_mutex_give(handle) );
}
