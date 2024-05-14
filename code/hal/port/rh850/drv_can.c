/**
 * @file        drv_can.c
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
 * @brief       CAN Driver Source file.
 */

 //Driver interface includes.
#include "drv_can.h"

#define READ_RX_COUNT           ( 8 )
static struct DRV_CAN_RX_BUFF_T{
    CAN_MSG_T asRxBuff[DRV_CAN_RX_BUFF_SIZE_D];     //Receive Buffer
    U16 u16BufWrPtr;                                //Receive Buffer Write pointer
    U16 u16BufRdPtr;                                //Receive Buffer Read pointer
}drv_can_sRxBuff;

static struct DRV_CAN_TX_FIFO_T{
    CAN_MSG_T asTxFifo[DRV_CAN_TX_FIFO_SIZE_D];      //Transmit Buffer
    U16 u16FifoWrPtr;                                //Transmit Buffer Write pointer
    U16 u16FifoRdPtr;                                //Transmit Buffer Read pointer
}drv_can_sTxFifo;

typedef enum 
{
    STATE_INIT,
    STATE_ACTIVE_OPERATION,
    STATE_ERR_HANDLING,
    STATE_SHUTDOWN
}drv_can_state_n;

static CANID_MASKVALUE_T g_CanIdMaskValDetails[] =
{
    { 0x30E, 0x00000FFE },                          //will allow ID 0x30E and 0x30F
    { 0x310, 0x00000F00 },                          //will allow ID 0x310, 0x31C, 0x31D and 0x361
    { 0x600, 0x00000F00 },                          //will allow ID 0x600, 0x601, 0x603
    { 0x09A, 0x00000088 }                           //will allow ID 0x09A, 0x0AD, 0x0AE
};

#define CANID_MASKVAL_TABLE_SIZE    (sizeof(g_CanIdMaskValDetails)/sizeof(g_CanIdMaskValDetails[0]))

/**
 *  @brief         CAN Driver Transmit handler.
*/
static DRV_CAN_BUFF_STAT_T drv_can_bTxHandler(void);

/**
 *  @brief         CAN Driver Transmit handler.
*/
static DRV_CAN_BUFF_STAT_T drv_can_bRxHandler(void);

static U8 g_taskState = STATE_INIT;

BOOL_T drv_can_bInit(void)
{
    BOOL_T bStat = FALSE;

    //Intialise Tx & Rx FIFO & Buffer 
    drv_can_sTxFifo.u16FifoRdPtr = 0;
    drv_can_sTxFifo.u16FifoWrPtr = 0;

    drv_can_sRxBuff.u16BufRdPtr = 0;
    drv_can_sRxBuff.u16BufWrPtr = 0;

    //TODO: Need to send mask table poninter for filter and masking.
    if ( hal_can_bInit(eCAN1, eCAN_BAUD_500K, FALSE) )
    {
        //Init Success
        bStat = TRUE;
        g_taskState = STATE_ACTIVE_OPERATION;
    }
    return bStat;
}

DRV_CAN_BUFF_STAT_T drv_can_bTxMessage(CAN_MSG_T* psMsg)
{
    volatile U8 u8LpCnt = 0;
    volatile U8 u8WriteDataPtr = 0;
    DRV_CAN_BUFF_STAT_T bStat = eBUFF_ERR;
    u8WriteDataPtr = drv_can_sTxFifo.u16FifoWrPtr + 1;

    //Check if u8WriteDataPtr reaches to max
    if(u8WriteDataPtr >= DRV_CAN_TX_FIFO_SIZE_D)
    {
        u8WriteDataPtr = 0;
    }
    //check for buffer full
    if(u8WriteDataPtr == drv_can_sTxFifo.u16FifoRdPtr)
    {
        //buffer is full
        bStat = eBUFF_ERR;
    }
    else
    {
        //copy the msg into TxFiFo
        drv_can_sTxFifo.asTxFifo[drv_can_sTxFifo.u16FifoWrPtr].u32MsgId = psMsg->u32MsgId;
        drv_can_sTxFifo.asTxFifo[drv_can_sTxFifo.u16FifoWrPtr].u8Dlc = psMsg->u8Dlc;

        //copy the data into TxFiFo
        for(u8LpCnt = 0; u8LpCnt < (psMsg->u8Dlc) ; u8LpCnt++)
        {
            drv_can_sTxFifo.asTxFifo[drv_can_sTxFifo.u16FifoWrPtr].aU8Data[u8LpCnt] = psMsg->aU8Data[u8LpCnt];
        }

        drv_can_sTxFifo.u16FifoWrPtr = u8WriteDataPtr;
        bStat =  eBUFF_SUCCESS;
    }
    
    return bStat;
}

