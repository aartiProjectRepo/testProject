/**
 * @file        hal_can.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        27 November 2023
 * @author      Tejas Kamod
 *
 * @brief       CAN Hardware Abstration Layer Header file.
 */

#ifndef __HAL_CAN_H__
#define __HAL_CAN_H__

#include "AEPL_types.h"

typedef struct 
{
    U32 u32MsgId;
    U8 u8Dlc;
    U8 aU8Data[8];
}CAN_MSG_T;

//:Need TBD
struct CANFD_MSG_T
{
    U32 u32MsgId;
    U8 u8Dlc;
    U8 aU8Data[64];
};

typedef enum 
{
    eCAN1 = 0,
    eCAN2,
    eCAN_MAX
}CAN_MCU_MODULE_T;

typedef enum 
{
    eCAN_BAUD_125K = 0,
    eCAN_BAUD_250K,
    eCAN_BAUD_500K,
    eCAN_BAUD_1M,
    eCAN_BAUD_4M,
    eCAN_BAUD_MAX,
}CAN_BAUD_RATE_T;

typedef struct 
{
    BOOL_T bBusOff;
    U8 u8TxErr;
    U8 u8RxErr;
}CAN_ERR_FLG_T;

typedef struct
{
    U32 u32CanID;
    U32 u32MaskValue;
}CANID_MASKVALUE_T;

/**
 * @brief                   Function to Initialization CAN Module.
 * @param      eModuleNo    CAN Module Number CAN1 or.
 * @param      eBaudRate    CAN Baudrate.
 * @param      nCanFdEn     CAN FD Enable bit.
 * @return                  TRUE:               Success.
 *                          FALSE:              If intialization error.
*/
extern BOOL_T hal_can_bInit (CAN_MCU_MODULE_T eModuleNo, CAN_BAUD_RATE_T eBaudRate, BOOL_T nCanFdEn);

/**
 * @brief                   Function to Set CAN Module baudrate.
 * @param      eBaudRate    CAN Baudrate.
 * @return                  TRUE:               Success.
 *                          FALSE:              If baudrate error.
*/
extern BOOL_T hal_can_bSetBaudRate (CAN_BAUD_RATE_T eBaudRate);

/**
 * @brief                   Function to Set CAN Module baudrate.
 * @param      sCanMsg      Pointer to Tx message
 * @return                  TRUE:               If Tx Success.
 *                          FALSE:              If Tx error.
*/
extern BOOL_T hal_can_bTxMsg (const CAN_MSG_T* sCanMsg);

/**
 * @brief                   Function to Set CAN Module baudrate.
 * @param      sCanMsg      Pointer to Rx message
 * @return                  TRUE:               If Rx Success.
 *                          FALSE:              If Rx error.
*/
extern BOOL_T hal_can_bRxMsg (CAN_MSG_T* sCanMsg);

/**
 * @brief                   Function to Get CAN Module Error state.
 * @param      sCANErr      Pointer to CAN_ERR_FLG_T
*/
extern void hal_can_vGetError (CAN_ERR_FLG_T sCANErr);

/**
 * @brief                   Function to Configure Rx filter
*/
extern BOOL_T hal_can_bConfRxFilter (void); 

#endif