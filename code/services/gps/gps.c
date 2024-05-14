/**
 * @file        gps.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        30 November 2023
 * @author      Aditya Patil <aditya.patil@accoladeelectronics.com>
 *
 * @brief       File contains source for GPS modem
 *              TODO: (optional) postprocessing for calculating epoch form date and time after fix quality is 1
 */

// Standard includes.
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Module includes.
#include "gps.h"
#include "tcu_board.h"
#include "hal_util.h"
#include "logger_can.h"

// Private defines.
#define RX_BUFF_MAX_LEN     (1024)
#define SENTENCE_MAX_LEN    (200)
#define FIELD_MAX_CHARS     (15)

typedef enum
{
    UTC_TIME = 1,
    DATA_VALIDITY,
    LATITUDE,
    LAT_DIR,
    LONGITUDE,
    LONG_DIR,
    SPEED_KNOTS,
    COD,
    DATE,
    MAGNETIC_VARIATION,
    MAGNETIC_VARIATION_DIR,
    NAVIGATIONAL_STATUS,
}RmcField_n;

typedef enum
{
    FIX_STATUS = 6,
    NUM_OF_SATELLITES = 7,
    ALTITUDE = 8,
}GgaField_n;

typedef enum
{
    SPEED_KMPH = 8
}VtgField_n;

/**
 * @brief Reads characters from the GPS UART into the provided buffer.
 * @param rxBuff        Pointer to the buffer to store received characters.
 * @param maxbuffsize   Maximum size of the buffer.
 */
static uint32_t gpsUartRead                 (uint8_t *rxBuff, uint32_t maxbuffsize);

/**
 * @brief Writes characters to the GPS UART.
 * @param txBuff        Pointer to the buffer containing characters to be transmitted.
 * @param txLen         Number of characters to transmit.
 */
static uint32_t gpsUartWrite                (uint8_t *txBuff, uint32_t txLen);

/**
 * @brief This function accepts the raw data from the modem in NMEA format and processes it.
 * @param rxBuff        Pointer to the NMEA sentence data
 * @param rxLen         Length of the NMEA data
 */
static void parseGpsPacket                  ( char *rxBuff, uint16_t rxLen);

/**
 * @brief This is a generic function to parse a single NMEA sentence and update the internal GpsInfo structure.
 * @param fieldNum      Field index corresponding to the enumerator RmcField_n or GgaField_n.
 * @param inputStr      Pointer to the buffer containing a single complete NMEA sentence.
 * @param len           Length of the passed buffer.
 */
static void parseNmeaSentence               (uint8_t fieldNum, char *inputStr, uint32_t len);

/**
 * @brief This is a function for calculating string CRC and comparing received CRC
 * @param date      NMEA String for calculating CRC. Multiple NMEA strings are not allowed in the input buffer
 * @param len      	String length of the NMEA sentence
 * @return          Returns 1 if CRC is correct and 0 if CRC is incorrect
 */
static uint8_t isCrcValid                   (char *sentence, uint8_t len);

/**
 * @brief This is a function for converting string hex value to the decimal . e.g char array "0E" will return 14
 * @param date      Hex value in string
 * @return          Returns the decimal equivalent of the hex string
 */
static int hexStringToDec                   (char *hexStr, uint8_t len);

/**
 * @brief Dummy function which ignores arguments.
 */
static void logger_dummy                    (char* fmt, ...);

/**
 * @brief Checks if data is ready to be read from the GPS UART.
 */
static uint32_t gpsDataReady(void);

// Private variables. 
static GpsInfo_t g_gpsInfo;
static logger_gps_t g_logger = logger_dummy;

void GpsInit                                (logger_gps_t logger)
{
    if ( logger )
    {
        g_logger = logger;
    }
}

static uint32_t gpsUartRead                 (uint8_t *rxBuff, uint32_t maxBuffSize)
{
    size_t readByte = 0;
    HalBuffer_t buf = {rxBuff, maxBuffSize};
    
    if ( HalUartErrOk == hal_uart_receive_available(g_UartGps, &readByte) )
    {
        if ( readByte )
        {
            if ( readByte > buf.sizeMem )
            {
                readByte = buf.sizeMem;
            }
            if ( HalUartErrOk == hal_uart_read(g_UartGps, buf, readByte) )
            {
                readByte = 0;
            }
        }
    }

    return readByte;
}

static uint32_t gpsUartWrite                (uint8_t *txBuff, uint32_t txLen)
{
    size_t writeByte = 0;
    HalBuffer_t buf = {txBuff, txLen};

    if ( HalUartErrOk == hal_uart_transmit_available(g_UartGps, &writeByte) )
    {
        if ( writeByte )
        {
            if ( writeByte > buf.sizeMem )
            {
                writeByte = buf.sizeMem;
            }
            if ( HalUartErrOk == hal_uart_write(g_UartGps, buf, writeByte) )
            {
                writeByte = 0;
            }
        }
    }
    return writeByte;
}

