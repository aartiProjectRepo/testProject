/**
 * @file        rh850_uart_defs.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        5 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Definitions related to RH850 RLIN3 (UART) registers.
 */

#ifndef RH850_UART_DEFS
#define RH850_UART_DEFS

/*
    LIN Wake-up Baud Rate Select Register (RLN3n.LWBR)
*/
/* Bit sampling count select (NSPB[3:0]) */
#define RH850_UART_6_SAMPLING                           (0x50U) /* 6 sampling */
#define RH850_UART_7_SAMPLING                           (0x60U) /* 7 sampling */
#define RH850_UART_8_SAMPLING                           (0x70U) /* 8 sampling */
#define RH850_UART_9_SAMPLING                           (0x80U) /* 9 sampling */
#define RH850_UART_10_SAMPLING                          (0x90U) /* 10 sampling */
#define RH850_UART_11_SAMPLING                          (0xA0U) /* 11 sampling */
#define RH850_UART_12_SAMPLING                          (0xB0U) /* 12 sampling */
#define RH850_UART_13_SAMPLING                          (0xC0U) /* 13 sampling */
#define RH850_UART_14_SAMPLING                          (0xD0U) /* 14 sampling */
#define RH850_UART_15_SAMPLING                          (0xE0U) /* 15 sampling */
#define RH850_UART_16_SAMPLING                          (0xF0U) /* 16 sampling */
/* Prescaler clock select (LPRS[2:0]) */
#define RH850_UART_PRESCALER_CLOCK_SELECT_1             (0x00U) /* Prescaler clock select 1/1 */
#define RH850_UART_PRESCALER_CLOCK_SELECT_2             (0x02U) /* Prescaler clock select 1/2 */
#define RH850_UART_PRESCALER_CLOCK_SELECT_4             (0x04U) /* Prescaler clock select 1/4 */
#define RH850_UART_PRESCALER_CLOCK_SELECT_8             (0x06U) /* Prescaler clock select 1/8 */
#define RH850_UART_PRESCALER_CLOCK_SELECT_16            (0x08U) /* Prescaler clock select 1/16 */
#define RH850_UART_PRESCALER_CLOCK_SELECT_32            (0x0AU) /* Prescaler clock select 1/32 */
#define RH850_UART_PRESCALER_CLOCK_SELECT_64            (0x0CU) /* Prescaler clock select 1/64 */
#define RH850_UART_PRESCALER_CLOCK_SELECT_128           (0x0EU) /* Prescaler clock select 1/128 */

/*
    UART Mode Register (RLN3n.LMD)
*/
/* UART reception data noise filtering disable (LRDNFS) */
#define RH850_UART_NOISE_FILTER_ENABLED                 (0x00U) /* The noise filter is enabled */
#define RH850_UART_NOISE_FILTER_DISABLED                (0x20U) /* The noise filter is disabled */
/* LIN or UART mode select (LMD[1:0]) */
#define RH850_UART_MODE_SELECT                          (0x01U) /* UART mode */

/*
    UART Configuration Register (RLN3n.LBFC)
*/
/* UART output polarity switch (UTPS) */
#define RH850_UART_TRANSMISSION_NORMAL                  (0x00U) /* Transmit data normal output */
#define RH850_UART_TRANSMITSSION_INVERTED               (0x40U) /* Transmit data with inversion output */
/* UART input polarity switch (URPS) */
#define RH850_UART_RECEPTION_NORMAL                     (0x00U) /* Reception data normal output */
#define RH850_UART_RECEPTION_INVERTED                   (0x20U) /* Reception data with inversion output */
/* UART parity select (UPS[1:0]) */
#define RH850_UART_PARITY_PROHIBITED                    (0x00U) /* Parity prohibited */
#define RH850_UART_PARITY_EVEN                          (0x08U) /* Even Parity */
#define RH850_UART_PARITY_ZERO                          (0x10U) /* 0 Parity */
#define RH850_UART_PARITY_ODD                           (0x18U) /* Odd parity */
/* UART stop bit length select (USBLS) */
#define RH850_UART_STOP_BIT_1                           (0x00U) /* Stop bit: 1 bit */
#define RH850_UART_STOP_BIT_2                           (0x04U) /* Stop bit: 2 bits */
/* UART transfer format order select (UBOS) */
#define RH850_UART_LSB                                  (0x00U) /* LSB first */
#define RH850_UART_MSB                                  (0x02U) /* MSB first */
/* UART character length select (UBLS) */
#define RH850_UART_LENGTH_8                             (0x00U) /* UART 8 bits communication */
#define RH850_UART_LENGTH_7                             (0x01U) /* UART 7 bits communication */

