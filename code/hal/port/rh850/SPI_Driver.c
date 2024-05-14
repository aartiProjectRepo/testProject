/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : Config_CSIH0.c
* Version      : 1.2.0
* Device(s)    : R7F701647
* Description  : This file implements device driver for Config_CSIH0.
***********************************************************************************************************************/
/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "SPI_Driver.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/* Note: Array of structure removed from here */


/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint32_t g_cg_sync_read;

/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: SPI_Create
* Description  : This function Initializes the SPI module.
* Arguments    : SPI Channel Number.
* Return Value : None
***********************************************************************************************************************/
void SPI_Create(uint8_t SPI_Ch)
{
	  SPITblList[SPI_Ch].CSIH->CTL0 = _CSIH_OPERATION_CLOCK_STOP;  //Stop SPI operation clock
	 
	  SPITblList[SPI_Ch].CSIH->CTL1 = _CSIH_CLOCK_INVERTING_HIGH | _CSIH_INTERRUPT_TIMING_NORMAL | _CSIH_DATA_CONSISTENCY_CHECK_DISABLE | 
      _CSIH_NO_DELAY | _CSIH_HANDSHAKE_DISABLE | _CSIH_CHIPSELECT_SIGNAL_HOLD_ACTIVE | 
      _CSIH_SLAVE_SELECT_DISABLE;
		 
	  SPITblList[SPI_Ch].CSIH->CTL2 =_CSIH0_SELECT_BASIC_CLOCK;  //clock 
	  SPITblList[SPI_Ch].CSIH->BRS0 = _CSIH0_BAUD_RATE_0;
	  SPITblList[SPI_Ch].CSIH->BRS1 = _CSIH0_BAUD_RATE_1;
	  SPITblList[SPI_Ch].CSIH->BRS2 = _CSIH0_BAUD_RATE_2;
	  SPITblList[SPI_Ch].CSIH->BRS3 = _CSIH0_BAUD_RATE_3;   // Baud Rate setting
	  
	  SPITblList[SPI_Ch].CSIH->CFG0 = _CSIH0_BAUD_RATE_0 | _CSIH_PARITY_NO | _CSIH_DATA_LENGTH_8 | _CSIH_DATA_DIRECTION_MSB | 
      _CSIH_PHASE_SELECTION_TYPE1 | _CSIH_IDLE_INSERTED_NOT_ALWAYS | _CSIH_IDLE_TIME_0 | 
      _CSIH_HOLD_TIME_0 | _CSIH_INTER_DATA_DELAY_TIME_0 | _CSIH_SETUP_TIME_0;   // SPI configuration like 8 bit data Transfer,MSB first.
	  
	  g_cg_sync_read=SPITblList[SPI_Ch].CSIH->CTL1;
	   __syncp();
	  /* Port Pin Configuration */   // Alternative Port Pin Function Selection.
	  R_PORT_SetAltFunc(SPITblList[SPI_Ch].PortNoMISO, SPITblList[SPI_Ch].PortPinMISO, SPITblList[SPI_Ch].AltFunMISO,SPITblList[SPI_Ch].InOutMISO);
      R_PORT_SetAltFunc(SPITblList[SPI_Ch].PortNoMOSI, SPITblList[SPI_Ch].PortPinMOSI, SPITblList[SPI_Ch].AltFunMOSI,SPITblList[SPI_Ch].InOutMOSI); 
	  R_PORT_SetAltFunc(SPITblList[SPI_Ch].PortNoCLK, SPITblList[SPI_Ch].PortPinCLK, SPITblList[SPI_Ch].AltFunCLK,SPITblList[SPI_Ch].InOutCLK);
	  R_PORT_SetGpioOutput(SPITblList[SPI_Ch].CsPort, SPITblList[SPI_Ch].CsPin, SPITblList[SPI_Ch].InOutCS);
}
/***********************************************************************************************************************
* Function Name: SPI_Start
* Description  : This function starts the SPI module operation.
* Arguments    : SPI Channel Number.
* Return Value : None
***********************************************************************************************************************/
void SPI_Start(uint8_t SPI_Ch)
{
     SPITblList[SPI_Ch].CSIH->CTL0 = _CSIH_OPERATION_CLOCK_PROVIDE | _CSIH_TRANSMISSION_PERMIT | _CSIH_RECEPTION_PERMIT | _CSIH_DIRECTACCESS;  // Config CTLO for enabling Transmit mode,Receive mode & Direct Access memory mode.
}

