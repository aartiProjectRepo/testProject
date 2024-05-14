/*===========================================================================*/
/* Project:  F1x StarterKit V3 Sample Software                               */
/* Module :  r_port.c                                                        */
/* Version:  V1.00                                                           */
/*===========================================================================*/
/*                                  COPYRIGHT                                */
/*===========================================================================*/
/* Copyright (c) 2016 by Renesas Electronics Europe GmbH,                    */
/*               a company of the Renesas Electronics Corporation            */
/*===========================================================================*/
/* In case of any question please do not hesitate to contact:                */
/*                                                                           */
/*        ABG Software Tool Support                                          */
/*                                                                           */
/*        Renesas Electronics Europe GmbH                                    */
/*        Arcadiastrasse 10                                                  */
/*        D-40472 Duesseldorf, Germany                                       */
/*                                                                           */
/*        e-mail: software_support-eu@lm.renesas.com                         */
/*        FAX:   +49 - (0)211 / 65 03 - 11 31                                */
/*                                                                           */
/*===========================================================================*/
/* Warranty Disclaimer                                                       */
/*                                                                           */
/* Because the Product(s) is licensed free of charge, there is no warranty   */
/* of any kind whatsoever and expressly disclaimed and excluded by Renesas,  */
/* either expressed or implied, including but not limited to those for       */
/* non-infringement of intellectual property, merchantability and/or         */
/* fitness for the particular purpose.                                       */
/* Renesas shall not have any obligation to maintain, service or provide bug */
/* fixes for the supplied Product(s) and/or the Application.                 */
/*                                                                           */
/* Each User is solely responsible for determining the appropriateness of    */
/* using the Product(s) and assumes all risks associated with its exercise   */
/* of rights under this Agreement, including, but not limited to the risks   */
/* and costs of program errors, compliance with applicable laws, damage to   */
/* or loss of data, programs or equipment, and unavailability or             */
/* interruption of operations.                                               */
/*                                                                           */
/* Limitation of Liability                                                   */
/*                                                                           */
/* In no event shall Renesas be liable to the User for any incidental,       */
/* consequential, indirect, or punitive damage (including but not limited    */
/* to lost profits) regardless of whether such liability is based on breach  */
/* of contract, tort, strict liability, breach of warranties, failure of     */
/* essential purpose or otherwise and even if advised of the possibility of  */
/* such damages. Renesas shall not be liable for any services or products    */
/* provided by third party vendors, developers or consultants identified or  */
/* referred to the User by Renesas in connection with the Product(s) and/or  */
/* the Application.                                                          */
/*                                                                           */
/*===========================================================================*/
/*                                                                           */
/* Source code for the port configuration functions.                         */
/*                                                                           */
/*===========================================================================*/

/*===========================================================================*/
/* Includes */
/*===========================================================================*/
#include "r_cg_macrodriver.h"
//#include "r_device.h"
#include "Config_PORT.h"
 

/*===========================================================================*/
/* Functions */
/*===========================================================================*/
/*****************************************************************************
** Function:    R_PORT_SetGpioOutput
** Description: Set Port_Pin to GPIO Output. Initial output level is low.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
**              Level: Output level High or Low
** Return:      None
******************************************************************************/
void R_PORT_SetGpioOutput(enum port_t Port, uint32_t Pin, enum level_t Level)
{
    if(Level == Low)
    {
        *PortList[Port].P_Reg &= ~(1u<<Pin);
    }
    else /* Level = High */
    {
        *PortList[Port].P_Reg |= (1u<<Pin);
    }
    *PortList[Port].PM_Reg &= ~(1u<<Pin);
    *PortList[Port].PMC_Reg &= ~(1u<<Pin);
}

/*****************************************************************************
** Function:    R_PORT_ToggleGpioOutput
** Description: Toggles the output level of Port_Pin.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_ToggleGpioOutput(enum port_t Port, uint32_t Pin)
{
    *PortList[Port].PNOT_Reg |= 1<<Pin;
}

/*****************************************************************************
** Function:    R_PORT_SetGpioInput
** Description: Set Port_Pin to GPIO Input with Port Input Buffer enabled.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_SetGpioInput(enum port_t Port, uint32_t Pin)
{
    *PortList[Port].PM_Reg |= 1<<Pin;
    *PortList[Port].PIBC_Reg |= 1<<Pin;
    *PortList[Port].PMC_Reg &= ~(1u<<Pin);
}

/*****************************************************************************
** Function:    R_PORT_SetGpioHighZ
** Description: Set Port_Pin to GPIO High Impedant Input (Port Input Buffer disabled).
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_SetGpioHighZ(enum port_t Port, uint32_t Pin)
{
    *PortList[Port].PIBC_Reg &= ~(1<<Pin);
    *PortList[Port].PM_Reg |= 1<<Pin;
}

/*****************************************************************************
** Function:    R_PORT_SetAltFunc
** Description: Configures Port_Pin to the chosen alternative function.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
**              Alt: Alternative Function (Alt1-Alt7)
**              IO: Input/Output direction
** Return:      None
******************************************************************************/
void R_PORT_SetAltFunc(enum port_t Port, uint32_t Pin, enum alt_t Alt, enum io_t IO)
{   
    switch(Alt)
    {
        case Alt1:
            *PortList[Port].PFCAE_Reg &= ~(1<<Pin);
            *PortList[Port].PFCE_Reg &= ~(1<<Pin);
            *PortList[Port].PFC_Reg &= ~(1<<Pin);
        break;
        
        case Alt2:
            *PortList[Port].PFCAE_Reg &= ~(1<<Pin);
            *PortList[Port].PFCE_Reg &= ~(1<<Pin);
            *PortList[Port].PFC_Reg |= 1<<Pin;
        break;
        
        case Alt3:
            *PortList[Port].PFCAE_Reg &= ~(1<<Pin);
            *PortList[Port].PFCE_Reg |= 1<<Pin;
            *PortList[Port].PFC_Reg &= ~(1<<Pin);
        break;
        
        case Alt4:
            *PortList[Port].PFCAE_Reg &= ~(1<<Pin);
            *PortList[Port].PFCE_Reg |= 1<<Pin;
            *PortList[Port].PFC_Reg |= 1<<Pin;
        break;
        
        case Alt5:
            *PortList[Port].PFCAE_Reg |= 1<<Pin;
            *PortList[Port].PFCE_Reg &= ~(1<<Pin);
            *PortList[Port].PFC_Reg &= ~(1<<Pin);
        break;
        
        case Alt6:
            *PortList[Port].PFCAE_Reg |= 1<<Pin;
            *PortList[Port].PFCE_Reg &= ~(1<<Pin);
            *PortList[Port].PFC_Reg |= 1<<Pin;
        break;
        
        case Alt7:
            *PortList[Port].PFCAE_Reg |= 1<<Pin;
            *PortList[Port].PFCE_Reg |= 1<<Pin;
            *PortList[Port].PFC_Reg &= ~(1<<Pin);
        break;
        
        default:
        break;
    }
    
    switch(IO)
    {
        case INPUT:
            *PortList[Port].PM_Reg |= 1<<Pin;
        break;
        
        case OUTPUT:
            *PortList[Port].PM_Reg &= ~(1<<Pin);
        break;
        
        default:
        break;
        
    }
    
    *PortList[Port].PMC_Reg |= 1u<<Pin;
}


/*****************************************************************************
** Function:    R_PORT_GetLevel
** Description: Gets the state of a Pin.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      0 - Pin is low
**              1 - Pin is high
******************************************************************************/
uint32_t R_PORT_GetLevel(enum port_t Port, uint32_t Pin)
{
    uint16_t PortLevel;
    
    PortLevel = *PortList[Port].PPR_Reg;
    PortLevel &= 1<<Pin;
    
    if(PortLevel == 0)
        {
            return 0;
        }
    else
        {
            return 1;
        }
}

/*****************************************************************************
** Function:    R_PORT_SetOpenDrain
** Description: Sets pin characteristics to open-drain.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_SetOpenDrain(enum port_t Port, uint32_t Pin)
{
  uint32_t loc_podcValue;
  loc_podcValue = *PortList[Port].PODC_Reg;
  loc_podcValue |= 1<<Pin;
  //protected_write(*PortList[Port].PPCMD_Reg, *PortList[Port].PPROTS_Reg, *PortList[Port].PODC_Reg, loc_podcValue);
}  

/*****************************************************************************
** Function:    R_PORT_SetPushPull
** Description: Sets pin characteristics to push-pull.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/ 
void R_PORT_SetPushPull(enum port_t Port, uint32_t Pin)
{
  uint32_t loc_podcValue;
  loc_podcValue = *PortList[Port].PODC_Reg;
  loc_podcValue &= ~(1<<Pin);
  //protected_write(*PortList[Port].PPCMD_Reg, *PortList[Port].PPROTS_Reg, *PortList[Port].PODC_Reg, loc_podcValue);
}

/*****************************************************************************
** Function:    R_PORT_SetAnalogFilter
** Description: Sets analog filter setting.
** Parameter:   fcla_signal_t InputSignal:  R_FCLA_NMI or
**                                          R_FLCA_INTPn (n=0-15)
**
**              FilterSetting
** Return:      None
******************************************************************************/ 
void R_PORT_SetAnalogFilter(enum fcla_signal_t InputSignal, uint8_t FilterSetting)
{
    //volatile uint8_t *loc_FCLA_INTP = (volatile uint8_t *)&FCLA0CTL0_INTPL;
    
   // if(InputSignal == R_FCLA_NMI)
    //    {
    //        FCLA0CTL0_NMI = (FilterSetting&&0x07);
     //   }
   // else
     //   {
     //       loc_FCLA_INTP += (InputSignal*4);
     //       *loc_FCLA_INTP = (FilterSetting&&0x07);
      //  }
}

/*****************************************************************************
** Function:    R_PORT_EnableIpControl
** Description: Enables Input/Output direction control by IP.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_EnableIpControl(enum port_t Port, uint32_t Pin)
{
    *PortList[Port].PIPC_Reg |= 1<<Pin;
}

/*****************************************************************************
** Function:    R_PORT_DisableIpControl
** Description: Disables Input/Output direction control by IP.
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_DisableIpControl(enum port_t Port, uint32_t Pin)
{
    *PortList[Port].PIPC_Reg &= ~(1<<Pin);
}

/*****************************************************************************
** Function:    R_PORT_EnableFastMode
** Description: Set output drive strength to high (fast mode).
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_EnableFastMode(enum port_t Port, uint32_t Pin)
{
    uint32_t loc_pdscValue;
    
    loc_pdscValue = (*PortList[Port].PDSC_Reg);
    loc_pdscValue |= (1<<Pin);
    
    //protected_write(*PortList[Port].PPCMD_Reg, *PortList[Port].PPROTS_Reg, *PortList[Port].PDSC_Reg, loc_pdscValue);
}

/*****************************************************************************
** Function:    R_PORT_DisableFastMode
** Description: Set output drive strength to low (slow mode).
** Parameter:   Port: Portgroup
**              Pin: Pin Number
** Return:      None
******************************************************************************/
void R_PORT_DisableFastMode(enum port_t Port, uint32_t Pin)
{
    uint32_t loc_pdscValue;
    
    loc_pdscValue = (*PortList[Port].PDSC_Reg);
    loc_pdscValue &= ~(1<<Pin);
    
    //protected_write(*PortList[Port].PPCMD_Reg, *PortList[Port].PPROTS_Reg, *PortList[Port].PDSC_Reg, loc_pdscValue);
}

