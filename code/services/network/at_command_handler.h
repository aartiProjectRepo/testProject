/**
 * @file        at_command_handler.h
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
 * @brief       AT Handler code headers
 * 
 * @version     v0.1    03 Nov 2023
 * @details     - Primary version of AT command handler for EC200G
 */

#ifndef AT_COMMAND_HANDLER
#define AT_COMMAND_HANDLER

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAX_AT_BUFF_SIZE ((uint16_t)(16 * 1024))
#define MAX_RCVD_BUF_LEN (16 * 1024)

// forward declare
typedef uint16_t (*fnPtrUrcCallBack)(uint8_t *buffer, uint16_t len, uint8_t *outBuff, uint16_t *outLen);

typedef enum
{
    AT_STATE_IDLE,
    AT_STATE_SELECT_COMMAND,
    AT_STATE_FILL_N_SND_CMD,
    AT_STATE_WAIT_FOR_RSP,
    AT_STATE_WAIT_FOR_NTFN,
} AtCommandStates_n;

typedef struct
{
    const char *command;              /* command is used to store the command. */
    char *successResponse;            /* successResponse is used to store the success response of the command. */
    char *errorResponse;              /* errorresponse is used to store the error response of the command. */
    char *otherResponse;              /* otherresponse is used to store the other response of the command. */
    uint8_t notificationFlag;         /* notificationFlag is 1 when notification will received otherwise it is 0. */
    uint32_t timeOutMs;               /* timeOutMs is used to store wait time after command fire. */
    uint8_t maxRetry;                 /* maxRetry is used to store the maximum retry count of the response. */
    uint8_t maxNotificationRetry;     /* maxNotificationRetry is used to store the maximum retry count of the notification. */
    uint8_t errorStopFlg;             /* errorStopFlg is 1 when error or retry over then do not fire remaining command, 0 then fire remaing command. */
    uint16_t waitTimerForNextCmd;     /*waitTimerForNextCmd is used, when we fire one command and wait for given time to fire next command*/
} AtCommands_t;

typedef struct
{
    char *string;
    fnPtrUrcCallBack urcHandler;
} UrcTable_t;

typedef enum
{
    AT_CB_SUCCESS_SINGLE_CMD,
    AT_CB_ERROR_NO_STOP,
    AT_CB_ERROR_STOP,
    AT_CB_OTHER_RSP,
    AT_CB_ALL_CMD_OVR,
    AT_CB_NOT_IN_LIST,
    AT_CB_ACCEPT_ALL,
} AtCallBack_n;

typedef enum
{
    AT_INVALID_MEMORY = -2,
    AT_FAILED,
    AT_SUCCESS,
} AtError_n;

typedef uint16_t (*fnPtrFillCmd)(uint8_t *cmdBuffer, int16_t offset, const AtCommands_t *atCmdTable, int8_t current_cmd_idx);
typedef int16_t (*fnPtrEventCallBack)(uint8_t commandIdx, uint8_t status, uint32_t buffLen, void *buffer);
typedef int8_t (*funPtrStoreResp)(const AtCommands_t *atCmdTable, uint8_t commandIdx, uint32_t buffLen, uint8_t *buffer, int8_t status);

/**
 * @brief   StartAT, maps the AT Command table and respective function pointers.
 * @param   atTable Pointer to the AT Commands table.
 * @param   maxCmd Maximum GSM commands count.
 * @return  Error code define in AtError_n
 */
AtError_n AtStart(const AtCommands_t *atTable, uint8_t maxCmd, fnPtrFillCmd fnPtrFill,
                  funPtrStoreResp fnPtrStore, fnPtrEventCallBack fnPtrCb);

/**
 * @brief   Init AT, helps configure GSM UART for Modem Communication.
 * @return  GsmHandle (Port Number).
 */
int16_t AtInit(void);

/**
 * @brief   ExecuteAT, this executes the ATCommandHandler, handling the state machine of AT.
 * @param   None
 * @return  0 if execution is successful.
 */
int8_t AtExe(void);

/**
 * @brief   Gets the current state of AT Execute.
 * @param   None
 * @return  Current State.
 */
uint8_t AtGetState(void);

/**
 * @brief   Used to register URC to be received from Modem.
 * @param   ExtraTableEntries Table of Extra URC, Count.
 * @return  Error code define in AtError_n
 */
AtError_n AtRegisterUrc(UrcTable_t *ExtraTableEntries, int count);

/**
 * @brief   Set AT State to IDLE.
 * @return  None.
 */
void AtForceStop(void);

#endif /*AT_COMMAND_HANDLER*/