static uint32_t gpsDataReady                (void)
{
    uint32_t readByte = 1;

    if (HalUartErrOk == hal_uart_receive_available(g_UartGps, (size_t *)&readByte) && 0 < readByte)
    {
        return 1;
    }

    return 0;
}

void GpsExe                                 (void)
{
    uint8_t rxBuff[RX_BUFF_MAX_LEN] = {0};
    uint32_t rxLen = 0;
    static uint16_t printTmr = 0;

    /* Read data which is received on gps uart and print on console */
    if (gpsDataReady())
    {
        hal_util_memset(rxBuff, 0x00, sizeof(rxBuff));
        rxLen = gpsUartRead(rxBuff, RX_BUFF_MAX_LEN);

        parseGpsPacket((char*)rxBuff, rxLen);
    }
    // assuming 20ms pooling, print every 2 sec
    if ( printTmr == 100 )
    {
        printTmr = 0;
        g_logger("gps utc_time %d date %u fix %d lat %f long %f speed_kph %.2f num_sat %d alt %.2f\n\r", \
        g_gpsInfo.utcTime, g_gpsInfo.date, g_gpsInfo.fixStatus, g_gpsInfo.latitude , g_gpsInfo.longitude , g_gpsInfo.speed , g_gpsInfo.numOfSatellites , g_gpsInfo.altitude);
    }
    printTmr++;

    LoggerCan_u sysGpsA = {0};
    sysGpsA.gpsA.latitude = g_gpsInfo.latitude; 
    sysGpsA.gpsA.longitude = g_gpsInfo.longitude; 
    logger_can_set(LoggerCan000_SysA, sysGpsA);
}

static void parseGpsPacket                  (char *rxBuff, uint16_t rxLen)
{
    char *startPtr = NULL;
    char *endPtr = NULL;
    uint32_t idx = 0;
    char sentence[SENTENCE_MAX_LEN] = {0};
    uint8_t validSof = 0;

    if(rxBuff == NULL || rxLen == 0)
    {
        return;
    }
    
    hal_util_memset(sentence, 0x00, sizeof(sentence));
    
    while(idx < rxLen)
    {
        // start of the NMEA sentence found
        if( rxBuff[idx] == '$' )
        {
            startPtr = (char*)rxBuff+idx;
            validSof = 1;
        }
        // end of the sentence found
        if(rxBuff[idx] == '\n' && validSof)
        {
            endPtr = (char*)rxBuff+idx;

            // complete sentence found, copy to temporary buffer for parsing
            strncpy(sentence, startPtr, endPtr-startPtr);

            // parse the sentence and update various results
            parseNmeaSentence(UTC_TIME, sentence, strlen(sentence));
            parseNmeaSentence(LATITUDE, sentence, strlen(sentence));
            parseNmeaSentence(LONGITUDE, sentence, strlen(sentence));
            parseNmeaSentence(SPEED_KMPH, sentence, strlen(sentence));
            parseNmeaSentence(DATE, sentence, strlen(sentence));
            parseNmeaSentence(FIX_STATUS, sentence, strlen(sentence));
            parseNmeaSentence(NUM_OF_SATELLITES, sentence, strlen(sentence));
            parseNmeaSentence(ALTITUDE, sentence, strlen(sentence));
        }
        idx++;
    }
}

