/**
 * @file        net_utility.h
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        30 November 2023
 * @author      Aditya P <aditya.prajapati@accoladeelectronics.com>
 * 
 * @brief       Common functions that used in network stack
 */

#ifndef NET_UTILITY_H
#define NET_UTILITY_H

#include <stdint.h>

/**
 * @brief   Calculate URC ParseLen,This function is used calculate the length of URC.It also maintain remaining data other than parsed URC in received buffer.
 * @param   inBuff      Pointer of received input buffer
 * @param   startPtr    Start Pointer of URC which is being parse.
 * @param   endPtr      End Pointer of URC which is being parse.
 * @param   outBuff     Remaining data pointer after removing parsed URC.
 * @param   outLen      Total remaining length of received buffer which is calculated after subtracting URC length from total received length
 * @param   totalLen    Total length of received input buffer.
 * @return  ParseLength It returns length of parsed URC
 */
uint16_t CalculateUrcParseLen(char * inBuff,char* startPtr,char* endPtr,char* outBuff, uint16_t *outLen,uint16_t totalLen);

#endif /* NET_UTILITY_H */