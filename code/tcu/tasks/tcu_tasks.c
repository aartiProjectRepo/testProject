/**
 * @file        tcu_tasks.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        1 January 2024
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU tasks - implementation.
 */

// Self.
#include "tcu_tasks.h"

// Standard.
#include <stdint.h>
#include "inttypes.h"
#include "printf.h"

// OSAL includes.
#include "osal.h"

// HAL includes.
#include "hal_uart.h"
#include "hal_util.h"

// Board includes.
#include "tcu_board.h"

// User-functionality includes.
#include "tcu_locks.h"
#include "logger.h"
#include "drv_can.h"
#include "tcu_test.h"
#include "gps.h"

// Network includes.
#include "at_command_handler.h"
#include "connection_mgr.h"
#include "modem_port.h"
#include "x_mqtt_manager.h"
#include "tcu_2w_test_mqtt.h"

// Filesystem includes.
#include "self_test_dev_file_io.h"

// Sibros includes.
#include "os_mutex.h"

// Private defines.
typedef enum
{
    TcuTasksSys,
    TcuTasksSvc,
    TcuTasksDebug,
    TcuTasksApp1,
    TcuTasksApp2,
    TcuTasksApp3,
    TcuTasksMax
} TcuTasks_n;

typedef struct
{
    OsalThread_t handle;
    const OsalThreadConfig_t config;
} TcuTasksInit_t;

// Private variables (for thread exec time diagnostics).
uint32_t g_var_sys;

// Private timer variables.
static uint32_t gTmrGpio;
static uint32_t gTmrOstime;
static uint32_t gTmrGsm;
static uint8_t g_memSvcThreadUartRx[8192UL];

// Private Sibros locks.
static os_mutex_s g_os_mutex_app1;
static os_mutex_s g_os_mutex_app2;
static os_mutex_s g_os_mutex_app3;

// Private functionality.
static void pre_start_hook                  (void);
static void initSvc                         (OsalThread_t thread);
static void initApp1                        (OsalThread_t thread);
static void initApp2                        (OsalThread_t thread);
static void initApp3                        (OsalThread_t thread);
static void runnerSys                       (OsalThread_t thread);
static void runnerSvc                       (OsalThread_t thread);
static void runnerDebug                     (OsalThread_t thread);
static void runnerApp1                      (OsalThread_t thread);
static void runnerApp2                      (OsalThread_t thread);
static void runnerApp3                      (OsalThread_t thread);
static void logger_for_service_thread       (char* fmt, ...);
static size_t consumeDebugRx                (void);

TcuTasksInit_t g_taskTable[TcuTasksMax] = 
{
    { { NULL }, { NULL,     runnerSys,      OsalThreadPriority_7,   1024UL,     2UL,    "System"        } },    // 2 ms.
    { { NULL }, { initSvc,  runnerSvc,      OsalThreadPriority_5,   4096UL,     20UL,   "Service"       } },    // 20 ms.
    { { NULL }, { NULL,     runnerDebug,    OsalThreadPriority_3,   4096UL,     250UL,  "Debug"         } },    // 250 ms.
    { { NULL }, { initApp1, runnerApp1,     OsalThreadPriority_1,   4096UL,     0UL,    "App1",         } },    // Unbounded.
    { { NULL }, { initApp2, runnerApp2,     OsalThreadPriority_1,   4096UL,     0UL,    "App2",         } },    // Unbounded.
    { { NULL }, { initApp3, runnerApp3,     OsalThreadPriority_1,   4096UL,     0UL,    "App3",         } },    // Unbounded.
};

void tcu_tasks                              (void)
{
    // Initialize OSAL and threads.
    hal_util_assert ( OsalErrOk == osal_global_init(logger) );
    for ( TcuTasks_n t = TcuTasksSys ; t < TcuTasksMax ; ++t ) 
    {
        hal_util_assert ( OsalErrOk == osal_thread_create(&g_taskTable[t].handle,  &g_taskTable[t].config) );
    }

    // Initialize TCU locks.
    tcu_locks_global_init();

    // Outside RTOS.
    pre_start_hook();
    
    // Start the OS.
    osal_start();
}

static void pre_start_hook                  (void)
{
    tcu_test_memory();
    tcu_test_flash();
}


static void initSvc                         (OsalThread_t thread)
{
    hal_util_assert ( OsalErrOk == osal_tmr_init(&gTmrGpio) );
    hal_util_assert ( OsalErrOk == osal_tmr_init(&gTmrOstime) );
    hal_util_assert ( OsalErrOk == osal_tmr_init(&gTmrGsm) );
    
    net_log_init(logger_for_service_thread);
    AtInit();
    ConnectionMgrInit();
    XMQTT_Init();
    appMqtt_Init();
    GpsInit(logger_for_service_thread);
}


static void initApp1                        (OsalThread_t thread)
{
    hal_util_assert ( true == os_mutex__init(&g_os_mutex_app1) );
}


static void initApp2                        (OsalThread_t thread)
{
    hal_util_assert ( true == os_mutex__init(&g_os_mutex_app2) );
}


static void initApp3                        (OsalThread_t thread)
{
    hal_util_assert ( true == os_mutex__init(&g_os_mutex_app3) );
}


