/**
 * @file        at_command_handler.c
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
 * @brief       AT Handler source for handling commands for modem
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "at_command_handler.h"
#include "modem_port.h"

#define MAX_URC_MODULES 24
#define MAX_RECOVERY_TIME 60 * 60 * 1000 //  don't reboot the system within MAX_RECOVERY_TIME
#define MAX_APPEDED_COUNT 20             //  don't reboot the system until reached to max count
#define MAX_REBOOT_TIME_PUB 60 * 1000    //  modem reboot after MAX_RECOVERY_TIME_PWR, if modem power off during publish
#define MAX_TOKEN_LIMIT 50

// forward declared
typedef uint32_t (*fnPtrSerialRead)(uint8_t *rxBuff, uint32_t maxBuffSize);
typedef uint32_t (*fnPtrSerialWrite)(uint8_t *txBuff, uint32_t txLen);

typedef struct
{
    uint8_t state;                      /* state is used to store the current state of the at module. */
    uint8_t respRetryCount;             /* respRetryCount is used to store the retry count of the response. */
    uint8_t notificationRetryCount;     /* notificationRetryCount is used to store the retry count of the notification. */
    uint32_t timer;                     /* timer is used to wait a perticuler time for either response or notification. */
    int8_t currentCmdIndex;             /* currentCmdIndex is used to store the command index which is running. */
    uint8_t maxCmd;                     /* maxCmd is used to store the max command count, which is fire on gsm uart. */
    uint32_t waitTimerForNxtCmd;        /* waitTimerForNxtCmd is used to store timer to fire next command*/
    const AtCommands_t *cmdTable;       /* cmdTable is used to at command table, which we have to fire. */
    fnPtrFillCmd fillCmdCallBack;       /* fill_cmd_ptr is used to store the adress of the function, which is used to fill the at command. */
    fnPtrEventCallBack eventCallBack;   /* call_back_ptr is used to store the adress of the function, which is used to give callback the parent. */
    funPtrStoreResp storeDataCallBack;  /* call_back_ptr is used to store the adress of the function, which is used to store the data */
    fnPtrSerialRead uartRead;           /*  */
    fnPtrSerialWrite uartWrite;
} AtContext_t;

uint16_t g_appendLength = 0;

/**
 * @brief   modemURCHandler, Parses the URC.
 * @param   buffer, Len
 * @return  Length of the response.
 */
uint16_t modemURCHandler(uint8_t *buffer, uint16_t len);

/**
 * @brief   handleErrForOtherResp, used to handle error responses.
 * @param   None.
 * @return  None.
 */
void handleErrForOtherResp(void);

/*
 * @brief   checkStopFlag, This API checks the stop flag and then change AT state according to that.
 * @param   AT commamd_table
 * @return  It configures AT State.
 */
static void checkStopFlag(const AtCommands_t *commandTable);

/*
 * @brief   readModemData, This func reads data received from modem and pass to the parsing.
 * @param   -
 * @return  It 0 if successful execution.
 */
static int8_t readModemData(void); // Returned minus value for errors

/*
 * @brief   trimString, This API is used to trim received string from Modem
 * @param   pointer to the string, string Length
 * @return  It returns string length.
 */
static uint16_t trimString(uint8_t *string, uint16_t stringLen);

UrcTable_t g_extraTable[MAX_URC_MODULES];
static uint8_t g_cmdTxBuff[MAX_AT_BUFF_SIZE];
static uint8_t g_cmdRxBuff[MAX_AT_BUFF_SIZE];
static uint8_t g_outBuffer[MAX_RCVD_BUF_LEN];

AtContext_t g_atContext;
static int16_t g_extraTbleCnt = 0;

AtError_n AtRegisterUrc(UrcTable_t *extraTableEntries, int count)
{
    if (NULL == extraTableEntries)
    {
        return AT_INVALID_MEMORY;
    }

    if ((g_extraTbleCnt + count) > MAX_URC_MODULES)
    {
        NETWORK_PRINT_ERROR("Failed to register URC, total size needed %d \r\n", (g_extraTbleCnt + count));
        return AT_INVALID_MEMORY;
    }

    for (int iterator = 0; iterator < count; iterator++, g_extraTbleCnt++)
    {
        g_extraTable[g_extraTbleCnt].string = extraTableEntries[iterator].string;
        g_extraTable[g_extraTbleCnt].urcHandler = extraTableEntries[iterator].urcHandler;
    }
    return AT_SUCCESS;
}

int16_t AtInit(void)
{
    memset(g_extraTable, 0x00, sizeof(g_extraTable));
    memset(g_cmdTxBuff, 0x00, MAX_AT_BUFF_SIZE);
    memset(g_cmdRxBuff, 0x00, MAX_AT_BUFF_SIZE);
    memset(g_outBuffer, 0x00, MAX_RCVD_BUF_LEN);
    memset(&g_atContext, 0, sizeof(g_atContext));
    ModemInit();
    g_extraTbleCnt = 0;
    g_atContext.uartRead = ModemRead;
    g_atContext.uartWrite = ModemWrite;
    AtForceStop();

    return 0xAA; // Putting fixed val as GsmHandle. This is legacy code requirement. Previously this was obtained from uart_init in legacy code
}

void AtForceStop(void)
{
    g_atContext.state = AT_STATE_IDLE;
}

AtError_n AtStart(const AtCommands_t *atTable, uint8_t maxCmd, fnPtrFillCmd fillCmdFuncPtr, funPtrStoreResp storeDataFuncPtr, fnPtrEventCallBack callBackFuncPtr)
{
    /* If received parameter are invalid then return error. */
    if (NULL == atTable || NULL == fillCmdFuncPtr || NULL == callBackFuncPtr || NULL == storeDataFuncPtr || 0 == maxCmd)
    {
        return AT_INVALID_MEMORY;
    }
    if (AtGetState() != AT_STATE_IDLE)
    {
        return AT_FAILED;
    }

    /* Assign all necessary parameters to the corresponding valued before doing a work. */
    g_atContext.respRetryCount = 0;
    g_atContext.notificationRetryCount = 0;
    g_atContext.timer = 0L;
    g_atContext.currentCmdIndex = -1;
    g_atContext.fillCmdCallBack = fillCmdFuncPtr;
    g_atContext.cmdTable = atTable; // TODO:Need to remove warning.
    g_atContext.eventCallBack = callBackFuncPtr;
    g_atContext.maxCmd = maxCmd;
    g_atContext.storeDataCallBack = storeDataFuncPtr;
    g_atContext.state = AT_STATE_SELECT_COMMAND;

    NETWORK_PRINT_DEBUG("start AT\r\n");
    return AT_SUCCESS;
}

