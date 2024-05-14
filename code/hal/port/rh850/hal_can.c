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
 * @brief       CAN Hardware Abstration Layer Source file.
 */
 
#include "hal_can.h"
#include "r_cg_userdefine.h"

//Demo code includes.
#include "r_cg_macrodriver.h"

BOOL_T hal_can_bInit (CAN_MCU_MODULE_T eModuleNo, CAN_BAUD_RATE_T eBaudRate, BOOL_T nCanFdEn)
{
    volatile BOOL_T bStatus = TRUE;

    //CAN clock Settings
    //CAN Clock devider = /1
    my_protected_write(WPROTR.PROTCMD1,CLKCTL.CKSC_ICANOSCD_CTL,0x01);
    /*todo: Add time bound while */  //:Need TBD
    while(CLKCTL.CKSC_ICANOSCD_ACT!=0x01);

    //PLL -> CAN Clock
    my_protected_write(WPROTR.PROTCMD1,CLKCTL.CKSC_ICANS_CTL,0x01);
    while(CLKCTL.CKSC_ICANS_ACT!=0x01);

    //Global Status Register
    while((RCFDC0.CFDGSTS.UINT8[LL] && 0x04)==0x04);    //Wait until CAN RAM is initialized, global stop status flag

    //Global Control Register 
    RCFDC0.CFDGCTR.UINT8[LL] &= 0xFB;                   //Transition to global reset mode from global stop mode
                                                        //GSLPR-->0 other than stop mode
                                                        //GSLPR-->1 stop mode

    //Channel Control Register
    RCFDC0.CFDC0CTR.UINT8[LL] &= 0xFB;                  //Transition to channel reset mode from channel stop mode
                                                        //CSLPR-->0 other than stop mode
                                                        //CSLPR-->1 stop mode

    //Configure Baudrate        //:Need TBD
    (void)hal_can_bSetBaudRate(eBaudRate);

    //Configure Rx Filter      //:Need TBD
    hal_can_bConfRxFilter();

    // Configure Tx MailBox.    //:Need TBD

    //Global Interrupt Register
   RCFDC0.CFDGCTR.UINT8[LH] = 0x07;                     //Transmit history interrupt, FIFO msg lost interrupt & DLC error interrupt enabled

    //Operating Mode
    RCFDC0.CFDGCTR.UINT8[0] = 0x00u;                    //Other than global stop mode
                                                        //Global operating mode
    for(U8 u8i = 0; u8i<0xff; u8i++);                   //wait for transistion

    //Set RFE bit in global operating mode 
    RCFDC0.CFDRFCC0.UINT8[0] |= 0x01;                   //Receive FIFO is used

    RCFDC0.CFDC0CTR.UINT8[0] = 0x00;                    //Other than channel stop mode
                                                        //Channel communication mode
    for(U8 u8i = 0; u8i<0xff; u8i++);                   //Wait for transistion

    return bStatus;
}

BOOL_T hal_can_bSetBaudRate (CAN_BAUD_RATE_T eBaudRate)
{
    volatile BOOL_T bStatus = TRUE;

    RCFDC0.CFDGCFG.UINT32 = 0x10;                       //Timestamp, clock selection(clkc-->40MHz), Priority on ID.
    RCFDC0.CFDC0FDCFG.UINT8[HH] = 0x40;                 //Classical CAN only mode is enabled.

    switch(eBaudRate)
    {
        case eCAN_BAUD_125K:
        {
            RCFDC0.CFDC0NCFG.UINT32 = 0x02030001;       //125kbps, 8Tq, 1,2,5, 75% sampling.
        }
        break;

        case eCAN_BAUD_250K:
        {
            RCFDC0.CFDC0NCFG.UINT32 = 0x00230003;       // 250kbps, 8Tq, 1,2,5, 75% sampling.
        }
        break;

        case eCAN_BAUD_500K:
        {
            RCFDC0.CFDC0NCFG.UINT32 = 0x02030001;       //500kbps, 8Tq, 1,4,3, 62.5% sampling.
        }
        break;

        case eCAN_BAUD_1M:
        {
            RCFDC0.CFDC0NCFG.UINT32 = 0x02030000;       //1Mbps, 8Tq, 1,4,3, 62.5% sampling.
        }
        break;

        case eCAN_BAUD_4M:
        default:
        {
            RCFDC0.CFDC0NCFG.UINT32 = 0x00140004;       //4Mbps, 8Tq, 1,2,5, 75% sampling.
            bStatus = FALSE;
        }
    }

    return bStatus;
}

