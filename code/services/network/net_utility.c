/**
 * @file        net_utility.c
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
 * @brief       Implementation of network common functions.
 */

// Standard includes.
#include <string.h>

//Port include 
#include "net_utility.h"
#include "modem_port.h"

uint16_t CalculateUrcParseLen(char * inBuff,char* startPtr,char* endPtr,char* outBuff, uint16_t *outLen,uint16_t totalLen)
{
    uint16_t RemainingLen;
    uint16_t ParseLength = 0;

    if ((NULL == inBuff)|| (NULL == startPtr) || (0 == totalLen))
    {
        NETWORK_PRINT_ERROR("Invalid argument\r\n");
        return ParseLength;
    }
    if(NULL == endPtr)
    {
        NETWORK_PRINT_ERROR("End prt NULL\r\n");
        endPtr = startPtr+ strlen(startPtr);
    }

   if(inBuff == startPtr) //If URC present at starting of the received buffer
    {
       ParseLength = (endPtr - startPtr); //Length of the URC
        *outLen = (totalLen - ParseLength);
        memcpy(outBuff, endPtr, *outLen);
    }
    else //If URC present in between total received buffer
    {
        ParseLength = (endPtr - startPtr); //ParseLength = Length of the URC
        *outLen = (startPtr - inBuff);
        memcpy(outBuff, inBuff, *outLen);
        if((*outLen) != (totalLen - ParseLength))
        {
            /*calculate remaining length */
            RemainingLen = totalLen - (endPtr - inBuff);
            memcpy((outBuff+(*outLen)),endPtr, RemainingLen);
            *outLen += RemainingLen;
        }
    }
    return ParseLength;
}