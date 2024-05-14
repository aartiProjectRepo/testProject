/**
 * @file        connection_mgr.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        03 November 2023
 * @author      Aditya P <aditya.prajapati@accoladeelectronics.com>
 *              Diksha J <diksha.jadhav@accoladeelectronics.com>
 *
 * @brief       Connection manager code contains AT command state machine.
 */

#include <string.h>
#include "connection_mgr.h"
#include "at_command_handler.h"
#include "modem_port.h"
#include "net_utility.h"

#include "logger_can.h"

#define MAX_NETWORK_REG_WAIT_TIME_SEC       (5 * 60 * 1000)

typedef enum
{
    CONNECTION_MGR_STATE_IDLE,
    CONNECTION_MGR_STATE_SWITCH_OFF_DEVICE,
    CONNECTION_MGR_STATE_SWITCH_ON_DEVICE,
    CONNECTION_MGR_STATE_STATUS,
    CONNECTION_MGR_STATE_NW_INIT,
    CONNECTION_MGR_STATE_WAIT_FOR_NW_INIT,
    CONNECTION_MGR_STATE_NW_REG,
    CONNECTION_MGR_STATE_WAIT_FOR_NW_REG,
    CONNECTION_MGR_STATE_MAX
} ConnectionMgrStates_n;

typedef struct
{
    ConnectionMgrStates_n state;
    uint32_t timer;
    ConnectionMgrConfig_t config;
} ConnectionMgrContext_t;
typedef enum
{
    NET_INIT_DIS_ECHO,
    NET_INIT_CPIN_QRY,
    NET_INIT_CONFIG_URC_PORT,
    NET_INIT_CONFIG_ALL_URC,
    NET_INIT_CONFIG_CSQ_URC,
    NET_INIT_GET_ICCID,
    NET_INIT_CREG_URC_EN,
    NET_INIT_CGREG_URC_EN,
    NET_INIT_CONFIG_APN,
    NET_INIT_ACT_PDP,
    NET_INIT_MAX_CMD
} NetInitCmd_n;

const AtCommands_t g_netInitTable[NET_INIT_MAX_CMD] =
{
    /* COMMAND                          SUCCESS_RSP    ERROR_RSP     OTHRRSP   NTFN FLG  TMOUT  MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR  WAITTIMER */
    {"E0",                              "OK",           "+CME ERROR",  "\0",    0,      3000,  1,        0,              0,              10},
    {"+CPIN?",                          "OK",           "+CME ERROR",  "\0",    0,      3000,  1,        0,              0,              10},
    {"+QURCCFG=\"urcport\",\"uart1\"",  "\0",           "+CME ERROR",  "\0",    0,      300,   5,        0,              0,              10}, /*Enable URC PORT */
    {"+QINDCFG=\"all\",1,1",            "OK",           "+CME ERROR",  "\0",    0,      300,   5,        0,              0,              10}, /*Enable All URC */
    {"+QINDCFG=\"csq\",1,1",            "OK",           "+CME ERROR",  "\0",    0,      300,   5,        0,              0,              10}, /*Enable CSQ URC */
    {"+QCCID",                          "OK",           "+CME ERROR",  "*",     0,      300,   5,        0,              0,              10},
    {"+CREG=1",                         "OK",           "+CME ERROR",  "\0",    0,      300,   1,        0,              0,              10},
    {"+CGREG=1",                        "OK",           "+CME ERROR",  "\0",    0,      300,   1,        0,              0,              10},
    {"+QICSGP=",                        "OK",           "ERROR",       "\0",    0,      300,   1,        0,              0,              10},
    {"+QIACT=1",                        "OK",           "+CME ERROR",  "\0",    0,      150000,1,        0,              0,              10},
};

/**
 * @brief   The group of functions fill, store and call back is helping function for net init table.
 */
static uint16_t fillInitCommand(uint8_t *buffer, int16_t offset, const AtCommands_t *aTCmdTble, int8_t commandIdx);
/**
 * @brief   The group of functions fill, store and call back is helping function for net init table.
 */
static int8_t storeInitResponse(const AtCommands_t *cmdTbl, uint8_t commandIdx, uint32_t bufferLen, uint8_t *buffer, int8_t status);
/**
 * @brief   The group of functions fill, store and call back is helping function for net init table.
 */
static int16_t initCallBack(uint8_t commandIdx, uint8_t status, uint32_t bufferLen, void *buffer);

typedef enum
{
    CONN_STATUS_CREG_QRY,
    CONN_STATUS_CGREG_QRY,
    CONN_STATUS_COPS_QRY,
    CONN_STATUS_PDP_ACT_QRY,
    CONN_STATUS_MAX_CMD
} ConnStatusCmd_n;

const AtCommands_t g_connStatusTable[CONN_STATUS_MAX_CMD] =
{
    /* COMMAND    SUCCESS_RSP  ERROR_RSP      OTHRRSP  NTFN FLG TMOUT    MAXRTRYCNT   MAXNTFNRTRYCNT  STOPONERROR   WAITTIMER */
    {"+CREG?",    "+CREG:",    "+CME ERROR",  "\0",    0,       3000,    1,           0,              0,            10},
    {"+CGREG?",   "+CGREG:",   "+CME ERROR",  "\0",    0,       3000,    1,           0,              0,            10},
    {"+COPS?",    "OK",        "+CME ERROR",  "*",     0,       180000,  1,           0,              0,            10},
    {"+QIACT?",   "OK",        "+CME ERROR",  "*",     0,       150000,  1,           0,              0,            10},
};

/**
 * @brief   The group of functions fill, store and call back is helping function for connection status table.
 */
static uint16_t fillConnStatusCommand(uint8_t *buffer, int16_t offset, const AtCommands_t *aTCmdTble, int8_t commandIdx);
/**
 * @brief   The group of functions fill, store and call back is helping function for connection status table.
 */
static int8_t storeConnStatusResponse(const AtCommands_t *cmdTbl, uint8_t commandIdx, uint32_t bufferLen, uint8_t *buffer, int8_t status);
/**
 * @brief   The group of functions fill, store and call back is helping function for connection status table.
 */
static int16_t connStatusCallBack(uint8_t commandIdx, uint8_t status, uint32_t bufferLen, void *buffer);

/**
 * @brief   Handle the control Urc by this function. This is call back function.
 */
static uint16_t controlUrcParser(uint8_t *buffer, uint16_t len, uint8_t *outBuff, uint16_t *outLen);
/**
 * @brief   Handle the CREG URC by this function. This is call back function.
 */
static uint16_t cregUrcParser(uint8_t *buffer, uint16_t len, uint8_t *outBuff, uint16_t *outLen);
/**
 * @brief   Handle the CGREG URC by this function. This is call back function.
 */
static uint16_t cgregUrcParser(uint8_t *buffer, uint16_t len, uint8_t *outBuff, uint16_t *outLen);

/**
 * @brief   Clear the connection manager info sturct and set to default state.
 */
static void clearConnection(void);

ConnectionMgrContext_t g_connectionMgrContext;
ConnectionMgrInfo_t g_connectionMgrInfo;

UrcTable_t g_connMgrUrcTable[] =
{
    {"+QIND: ",     &controlUrcParser}, // Received URC for csq, PB DONE etc
    {"+CREG: ",     &cregUrcParser},    // Received URC for creg
    {"+CGREG: ",    &cgregUrcParser},  // Received URC for cgreg
};


ConnMgrErrorCode_n ConnectionMgrInit(void)
{
    memset((void *)&g_connectionMgrInfo, 0x00, sizeof(g_connectionMgrInfo));
    sprintf((char *)g_connectionMgrInfo.operatorName, "DEFAULT");
    g_connectionMgrContext.state = CONNECTION_MGR_STATE_SWITCH_OFF_DEVICE;
    AtRegisterUrc(g_connMgrUrcTable, sizeof(g_connMgrUrcTable) / sizeof(g_connMgrUrcTable[0]));
    sprintf((char *)g_connectionMgrContext.config.apnName, "sensem2m2"); 
    return CONN_MGR_SUCCESS;
}

ConnectionMgrInfo_t ConnectionMgrGetInfo(void)
{
    return g_connectionMgrInfo;
}

bool ConnectionMgrIsNetAvailable(void)
{
    return g_connectionMgrInfo.networkStatus;
}

ConnMgrErrorCode_n ConnectionMgrExe(void)
{
    AtError_n atErrorResp = AT_FAILED;

    switch (g_connectionMgrContext.state)
    {
    case CONNECTION_MGR_STATE_IDLE:
        break;

    case CONNECTION_MGR_STATE_SWITCH_OFF_DEVICE:
        ModemRstKeyHigh();
        NETWORK_PRINT_DEBUG("power down\r\n");
        RESET_TIMER(g_connectionMgrContext.timer, 1 * 1000);
        g_connectionMgrContext.state = CONNECTION_MGR_STATE_SWITCH_ON_DEVICE;
        break;

    case CONNECTION_MGR_STATE_SWITCH_ON_DEVICE:
        if (!IS_TIMER_ELAPSED(g_connectionMgrContext.timer))
        {
            break;
        }
        ModemRstKeyLow();
        NETWORK_PRINT_DEBUG("power up\r\n");
        g_connectionMgrContext.state = CONNECTION_MGR_STATE_STATUS;
        break;

    case CONNECTION_MGR_STATE_STATUS:
        if (!IS_TIMER_ELAPSED(g_connectionMgrContext.timer))
        {
            break;
        }
        g_connectionMgrContext.state = CONNECTION_MGR_STATE_NW_INIT;
        RESET_TIMER(g_connectionMgrContext.timer, 10 * 1000);
        g_connectionMgrInfo.modemReady = 1;
        break;

    case CONNECTION_MGR_STATE_NW_INIT:
        if (!IS_TIMER_ELAPSED(g_connectionMgrContext.timer))
        {
            break;
        }
        atErrorResp = AtStart(g_netInitTable, (uint8_t)NET_INIT_MAX_CMD, fillInitCommand, storeInitResponse, initCallBack);
        if (AT_SUCCESS == atErrorResp)
        {
            RESET_TIMER(g_connectionMgrContext.timer, 60 * 1000);
            g_connectionMgrContext.state = CONNECTION_MGR_STATE_WAIT_FOR_NW_INIT;
        }
        break;

    case CONNECTION_MGR_STATE_WAIT_FOR_NW_INIT:
        if (!IS_TIMER_ELAPSED(g_connectionMgrContext.timer))
        {
            break;
        }
        break;

    case CONNECTION_MGR_STATE_NW_REG:
        atErrorResp = AtStart(g_connStatusTable, (uint8_t)CONN_STATUS_MAX_CMD, fillConnStatusCommand, storeConnStatusResponse, connStatusCallBack);
        if (AT_SUCCESS == atErrorResp)
        {
            RESET_TIMER(g_connectionMgrContext.timer, 2 * 60 * 1000);
            g_connectionMgrContext.state = CONNECTION_MGR_STATE_WAIT_FOR_NW_REG;
        }
        break;

    case CONNECTION_MGR_STATE_WAIT_FOR_NW_REG:
        if (!IS_TIMER_ELAPSED(g_connectionMgrContext.timer))
        {
            break;
        }
        g_connectionMgrContext.state = CONNECTION_MGR_STATE_NW_REG;
        break;

    default:
        NETWORK_PRINT_DEBUG("reached to undefine state, %d\r\n", __LINE__);
        break;
    }
    return CONN_MGR_SUCCESS;
}

uint16_t fillInitCommand(uint8_t *buffer, int16_t offset, const AtCommands_t *aTCmdTble, int8_t commandIdx)
{
    int32_t length = 0;
    char *bufferPtr = (char *)&buffer[offset];

    length += sprintf(&bufferPtr[length], "AT%s", aTCmdTble->command);
    if (NET_INIT_CONFIG_APN == commandIdx)
    {
        length += sprintf(&bufferPtr[length], "1,1,\"%s\",\"\",\"\",0", g_connectionMgrContext.config.apnName);
    }
    length += sprintf(&bufferPtr[length], "\r\n", aTCmdTble->command);
    return length;
}

static int8_t storeInitResponse(const AtCommands_t *cmdTbl, uint8_t commandIdx, uint32_t bufferLen, uint8_t *buffer, int8_t status)
{
    return 0;
}

static int16_t initCallBack(uint8_t commandIdx, uint8_t status, uint32_t bufferLen, void *buffer)
{
    if (AT_CB_ALL_CMD_OVR == status)
    {
        g_connectionMgrContext.state = CONNECTION_MGR_STATE_NW_REG;
    }
    return 0;
}