int8_t AtExe(void)
{
    uint32_t offset;                         /* offset is used to indicate the valid fill length in the cmd_buffer.  */
    const AtCommands_t *atCmdTable = NULL; /* at_cmd_table is used to store the command table which is fire on uart. */

    if (ModemDataReady())
    {
        readModemData(); /* Read data which is received on gsm uart. */
    }

    switch (g_atContext.state)
    {
    case AT_STATE_IDLE:
        break;

    case AT_STATE_SELECT_COMMAND:
    {
        /* Set wait for response timer and increment retry count. */
        /* Select command. */
        g_atContext.currentCmdIndex++;

        /* If command is over then tell to the caller function that all command over. */
        if (g_atContext.currentCmdIndex >= g_atContext.maxCmd)
        {
            NETWORK_PRINT_DEBUG("CMD OVR,GV CB\n\r");
            g_atContext.state = AT_STATE_IDLE;
            g_atContext.eventCallBack(g_atContext.currentCmdIndex, AT_CB_ALL_CMD_OVR, 0, NULL);
            break;
        }

        /* Point to the command which is fire. */
        atCmdTable = &(g_atContext.cmdTable[g_atContext.currentCmdIndex]);

        /* Clear required variables.  */
        g_atContext.respRetryCount = 0;
        g_atContext.notificationRetryCount = 0;
        g_atContext.state = AT_STATE_FILL_N_SND_CMD;
    }
    break;

    case AT_STATE_FILL_N_SND_CMD:
    {
        /* Point to the command which is fire. */
        atCmdTable = &(g_atContext.cmdTable[g_atContext.currentCmdIndex]);

        if (NULL == atCmdTable)
        {
            break;
        }

        /* Wait timer for next timer. */
        if (!IS_TIMER_ELAPSED(g_atContext.waitTimerForNxtCmd))
        {
            break;
        }
        RESET_TIMER(g_atContext.waitTimerForNxtCmd, atCmdTable->waitTimerForNextCmd);

        offset = 0;
        memset(g_cmdTxBuff, 0x00, MAX_AT_BUFF_SIZE);
        if (NULL != g_atContext.fillCmdCallBack)
        {
            /* Fill remaing comand which is change at runtime hence we have to fill at run time. */
            offset += g_atContext.fillCmdCallBack(g_cmdTxBuff, offset, &(g_atContext.cmdTable[g_atContext.currentCmdIndex]), g_atContext.currentCmdIndex);
        }

        // To print packet less than 100 bytes
        if (offset < 100)
        {
            NETWORK_PRINT_DEBUG("GSM_TX(%ld): |%s|\r\n", offset, g_cmdTxBuff);
        }
        else
        {
            NETWORK_PRINT_DEBUG("GSM_TX(%ld):Length is greater than 100 Byte, data will not be printed\r\n", offset);
        }

        /***** Write data on uart.**********/
        ModemWrite(g_cmdTxBuff, offset);
        RESET_TIMER(g_atContext.timer, atCmdTable->timeOutMs);
        g_atContext.respRetryCount++;

        if (atCmdTable->successResponse != NULL)
        {
            g_atContext.state = AT_STATE_WAIT_FOR_RSP;
        }
        else
        {
            g_atContext.state = AT_STATE_SELECT_COMMAND;
        }
    }
    break;

    case AT_STATE_WAIT_FOR_RSP:
    {

        if (!IS_TIMER_ELAPSED(g_atContext.timer))
        {
            break;
        }

        /* Point to the command which is fire. */
        atCmdTable = &(g_atContext.cmdTable[g_atContext.currentCmdIndex]);

        /* Device is wating for gsm response. Check response retry is over or not after a perticuler time. */
        if (atCmdTable->maxRetry <= g_atContext.respRetryCount)
        {
            /* If response retry over then inform to the caller function and do nuthing i.e. go to IDLE. */
            checkStopFlag(atCmdTable);
        }
        else
        {
            /* If response retry count is not over then do retry for a current command. */
            g_atContext.state = AT_STATE_FILL_N_SND_CMD;
        }
    }
    break;

    case AT_STATE_WAIT_FOR_NTFN:
    {
        if (!IS_TIMER_ELAPSED(g_atContext.timer))
        {
            break;
        }

        /* Point to the command which is fire. */
        atCmdTable = &(g_atContext.cmdTable[g_atContext.currentCmdIndex]);

        /* Until notification is not received, so increment notification retry count. */
        g_atContext.notificationRetryCount++;

        /* Device is waiting for GSM notification. Check notification retry is over or not after a particular time. */
        if (atCmdTable->maxNotificationRetry <= g_atContext.notificationRetryCount)
        {
            /* If notification retry over then inform to the caller function and do nuthing i.e. go to IDLE. */
            checkStopFlag(atCmdTable);
        }
        else
        {
            /* If notification retry count is not over then do retry for a current command. */
            g_atContext.state = AT_STATE_FILL_N_SND_CMD;
        }
    }
    break;
    }
    return 0;
}

static void checkStopFlag(const AtCommands_t *commandTable)
{
    /* If errorStopFlg is set the give call back to the caller so caller can handle this and at go in IDLE state. */
    if (1 == commandTable->errorStopFlg)
    {
        g_atContext.state = AT_STATE_IDLE;
        g_atContext.eventCallBack(g_atContext.currentCmdIndex, AT_CB_ERROR_STOP, 0, NULL);
    }
    else
    {
        g_atContext.eventCallBack(g_atContext.currentCmdIndex, AT_CB_ERROR_NO_STOP, 0, NULL);
        /* If errorStopFlg is not set then give callback to the caller and fire next command because there is no need to stop. */
        g_atContext.state = AT_STATE_SELECT_COMMAND;
    }
}

