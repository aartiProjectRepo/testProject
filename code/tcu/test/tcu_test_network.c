/**
 * @file        network_test.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        28 November 2023
 * @author      Aditya P <aditya.prajapati@accoladeelectronics.com>
 *              Adwait Patil <adwait.patil@gmail.com>
 *
 * @brief       TCU-2W test code - Network test.
 */

// Standard includes.
#include <stdint.h>

// HAL interface includes.
#include "hal_util.h"

// Logger include.
#include "logger.h"

// Network includes.
#include "at_command_handler.h"
#include "connection_mgr.h"
#include "modem_port.h"

// TCU 2W includes.
#include "tcu_test.h"

// Private defines.
#define NETWORK_TEST_MEM_SIZE               ( 100 )

// Private variables.
uint8_t g_network_test_mem[NETWORK_TEST_MEM_SIZE];
uint32_t g_timer = 0;

void tcu_test_network(void)
{
    uint8_t receivedString = 0;
    
    AtInit();
    ConnectionMgrInit();
    
    RESET_TIMER(g_timer, 0);

    while (1)
    {
        AtExe();
        ConnectionMgrExe();
        if (IS_TIMER_ELAPSED(g_timer))
        {
            ConnectionMgrInfo_t connInfo = ConnectionMgrGetInfo();
            logger("[NET] network status: %d, CREG: %d, CGREG: %d, CSQ: %d, op name: %s %d, IP: %s \r\n",
                   connInfo.networkStatus, connInfo.creg, connInfo.cgreg, connInfo.csq, connInfo.operatorName,
                   connInfo.accessTechnology, connInfo.ipAddress);
            RESET_TIMER(g_timer, 3 * 1000);
        }
        receivedString = logger_read(g_network_test_mem, 100);
        if (0 < receivedString)
        {
            receivedString = 0;
            logger("%s", g_network_test_mem);
            ModemWrite(g_network_test_mem, strlen((char *)g_network_test_mem));
        }
        hal_util_delay(10);
        memset(g_network_test_mem, 0, sizeof(g_network_test_mem));
    }
}
