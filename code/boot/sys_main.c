/**
 * @file        sys_main.c
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
 * @brief       C code entry function.
 */

#include "sys_defs.h"

extern void main(void);
extern void tcu_main(void);

void sys_main(void)
{
#ifdef TCU_2W
    tcu_main();
#else
    main();
#endif
}