static int8_t readModemData(void) // Minus value retruned
{
    uint8_t *rcvdBuffer = g_cmdRxBuff;
    uint16_t rcvdLength = 0; /* rcvdLength is used to stored the received number of bytes on gsm uart. */

    const AtCommands_t *atCmdTable = &(g_atContext.cmdTable[g_atContext.currentCmdIndex]);
    uint8_t *tokens[MAX_TOKEN_LIMIT];
    AtCallBack_n status = AT_CB_ERROR_STOP;
    int i = 0;
    int tokenLength;
    int multiStepResponseError = 0;
    int8_t retVal = 0;
    static uint32_t appendCount = 0;
    static char storedBytes[2] = {0};

    if (g_appendLength)
    {
        rcvdLength = g_atContext.uartRead(((uint8_t *)rcvdBuffer) + g_appendLength, MAX_AT_BUFF_SIZE - g_appendLength);
        rcvdLength = rcvdLength + g_appendLength;
        appendCount++;
        NETWORK_PRINT_DEBUG("Count: %lu A_GSM_Rx(%lu): |%s|\r\n", appendCount, rcvdLength,
                            ((rcvdLength > 100) ? "Length is greater than 100 Byte, data will not be printed\r\n" : (char *)rcvdBuffer));
    }
    else
    {
        memset(rcvdBuffer, 0x00, MAX_AT_BUFF_SIZE);
        if (storedBytes[0] == '\0')
        {
            rcvdLength = g_atContext.uartRead(rcvdBuffer, MAX_AT_BUFF_SIZE);
        }
        else
        {
            memcpy(rcvdBuffer, storedBytes, sizeof(storedBytes));
            rcvdLength = g_atContext.uartRead(rcvdBuffer + sizeof(storedBytes), MAX_AT_BUFF_SIZE - sizeof(storedBytes));
            rcvdLength = rcvdLength + sizeof(storedBytes);
            memset(storedBytes, 0x00, sizeof(storedBytes));
        }
        NETWORK_PRINT_DEBUG("GSM_Rx(%lu): |%s|\r\n", rcvdLength,
                           ((rcvdLength > 100) ? "Length is greater than 100 Byte, data will not be printed\r\n" : (char *)rcvdBuffer));
    }

    if (rcvdBuffer[rcvdLength - 2] == '\r' && rcvdBuffer[rcvdLength - 1] == '\n')
    {
        /* Rare condition, if \r\n of next URC is found at the end of GSM_RX data, so \r\n stored and
         * will be appended at the front of next GSM_RX data.
         * e.g expected {\r\nURC1\r\n\r\nURC2\r\n} or {> \r\nURC1\r\n}
         *     received {\r\nURC1\r\n\r\n} or {> \r\n}  */
        if ((rcvdBuffer[rcvdLength - 4] == '\r' && rcvdBuffer[rcvdLength - 3] == '\n') ||
            (rcvdBuffer[rcvdLength - 4] == '>' && rcvdBuffer[rcvdLength - 3] == ' '))
        {
            storedBytes[0] = rcvdBuffer[rcvdLength - 4];
            storedBytes[1] = rcvdBuffer[rcvdLength - 3];
            NETWORK_PRINT_DEBUG("0x0A,0x0D after a complete urc, stored data |%s|\r\n", storedBytes);
        }
        appendCount = 0;
        g_appendLength = 0;
    }
    else if (rcvdBuffer[rcvdLength - 2] == '>' && rcvdBuffer[rcvdLength - 1] == ' ')
    {
        appendCount = 0;
        g_appendLength = 0;
    }
    else
    {
        if (rcvdLength >= MAX_AT_BUFF_SIZE)
        {
            NETWORK_PRINT_DEBUG("Append length reached to maximum[%lu]\r\n", rcvdLength);
        }
        g_appendLength = rcvdLength;
        return 0;
    }

    if (0 == rcvdLength)
    {
        return AT_FAILED;
    }

    rcvdLength = modemURCHandler(rcvdBuffer, rcvdLength);

    NETWORK_PRINT_TRACE("AURC_RX(%lu): |%s|\r\n", rcvdLength, rcvdBuffer);

    if (0 >= rcvdLength)
    {
        return AT_FAILED;
    }

    rcvdLength = trimString(rcvdBuffer, rcvdLength);

    tokens[i] = (uint8_t *)strtok((char *)rcvdBuffer, "\r\n");

    /* If received bytes 1 length and toekn[0] is null then return for error handling like adress trap. */
    if (tokens[0] == NULL)
    {
        return AT_FAILED;
    }

    while ((tokens[i] != NULL) && i < (MAX_TOKEN_LIMIT - 1))
    {
        tokens[++i] = (uint8_t *)strtok(NULL, "\r\n");
    }

    i = 0;
    if (tokens[0] == NULL)
    {
        return AT_FAILED;
    }

    do
    {
        tokenLength = strlen((char *)tokens[i]);

        /* We have fire a AT command and now we are waiting for that response for that case we want to check received response is success,
         * error, other response
         * If we not fire at command that means we are in data mode at that time if we get a response,
         * we not a table for that hence it is ot possible to check
         * error, success, other response hence only check extra response is received and take action accordingly.
         */

        if (AT_STATE_WAIT_FOR_RSP == g_atContext.state || AT_STATE_WAIT_FOR_NTFN == g_atContext.state)
        {
            /* Check received response is success response. */
            if (0 == strncmp((char *)tokens[i], atCmdTable->successResponse, strlen((char *)atCmdTable->successResponse)))
            {
                if (NULL != g_atContext.storeDataCallBack)
                {
                    g_atContext.storeDataCallBack(g_atContext.cmdTable, g_atContext.currentCmdIndex, tokenLength, (uint8_t *)tokens[i], AT_CB_SUCCESS_SINGLE_CMD);
                }

                if (multiStepResponseError == 0)
                {
                    g_atContext.respRetryCount = 0;
                    /* If we are waiting for the response and then response is received then it is valid otherwise ignore it.  */
                    if (AT_STATE_WAIT_FOR_RSP == g_atContext.state)
                    {
                        /* Success response is received, and notificationFlag is set then notification will received wait for that. */
                        if (1 == atCmdTable->notificationFlag)
                        {
                            g_atContext.state = AT_STATE_WAIT_FOR_NTFN;
                            RESET_TIMER(g_atContext.timer, atCmdTable->timeOutMs);
                        }
                        else
                        {
                            /* If notificationFlag is not set that means notification wioll not received then fire next command. */
                            g_atContext.state = AT_STATE_SELECT_COMMAND;
                        }
                    }
                    else
                    {
                        /* We are waiting for notification and success notification is received then fire next command. */
                        g_atContext.notificationRetryCount = 0;
                        g_atContext.state = AT_STATE_SELECT_COMMAND;
                    }
                }
                else
                {
                    NETWORK_PRINT_ERROR("MULT RSP ERR \r\n");
                }
            }

            /* Check received response is error response. */
            else if (0 == strncmp((char *)tokens[i], atCmdTable->otherResponse, strlen(atCmdTable->otherResponse)))
            {
                /* Check error on stop flag and take action. */
                checkStopFlag(atCmdTable);
            }
            /* Check received response is other response. */
            else if ((atCmdTable->notificationFlag) && (strstr((char *)tokens[i], atCmdTable->otherResponse)))
            {
                g_atContext.state = AT_STATE_SELECT_COMMAND;
                if (NULL != g_atContext.storeDataCallBack)
                {
                    g_atContext.storeDataCallBack(g_atContext.cmdTable, g_atContext.currentCmdIndex, tokenLength, tokens[i], AT_CB_OTHER_RSP);
                }
            }
            else if (0 == (strncmp(atCmdTable->otherResponse, "*", 1)))
            {
                if (NULL != g_atContext.storeDataCallBack)
                {
                    retVal = g_atContext.storeDataCallBack(g_atContext.cmdTable, g_atContext.currentCmdIndex, tokenLength, (uint8_t *)tokens[i], AT_CB_ACCEPT_ALL);
                }

                if (0 == retVal)
                {
                    /* If not success and not fail response is received then do nothing. */
                }
                else if (0 > retVal)
                {
                    /* If fail response is received then go IDLE. */
                    retVal = g_atContext.eventCallBack(g_atContext.currentCmdIndex, AT_CB_OTHER_RSP, tokenLength, tokens[i]);
                    g_atContext.state = AT_STATE_IDLE;
                }
                else
                {
                    /* If success response is received then fire next command. */
                    g_atContext.state = AT_STATE_SELECT_COMMAND;
                }
            }
        }
        else
        {
            /* Not in list. */
            status = AT_CB_NOT_IN_LIST;
        }

        if (AT_CB_NOT_IN_LIST == status)
        {
            /* Now it is comfirm not in list response. */
            if (NULL != g_atContext.storeDataCallBack)
            {
                g_atContext.storeDataCallBack(g_atContext.cmdTable, g_atContext.currentCmdIndex, tokenLength, (uint8_t *)tokens[i], AT_CB_NOT_IN_LIST);
            }
        }
        i++;
    } while ((tokens[i] != NULL) && (i < MAX_TOKEN_LIMIT));
    return 0;
}

