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

#include <stddef.h>
#include <stdint.h>
#include "hal_util.h"
#include "iodefine.h"
#include "rh850_intc2.h"

/* Interrupt request flag (RFxxx) */
#define RH850_INTC2_REQUEST_CLEAR           (0x0U)  /* No interrupt request is made */
#define RH850_INTC2_REQUEST_SET             (0x1U)  /* Interrupt request is made */
/* Interrupt mask (MKxxx) */
#define RH850_INTC2_INTERRUPT_ENABLED       (0x0U)  /* Enables interrupt processing */
#define RH850_INTC2_INTERRUPT_DISABLED      (0x1U)  /* Disables interrupt processing */
/* Interrupt vector method select (TBxxx) */
#define RH850_INTC2_DIRECT_VECTOR           (0x0U)  /* Direct jumping to an address by the level of priority */
#define RH850_INTC2_TABLE_VECTOR            (0x1U)  /* Table reference */
/* Specify 16 interrupt priority levels (P3xxx,P2xxx,P1xxx,P0xxx) */
#define RH850_INTC2_PRIORITY_HIGHEST        (0x00C0U)  /* Level 0 (highest) */
#define RH850_INTC2_PRIORITY_LEVEL1         (0x00C1U)  /* Level 1 */
#define RH850_INTC2_PRIORITY_LEVEL2         (0x00C2U)  /* Level 2 */
#define RH850_INTC2_PRIORITY_LEVEL3         (0x00C3U)  /* Level 3 */
#define RH850_INTC2_PRIORITY_LEVEL4         (0x00C4U)  /* Level 4 */
#define RH850_INTC2_PRIORITY_LEVEL5         (0x00C5U)  /* Level 5 */
#define RH850_INTC2_PRIORITY_LEVEL6         (0x00C6U)  /* Level 6 */
#define RH850_INTC2_PRIORITY_LEVEL7         (0x00C7U)  /* Level 7 */
#define RH850_INTC2_PRIORITY_LEVEL8         (0x00C8U)  /* Level 8 */
#define RH850_INTC2_PRIORITY_LEVEL9         (0x00C9U)  /* Level 9 */
#define RH850_INTC2_PRIORITY_LEVEL10        (0x00CAU)  /* Level 10 */
#define RH850_INTC2_PRIORITY_LEVEL11        (0x00CBU)  /* Level 11 */
#define RH850_INTC2_PRIORITY_LEVEL12        (0x00CCU)  /* Level 12 */
#define RH850_INTC2_PRIORITY_LEVEL13        (0x00CDU)  /* Level 13 */
#define RH850_INTC2_PRIORITY_LEVEL14        (0x00CEU)  /* Level 14 */
#define RH850_INTC2_PRIORITY_LOWEST         (0x00CFU)  /* Level 15 (lowest) */

typedef struct
{
    uint16_t P0:1, P1:1, P2:1, P3:1, :2, TB:1,  MK:1, :4, RF:1, :2, CT:1;
} RH850Intc2RegBits_t;

typedef union
{
    RH850Intc2RegBits_t BITS;
    uint16_t U16;
} RH850Intc2Reg_n;

typedef struct
{
    /* These are for holding addresses of UR0, UR1 and UR2 respectively. */
    volatile RH850Intc2Reg_n* const reg[RH850Intc2UartMax];
} RH850Intc2UartRegs_t;

RH850Intc2UartRegs_t const g_Intc2UartRegsTable[PortDefsUartMax] = 
{
    { (volatile RH850Intc2Reg_n*) 0xFFFFB044,   (volatile RH850Intc2Reg_n*) 0xFFFFB046, (volatile RH850Intc2Reg_n*) 0xFFFFB048 },
    { (volatile RH850Intc2Reg_n*) 0xFFFFB0F2,   (volatile RH850Intc2Reg_n*) 0xFFFFB0F4, (volatile RH850Intc2Reg_n*) 0xFFFFB0F6 },
    { (volatile RH850Intc2Reg_n*) 0xFFFFB14A,   (volatile RH850Intc2Reg_n*) 0xFFFFB14C, (volatile RH850Intc2Reg_n*) 0xFFFFB14E }
};

