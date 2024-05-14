/**
 * @file        tcu_2w_test.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        27 November 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       TCU-2W test code - header.
 */

#ifndef TCU_2W_TEST_H
#define TCU_2W_TEST_H

/**
 *  @brief                                  Print CPU up time.
*/
void printUpTime();

/**
 *  @brief                                  Tests memory (RAM) memories and validates BSS and DATA correctly configured.
*/
void tcu_test_memory(void);

/**
 *  @brief                                  Tests SPI interface and flash memory.
*/
void tcu_test_flash(void);

/**
 *  @brief                                  Tests UART interface.
*/
void tcu_test_uart(void);

/**
 *  @brief                                  Tests Network tasks.
*/
void tcu_test_network(void);

/**
 *  @brief                                  Tests CAN tasks.
*/
void tcu_test_can(void);

/**
 *  @brief                                  Tests OsTime tasks.
*/
void tcu_test_ostime(void);

/**
 *  @brief                                  Tests GPIO interface on DIET PCB
*/
void tcu_test_gpio(void);

/**
 *  @brief                                  Tests GPS interface on DIET PCB.
*/
void tcu_test_gps(void);

#endif /* TCU_TEST_H */