uint16_t fillConnStatusCommand(uint8_t *buffer, int16_t offset, const AtCommands_t *aTCmdTble, int8_t commandIdx)
{
    int32_t length = 0;

    length = sprintf((char *)&buffer[offset], "AT%s\r\n", aTCmdTble->command);
    return length;
}

static int8_t storeConnStatusResponse(const AtCommands_t *cmdTbl, uint8_t commandIdx, uint32_t bufferLen, uint8_t *buffer, int8_t status)
{
    char *workingPtr = NULL;
    char *startPtr = NULL;
    char *endPtr = NULL;

    if (CONN_STATUS_COPS_QRY == commandIdx)
    { 
        if (NULL != (workingPtr = strstr((char *)buffer, "+COPS: "))) /*  +COPS: 0   or +COPS: 0,0,"AIRTEL",7 */
        {
            if (NULL != (workingPtr = strstr(workingPtr, ",\"")))
            {
                workingPtr += 2;
                startPtr = workingPtr;
                if (NULL != (endPtr = strstr(workingPtr, "\"")))
                {
                    if (endPtr > startPtr)
                    {
                        memcpy((char *)g_connectionMgrInfo.operatorName, startPtr, endPtr - startPtr);
                        g_connectionMgrInfo.accessTechnology = atoi(endPtr + 2);
                        NETWORK_PRINT_TRACE("received op name: %s and access tech: %d\r\n", g_connectionMgrInfo.operatorName, g_connectionMgrInfo.accessTechnology);
                    }
                    else
                    {
                        NETWORK_PRINT_ERROR("op name parsing error %d\r\n", __LINE__);
                    }
                    if (NULL != (workingPtr = strstr(endPtr, ",")))
                    {
                        g_connectionMgrInfo.accessTechnology = atoi(++workingPtr);
                    }
                }
            }
        }
    }

    if (CONN_STATUS_PDP_ACT_QRY == commandIdx)
    {
        if (NULL != (workingPtr = strstr((char *)buffer, "+QIACT: "))) /* +QIACT: 1,1,1,"10.142.150.202" */
        {
            if (NULL != (workingPtr = strstr(workingPtr, ",\"")))
            {
                startPtr = workingPtr + 2;
                if (NULL != (endPtr = strstr(startPtr, "\"")))
                {
                    if (endPtr > startPtr)
                    {
                        memcpy((char *)g_connectionMgrInfo.ipAddress, startPtr, endPtr - startPtr);
                        NETWORK_PRINT_TRACE("received ip address: %s\r\n", g_connectionMgrInfo.ipAddress);
                    }
                    else
                    {
                        NETWORK_PRINT_ERROR("ip address parsing error %d\r\n", __LINE__);
                    }
                }
            }
        }
    }
    return 0;
}

static int16_t connStatusCallBack(uint8_t commandIdx, uint8_t status, uint32_t bufferLen, void *buffer)
{
    return 0;
}

uint16_t controlUrcParser(uint8_t *buffer, uint16_t len, uint8_t *outBuff, uint16_t *outLen)
{
    char *workingPtr = NULL;
    char *startPtr = NULL;
    char *endPtr = NULL;
    const char *urcString = "+QIND: ";
    uint16_t parseLen = 0;

    if (NULL != (startPtr = strstr((char *)buffer, urcString)))
    {
        if (NULL != (workingPtr = strstr(startPtr, "+QIND: \"csq\","))) /* +QIND: "csq",28,99 */
        {
            workingPtr += strlen("+QIND: \"csq\",");
            g_connectionMgrInfo.csq = atoi(workingPtr);

            if (NULL != (endPtr = strstr(workingPtr, "\r\n")))
            {
                endPtr += 2;
            }
            else
            {
                endPtr = workingPtr + 2;
            }
        }
        if (NULL == endPtr)
        {
            endPtr = startPtr + strlen(urcString);
        }
        parseLen = CalculateUrcParseLen((char *)buffer, startPtr, endPtr, (char *)outBuff, outLen, len);
        return parseLen;
    }
    return 0;
}

