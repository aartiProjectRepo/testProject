/**
 * @file        tcu_2w_test_can.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        02 Dec 2023
 * @author      Swati Sinare <swati.sinare@accoladeelectronics.com>
 *
 * @brief       CAN Driver Test file.
 */

// Standard includes.
#include <stdint.h>

// Driver interface includes.
#include "drv_can.h"

// Logger includes
#include "logger.h"
#include "logger_can.h"

void tcu_test_can(void)
{
    static uint64_t rxCount = 0;
    CAN_MSG_T canMsg = {0};
    uint8_t idx = 0;
    canMsg.u8Dlc = 8;
    
    LoggerCan000_SysA_t sysA;
    logger_can_get(LoggerCan000_SysA, &sysA);
    
    canMsg.u32MsgId = 0x100;
    canMsg.aU8Data[0] = (sysA.uptimeMs >> 24) & 0xFF;  // Extract the most significant byte
    canMsg.aU8Data[1] = (sysA.uptimeMs >> 16) & 0xFF;  // Extract the second most significant byte
    canMsg.aU8Data[2] = (sysA.uptimeMs >> 8) & 0xFF;   // Extract the second least significant byte
    canMsg.aU8Data[3] = sysA.uptimeMs & 0xFF;          // Extract the least significant byte
    drv_can_bTxMessage(&canMsg);
    
    canMsg.u32MsgId = 0x200;
    LoggerCan011_GsmA_t gsmA;
    logger_can_get(LoggerCan011_GsmA, &gsmA);
    strncpy(canMsg.aU8Data, gsmA.opratingName, sizeof(canMsg.aU8Data));
    drv_can_bTxMessage(&canMsg);
    
    canMsg.u32MsgId = 0x300;
    LoggerCan008_GpsA_t gpsA;
    logger_can_get(LoggerCan008_GpsA, &gpsA);
    uint32_t latCpy = gpsA.latitude;
    uint32_t longCpy = gpsA.longitude;
    
    // Copy latitude (32 bits) into data[0] to data[3]
    canMsg.aU8Data[0] = (latCpy >> 24) & 0xFF;  // Most significant byte
    canMsg.aU8Data[1] = (latCpy >> 16) & 0xFF;
    canMsg.aU8Data[2] = (latCpy >> 8) & 0xFF;
    canMsg.aU8Data[3] = latCpy & 0xFF;          // Least significant byte

    // Copy longitude (32 bits) into data[4] to data[7]
    canMsg.aU8Data[4] = (longCpy >> 24) & 0xFF; // Most significant byte
    canMsg.aU8Data[5] = (longCpy >> 16) & 0xFF;
    canMsg.aU8Data[6] = (longCpy >> 8) & 0xFF;
    canMsg.aU8Data[7] = longCpy & 0xFF;
    drv_can_bTxMessage(&canMsg);
    
    // Receive as many Rx messages as available.
    // while ( drv_can_bReadRecvMsg(&canMsg) )
    // {
    //     ++rxCount;
    // }

    // // Load the count into D[0]
    // canMsg.u32MsgId = 0xCC;
    // canMsg.u8Dlc = 4;
    // canMsg.aU8Data[0] = rxCount >> 24;
    // canMsg.aU8Data[1] = rxCount >> 16;
    // canMsg.aU8Data[2] = rxCount >> 8;
    // canMsg.aU8Data[3] = rxCount >> 0;
    // for ( idx = 0 ; idx < 20 ; ++idx  )
    // {
    //     canMsg.u32MsgId++;
    //     drv_can_bTxMessage(&canMsg);
    // }
}