/*
    UART Error Detection Enable Register (RLN3n.LEDE)
*/
/* Framing error detection enable (FERE) */
#define RH850_UART_FRAMING_ERROR_DETECTED               (0x08U) /* Enables framing error detection */
/* Overrun error detection enable (OERE) */
#define RH850_UART_OVERRUN_ERROR_DETECTED               (0x04U) /* Enables overrun error detection */
/* Bit error detection enable (BERE) */
#define RH850_UART_BIT_ERROR_DETECTED                   (0x01U) /* Enables bit error detection */

/*
    UART Control Register (RLN3n.LCUC)
*/
/* LIN reset (OM0) */
#define RH850_UART_LIN_RESET_MODE_CAUSED                (0x00U) /* LIN reset mode is caused */
#define RH850_UART_LIN_RESET_MODE_CANCELED              (0x01U) /* LIN reset mode is canceled. */

/*
    UART Transmission Control Register (RLN3n.LTRC)
*/
/* UART buffer transmission start (RTS) */
#define RH850_UART_BUFFER_TRANSMISSION_IS_STOPPED       (0x00U) /* UART buffer transmission is stopped */
#define RH850_UART_BUFFER_TRANSMISSION_IS_STARTED       (0x02U) /* UART buffer transmission is started */

/*
    UART Status Register (RLN3n.LST)
*/
/* Reception status flag (URS) */
#define RH850_UART_RECEPTION_ISNOT_OPERATED             (0x00U) /* Reception is not operated */
#define RH850_UART_RECEPTION_OPERATED                   (0x20U) /* Reception is operated */
/* Transmission status flag (UTS) */
#define RH850_UART_TRANSMISSION_ISNOT_OPERATED          (0x00U) /* Transmission is not operated */
#define RH850_UART_TRANSMISSION_OPERATED                (0x10U) /* Transmission is operated */
/* Error detection flag (ERR) */
#define RH850_UART_REC_NO_ERROR_DETECTED                (0x00U) /* No error has been detected */
#define RH850_UART_RECEPTION_ERROR_DETECTED             (0x08U) /* Error has been detected */
/* Successful UART buffer transmission flag (FTC)*/
#define RH850_UART_BUFFER_TRANSMIT_NOT_COMPLETED        (0x00U) /* UART buffer transmission has not been completed */
#define RH850_UART_TRANSMISSION_COMPLETED               (0x01U) /* UART buffer transmission has been completed */

/*
    UART Error Status Register (RLN3n.LEST)
*/
/* Parity error flag (UPER) */
#define RH850_UART_PARITY_ERROR_NOT_DETECTED            (0x00U) /* Parity error has not been detected */
#define RH850_UART_PARITY_ERROR_FLAG                    (0x40U) /* Parity error has been detected */
/* Framing error flag (FER) */
#define RH850_UART_FRAMING_ERROR_NOT_DETECTED           (0x00U) /* Framing error has not been detected */
#define RH850_UART_FRAMING_ERROR_FLAG                   (0x08U) /* Framing error has been detected */
/* Overrun error flag (OER) */
#define RH850_UART_OVERRUN_ERROR_NOT_DETECTED           (0x00U) /* Overrun error has not been detected */
#define RH850_UART_OVERRUN_ERROR_FLAG                   (0x04U) /* Overrun error has been detected */
/* Bit error flag (BER) */
#define RH850_UART_BIT_ERROR_NOT_DETECTED               (0x00U) /* Bit error has not been detected */
#define RH850_UART_BIT_ERROR_FLAG                       (0x01U) /* Bit error has been detected */

/*
    UART Operation Enable Register (RLN3n.LUOER)
*/
/* Reception enable (UROE) */
#define RH850_UART_RECEPTION_DISABLED                   (0x00U) /* Disables reception */
#define RH850_UART_RECEPTION_ENABLED                    (0x02U) /* Enables reception */
/* Transmission enable (UTOE) */
#define RH850_UART_TRANSMISSION_DISABLED                (0x00U) /* Disables transmission */
#define RH850_UART_TRANSMISSION_ENABLED                 (0x01U) /* Enables transmission */

/*
    UART Option Register 1 (RLN3n.LUOR1)
*/
/* Transmission interrupt generation timing select  (UTIGTS) */
#define RH850_UART_INT_TRANSMISSION_START               (0x00U) /* INT is generated at the start of transmission */
#define RH850_UART_INT_TRANSMISSION_END                 (0x08U) /* INT is generated at the completion of transmission */

/*
    UART Error Clear Flag Macro Definition
*/
#define RH850_UART_CLEAR_ERROR_FLAG                     (0x7D)

#endif /* RH850_UART_DEFS */