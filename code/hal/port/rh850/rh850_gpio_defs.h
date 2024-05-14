/**
 * @file        rh850_gpio_defs.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        24 November 2023
 * @author      Aarti B <aarti.bhelke@accoladeelectronics.com>
 *
 * @brief       Structure related to RH850 GPIO PORT registers.
 */

#ifndef RH850_GPIO_DEFS
#define RH850_GPIO_DEFS

#include <stdint.h>
#include "iodefine.h"

/* Structure of registers for each port */
struct pregs_t
{
    volatile uint16_t * P_Reg;          // Port register
    volatile uint16_t * PNOT_Reg;       // Port NOT register
    volatile uint16_t * PM_Reg;         // Port mode register
    volatile uint16_t * PMC_Reg;        // Port mode control register
    volatile uint16_t * PFC_Reg;        // Port function control register
    volatile uint16_t * PFCE_Reg;       // Port function control expansion register
    volatile uint16_t * PFCAE_Reg;      // Port function control additional expansion register
    volatile uint16_t * PIPC_Reg;       // Port input control register
    volatile uint16_t * PIBC_Reg;       // Port input buffer control register
    volatile const uint16_t * PPR_Reg;  // Port pin read register
    volatile uint16_t * PD_Reg;         // Port pull down option register
    volatile uint16_t * PU_Reg;         // Port pull up option register
    volatile uint32_t * PODC_Reg;       // Port open drain control register
    volatile uint32_t * PDSC_Reg;       // Port drive strength control register
    volatile uint32_t * PPROTS_Reg;     // Port protection status register
    volatile uint32_t * PPCMD_Reg;      // Port protection command register
};

/* Enumeration for ports corresponding to the PortList */
typedef enum
{
    Port0 = 0,
    Port1,
    Port8,
    Port9,
    Port10,
    Port11,
    Port12,
    Port18,
    Port20,
    APort0,
    APort1
}GpioPort_n;

typedef enum
{
    Pin_0,  Pin_1,  Pin_2,  Pin_3,  Pin_4,  Pin_5,  Pin_6,  Pin_7,
    Pin_8,  Pin_9,  Pin_10, Pin_11, Pin_12, Pin_13, Pin_14, Pin_15,
}GpioPin_n;