static void parseNmeaSentence               (uint8_t fieldNum, char *inputStr, uint32_t len)
{
    char resultStr[FIELD_MAX_CHARS] = {0};
    uint32_t startIdx = 0;
    uint32_t endIdx = 0;
    int skipCount = 0;
    uint8_t resultLen = 0;

    if(inputStr == NULL || len == 0)
    {
        return;
    }

    // find the start index
    for(; startIdx < len; startIdx++)
    {
        if(inputStr[startIdx] == ',')
        {
            skipCount++;
            if(skipCount == fieldNum)
            {
                // delimiter was found at exact this index, to skip it during string copy, increment by 1
                startIdx++;
                break;
            }
        }
    }

    // find the end index
    for(endIdx = startIdx + 1; endIdx < len; endIdx++)
    {
        if(inputStr[endIdx] == ',')
        {
            break;
        }
    }

    // copy chars which occur between the start and end from the offset w.r.t original string
    hal_util_memset(resultStr, 0x00, FIELD_MAX_CHARS);
    strncpy(resultStr, inputStr+startIdx, endIdx-startIdx);

    resultLen = strlen(resultStr);

    // quick validate for invalid chars in the result string. Only digits and decimal allowed. It also rejects empty data (eg. ,,,)
    for(int i=0; i<resultLen; i++)
    {
        if( (resultStr[i] >= '0' && resultStr[i] <= '9') || resultStr[i] == '.')
        {
            continue;
        }
        else
        {
            return;     // unwanted char found
        }
    }

    // save the requested field to global stuct
    if((strstr(inputStr, "$GNRMC,")) && isCrcValid(inputStr, strlen(inputStr)))
    {
        switch(fieldNum)
        {
            case UTC_TIME           : {g_gpsInfo.utcTime         = atof(resultStr);  break; }
            case LATITUDE           : {g_gpsInfo.latitude        = atof(resultStr);  break; }
            case LONGITUDE          : {g_gpsInfo.longitude       = atof(resultStr);  break; }
            case DATE               : {g_gpsInfo.date            = atoi(resultStr);  break; }
            
        }
    }
    if((strstr(inputStr, "$GNGGA,")) && isCrcValid(inputStr, strlen(inputStr)))
    {
        switch(fieldNum)
        {
            case FIX_STATUS         : {g_gpsInfo.fixStatus       = atoi(resultStr);  break; }
            case NUM_OF_SATELLITES  : {g_gpsInfo.numOfSatellites = atoi(resultStr);  break; }
            case ALTITUDE           : {g_gpsInfo.altitude        = atof(resultStr);  break; }
        }
    }
    if((strstr(inputStr, "$GNVTG,")) && isCrcValid(inputStr, strlen(inputStr)))
    {
        switch(fieldNum)
        {
            case SPEED_KMPH         : {g_gpsInfo.speed           = atof(resultStr);  break; }
        }
    }
}

GpsInfo_t GpsGetInfo                        (void)
{
    GpsInfo_t tmpStruct = g_gpsInfo;
    return tmpStruct;
}

static uint8_t isCrcValid                   (char *sentence, uint8_t len)
{
    char *startPtr = NULL;
    char *endPtr = NULL;
    char payload[SENTENCE_MAX_LEN] = {0};
    char rxCrcStr[3] = {0};
    int calCrc = 0, rxCrc = 0;
    uint8_t validSof = 0;
    uint8_t retVal = 0;
    
    if(sentence == NULL || len == 0)
    {
        return 0;
    }

    hal_util_memset(payload, 0x00, sizeof(payload));
    hal_util_memset(rxCrcStr, 0x00, sizeof(rxCrcStr));

    for(int itr=0; itr < len; itr++)
    {
        // find the SOF
        if( *(sentence + itr) == '$')
        {
            startPtr = (sentence + itr);
            // CRC must be calculated on the data excluding the $ and *
            startPtr++;
            validSof = 1;
        }
        if( *(sentence + itr) == '*' && validSof == 1)
        {
            endPtr = (sentence + itr);
            
            // extract the payload excluding $ and *
            strncpy(payload, startPtr, endPtr-startPtr);
            
            // extract the CRC string from the NMEA string
            // endPtr is now at * so increment by 1 for pointing to CRC
            strncpy(rxCrcStr, ++endPtr, 2);
            
            // calculate the CRC of the payload
            for(int itr=0; itr < strlen(payload); itr++)
            {
                calCrc ^= payload[itr];
            }
            
            // convert hex in format of ascii string to decimal 
            rxCrc = hexStringToDec(rxCrcStr, strlen(rxCrcStr));
            
            if(rxCrc == calCrc)
            {
                retVal = 1;
            }
            else
            {
                retVal = 0;
            }
        }
    }
    return retVal;
}

static int hexStringToDec                   (char *hexStr, uint8_t len)
{
    int base = 1;
    int decimal = 0;

    if(hexStr == NULL || len == 0)
    {
        return 0;
    }
 
    // Instead of reading characters from Right-To-Left we can also read character from Left-To-Right
    // We just have to initialize p with strlen(c) - 1 and decrement p in each iteration
    for ( int i = len - 1 ; i >= 0 ; --i )
    {
        if ( hexStr[i] >= '0' && hexStr[i] <= '9' )
        {
            decimal += (hexStr[i] - 48) * base;
            base *= 16;
        }
        else if ( hexStr[i] >= 'A' && hexStr[i] <= 'F' )
        {
            decimal += (hexStr[i] - 55) * base;
            base *= 16;
        }
        else if ( hexStr[i] >= 'a' && hexStr[i] <= 'f' )
        {
            decimal += (hexStr[i] - 87) * base;
            base *= 16;
        }
    }
    return decimal;
}

static void logger_dummy                    (char* fmt, ...)
{
    (void) fmt;
}