static void runnerSys                       (OsalThread_t thread)
{
    // System thread code.
    drv_can_bPeriodicTask();
    hal_uart_process(g_UartGps);
    hal_uart_process(g_UartGsm);
    hal_uart_process(g_UartDbg);
    osal_service_execute();
    
    // Trace code, don't remove.
    __nop();
    g_var_sys += g_taskTable[TcuTasksSys].config.periodicityMs;
    hal_util_delay_us( ( g_taskTable[TcuTasksSys].config.periodicityMs * 1000UL ) * 0.10 ); // 0.2 ms
    __nop();
}


static void runnerSvc                       (OsalThread_t thread)
{
    // Service thread code.
    tcu_test_can();         //FIXME: Uses raw logger!
    AtExe();
    ConnectionMgrExe();
    GpsExe();
    XMQTT_Execute();
    appMqtt_Exe();
    
    hal_util_assert ( OsalErrOk == osal_tmr_exec(tcu_test_gpio,            500,     &gTmrGpio) );        //FIXME: Uses raw logger!
    hal_util_assert ( OsalErrOk == osal_tmr_exec(printUpTime,              5000,    &gTmrOstime) );      //FIXME: Uses raw logger!
    hal_util_assert ( OsalErrOk == osal_tmr_exec(ConnectionMgrPrintInfo,   10000,   &gTmrGsm) );
    
    // Trace code, don't remove.
    __nop();
    hal_util_delay_us( ( g_taskTable[TcuTasksSvc].config.periodicityMs * 1000UL ) * 0.20 ); // 4 ms
    __nop();
}


static void runnerDebug                     (OsalThread_t thread)
{
    OsalThreadStats_t stats[TcuTasksMax] = {0};
    TcuTasks_n taskIndex = TcuTasksSys;
    size_t countRx = 0;
    static size_t totalRx = 0;

    // Consume Debug Rx.
    countRx = consumeDebugRx();
    totalRx += countRx;

    // Print Stats.
    osal_logger(thread, "Stats:");
    for ( taskIndex = TcuTasksSys ; taskIndex < TcuTasksMax ; ++taskIndex )
    {
        osal_thread_get_stats(g_taskTable[taskIndex].handle, &stats[taskIndex]);
        osal_logger(thread, " %7d", stats[taskIndex].countLoops);
    }
    osal_logger(thread, " <%7d, %7d>\r\n", countRx, totalRx);

    // Trace code, don't remove.
    __nop();
    hal_util_delay_us( ( g_taskTable[TcuTasksDebug].config.periodicityMs * 1000UL ) * 0.20 ); // 50 ms
    __nop();
}


static void runnerApp1                      (OsalThread_t thread)
{
    static uint32_t success = 0;
    static uint32_t total = 0;

    hal_util_assert ( os_mutex__lock(&g_os_mutex_app1) );
    if ( 0 == self_test_dev_file_io__run(SL_DRIVE__PARTITION_UPDATER, true) )
    {
        success++;
    }
    hal_util_assert ( os_mutex__unlock(&g_os_mutex_app1) );
    total++;
    osal_logger(thread, "Test Filesystem Updater - %"PRIu32" / %"PRIu32"\r\n", success, total);
}


static void runnerApp2                      (OsalThread_t thread)
{
    static uint32_t success = 0;
    static uint32_t total = 0;

    hal_util_assert ( os_mutex__lock(&g_os_mutex_app2) );
    if ( 0 == self_test_dev_file_io__run(SL_DRIVE__PARTITION_SECURE, true) )
    {
        success++;
    }
    hal_util_assert ( os_mutex__unlock(&g_os_mutex_app2) );
    total++;
    osal_logger(thread, "Test Filesystem Secure - %"PRIu32" / %"PRIu32"\r\n", success, total);
}


static void runnerApp3                      (OsalThread_t thread)
{
    static uint32_t success = 0;
    static uint32_t total = 0;

    hal_util_assert ( os_mutex__lock(&g_os_mutex_app3) );
    if ( 0 == self_test_dev_file_io__run(SL_DRIVE__PARTITION_NORMAL, true) )
    {
        success++;
    }
    hal_util_assert ( os_mutex__unlock(&g_os_mutex_app3) );
    total++;
    osal_logger(thread, "Test Filesystem Normal - %"PRIu32" / %"PRIu32"\r\n", success, total);
}


static void logger_for_service_thread       (char* fmt, ...)
{
    char printMem[128UL] = {0};
    va_list args;
    
    va_start(args, fmt);
    (void) vsnprintf(printMem, sizeof(printMem), fmt, args);
    va_end(args);
    osal_logger(g_taskTable[TcuTasksSvc].handle, "%s", printMem);
}


static size_t consumeDebugRx                (void)
{
    HalBuffer_t bufRx = { .mem = g_memSvcThreadUartRx, .sizeMem = sizeof(g_memSvcThreadUartRx) };
    size_t countRx = 0;
    
    if ( HalUartErrOk == hal_uart_receive_available(g_UartDbg, &countRx ) )
    {
        if ( countRx )
        {
            hal_util_assert ( countRx < bufRx.sizeMem );
            hal_util_assert ( HalUartErrOk == hal_uart_read(g_UartDbg, bufRx, countRx) );
        }
    }
    else
    {
        countRx = 0;
    }
    
    return countRx;
}