BOOL_T hal_can_bTxMsg (const CAN_MSG_T* sCanMsg)
{
    volatile BOOL_T bStat = FALSE;
    U32 u32DataL = 0;
    U32 u32DataH = 0;

    if((RCFDC0.CFDTMSTS0 & 0x01) == 0x00)               //Check if no Tx request is pending
    {
        RCFDC0.CFDTMIEC0.UINT8[0] = 0x01;               //Transmit Buffer 0 interrupt enabled
        RCFDC0.CFDTMID0.UINT32 = sCanMsg->u32MsgId;     //Transmit buffer ID register

        //Transmit buffer pointer register
        RCFDC0.CFDTMPTR0.UINT16[1] = 0x8012;            //8 data bytes, label value as 0x12
        RCFDC0.CFDTMFDCTR0.UINT8[LL] = 0x00;            //TO CONFIGURE FRAME MODE FOR Classic CAN 

        for (U8 u8i = 0; u8i < sCanMsg->u8Dlc; u8i++)
        {
            if(u8i <= 3)
            {
                u32DataL |= ((U32)sCanMsg->aU8Data[u8i] << (u8i * 8));
                RCFDC0.CFDTMDF0_0.UINT32 = u32DataL;
            }
            else
            {
                u32DataH |= ((U32)sCanMsg->aU8Data[u8i] << (u8i * 8));
                RCFDC0.CFDTMDF1_0.UINT32 = u32DataH; 
            }
        }
        RCFDC0.CFDTMSTS0 = 0x00;
        RCFDC0.CFDTMC0 = 0x01;                          //Request to transmit, Set TMTR bit
        bStat = TRUE;
    }

    return bStat;
}

BOOL_T hal_can_bRxMsg (CAN_MSG_T* sCanMsg)
{
    volatile BOOL_T bStat = FALSE;

    RCFDC0.CFDRFSTS0.UINT8[0] &= 0xF7;                  //CLEAR RFIF (Receive FIFO Interrupt Request) Flag

    if((RCFDC0.CFDRFSTS0.UINT8[0] & 0x01) != 0x01)      //Receive FIFO Buffer Status Register, Check RFEMP Flag (Receive FIFO Buffer Empty Status Flag)
    {
        bStat = TRUE;

        sCanMsg->u32MsgId = RCFDC0.CFDRFID0.UINT16[0];              //Receive FIFO Buffer Access ID Register
        sCanMsg->u8Dlc = (RCFDC0.CFDRFPTR0.UINT8[3] & 0xF0) >> 4;   //Receive FIFO Buffer Access Pointer Register

        sCanMsg->aU8Data[0] = (U8)(RCFDC0.CFDRFDF0_0.UINT32 & 0x000000FF);
        sCanMsg->aU8Data[1] = (U8)((RCFDC0.CFDRFDF0_0.UINT32 & 0x0000FF00) >> 8);
        sCanMsg->aU8Data[2] = (U8)((RCFDC0.CFDRFDF0_0.UINT32 & 0x00FF0000) >> 16);
        sCanMsg->aU8Data[3] = (U8)((RCFDC0.CFDRFDF0_0.UINT32 & 0xFF000000) >> 24);
        sCanMsg->aU8Data[4] = (U8)(RCFDC0.CFDRFDF1_0.UINT32 & 0x000000FF);
        sCanMsg->aU8Data[5] = (U8)((RCFDC0.CFDRFDF1_0.UINT32 & 0x0000FF00) >> 8);
        sCanMsg->aU8Data[6] = (U8)((RCFDC0.CFDRFDF1_0.UINT32 & 0x00FF0000) >> 16);
        sCanMsg->aU8Data[7] = (U8)((RCFDC0.CFDRFDF1_0.UINT32 & 0xFF000000) >> 24);

        RCFDC0.CFDRFPCTR0.UINT8[0] = 0xFF;                          //Receive FIFO Buffer Pointer Control Register
                                                                    //(When these bits are set to FFH, the read pointer moves to the next
                                                                    //unread message in the receive FIFO buffer)
    }

    return bStat;
}