DRV_CAN_RX_BUFF_STAT_T drv_can_bReadRecvMsg(CAN_MSG_T *pRxMsg)
{
    volatile U8 u8LpCnt = 0;
    volatile U16 u16ReadDataPtr  = 0;
    DRV_CAN_RX_BUFF_STAT_T bStat = eBUFF_EMPTY;
    u16ReadDataPtr = drv_can_sRxBuff.u16BufRdPtr  + 1;
    
    //Check if u16ReadDataPtr reaches to max
    if ( u16ReadDataPtr >= DRV_CAN_RX_BUFF_SIZE_D )
    {
        u16ReadDataPtr = 0;
    }
    //check for buffer empty
    if ( drv_can_sRxBuff.u16BufRdPtr == drv_can_sRxBuff.u16BufWrPtr )
    {
        //buffer Empty
        bStat = eBUFF_EMPTY;
    }
    else
    {
        //Copy the Message from Rx msg buffer into software buffer
        pRxMsg->u32MsgId = drv_can_sRxBuff.asRxBuff[drv_can_sRxBuff.u16BufRdPtr].u32MsgId;
        pRxMsg->u8Dlc = drv_can_sRxBuff.asRxBuff[drv_can_sRxBuff.u16BufRdPtr].u8Dlc;
        
        //Copy the Data from Rx msg buffer into software buffer
        for ( u8LpCnt = 0 ; u8LpCnt < (drv_can_sRxBuff.asRxBuff[drv_can_sRxBuff.u16BufRdPtr].u8Dlc) ; u8LpCnt++ )
        {
            pRxMsg->aU8Data[u8LpCnt] = drv_can_sRxBuff.asRxBuff[drv_can_sRxBuff.u16BufRdPtr].aU8Data[u8LpCnt];
        }
        drv_can_sRxBuff.u16BufRdPtr = u16ReadDataPtr;
        bStat = eBUFF_READ_SUCCESS;
    }
    return bStat;
}

static DRV_CAN_BUFF_STAT_T drv_can_bTxHandler(void)
{ 
    volatile U16 u16TxDataPtr = 0;
    DRV_CAN_BUFF_STAT_T bStat = eBUFF_ERR;
    u16TxDataPtr = drv_can_sTxFifo.u16FifoRdPtr + 1;
    
    //Check if msg present in TxFiFo
    if ( drv_can_sTxFifo.u16FifoRdPtr != drv_can_sTxFifo.u16FifoWrPtr )
    {
        //Check if u16TxDataPtr reaches to max
        if ( u16TxDataPtr >= DRV_CAN_TX_FIFO_SIZE_D )
        {
            u16TxDataPtr = 0;
        }

        //Send Message to Transmit
        if ( hal_can_bTxMsg((CAN_MSG_T*)&(drv_can_sTxFifo.asTxFifo[drv_can_sTxFifo.u16FifoRdPtr])) )
        {
            drv_can_sTxFifo.u16FifoRdPtr = u16TxDataPtr;
            bStat = eBUFF_SUCCESS;
        }
    }

    return bStat;
}

static DRV_CAN_BUFF_STAT_T drv_can_bRxHandler(void)
{
    volatile U16 u16RxDataPtr = 0;
    DRV_CAN_BUFF_STAT_T bStat = eBUFF_ERR;
    u16RxDataPtr = drv_can_sRxBuff.u16BufWrPtr + 1;

    //Check if u16RxDataPtr reaches to max
    if(u16RxDataPtr >= DRV_CAN_RX_BUFF_SIZE_D)
    {
        u16RxDataPtr = 0;
    }
    //check for buffer full
    if(u16RxDataPtr == drv_can_sRxBuff.u16BufRdPtr)
    {
        //buffer full
        bStat = eBUFF_ERR;
    }
    else
    {
        //Store Received Message in RxBuff
        if(hal_can_bRxMsg((CAN_MSG_T*)&(drv_can_sRxBuff.asRxBuff[drv_can_sRxBuff.u16BufWrPtr])))
        {
            drv_can_sRxBuff.u16BufWrPtr = u16RxDataPtr;
            bStat = eBUFF_SUCCESS;
        }
    }
    return bStat;
}

void drv_can_bPeriodicTask(void)
{
    switch ( g_taskState )
    {
        case STATE_INIT:
        {
            if ( drv_can_bInit() )
            {
                g_taskState = STATE_ACTIVE_OPERATION;
            }
        }
        break;
        
        case STATE_ACTIVE_OPERATION:
        {
            if ( 0 )    //TODO: if error occurs change state to STATE_ERR_HANDLING else do normal operations
            {
                g_taskState = STATE_ERR_HANDLING;
            }
            else
            {
                //FIXME: Return code not handled
                drv_can_bTxHandler();
                //TODO:Read till Message available in Rx FIFO
                for ( U8 u8i = 0 ; u8i < READ_RX_COUNT ; u8i++ )
                {
                    if( eBUFF_ERR == drv_can_bRxHandler() )
                    {
                        break;
                    }
                }
            }
        }
        break;

        case STATE_ERR_HANDLING:
        {
            // Handle Bus-off here.
        }
        break;

        case STATE_SHUTDOWN:
        {
            // Call CAN De-init function here.
        }
        break;
        
        default:
        {
        }
        break;
    }
}
