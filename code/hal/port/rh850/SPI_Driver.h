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
* File Name    : Config_CSIH0.h
* Version      : 1.2.0
* Device(s)    : R7F701647
* Description  : This file implements device driver for Config_CSIH0.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "rh850_csih.h"
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_PORT.h"

#ifndef CFG_Config_CSIH0_H
#define CFG_Config_CSIH0_H

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define _CSIH0_SELECT_BASIC_CLOCK     (0x0000U) /* Selects the basic clock */
#define _CSIH0_BAUD_RATE_0            (0x0002U) /* baudrate set */
#define _CSIH0_BAUD_RATE_1            (0x0002U) /* baudrate set */
#define _CSIH0_BAUD_RATE_2            (0x0002U) /* baudrate set */
#define _CSIH0_BAUD_RATE_3            (0x0002U) /* baudrate set */

#define SPI_CHN                        2        // SPI channel number to select required channel.

struct SPITbl_t 
{
	volatile struct __tag594 *CSIH;    // Accessing structure of SPI Registers.
    
    enum alt_t          AltFunMISO;    // Alternating function for MISO pin.
    enum port_t         PortNoMISO; 
    enum Pin_t          PortPinMISO;
    enum io_t           InOutMISO;
    
    enum alt_t          AltFunMOSI;    // Alternating function for MOSI pin.
    enum port_t         PortNoMOSI; 
    enum Pin_t          PortPinMOSI;    
    enum io_t           InOutMOSI;
    
    enum alt_t          AltFunCLK;     // Alternating function for CLK pin.
    enum port_t         PortNoCLK; 
    enum Pin_t          PortPinCLK;
    enum io_t           InOutCLK;
	
	enum port_t         CsPort;        // Making CS pin as Output
	enum Pin_t          CsPin;
	enum io_t           InOutCS;
	
	enum level_t        CsLvlLow;      // Making CS pin as Low & High
	enum level_t        CsLvlHigh;
};

static const struct SPITbl_t SPITblList[]=
{
	{(volatile struct __tag594 *)&CSIH0,Alt4 , Port0, Pin_1,  INPUT, Alt4, Port0, Pin_3,  OUTPUT, Alt4 , Port0, Pin_2, OUTPUT, Port0,Pin_0,OUTPUT,Low,High},// Not Used in Initial Schematic
	
	{(volatile struct __tag594 *)&CSIH1,Alt3 , Port0, Pin_4,  INPUT, Alt3, Port0, Pin_5,  OUTPUT, Alt3 , Port0, Pin_6, OUTPUT, Port0,Pin_11,OUTPUT,Low,High},
	
	{(volatile struct __tag594 *)&CSIH2,Alt1 , Port11, Pin_4,  INPUT, Alt1, Port11, Pin_2,  OUTPUT, Alt1 , Port11, Pin_3, OUTPUT, Port11,Pin_1,OUTPUT,Low,High},
	
	{(volatile struct __tag594 *)&CSIH3,Alt3 , Port11, Pin_5,  INPUT, Alt3, Port11, Pin_6,  OUTPUT, Alt3 , Port11, Pin_7, OUTPUT, Port10,Pin_0,OUTPUT,Low,High}
};

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void SPI_Create(uint8_t);
void SPI_Start(uint8_t);
void SPI_Stop(uint8_t);
void SPI_Transmit(uint8_t SPI_Ch,uint8_t* tx_buf, uint16_t tx_num);
void SPI_Receive(uint8_t SPI_Ch,uint8_t* rx_buf, uint16_t rx_num);

/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif
