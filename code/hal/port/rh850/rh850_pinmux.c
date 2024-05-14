/**
 * @file        TODO.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        25 October 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TODO
 */
 
#include <stdint.h>
#include "rh850_pinmux.h"
#include "hal_util.h"
#include "iodefine.h"
#include "Pin.h"

#define RH850_PINMUX_WRITE_PROTECT_COMMAND          (0x000000A5UL)  /* Write protected */
#define RH850_PINMUX_PORT_PDSCn3_SLOW_MODE_SELECT   (0x00000000UL)  /* Low drive strength */
#define RH850_PINMUX_PORT_PODCn3_PUSH_PULL          (0x00000000UL) /* Push-pull */
#define RH850_PINMUX_PORT_PBDCn3_MODE_DISABLED      (0x0000U) /* Bidirectional mode disabled */
#define RH850_PINMUX_PORT_Pn3_OUTPUT_LOW            (0x0000U) /* Outputs low level */

static void rh850_pinmux_rlin30(void);
static void rh850_pinmux_rlin31(void);
static void rh850_pinmux_rlin32(void);

void rh850_pinmux_rlin3(PortDefsUart_n port)
{
    switch ( port )
    {
        case PortDefsUartGps:   rh850_pinmux_rlin30();      break;
        case PortDefsUartGsm:   rh850_pinmux_rlin31();      break;
        case PortDefsUartDbg:   rh850_pinmux_rlin32();      break;
        default:                hal_util_assert(0);         break;
    }
}

static void rh850_pinmux_rlin30(void)
{
    uint32_t tmp_port;
    /* Set RLIN30RX pin */
    PORT.PIBC10 &= _PORT_CLEAR_BIT9;
    PORT.PBDC10 &= _PORT_CLEAR_BIT9;
    PORT.PM10 |= _PORT_SET_BIT9;
    PORT.PMC10 &= _PORT_CLEAR_BIT9;
    PORT.PFC10 &= _PORT_CLEAR_BIT9;
    PORT.PFCE10 |= _PORT_SET_BIT9;
    PORT.PFCAE10 |= _PORT_SET_BIT9;
    PORT.PMC10 |= _PORT_SET_BIT9;
    /* Set RLIN30TX pin */
    PORT.PIBC10 &= _PORT_CLEAR_BIT10;
    PORT.PBDC10 &= _PORT_CLEAR_BIT10;
    PORT.PM10 |= _PORT_SET_BIT10;
    PORT.PMC10 &= _PORT_CLEAR_BIT10;
    tmp_port = PORT.PDSC10;
    PORT.PPCMD10 = RH850_PINMUX_WRITE_PROTECT_COMMAND;
    PORT.PDSC10 = (tmp_port & _PORT_CLEAR_BIT10);
    PORT.PDSC10 = (uint32_t) ~(tmp_port & _PORT_CLEAR_BIT10);
    PORT.PDSC10 = (tmp_port & _PORT_CLEAR_BIT10);
    PORT.PFC10 |= _PORT_SET_BIT10;
    PORT.PFCE10 &= _PORT_CLEAR_BIT10;
    PORT.PFCAE10 &= _PORT_CLEAR_BIT10;
    PORT.PMC10 |= _PORT_SET_BIT10;
    PORT.PM10 &= _PORT_CLEAR_BIT10;
}

static void rh850_pinmux_rlin31(void)
{
    uint32_t tmp_port;
    /* Set RLIN31RX pin */
    PORT.PIBC10 &= _PORT_CLEAR_BIT11;
    PORT.PBDC10 &= _PORT_CLEAR_BIT11;
    PORT.PM10 |= _PORT_SET_BIT11;
    PORT.PMC10 &= _PORT_CLEAR_BIT11;
    PORT.PFC10 &= _PORT_CLEAR_BIT11;
    PORT.PFCE10 |= _PORT_SET_BIT11;
    PORT.PFCAE10 |= _PORT_SET_BIT11;
    PORT.PMC10 |= _PORT_SET_BIT11;
    /* Set RLIN31TX pin */
    PORT.PIBC10 &= _PORT_CLEAR_BIT12;
    PORT.PBDC10 &= _PORT_CLEAR_BIT12;
    PORT.PM10 |= _PORT_SET_BIT12;
    PORT.PMC10 &= _PORT_CLEAR_BIT12;
    tmp_port = PORT.PDSC10;
    PORT.PPCMD10 = RH850_PINMUX_WRITE_PROTECT_COMMAND;
    PORT.PDSC10 = (tmp_port & _PORT_CLEAR_BIT12);
    PORT.PDSC10 = (uint32_t) ~(tmp_port & _PORT_CLEAR_BIT12);
    PORT.PDSC10 = (tmp_port & _PORT_CLEAR_BIT12);
    PORT.PFC10 |= _PORT_SET_BIT12;
    PORT.PFCE10 &= _PORT_CLEAR_BIT12;
    PORT.PFCAE10 &= _PORT_CLEAR_BIT12;
    PORT.PMC10 |= _PORT_SET_BIT12;
    PORT.PM10 &= _PORT_CLEAR_BIT12;
}

static void rh850_pinmux_rlin32(void)
{
    uint32_t tmp_port;
    /* Set RLIN32RX pin */
    PORT.PIBC0 &= _PORT_CLEAR_BIT13;
    PORT.PBDC0 &= _PORT_CLEAR_BIT13;
    PORT.PM0 |= _PORT_SET_BIT13;
    PORT.PMC0 &= _PORT_CLEAR_BIT13;
    PORT.PIPC0 &= _PORT_CLEAR_BIT13;
    PORT.PFC0 |= _PORT_SET_BIT13;
    PORT.PFCE0 &= _PORT_CLEAR_BIT13;
    PORT.PFCAE0 |= _PORT_SET_BIT13;
    PORT.PMC0 |= _PORT_SET_BIT13;
    /* Set RLIN32TX pin */
    PORT.PIBC0 &= _PORT_CLEAR_BIT14;
    PORT.PBDC0 &= _PORT_CLEAR_BIT14;
    PORT.PM0 |= _PORT_SET_BIT14;
    PORT.PMC0 &= _PORT_CLEAR_BIT14;
    PORT.PIPC0 &= _PORT_CLEAR_BIT14;
    tmp_port = PORT.PDSC0;
    PORT.PPCMD0 = RH850_PINMUX_WRITE_PROTECT_COMMAND;
    PORT.PDSC0 = (tmp_port & _PORT_CLEAR_BIT14);
    PORT.PDSC0 = (uint32_t) ~(tmp_port & _PORT_CLEAR_BIT14);
    PORT.PDSC0 = (tmp_port & _PORT_CLEAR_BIT14);
    PORT.PFC0 &= _PORT_CLEAR_BIT14;
    PORT.PFCE0 &= _PORT_CLEAR_BIT14;
    PORT.PFCAE0 &= _PORT_CLEAR_BIT14;
    PORT.PMC0 |= _PORT_SET_BIT14;
    PORT.PM0 &= _PORT_CLEAR_BIT14;
}

void rh850_pinmux_rcan(void)
{
    PORT.PMC0 |= 0x0003;                //Select alternate pin function for P0.0,P0.1
    PORT.PM0 &= _PORT_CLEAR_BIT0;       //Set output mode for P0.0 as it is CAN0TX
    PORT.PM0 |= _PORT_SET_BIT1;         //Set input mode for P0.1 as it is CAN0RX
    PORT.PFC0 = 0x0003;
    PORT.PFCAE0 = 0x0000;
    PORT.PFCE0 = 0x0000;

    /* PORT10 setting */                //Set standby pin for PORT P10.3 for CAN0
    PORT.PPCMD10 = RH850_PINMUX_WRITE_PROTECT_COMMAND;
    PORT.PDSC10 = RH850_PINMUX_PORT_PDSCn3_SLOW_MODE_SELECT;
    PORT.PDSC10 = (uint32_t) ~(RH850_PINMUX_PORT_PDSCn3_SLOW_MODE_SELECT);
    PORT.PDSC10 = RH850_PINMUX_PORT_PDSCn3_SLOW_MODE_SELECT;
    PORT.PPCMD10 = RH850_PINMUX_WRITE_PROTECT_COMMAND;
    PORT.PODC10 = RH850_PINMUX_PORT_PODCn3_PUSH_PULL;
    PORT.PODC10 = (uint32_t) ~(RH850_PINMUX_PORT_PODCn3_PUSH_PULL);
    PORT.PODC10 = RH850_PINMUX_PORT_PODCn3_PUSH_PULL;
    PORT.PBDC10 = RH850_PINMUX_PORT_PBDCn3_MODE_DISABLED;
    PORT.P10 = RH850_PINMUX_PORT_Pn3_OUTPUT_LOW;
    PORT.PM10 &= _PORT_CLEAR_BIT3;      //Set P10.3 pin as output mode
    PORT.P10 &= _PORT_CLEAR_BIT3;       //Set standby pin low
}