uint16_t cregUrcParser(uint8_t *buffer, uint16_t len, uint8_t *outBuff, uint16_t *outLen)
{
    char *workingPtr = NULL;
    char *startPtr = NULL;
    char *endPtr = NULL;
    const char *urcString = "+CREG: ";
    uint16_t parseLen = 0;

    if (NULL != (startPtr = strstr((char *)buffer, urcString)))
    {
        if (NULL != (workingPtr = strstr(startPtr, ",")))
        {
            g_connectionMgrInfo.creg = atoi(++workingPtr);
            NETWORK_PRINT_TRACE("CREG: %d\r\n", g_connectionMgrInfo.creg);
            endPtr = workingPtr + 2; // move to end of urc i.e "/r/n"
        }
        else
        {
            workingPtr = startPtr + strlen(urcString);
            g_connectionMgrInfo.creg = atoi(workingPtr);
            NETWORK_PRINT_TRACE("CREG: %d\r\n", g_connectionMgrInfo.creg);
            endPtr = workingPtr + 2; // move to end of urc i.e "/r/n"
        }
        if (NULL == endPtr)
        {
            endPtr = startPtr + strlen(urcString);
        }

        parseLen = CalculateUrcParseLen((char *)buffer, startPtr, endPtr, (char *)outBuff, outLen, len);

        if (((1 == g_connectionMgrInfo.creg) || (5 == g_connectionMgrInfo.creg)) &&
            ((1 == g_connectionMgrInfo.cgreg) || (5 == g_connectionMgrInfo.cgreg)))
        {
            g_connectionMgrInfo.networkStatus = true;
        }
        else
        {
            clearConnection();
        }
        return parseLen;
    }
    return 0;
}

uint16_t cgregUrcParser(uint8_t *buffer, uint16_t len, uint8_t *outBuff, uint16_t *outLen)
{
    char *workingPtr = NULL;
    char *startPtr = NULL;
    char *endPtr = NULL;
    const char *urcString = "+CGREG: ";
    uint16_t parseLen = 0;

    if (NULL != (startPtr = strstr((char *)buffer, urcString)))
    {
        if (NULL != (workingPtr = strstr(startPtr, ","))) /* if received string is +CREG: 1,1 or +CGREG: 1,5 */
        {
            g_connectionMgrInfo.cgreg = atoi(++workingPtr);
            NETWORK_PRINT_TRACE("CGREG: %d\r\n", g_connectionMgrInfo.cgreg);
            endPtr = workingPtr + 2; // move to end of urc i.e "/r/n"
        }
        else /* if received string is +CREG: 1 or +CGREG: 5 */
        {
            workingPtr = startPtr + strlen(urcString);
            g_connectionMgrInfo.cgreg = atoi(workingPtr);
            NETWORK_PRINT_TRACE("CGREG: %d\r\n", g_connectionMgrInfo.cgreg);
            endPtr = workingPtr + 2; // move to end of urc i.e "/r/n"
        }
        if (NULL == endPtr)
        {
            endPtr = startPtr + strlen(urcString);
        }

        parseLen = CalculateUrcParseLen((char *)buffer, startPtr, endPtr, (char *)outBuff, outLen, len);

        if (((1 == g_connectionMgrInfo.creg) || (5 == g_connectionMgrInfo.creg)) &&
            ((1 == g_connectionMgrInfo.cgreg) || (5 == g_connectionMgrInfo.cgreg)))
        {
            g_connectionMgrInfo.networkStatus = true;
        }
        else
        {
            clearConnection();
        }
        return parseLen;
    }
    return 0;
}

static void clearConnection(void)
{
    g_connectionMgrInfo.networkStatus = false;
    g_connectionMgrInfo.accessTechnology = 0; // needs to check with datasheet
    memset(g_connectionMgrInfo.ipAddress, 0x00, sizeof(g_connectionMgrInfo.ipAddress));
    sprintf((char *)g_connectionMgrInfo.operatorName, "DEFAULT");
}

void ConnectionMgrPrintInfo(void)
{
    ConnectionMgrInfo_t connInfo = ConnectionMgrGetInfo();
    NETWORK_PRINT_INFO("[NET] network status: %d, CREG: %d, CGREG: %d, CSQ: %d, op name: %s %d, IP: %s \r\n",
           connInfo.networkStatus, connInfo.creg, connInfo.cgreg, connInfo.csq, connInfo.operatorName,
           connInfo.accessTechnology, connInfo.ipAddress);

    LoggerCan_u sysGsmA = {0};
    strncpy( sysGsmA.gsmA.opratingName, connInfo.operatorName, sizeof(sysGsmA.gsmA.opratingName) ); 
    logger_can_set(LoggerCan011_GsmA, sysGsmA);
}