/**
*   @file   logger_can.c
*/

#include "logger_can.h"

typedef struct
{
    LoggerCan_u record;
    uint32_t lockBit;       // Single bit is sufficient but 32-bit padding has better perfomance.
} LoggerCanRamTable_t;

static LoggerCanRamTable_t g_ramTable[LoggerCanMax];

// Set the data into RAM table (for state update operation).
LoggerCanErr_n logger_can_set(LoggerCanType_n type, LoggerCan_u data)
{
    LoggerCanErr_n err = LoggerCanErrParam;
    
    if ( type < ( sizeof(g_ramTable)/sizeof(g_ramTable[0]) ) )
    {
        while ( g_ramTable[type].lockBit );
        g_ramTable[type].lockBit = 1;
        g_ramTable[type].record.gen.data[0] = data.gen.data[0];
        g_ramTable[type].record.gen.data[1] = data.gen.data[1];
        g_ramTable[type].record.gen.data[2] = data.gen.data[2];
        g_ramTable[type].record.gen.data[3] = data.gen.data[3];
        g_ramTable[type].record.gen.data[4] = data.gen.data[4];
        g_ramTable[type].record.gen.data[5] = data.gen.data[5];
        g_ramTable[type].record.gen.data[6] = data.gen.data[6];
        g_ramTable[type].record.gen.data[7] = data.gen.data[7];
        g_ramTable[type].lockBit = 0;
        err = LoggerCanErrOk;
    }
    else
    {
        err = LoggerCanErrUnexpected;
    }

    return err;
}

// Get the data from RAM table (for CAN transmit operation).
LoggerCanErr_n logger_can_get(LoggerCanType_n type, LoggerCan_u* data)
{
    LoggerCanErr_n err = LoggerCanErrParam;
    
    if ( type < ( sizeof(g_ramTable)/sizeof(g_ramTable[0]) ) )
    {
        if(LoggerCan000_SysA == type)
        {
            data->sysA = g_ramTable[type].record.sysA;
        }
        else if(LoggerCan008_GpsA == type)
        {
            data->gpsA = g_ramTable[type].record.gpsA;
        }
        else if(LoggerCan009_GpsB == type)
        {
            data->gpsB = g_ramTable[type].record.gpsB;
        }
        else if(LoggerCan011_GsmA == type)
        {
            data->gsmA = g_ramTable[type].record.gsmA;
        }

        err = LoggerCanErrOk;
    }
    else
    {
        err = LoggerCanErrUnexpected;
    }

    return err;
}