static uint16_t trimString(uint8_t *string, uint16_t stringLen)
{
    uint16_t iterator = 0;
    uint16_t counter = 0;

    /* Check the length of the string. */
    if (0 == stringLen)
    {
        return 0;
    }

    for (iterator = 0; iterator < stringLen;)
    {
        /* If \r\n character are found */
        if (string[iterator] == '\r' && string[iterator + 1] == '\n')
        {
            /* Point ot the after \r\n in the string. */
            counter++;
            iterator += 2;
        }
        else
        {
            /* If \r\n not found in to the string then break. */
            break;
        }
    }
    stringLen -= (counter * 2);

    if (counter)
    {
        memmove(string, &string[(counter * 2)], stringLen);
    }

    counter = 0;
    for (iterator = (stringLen - 1); iterator > 0;)
    {
        if (string[iterator - 1] == '\r' && string[iterator] == '\n')
        {
            string[iterator - 1] = '\0';
            counter++;
            iterator -= 2;
        }
        else
        {
            break;
        }
    }
    stringLen -= (counter * 2);
    return stringLen;
}

uint16_t modemURCHandler(uint8_t *buffer, uint16_t len)
{
    uint16_t iterator = 0;
    uint16_t outLen = 0;
    uint8_t *urcStartPtr = NULL;

    memset(g_outBuffer, 0, MAX_RCVD_BUF_LEN);
    for (iterator = 0; iterator < g_extraTbleCnt; iterator++)
    {
        if ((NULL != g_extraTable[iterator].urcHandler) && NULL != (urcStartPtr = (uint8_t *)strstr((const char *)buffer, g_extraTable[iterator].string)))
        {
            if (urcStartPtr - 2 >= buffer)
            {
                if (*(urcStartPtr - 2) == '\r' && *(urcStartPtr - 1) == '\n')
                {
                    g_extraTable[iterator].urcHandler(buffer, len, g_outBuffer, &outLen);
                    if (outLen)
                    {
                        memcpy(buffer, g_outBuffer, outLen + 1);
                        memset(g_outBuffer, 0, sizeof g_outBuffer);
                    }
                    len = outLen;
                }
                else
                {
                    NETWORK_PRINT_DEBUG("URC string found in other URC data skip\r\n");
                }
            }
            else
            {
                NETWORK_PRINT_DEBUG("0x0D, 0x0A  not found invalid condition\r\n");
            }
        }
    }
    return len;
}

uint8_t AtGetState(void)
{
    return g_atContext.state;
}

void handleErrForOtherResp(void)
{
    NETWORK_PRINT_DEBUG("handle error, At state, %d \r\n", __func__, g_atContext.state);
    if (AT_STATE_WAIT_FOR_NTFN == g_atContext.state)
    {
        RESET_TIMER(g_atContext.timer, 0);
    }
}
