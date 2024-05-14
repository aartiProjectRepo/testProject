/**
 * @file        injectable.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2022-23
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        28 October 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Common inject-able interfaces used throughout project.
 *
 * @note        TODO THIS IS A WORK IN PROGRESS, IT WILL UPDATE AND CHANGE . . . .
 */

#ifndef INJECTABLE_H
#define INJECTABLE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief                   Functions that take no arguments and return nothing.
 */
typedef void (*iface_procedure_t) (void);

typedef struct
{
    iface_procedure_t lock;
    iface_procedure_t unlock;
} iface_mutex_t;

/**
 * @brief                   Functions that take requested length as argument and return pointer to allocated memory when successful.
 */
typedef void* (*iface_alloc_t) (size_t size);
typedef void (*iface_free_t) (void* ptr);

typedef struct
{
    iface_alloc_t alloc;
    iface_free_t free;
} iface_allocator_t;

/**
 * @brief                   Functions that take no arguments and return a uint32_t.
 */
typedef uint32_t (*iface_ret32_t) (void);

/**
 * @brief                   Functions that take no arguments and return a uint32_t.
 */
typedef uint64_t (*iface_ret64_t) (void);

/**
 * @brief                   One Argument Functions (OAF).
*/
typedef void        (*iface_v_oaf_v_t)      (void);
typedef void        (*iface_v_oaf_p_t)      (void*);
typedef void        (*iface_v_oaf_8_t)      (uint8_t);
typedef void        (*iface_v_oaf_16_t)     (uint16_t);
typedef void        (*iface_v_oaf_32_t)     (uint32_t);
typedef void        (*iface_v_oaf_64_t)     (uint64_t);
typedef void*       (*iface_p_oaf_v_t)      (void);
typedef void*       (*iface_p_oaf_p_t)      (void*);
typedef void*       (*iface_p_oaf_8_t)      (uint8_t);
typedef void*       (*iface_p_oaf_16_t)     (uint16_t);
typedef void*       (*iface_p_oaf_32_t)     (uint32_t);
typedef void*       (*iface_p_oaf_64_t)     (uint64_t);
typedef uint8_t     (*iface_8_oaf_v_t)      (void);
typedef uint8_t     (*iface_8_oaf_p_t)      (void*);
typedef uint8_t     (*iface_8_oaf_8_t)      (uint8_t);
typedef uint8_t     (*iface_8_oaf_16_t)     (uint16_t);
typedef uint8_t     (*iface_8_oaf_32_t)     (uint32_t);
typedef uint8_t     (*iface_8_oaf_64_t)     (uint64_t);
typedef uint16_t    (*iface_16_oaf_v_t)     (void);
typedef uint16_t    (*iface_16_oaf_p_t)     (void*);
typedef uint16_t    (*iface_16_oaf_8_t)     (uint8_t);
typedef uint16_t    (*iface_16_oaf_16_t)    (uint16_t);
typedef uint16_t    (*iface_16_oaf_32_t)    (uint32_t);
typedef uint16_t    (*iface_16_oaf_64_t)    (uint64_t);
typedef uint32_t    (*iface_32_oaf_v_t)     (void);
typedef uint32_t    (*iface_32_oaf_p_t)     (void*);
typedef uint32_t    (*iface_32_oaf_8_t)     (uint8_t);
typedef uint32_t    (*iface_32_oaf_16_t)    (uint16_t);
typedef uint32_t    (*iface_32_oaf_32_t)    (uint32_t);
typedef uint32_t    (*iface_32_oaf_64_t)    (uint64_t);
typedef uint64_t    (*iface_64_oaf_v_t)     (void);
typedef uint64_t    (*iface_64_oaf_p_t)     (void*);
typedef uint64_t    (*iface_64_oaf_8_t)     (uint8_t);
typedef uint64_t    (*iface_64_oaf_16_t)    (uint16_t);
typedef uint64_t    (*iface_64_oaf_32_t)    (uint32_t);
typedef uint64_t    (*iface_64_oaf_64_t)    (uint64_t);

/**
* @brief                    Logging functionality.
* @param   fmt              String optionally containing format specifiers.
*/
typedef void (*iface_logger_t) (const char* fmt, ...);

/**
* @brief                    I2C read/write functionality.
* @param   fmt              String optionally containing format specifiers.
*/
typedef void (*iface_i2c_xfer_t) (uint8_t addr, uint8_t reg, uint8_t *buff, int len);


