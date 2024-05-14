/**
 * @file        sl_drive.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        27 December 2023
 * @author      Adwait Patil <adwait.patil@accoladeelectronics.com>
 *
 * @brief       Temporary implementation of Sibros API - to make 'dev-file-io-self-test' work.
 */

#include "sl_drive.h"

sl_drive__prefix_s sl_drive__get_prefix(sl_drive__partition_e partition)
{
    sl_drive__prefix_s drive;

    switch ( partition )
    {
        case SL_DRIVE__PARTITION_UPDATER:
        drive.cstring[0] = 'u';
        drive.cstring[1] = ':';
        drive.cstring[2] = '\0';
        break;

        case SL_DRIVE__PARTITION_SECURE:
        drive.cstring[0] = 's';
        drive.cstring[1] = ':';
        drive.cstring[2] = '\0';
        break;

        case SL_DRIVE__PARTITION_NORMAL:
        drive.cstring[0] = 'n';
        drive.cstring[1] = ':';
        drive.cstring[2] = '\0';
        break;
    }
    
    return drive;
}