/***********************************************************************************************************************
* Function Name: SPI_Stop
* Description  : This function stops the SPI module operation.
* Arguments    : SPI Channel Number.
* Return Value : None
***********************************************************************************************************************/
void SPI_Stop(uint8_t SPI_Ch)
{
    SPITblList[SPI_Ch].CSIH->CTL0 &= (uint8_t) ~_CSIH_RECEPTION_PERMIT;     // Disable Reception
    SPITblList[SPI_Ch].CSIH->CTL0 &= (uint8_t) ~_CSIH_TRANSMISSION_PERMIT;   // Disable Transmission
    SPITblList[SPI_Ch].CSIH->CTL0 &= (uint8_t) ~_CSIH_OPERATION_CLOCK_PROVIDE;  // Disable clock.
    /* Synchronization processing */
    g_cg_sync_read = SPITblList[SPI_Ch].CSIH->CTL0;
    __syncp();
    g_cg_sync_read = INTC1.ICCSIH0IC.UINT16;
    __syncp();
}
/***********************************************************************************************************************
* Function Name: SPI_Transmit
* Description  : This function sends SPI data.
* Arguments    : 
                     SPI Channel Number-
                     tx_buf -
*                    send buffer pointer
*                    tx_num -
*                    buffer size
* Return Value : status -
*                    MD_OK or MD_ARGERROR
***********************************************************************************************************************/
void SPI_Transmit(uint8_t SPI_Ch,uint8_t* tx_buf, uint16_t tx_num)
{
    uint8_t * txPtr = tx_buf;  
    uint16_t k;
	uint32_t stsReg;
	for(k=0;k<tx_num;k++)
	    {
			SPITblList[SPI_Ch].CSIH->STCR0 = 0x0100;    
			
			SPITblList[SPI_Ch].CSIH->TX0H = (*txPtr++); // Data is Transmitted through Transmit word register.
			stsReg = SPITblList[SPI_Ch].CSIH->STR0;	
			while(stsReg & 0x00000080)        // Check for the Transfer Status Flag bit
			{
				stsReg = SPITblList[SPI_Ch].CSIH->STR0;	
			}
	    }
}

/***********************************************************************************************************************
* Function Name: SPI_Receive
* Description  : This function  receives CSIH0 data.
* Arguments    :
                     SPI Channel Number-
*                    rx_buf -
*                    receive buffer pointer
                     rx_num -
*                    buffer size
* Return Value : status -
*                    MD_OK or MD_ARGERROR
***********************************************************************************************************************/
void SPI_Receive(uint8_t SPI_Ch,uint8_t* rx_buf, uint16_t rx_num)
{
    uint16_t k;
	uint32_t sts_reg;
    uint8_t *rxPtr = rx_buf;
	for(k=0;k<rx_num;k++)
    	{
			
	        if (0U != rxPtr)
            {
				SPITblList[SPI_Ch].CSIH->STCR0 = 0x0100;
			   	SPITblList[SPI_Ch].CSIH->TX0H = 0x00;
					   
				sts_reg = SPITblList[SPI_Ch].CSIH->STR0;	
				while(sts_reg & 0x00000080)       // Check for the Transfer Status Flag bit
				{
					sts_reg = SPITblList[SPI_Ch].CSIH->STR0;
				}
				*(rxPtr) = (uint8_t)SPITblList[SPI_Ch].CSIH->RX0H;  // Data from SPI Register(RX0H) taken into local read buffer.
				rxPtr++; 	
	        }
	    }

    	
}

/* End user code. Do not edit comment generated here */
