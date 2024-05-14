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
 * @date        14 December 2023
 * @author      Aarti B <aarti.bhelke@accoladeelectronics.com>
 *
 * @brief       R8580 port for CAN interrupt setting initialization.
 */

#include <stddef.h>
#include <stdint.h>
#include "iodefine.h"
#include "rh850_intc1.h"

/* Interrupt request flag (RFxxx) */
#define RH850_INTC1_REQUEST_CLEAR           (0x0U)  /* No interrupt request is made */
#define RH850_INTC1_REQUEST_SET             (0x1U)  /* Interrupt request is made */

/* Interrupt mask (MKxxx) */
#define RH850_INTC1_INTERRUPT_ENABLED       (0x0U)  /* Enables interrupt processing */
#define RH850_INTC1_INTERRUPT_DISABLED      (0x1U)  /* Disables interrupt processing */

/* Interrupt vector method select (TBxxx) */
#define RH850_INTC1_DIRECT_VECTOR           (0x0U)  /* Direct jumping to an address by the level of priority */
#define RH850_INTC1_TABLE_VECTOR            (0x1U)  /* Table reference */

typedef struct
{
    uint16_t P0:1, P1:1, P2:1, P3:1, :2, TB:1,  MK:1, :4, RF:1, :2, CT:1;
} RH850Intc1RegBits_t;

typedef union
{
    RH850Intc1RegBits_t BITS;
} RH850Intc1Reg_n;

typedef struct
{
    /* These are for holding addresses of ICRCANGERR0, ICRCANGRECC0, ICRCAN0ERR, ICRCAN0REC and ICRCAN0TRX respectively. */
    volatile RH850Intc1Reg_n* const reg[1];
} RH850Intc1CanRegs_t;

RH850Intc1CanRegs_t const g_Intc1CanRegsTable[] = \
{
    (volatile RH850Intc1Reg_n*) &(INTC1.ICRCANGERR0),
    (volatile RH850Intc1Reg_n*)  &(INTC1.ICRCANGRECC0),
    (volatile RH850Intc1Reg_n*)  &(INTC1.ICRCAN0ERR),
    (volatile RH850Intc1Reg_n*) &(INTC1.ICRCAN0REC),
    (volatile RH850Intc1Reg_n*) &(INTC1.ICRCAN0TRX)
};

void rh850_intc1_can_init                  (void)
{
    RH850Intc1CanRegs_t* canRegs = NULL;

    /* Ensure correct parameter and get corresponding table entry. */
    canRegs = (RH850Intc1CanRegs_t*) &g_Intc1CanRegsTable[0];

    /* Interrupt settings. */
    canRegs->reg[0]->BITS.TB = RH850_INTC1_TABLE_VECTOR;            // 1 - Table method.
    canRegs->reg[0]->BITS.MK = RH850_INTC1_INTERRUPT_DISABLED;      // 1 - Disables interrupt processing.
    canRegs->reg[0]->BITS.RF = RH850_INTC1_REQUEST_CLEAR;           // 0 - No interrupt request is made.

    canRegs->reg[1]->BITS.TB = RH850_INTC1_TABLE_VECTOR;            // 1 - Table method.
    canRegs->reg[1]->BITS.MK = RH850_INTC1_INTERRUPT_DISABLED;      // 1 - Disables interrupt processing.
    canRegs->reg[1]->BITS.RF = RH850_INTC1_REQUEST_CLEAR;           // 0 - No interrupt request is made.

    canRegs->reg[2]->BITS.TB = RH850_INTC1_TABLE_VECTOR;            // 1 - Table method.
    canRegs->reg[2]->BITS.MK = RH850_INTC1_INTERRUPT_DISABLED;      // 1 - Disables interrupt processing.
    canRegs->reg[2]->BITS.RF = RH850_INTC1_REQUEST_CLEAR;           // 0 - No interrupt request is made.
    
    canRegs->reg[3]->BITS.TB = RH850_INTC1_TABLE_VECTOR;            // 1 - Table method.
    canRegs->reg[3]->BITS.MK = RH850_INTC1_INTERRUPT_DISABLED;      // 1 - Disables interrupt processing.
    canRegs->reg[3]->BITS.RF = RH850_INTC1_REQUEST_CLEAR;           // 0 - No interrupt request is made.
    
    canRegs->reg[4]->BITS.TB = RH850_INTC1_TABLE_VECTOR;            // 1 - Table method.
    canRegs->reg[4]->BITS.MK = RH850_INTC1_INTERRUPT_DISABLED;      // 1 - Disables interrupt processing.
    canRegs->reg[4]->BITS.RF = RH850_INTC1_REQUEST_CLEAR;           // 0 - No interrupt request is made.
}