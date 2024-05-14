/**
 * @file        connection_mgr.h
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
 * @brief       Connection manager headers.
 * 
 * @version     v0.1    03 Nov 2023
 * @details     - Created
 */

#ifndef CONNECTION_MGR_H
#define CONNECTION_MGR_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_OPERATOR_NAME_LEN 20
#define MAX_IP_ADDRESS_LEN 16
#define MAX_APN_NAME_LEN 20

typedef struct
{
    bool networkStatus;
    uint8_t creg;
    uint8_t cgreg;
    bool modemReady;
    uint8_t csq;
    uint8_t operatorName[MAX_OPERATOR_NAME_LEN];
    uint8_t accessTechnology;
    uint8_t ipAddress[MAX_IP_ADDRESS_LEN];
}ConnectionMgrInfo_t;

typedef enum
{
    CONN_MGR_INVALID_MEMORY = -2,
    CONN_MGR_ERROR,
    CONN_MGR_SUCCESS
} ConnMgrErrorCode_n;
typedef struct
{
    uint8_t apnName[MAX_APN_NAME_LEN];
}ConnectionMgrConfig_t;

/**
 * @brief   Initializes the connection manager.
 * @return  Return the error code defined in ConnMgrErrorCode_n
 */
ConnMgrErrorCode_n ConnectionMgrInit(void);

/**
 * @brief   Starts the execute of the connection manager state machine.
 * @return  Return the error code defined in ConnMgrErrorCode_n
 * 
 */
ConnMgrErrorCode_n ConnectionMgrExe(void);

/**
 * @brief   Retrieves information about the connection manager.
 * @return  Return the status of network connection.
 * 
 */
ConnectionMgrInfo_t ConnectionMgrGetInfo(void);

/**
 * @brief   Retrieves the network status.
 * @return  Return true if CERG and CGREG available.
 * 
 */
bool ConnectionMgrIsNetAvailable(void);

/**
 * @brief   Print the connection status.
 * 
 */
void ConnectionMgrPrintInfo(void);

#endif /*CONNECTION_MGR_H*/
