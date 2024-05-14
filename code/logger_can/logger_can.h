/**
*   @file   logger_can.h
*/

#include <stdint.h>

typedef struct {
    uint8_t data[8];
} LooggerCanGeneric_t;

typedef struct {
    uint32_t uptimeMs;
    uint32_t epoch;
} LoggerCan000_SysA_t;

typedef struct {
    float latitude;
    float longitude;
} LoggerCan008_GpsA_t;

typedef struct {
    uint32_t bitInit : 1;
    uint32_t bitRunning : 1;
    uint32_t bitsReserved : 30;
    uint32_t countTx;
} LoggerCan009_GpsB_t;

typedef struct {
    uint32_t countRx;
    uint32_t countRxLost;
} LoggerCan010_GpsC_t;

typedef struct {
    uint8_t opratingName[8];
} LoggerCan011_GsmA_t;

typedef enum
{
    LoggerCan000_SysA = 0,
    LoggerCan008_GpsA = 8,
    LoggerCan009_GpsB = 9,
    LoggerCan010_GpsC = 10,
    LoggerCan011_GsmA = 11,
    LoggerCanMax
} LoggerCanType_n;

// TODO: add structs and corresponding enum entries as required....

typedef union
{
    LooggerCanGeneric_t gen;
    LoggerCan000_SysA_t sysA;
    LoggerCan008_GpsA_t gpsA;
    LoggerCan009_GpsB_t gpsB;
    LoggerCan010_GpsC_t gpsC;
    LoggerCan011_GsmA_t gsmA;
} LoggerCan_u;

typedef enum 
{
    LoggerCanErrOk,
    LoggerCanErrParam,
    LoggerCanErrUnexpected
} LoggerCanErr_n;

// Set the data into RAM table (for state update operation).
LoggerCanErr_n logger_can_set(LoggerCanType_n type, LoggerCan_u data);

// Get the data from RAM table (for CAN transmit operation).
LoggerCanErr_n logger_can_get(LoggerCanType_n type, LoggerCan_u* data);
