/**
 * @file        tcu_test_gps.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        30 November 2023
 * @author      Aditya Patil <aditya.patil@accoladeelectronics.com>
 *
 * @brief       TCU-2W test code - GPS test.
 *              UART init fot GPS and GSM must be done before hand
 */

// Standard includes.
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// HAL interface includes.
#include "hal_uart.h"
#include "hal_util.h"

// Port includes.
#include "rh850_uart.h"

// TCU 2W includes.
#include "tcu_test.h"

// Module includes
#include "logger.h"
#include "modem_port.h"
#include "gps.h"

// Private defines.
#define RX_BUFF_MAX_LEN                     ( 1024UL )

// Private variables.
static uint8_t g_receiveBuff[RX_BUFF_MAX_LEN];
static uint32_t g_printTmr = 0;

void tcu_test_gps(void)
{
    bool initDone = false;
    logger("modem uart init done\r\n");

    // turn on the modem
    ModemRstKeyHigh();
    hal_util_delay(1000);
    ModemRstKeyLow();
    logger("modem powered on\r\n");

    while (1)
    {
        // Read data which is received on gsm uart and print on console
        if (ModemDataReady())
        {
            ModemRead(g_receiveBuff, RX_BUFF_MAX_LEN);
            logger("%s", g_receiveBuff);

            // once modem is ready, configure the GPS port to start sending data NMEA strings
            if ( (initDone == false) && strstr((const char*)g_receiveBuff, "RDY"))
            {
                initDone = true;

                logger("\n\rconfiguring gps uart from modem side (reboot may be required for first time)");
                hal_util_delay(100);
                uint8_t atCmd[] = "AT+QGPSCFG=\"outport\",\"uart5\";+QGPS=1;+QGPSCFG=\"autogps\",1\r\n";
                ModemWrite(atCmd, sizeof(atCmd));

                logger("%s\n\r", atCmd);
                memset(atCmd, 0, sizeof(atCmd));
                hal_util_delay(10);

                RESET_TIMER(g_printTmr, 1000);
            }
        }

        GpsExe();

        // print gnss data periodically
        if (IS_TIMER_ELAPSED(g_printTmr))
        {
            RESET_TIMER(g_printTmr, 1000);

            GpsInfo_t gpsInfo = GpsGetInfo();
            logger("gps utc_time %d date %u fix %d lat %f long %f speed_kph %.2f num_sat %d alt %.2f\n\r", \
            gpsInfo.utcTime, gpsInfo.date, gpsInfo.fixStatus, gpsInfo.latitude , gpsInfo.longitude , gpsInfo.speed , gpsInfo.numOfSatellites , gpsInfo.altitude);
        }

        hal_util_memset(g_receiveBuff, 0, sizeof(g_receiveBuff));
        hal_util_delay(100);
    }
}