/**
 * @note
 * The 'pre' and 'post' are common to all macros.
 * pre  -   Use for module name (the module where this macro is being extracted).
 * post -   Use for the class name (the type of object being injected).
 * Each declaration in header should have corresponding definition in source.
 * Usage example:
 *
 *
 *
 * // ----------------------------------------------------------
 * // -------------------------HEADER---------------------------
 * // ----------------------------------------------------------
 * #include "injectable.h"
 *
 * INJECTABLE_DECL_PROC(cam_driver, lock_take);
 * INJECTABLE_DECL_PROC(cam_driver, lock_release);
 * INJECTABLE_DECL_LOGGER(cam_driver, logger);
 * void cam_driver_init(void);
 * void cam_driver_grab_frame(void* frameBuf);
 * void cam_driver_print_stats(void);
 * void cam_driver_set_frame_size(uint32_t width, uint32_t height);
 *
 *
 *
 * // ----------------------------------------------------------
 * // ---------------------IMPLEMENTATION-----------------------
 * // ----------------------------------------------------------
 * #include "cam_driver.h"
 *
 * INJECTABLE_DEFN_PROC(cam_driver, lock_take)
 * INJECTABLE_DEFN_PROC(cam_driver, lock_release)
 * INJECTABLE_DEFN_LOGGER(cam_driver, logger);
 *
 * void cam_driver_init(void)
 * {
 *      assert(g_fn_cam_driver_lock_take);              // This function has compulsory run-time dependency for lock take.
 *      assert(g_fn_cam_driver_lock_release);           // This function has compulsory run-time dependency for lock release.
 *      // useful code . . .
 * }
 *
 * void cam_driver_grab_frame(void* frameBuf)
 * {
 *      assert(g_fn_cam_driver_lock_take);              // This function has compulsory run-time dependency for lock take.
 *      assert(g_fn_cam_driver_lock_release);           // This function has compulsory run-time dependency for lock release.
 *      // useful code . . .
 * }
 *
 * void cam_driver_print_stats(void)
 * {
 *      assert(g_fn_cam_driver_logger);                 // This function has compulsory run-time dependency for logger.
 *      // useful code . . .
 * }
 *
 * void cam_driver_set_frame_size(uint32_t width, uint32_t height)
 * {
 *      // This function has no compulsory run-time dependencies.
 *      // useful code . . .
 * }
 *
 *
 *
 * // ----------------------------------------------------------
 * // ---------------------- END - USAGE -----------------------
 * // ----------------------------------------------------------
 * #include <stdio.h>           // Fulfills logger dependency.
 * #include "capture.h"         // Sets up and then uses the dependency injected module.
 * #include "cam_driver.h"      // Dependent.
 * #include "lock.h"            // Fulfills lock dependency.
 *
 * void capture_init(void)
 * {
 *
 *      cam_driver_inject_lock_take ( lock_take_concrete_fn );
 *      cam_driver_inject_lock_release ( lock_release_concrete_fn );
 *      cam_driver_inject_logger ( printf );
 *      cam_driver_init();
 *      capture_start_worker_threads();
 * }
 *
 * // ..... other useful implementation.
 *
 */
#define INJECTABLE_DECL_PROC(pre, post)             void pre##_inject_##post(iface_procedure_t fn);
#define INJECTABLE_DECL_RET32(pre, post)            void pre##_inject_##post(iface_ret32_t fn);
#define INJECTABLE_DECL_RET64(pre, post)            void pre##_inject_##post(iface_ret64_t fn);
#define INJECTABLE_DECL_LOGGER(pre, post)           void pre##_inject_##post(iface_logger_t fn);
#define INJECTABLE_DECL_I2C_XFER(pre, post)         void pre##_inject_##post(iface_i2c_xfer_t fn);
#define INJECTABLE_DEFN_PROC(pre, post)             static iface_procedure_t g_fn_##post = NULL; void pre##_inject_##post(iface_procedure_t fn) { g_fn_##post = fn; }
#define INJECTABLE_DEFN_RET32(pre, post)            static iface_ret32_t g_fn_##post = NULL; void pre##_inject_##post(iface_ret32_t fn) { g_fn_##post = fn; }
#define INJECTABLE_DEFN_RET64(pre, post)            static iface_ret64_t g_fn_##post = NULL; void pre##_inject_##post(iface_ret64_t fn) { g_fn_##post = fn; }
#define INJECTABLE_DEFN_LOGGER(pre, post)           static iface_logger_t g_fn_##post = NULL; void pre##_inject_##post(iface_logger_t fn) { g_fn_##post = fn; }
#define INJECTABLE_DEFN_I2C_XFER(pre, post)         static iface_i2c_xfer_t g_fn_##post = NULL; void pre##_inject_##post(iface_logger_t fn) { g_fn_##post = fn; }

#endif /* INJECTABLE_H */