void rh850_intc2_uart_init                  (PortDefsUart_n portDefUart)
{
    RH850Intc2UartRegs_t* uartRegs = NULL;

    /* Ensure correct parameter and get corresponding table entry. */
    hal_util_assert ( portDefUart < PortDefsUartMax );
    uartRegs = (RH850Intc2UartRegs_t*) &g_Intc2UartRegsTable[portDefUart];

    /* Disable interrupt and clear interrupt pairs. */
    uartRegs->reg[0]->BITS.MK = RH850_INTC2_INTERRUPT_DISABLED;
    uartRegs->reg[0]->BITS.RF = RH850_INTC2_REQUEST_CLEAR;
    uartRegs->reg[1]->BITS.MK = RH850_INTC2_INTERRUPT_DISABLED;
    uartRegs->reg[1]->BITS.RF = RH850_INTC2_REQUEST_CLEAR;
    uartRegs->reg[2]->BITS.MK = RH850_INTC2_INTERRUPT_DISABLED;
    uartRegs->reg[2]->BITS.RF = RH850_INTC2_REQUEST_CLEAR;

    /* Set interrupt method and set priority. */
    uartRegs->reg[0]->BITS.TB = RH850_INTC2_TABLE_VECTOR;
    uartRegs->reg[0]->U16 &= RH850_INTC2_PRIORITY_LEVEL1;
    uartRegs->reg[1]->BITS.TB = RH850_INTC2_TABLE_VECTOR;
    uartRegs->reg[1]->U16 &= RH850_INTC2_PRIORITY_LEVEL2;
    uartRegs->reg[2]->BITS.TB = RH850_INTC2_TABLE_VECTOR;
    uartRegs->reg[2]->U16 &= RH850_INTC2_PRIORITY_LEVEL3;
}

void rh850_intc2_uart_enable                (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType)
{
    RH850Intc2UartRegs_t* uartRegs = NULL;

    /* Ensure correct parameter and get corresponding table entry. */
    hal_util_assert ( portDefUart < PortDefsUartMax );
    hal_util_assert ( interruptType < RH850Intc2UartMax );
    uartRegs = (RH850Intc2UartRegs_t*) &g_Intc2UartRegsTable[portDefUart];

    /* Set corresponding bit. */
    uartRegs->reg[interruptType]->BITS.MK = RH850_INTC2_INTERRUPT_ENABLED;
}

void rh850_intc2_uart_disable               (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType)
{
    RH850Intc2UartRegs_t* uartRegs = NULL;

    /* Ensure correct parameter and get corresponding table entry. */
    hal_util_assert ( portDefUart < PortDefsUartMax );
    hal_util_assert ( interruptType < RH850Intc2UartMax );
    uartRegs = (RH850Intc2UartRegs_t*) &g_Intc2UartRegsTable[portDefUart];

    /* Set corresponding bit. */
    uartRegs->reg[interruptType]->BITS.MK = RH850_INTC2_INTERRUPT_DISABLED;
}

void rh850_intc2_uart_set                   (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType)
{
    RH850Intc2UartRegs_t* uartRegs = NULL;

    /* Ensure correct parameter and get corresponding table entry. */
    hal_util_assert ( portDefUart < PortDefsUartMax );
    hal_util_assert ( interruptType < RH850Intc2UartMax );
    uartRegs = (RH850Intc2UartRegs_t*) &g_Intc2UartRegsTable[portDefUart];

    /* Set corresponding bit. */
    uartRegs->reg[interruptType]->BITS.RF = RH850_INTC2_REQUEST_SET;
}

void rh850_intc2_uart_clear                 (PortDefsUart_n portDefUart, RH850Intc2Uart_n interruptType)
{
    RH850Intc2UartRegs_t* uartRegs = NULL;

    /* Ensure correct parameter and get corresponding table entry. */
    hal_util_assert ( portDefUart < PortDefsUartMax );
    hal_util_assert ( interruptType < RH850Intc2UartMax );
    uartRegs = (RH850Intc2UartRegs_t*) &g_Intc2UartRegsTable[portDefUart];

    /* Set corresponding bit. */
    uartRegs->reg[interruptType]->BITS.RF = RH850_INTC2_REQUEST_CLEAR;
}