/* Array with available registers for the available ports. Registers that are not available */
/* for a port are 0.                                                                        */
static const struct pregs_t PortList[]=
{
  {(volatile uint16_t *)&PORT.P0, (volatile uint16_t *)&PORT.PNOT0,  (volatile uint16_t *)&PORT.PM0, (volatile uint16_t *) &PORT.PMC0, (volatile uint16_t *)&PORT.PFC0, (volatile uint16_t *)&PORT.PFCE0, (volatile uint16_t *)&PORT.PFCAE0, (volatile uint16_t *)&PORT.PIPC0, (volatile uint16_t *)&PORT.PIBC0,(volatile uint16_t *) &PORT.PPR0, (volatile uint16_t *)&PORT.PD0, (volatile uint16_t *)&PORT.PU0,(volatile uint32_t *)&PORT.PODC0, (volatile uint32_t *)&PORT.PDSC0, (volatile uint32_t *)&PORT.PPROTS0, (volatile uint32_t *)&PORT.PPCMD0},
  {(volatile uint16_t *)&PORT.P1, ( volatile uint16_t *)&PORT.PNOT1, (volatile uint16_t *)&PORT.PM1, (volatile uint16_t *)&PORT.PMC1, (volatile uint16_t *)&PORT.PFC1, (volatile uint16_t *)&PORT.PFCE1, (volatile uint16_t *)&PORT.PFCAE1, 0, (volatile uint16_t *)&PORT.PIBC1, (volatile uint16_t *)&PORT.PPR1, (volatile uint16_t *)&PORT.PD1, (volatile uint16_t *)&PORT.PU1, (volatile uint32_t *)&PORT.PODC1, (volatile uint32_t *)&PORT.PDSC1, (volatile uint32_t *)&PORT.PPROTS1, (volatile uint32_t *)&PORT.PPCMD1},
  {(volatile uint16_t *)&PORT.P8, (volatile uint16_t *)&PORT.PNOT8,  (volatile uint16_t *)&PORT.PM8, (volatile uint16_t *)&PORT.PMC8, (volatile uint16_t *)&PORT.PFC8, (volatile uint16_t *)&PORT.PFCE8, 0, 0, (volatile uint16_t *)&PORT.PIBC8, (volatile uint16_t *)&PORT.PPR8, (volatile uint16_t *)&PORT.PD8, (volatile uint16_t *)&PORT.PU8, (volatile uint32_t *)&PORT.PODC8, 0, (volatile uint32_t *)&PORT.PPROTS8, (volatile uint32_t *)&PORT.PPCMD8},
  {(volatile uint16_t *)&PORT.P9, (volatile uint16_t *)&PORT.PNOT9,  (volatile uint16_t *)&PORT.PM9, (volatile uint16_t *)&PORT.PMC9, (volatile uint16_t *)&PORT.PFC9, (volatile uint16_t *)&PORT.PFCE9, 0, 0, (volatile uint16_t *)&PORT.PIBC9, (volatile uint16_t *)&PORT.PPR9, (volatile uint16_t *)&PORT.PD9, (volatile uint16_t *)&PORT.PU9, (volatile uint32_t *)&PORT.PODC9, 0, (volatile uint32_t *)&PORT.PPROTS9, (volatile uint32_t *)&PORT.PPCMD9},
  {(volatile uint16_t *)&PORT.P10,(volatile uint16_t *)&PORT.PNOT10, (volatile uint16_t *)&PORT.PM10, (volatile uint16_t *)&PORT.PMC10, (volatile uint16_t *)&PORT.PFC10, (volatile uint16_t *)&PORT.PFCE10, (volatile uint16_t *)&PORT.PFCAE10, (volatile uint16_t *)&PORT.PIPC10, (volatile uint16_t *)&PORT.PIBC10, (volatile uint16_t *)&PORT.PPR10, (volatile uint16_t *)&PORT.PD10, (volatile uint16_t *)&PORT.PU10, (volatile uint32_t *)&PORT.PODC10, (volatile uint32_t *)&PORT.PDSC10, (volatile uint32_t *)&PORT.PPROTS10, (volatile uint32_t *)&PORT.PPCMD10},
  {(volatile uint16_t *)&PORT.P11,(volatile uint16_t *)&PORT.PNOT11, (volatile uint16_t *)&PORT.PM11, (volatile uint16_t *)&PORT.PMC11, (volatile uint16_t *)&PORT.PFC11, (volatile uint16_t *)&PORT.PFCE11, (volatile uint16_t *)&PORT.PFCAE11, (volatile uint16_t *)&PORT.PIPC11, (volatile uint16_t *)&PORT.PIBC11, (volatile uint16_t *)&PORT.PPR11, (volatile uint16_t *)&PORT.PD11, (volatile uint16_t *)&PORT.PU11, (volatile uint32_t *)&PORT.PODC11, (volatile uint32_t *)&PORT.PDSC11, (volatile uint32_t *)&PORT.PPROTS11, (volatile uint32_t *)&PORT.PPCMD11},
  {(volatile uint16_t *)&PORT.P12,(volatile uint16_t *)&PORT.PNOT12, (volatile uint16_t *)&PORT.PM12, (volatile uint16_t *)&PORT.PMC12, (volatile uint16_t *)&PORT.PFC12, (volatile uint16_t *)&PORT.PFCE12, (volatile uint16_t *)&PORT.PFCAE12, 0, (volatile uint16_t *)&PORT.PIBC12, (volatile uint16_t *)&PORT.PPR12, (volatile uint16_t *)&PORT.PD12, (volatile uint16_t *)&PORT.PU12, (volatile uint32_t *)&PORT.PODC12, (volatile uint32_t *)&PORT.PDSC12, (volatile uint32_t *)&PORT.PPROTS12, (volatile uint32_t *)&PORT.PPCMD12},
  {(volatile uint16_t *)&PORT.P18,(volatile uint16_t *)&PORT.PNOT18, (volatile uint16_t *)&PORT.PM18, (volatile uint16_t *)&PORT.PMC18, (volatile uint16_t *)&PORT.PFC18, 0, 0, 0, (volatile uint16_t *)&PORT.PIBC18, (volatile uint16_t *)&PORT.PPR18, (volatile uint16_t *)&PORT.PD18, (volatile uint16_t *)&PORT.PU18, (volatile uint32_t *)&PORT.PODC18, (volatile uint32_t *)&PORT.PDSC18, (volatile uint32_t *)&PORT.PPROTS18, (volatile uint32_t *)&PORT.PPCMD18},
  {(volatile uint16_t *)&PORT.P20,(volatile uint16_t *)&PORT.PNOT20, (volatile uint16_t *)&PORT.PM20, (volatile uint16_t *)&PORT.PMC20, (volatile uint16_t *)&PORT.PFC20, (volatile uint16_t *)&PORT.PFCE20, (volatile uint16_t *)&PORT.PFCAE20, 0, (volatile uint16_t *)&PORT.PIBC20, (volatile uint16_t *)&PORT.PPR20, (volatile uint16_t *)&PORT.PD20, (volatile uint16_t *)&PORT.PU20, (volatile uint32_t *)&PORT.PODC20, (volatile uint32_t *)&PORT.PDSC20, (volatile uint32_t *)&PORT.PPROTS20, (volatile uint32_t *)&PORT.PPCMD20},
  {(volatile uint16_t *)&PORT.AP0,(volatile uint16_t *)&PORT.APNOT0, (volatile uint16_t *)&PORT.APM0, 0, 0, 0, 0, 0, (volatile uint16_t *)&PORT.APIBC0, (volatile uint16_t *)&PORT.APPR0, 0, 0, 0, 0, 0, 0},
  {(volatile uint16_t *)&PORT.AP1,(volatile uint16_t *)&PORT.APNOT1, (volatile uint16_t *)&PORT.APM1, 0, 0, 0, 0, 0, (volatile uint16_t *)&PORT.APIBC1, (volatile uint16_t *)&PORT.APPR1, 0, 0, 0, 0, 0, 0}
};

#endif /* RH850_GPIO_DEFS */