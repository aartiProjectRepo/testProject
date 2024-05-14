/**
 * @file        drv_can.h
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
                Swati Sinare <swati.sinare@accoladeelectronics.com>
 *
 * @brief       CAN Driver Header file.
 */

#ifndef __DRV_CAN_H__
#define __DRV_CAN_H__

#include "AEPL_types.h"
#include "hal_can.h"

#define DRV_CAN_TX_FIFO_SIZE_D          (80)                    //Transmit FIFO Buffer Size
#define DRV_CAN_RX_BUFF_SIZE_D          (80)                    //Receive FIFO Buffer Size
#define DRV_CAN_TASK_RATE_MSEC_D        (1)                     //Transmit MSG Rate
#define DRV_CAN_RX_MSG_RATE_MSEC_D      (10)                    //Receive MSG Rate
#define DRV_CAN_MSG_FILT_NO_D           (10)                    //No of filters to be applied

typedef enum
{
    CAN_BUS_NORMAL,
    CAN_BUS_OFF
}DRV_CAN_BUS_STAT_T;

typedef enum 
{
    eBUFF_ERR,
    eBUFF_SUCCESS
}DRV_CAN_BUFF_STAT_T;

typedef enum
{
    eBUFF_EMPTY,
    eBUFF_READ_SUCCESS
}DRV_CAN_RX_BUFF_STAT_T;

/**
 * @brief                   Function to CAN Driver Initialization.
 * @return                  TRUE:               Success.
 *                          FALSE:              If intialization error.
*/
extern BOOL_T drv_can_bInit(void);

/**
 * @brief                   CAN Driver Transmit Message function copies data from application buffer to TxFIFO.
 * @param      psMsg        A pointer to CAN_MSG_T
 * @return                  eBUFF_SUCCESS:              Buffer write Success.
 *                          eBUFF_ERR:                  If TxFIFO is full.
*/
extern DRV_CAN_BUFF_STAT_T drv_can_bTxMessage(CAN_MSG_T* psMsg);

/**
 * @brief                   CAN Driver Receive Message function copies data from Rx Message to application buffer.
 * @param      pRxMsg       A pointer to CAN_MSG_T
 * @return                  eBUFF_READ_SUCCESS:           Buffer Read Success.
 *                          eBUFF_EMPTY:                  If Rx Buffer is Empty, no message to read.
*/
extern DRV_CAN_RX_BUFF_STAT_T drv_can_bReadRecvMsg(CAN_MSG_T *pRxMsg);

/**
 * @brief                   CAN Driver Periodic function.
*/
extern void drv_can_bPeriodicTask(void);

#endif