void hal_can_vGetError (CAN_ERR_FLG_T sCANErr)
{
    //can_error_status_s status = candrv_Chnl_GetErrorStatus(CAN0_CHANNEL_0);
}

BOOL_T hal_can_bConfRxFilter (void)
{
    // RX FIFO
    RCFDC0.CFDGAFLCFG0.UINT8[HH] = 0x04;        //No. of rules for channel 0
    RCFDC0.CFDGAFLECTR.UINT8[LH] = 0x01;        //Enable write to receive rule table
    RCFDC0.CFDGAFLECTR.UINT8[LL] = 0x00;        //Receive rule page no.configuration

    //Receive rule 1
    //Receive Rule ID Register
    RCFDC0.CFDGAFLID0.UINT16[L] = 0x030E;       //Standard, Data frame, 11 bit ID

    //Receive Rule Mask Register
    //RCFDC0.CFDGAFLM0.UINT32 = 0x00000FFE;       //ID bits are compared
    RCFDC0.CFDGAFLM0.UINT32 = 0x00000000;       //ID bits are compared

    //Receive Rule Pointer 0 Register
    RCFDC0.CFDGAFLP0_0.UINT8[LH] = 0x80;        //Use message buffer no. 0
    RCFDC0.CFDGAFLP0_0.UINT8[LL] = 0x08;        //DLC 8bytes

    //Receive Rule Pointer 1 Register
    RCFDC0.CFDGAFLP1_0.UINT32 = 0x01;           //Rx FIFO 0 selected

    //Receive rule 2
    //Receive Rule ID Register
    RCFDC0.CFDGAFLID1.UINT16[0] = 0x0310;       //Standard, Data frame, 11 bit ID

    //Receive Rule Mask Register
    RCFDC0.CFDGAFLM1.UINT32 = 0x00000F00;       //ID bits are compared

    //Receive Rule Pointer 0 Register
    RCFDC0.CFDGAFLP0_1.UINT8[LH] = 0x80;        //Use message buffer no. 0
    RCFDC0.CFDGAFLP0_1.UINT8[LL] = 0x08;        //DLC 8bytes

    //Receive Rule Pointer 1 Register
    RCFDC0.CFDGAFLP1_1.UINT32 = 0x01;           //Receive FIFO 0 selected

    //Receive Rule Entry Control Register
    RCFDC0.CFDGAFLECTR.UINT8[LH] = 0x00;        //Disable write to receive rule table
    RCFDC0.CFDRFCC0.UINT16[L] = 0x1271;         // RFIGCV = xxx(don't care),RFIM = 0(interrupt not occurs each time a message has been received.)
                                                // RFDC =0x010 Receive FIFO Buffer Depth Configuration 
                                                // RFPLS = 0x111 Receive FIFO Buffer Payload Storage Size Select
                                                // RFIE =1 Receive FIFO Interrupt Enable

    return TRUE;

}

//**************************************************************************************************************************

#pragma interrupt CAN_Global_Error_ISR(enable=false, channel=22, fpu=true, callt=false)
 void CAN_Global_Error_ISR(void)
{
     if(RCFDC0.CFDRFSTS0.UINT8[0] & 0x04)   //Check if FIFO message is lost
     {
         //:Need TBD
//        RX_Fifo_msg_lost = 1;
        //return;
     }
}

//**************************************************************************************************************************

#pragma interrupt CAN0_Error_ISR(enable=false, channel=24, fpu=true, callt=false)
 void CAN0_Error_ISR(void)
{
     __nop();
}
