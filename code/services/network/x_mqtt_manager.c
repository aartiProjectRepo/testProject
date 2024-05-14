/**
 * @file        x_mqtt_manager.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        12 December 2023
 * @author      Nitin Rajeev <nitin.rajeev@accoladeelectronics.com>
 *              Diksha J     <diksha.jadhav@accoladeelectronics.com>
 * 
 * @brief       EC200 MQTT MANAGER
 */

/***********************************************************************************************************************
 *                                                  I N C L U D E S
 **********************************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "x_mqtt_manager.h"

#include "at_command_handler.h"
#include "modem_port.h"
#include "connection_mgr.h"

/***********************************************************************************************************************
 *                                                    D E F I N E S
 **********************************************************************************************************************/

/*
 * MQTT MANAGER CONFIG
 */
#define XMQTT_MAX_SOCKETS              (1) // EC200 MAX 6 SOCKETS
#define XMQTT_MAX_RECEIVE_MAILBOX      (2)
#define XMQTT_RECEIVE_SIZE             (10 * 1024)

/*
 * REQUEST QUEUE SIZE
 */
#define SIZE_QUEUE_REQUEST_CONFIGURE   (6)
#define SIZE_QUEUE_REQUEST_CONNECT     (6)
#define SIZE_QUEUE_REQUEST_DISCONNECT  (6)
#define SIZE_QUEUE_REQUEST_SUBSCRIBE   (20)
#define SIZE_QUEUE_REQUEST_UNSUBSCRIBE (10)
#define SIZE_QUEUE_REQUEST_PUBLISH     (20)

/***********************************************************************************************************************
 *                                        D A T A  T Y P E S  &  T Y P E D E F
 **********************************************************************************************************************/

/**************************************************
 * FINITE STATE MACHINES
 **************************************************/

typedef struct
{
    uint8_t  previousState;
    uint8_t  state;
    uint8_t  mode;
    uint32_t timeout;
}XMQTT_FsmContext_t;

typedef enum
{
    XMQTT_FSMM_INITIATED,
    XMQTT_FSMM_EXECUTED,
    XMQTT_FSMM_COMPLETED,
}XMQTT_FSM_MODES;

typedef enum
{
    XMQTT_FSMS_MAIN_IDLE,
    XMQTT_FSMS_MAIN_ABORT,

    XMQTT_FSMS_MAIN_NO_NETWORK,
    XMQTT_FSMS_MAIN_ACTIVE_NETWORK,
}XMQTT_FSM_STATES_MAIN;

typedef enum
{
    XMQTT_FSMS_CONNECT_IDLE,
    XMQTT_FSMS_CONNECT_ABORT,
    XMQTT_FSMS_CONNECT_SUPERVISE,

    XMQTT_FSMS_CONNECT_SSL_FILES,
    XMQTT_FSMS_CONNECT_SSL_CONFIG,
    XMQTT_FSMS_CONNECT_MQTT_CONFIG,
    XMQTT_FSMS_CONNECT_CLOSE_TCP,
    XMQTT_FSMS_CONNECT_OPEN_TCP,
    XMQTT_FSMS_CONNECT_MQTT_CONNECT,
}XMQTT_FSM_STATES_CONNECT;

typedef enum
{
    XMQTT_FSMS_DISCONNECT_IDLE,
    XMQTT_FSMS_DISCONNECT_ABORT,
    XMQTT_FSMS_DISCONNECT_SUPERVISE,

    XMQTT_FSMS_DISCONNECT_MQTT_DISCONNECT,
    XMQTT_FSMS_DISCONNECT_CLOSE_TCP,
}XMQTT_FSM_STATES_DISCONNECT;

typedef enum
{
    XMQTT_FSMS_SUBSCRIBE_IDLE,
    XMQTT_FSMS_SUBSCRIBE_ABORT,
    XMQTT_FSMS_SUBSCRIBE_SUPERVISE,

    XMQTT_FSMS_SUBSCRIBE_SUB,
}XMQTT_FSM_STATES_SUBSCRIBE;

typedef enum
{
    XMQTT_FSMS_UNSUBSCRIBE_IDLE,
    XMQTT_FSMS_UNSUBSCRIBE_ABORT,
    XMQTT_FSMS_UNSUBSCRIBE_SUPERVISE,

    XMQTT_FSMS_UNSUBSCRIBE_UNSUB,
}XMQTT_FSM_STATES_UNSUBSCRIBE;

typedef enum
{
    XMQTT_FSMS_PUBLISH_IDLE,
    XMQTT_FSMS_PUBLISH_ABORT,
    XMQTT_FSMS_PUBLISH_SUPERVISE,

    XMQTT_FSMS_PUBLISH_PUB,
}XMQTT_FSM_STATES_PUBLISH;

/**************************************************
 * QUECTEL EC200 4G LTE MQTT AT-COMMANDS
 **************************************************/

/*
 * SSL FILES
 */
typedef enum
{
    XMQTT_ATABLE_SSL_DELETE_CA,
    XMQTT_ATABLE_SSL_DELETE_CC,
    XMQTT_ATABLE_SSL_DELETE_CK,
    XMQTT_ATABLE_SSL_WRITE_CA,
    XMQTT_ATABLE_SSL_WRITE_DATA_CA,
    XMQTT_ATABLE_SSL_WRITE_CC,
    XMQTT_ATABLE_SSL_WRITE_DATA_CC,
    XMQTT_ATABLE_SSL_WRITE_CK,
    XMQTT_ATABLE_SSL_WRITE_DATA_CK,
    XMQTT_ATABLE_SSL_MAX,
}XMQTT_ATABLE_SSL;
 
const AtCommands_t atableSslFiles[XMQTT_ATABLE_SSL_MAX] =
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
 
       {"+QFDEL=",                                           "OK",           "+CME ERROR",       "\0",           0,          1000,           3,          0,              0,              0},
       {"+QFDEL=",                                           "OK",           "+CME ERROR",       "\0",           0,          1000,           3,          0,              0,              0},
       {"+QFDEL=",                                           "OK",           "+CME ERROR",       "\0",           0,          1000,           3,          0,              0,              0},
       {"+QFUPL=",                                           "CONNECT",      "+CME ERROR",       "\0",           0,          1000,           2,          0,              1,              0},
       {"",                                                 "+QFUPL",       "+CME ERROR",       "\0",           0,          25*1000,        2,          0,              1,              0},
       {"+QFUPL=",                                           "CONNECT",      "+CME ERROR",       "\0",           0,          1000,           2,          0,              1,              0},
       {"",                                                 "+QFUPL",       "+CME ERROR",       "\0",           0,          25*1000,        2,          0,              1,              0},
       {"+QFUPL=",                                          "CONNECT",      "+CME ERROR",       "\0",           0,          1000,           2,          0,              1,              0},
       {"",                                                 "+QFUPL",       "+CME ERROR",       "\0",           0,          25*1000,        2,          0,              1,              0},
};

/*
 * SSL CONFIGURE
 */
typedef enum
{
    XMQTT_ATT_CONFIG_CA,
    XMQTT_ATT_CONFIG_CC,
    XMQTT_ATT_CONFIG_CK,
    XMQTT_ATT_CONFIG_SEC_LEVEL,
    XMQTT_ATT_CONFIG_SSL_VERSION,
    XMQTT_ATT_CONFIG_CIPHERSUITE,
    XMQTT_ATT_CONFIG_IGNORE_LOCALTIME,
    XMQTT_ATT_SSL_CONFIG_MAX
}XMQTT_ATABLE_SSL_CONFIG;

const AtCommands_t atableSslConfig[XMQTT_ATT_SSL_CONFIG_MAX]=
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QSSLCFG=\"cacert\",",                            "OK",           "ERROR",            "\0",           0,          1000,           2,          0,              1,              0},
        {"+QSSLCFG=\"clientcert\",",                        "OK",           "ERROR",            "\0",           0,          1000,           2,          0,              1,              0},
        {"+QSSLCFG=\"clientkey\",",                         "OK",           "ERROR",            "\0",           0,          1000,           2,          0,              1,              0},
        {"+QSSLCFG=\"seclevel\",2,2",                       "OK",           "ERROR",            "\0",           0,          1000,           2,          0,              1,              0},
        {"+QSSLCFG=\"sslversion\",2,4",                     "OK",           "ERROR",            "\0",           0,          1000,           2,          0,              1,              0},
        {"+QSSLCFG=\"ciphersuite\",2",                      "+QSSLCFG",     "ERROR",            "\0",           0,          1000,           2,          0,              1,              0},
        {"+QSSLCFG=\"ignorelocaltime\",2,1",                "OK",           "ERROR",            "\0",           0,          1000,           2,          0,              1,              0},
};

/*
 * MQTT CONFIGURE
 */
typedef enum
{  
    XMQTT_ATT_CONFIG_KEEP_ALIVE,
    XMQTT_ATT_CONFIG_TIMEOUT,
    XMQTT_ATT_CONFIG_RECV_MODE,
    XMQTT_ATT_CONFIG_SSL,
    XMQTT_ATT_CONFIG_MAX,
}XMQTT_ATABLE_CONFIG;
 
const AtCommands_t atableConfig[XMQTT_ATT_CONFIG_MAX]=
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/        
        {"+QMTCFG=\"KEEPALIVE\",",                          "OK",           "ERROR",            "\0",           0,          300,            3,          0,              0,              0},
        {"+QMTCFG=\"TIMEOUT\",",                            "OK",           "ERROR",            "\0",           0,          300,            3,          0,              0,              0},
        {"+QMTCFG=\"recv/mode\",",                          "OK",           "ERROR",            "\0",           0,          300,            3,          0,              0,              0},
        {"+QMTCFG=\"SSL\",",                                "OK",           "ERROR",            "\0",           0,          300,            3,          0,              0,              0},
};

/*
 * OPEN TCP SOCKET
 */
typedef enum
{
    XMQTT_ATT_OPEN_OPEN,
    XMQTT_ATT_OPEN_MAX,
} XMQTT_ATABLE_OPEN;

const AtCommands_t atableOpen[XMQTT_ATT_OPEN_MAX]=
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QMTOPEN=",                                       "OK",           "ERROR",            "*",           0,          30*1000,       1,          1,              1,              0},
};

/*
 * CLOSE TCP SOCKET
 */
typedef enum
{
    XMQTT_ATT_CLOSE_CLOSE,
    XMQTT_ATT_CLOSE_MAX,
} XMQTT_ATABLE_CLOSE;

const AtCommands_t atableClose[XMQTT_ATT_CLOSE_MAX] =
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QMTCLOSE=",                                      "OK",           "+CME ERROR",       "*",            0,        30 * 1000,        2,          5,              0,             20},
};

/*
 * CONNECT WITH MQTT BROKER
 */
typedef enum
{
    XMQTT_ATT_CONNECT_CONN,
    XMQTT_ATT_CONNECT_MAX,
}XMQTT_ATABLE_CONNECT;

const AtCommands_t atableConnect[XMQTT_ATT_CONNECT_MAX]=
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QMTCONN=",                                       "OK",           "ERROR",    "+QMTCONN: ",           1,          30*1000,        1,          1,              1,              0},
};

/*
 * DISCONNECT WITH MQTT BROKER
 */
typedef enum
{
    XMQTT_ATT_DISCONNECT_DISC,
    XMQTT_ATT_DISCONNECT_MAX,
}XMQTT_ATABLE_DISCONNECT;

const AtCommands_t atableDisconnect[XMQTT_ATT_DISCONNECT_MAX]=
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QMTDISC=",                                       "OK",           "ERROR",            "\0",           0,          30*1000,        2,          0,              0,              0},
};

/*
 * SUBSCRIBE TOPIC
 */
typedef enum
{
    XMQTT_ATT_SUBSCRIBE_SUB,
    XMQTT_ATT_SUBSCRIBE_MAX
} XMQTT_ATABLE_SUBSCRIBE;

const AtCommands_t atableSubscribe[XMQTT_ATT_SUBSCRIBE_MAX]=
{
        /* COMMAND                                       SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QMTSUB=",                                       "OK",            "+CME ERROR",        "\0",           0,          20 * 1000,      3,          5,              1,             0},
};

/*
 * UNSUBSCRIBE TOPIC
 */
typedef enum
{
    XMQTT_ATT_UNSUBSCRIBE_UNSUB,
    XMQTT_ATT_UNSUBSCRIBE_MAX
} XMQTT_ATABLE_UNSUBSCRIBE;

const AtCommands_t atableUnsubscribe[XMQTT_ATT_UNSUBSCRIBE_MAX]=
{
        /* COMMAND                                       SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QMTUNS=",                                       "OK",            "+CME ERROR",        "\0",           0,          20 * 1000,      3,          5,              1,             0},
};

/*
 * PUBLISH PAYLOAD
 */
typedef enum
{
    XMQTT_ATT_PUBLISH_PUB,
    XMQTT_ATT_PUBLISH_PAYLOAD,
    XMQTT_ATT_PUBLISH_MAX
} XMQTT_ATABLE_PUBLISH;

const AtCommands_t atablePublish[XMQTT_ATT_PUBLISH_MAX] =
{
        /* COMMAND                                      SUCCESS RSP         ERROR RSP           OTHRRSP     NTFNFLG         TMOUT       MAXRTRYCNT  MAXNTFNRTRYCNT  STOPONERROR     WAITTIMER*/
        {"+QMTPUBEX=",                                      ">",            "ERROR",            "\0",           0,          2000,           1,          0,              1,              0},
        {"",                                                "OK",           "ERROR",            "\0",           0,          300,            1,          0,              1,              0},
};

/*
 * AT CALLERS
 */
typedef enum
{
    XMQTT_AT_CALLER_ID_UNKNOWN,
    XMQTT_AT_CALLER_ID_CONFIGURE
}XMQTT_AT_CALLER_ID;

/**************************************************
 * UNIT DEFINITIONS
 **************************************************/

typedef struct
{
    xmqtt_configuration_t config;

    uint8_t occupy;
    uint8_t socketId;
    uint8_t connection;

    uint8_t flagSslFiles;
    uint8_t flagSslConfig;
    uint8_t flagMqttConfig;
    uint8_t flagClose;
    uint8_t flagOpen;
    uint8_t flagConnect;

    uint8_t errorCountConnect;
    uint8_t errorCountSubscribe;
    uint8_t errorCountPublish;

    uint8_t serviceStatus;
}xmqtt_contextConnect_t;

typedef struct
{
    uint8_t socketId;

    uint8_t flagDisconnect;
    uint8_t flagClose;

    uint8_t serviceStatus;
}xmqtt_contextDisconnect_t;

typedef struct
{
    uint8_t  socketId;
    uint8_t  topicId;
    char     *topic;
    uint16_t length;
    uint8_t  qos;

    uint8_t flagSubscribe;

    uint8_t serviceStatus;
}xmqtt_contextSubscribe_t;

typedef struct
{
    uint8_t  socketId;
    char     *topic;
    uint16_t length;

    uint8_t serviceStatus;
}xmqtt_contextUnsubscribe_t;

typedef struct
{
    uint8_t  socketId;
    char     *payload;
    uint16_t length;
    char     *topic;
    uint8_t  qos;

    uint8_t flagPublish;

    uint8_t serviceStatus;
}xmqtt_contextPublish_t;

/***********************************************************************************************************************
 *                                  P R I V A T E  F U N C T I O N  D E C L A R A T I O N S
 **********************************************************************************************************************/

/*
 * STATE MACHINE AND TRANSITION
 */
static void fsmMain(void);
static void fsmTransitionMain(uint8_t switchState);

static void fsmConnect(void);
static void fsmTransitionConnect(uint8_t switchState);

static void fsmDisconnect(void);
static void fsmTransitionDisconnect(uint8_t switchState);

static void fsmSubscribe(void);
static void fsmTransitionSubscribe(uint8_t switchState);

static void fsmUnsubscribe(void);
static void fsmTransitionUnsubscribe(uint8_t switchState);

static void fsmPublish(void);
static void fsmTransitionPublish(uint8_t switchState);

/*
 * MQTT URC CALLBACKS
 */
static uint16_t urcMqttOpen     (uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen);
static uint16_t urcMqttClose    (uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen);
static uint16_t urcMqttConnect  (uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen);
static uint16_t urcMqttPublish  (uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen);
static uint16_t urcMqttReceive  (uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen);
static uint16_t urcMqttSubscribe(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen);
static uint16_t urcMqttStatus   (uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen);

/*
 * AT CALLBACKS FILLER, RESPONSE & STATUS
 */
static uint16_t fillerSslFiles(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondSslFiles(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusSslFiles(uint8_t atableIndex, uint8_t status, uint32_t length, void *buffer);

static uint16_t fillerSslConfig(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondSslConfig(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusSslConfig(uint8_t atableIndex, uint8_t status, uint32_t length, void *buffer);

static uint16_t fillerConfiguration(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondConfiguration(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusConfiguration(uint8_t atableIndex, uint8_t status, uint32_t length, void *buffer);

static uint16_t fillerOpen(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondOpen(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusOpen(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer);

static uint16_t fillerClose(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondClose(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusClose(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer);
static int16_t  cbStatusCloseDisconnect(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer);

static uint16_t fillerConnect(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondConnect(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusConnect(uint8_t atableIndex, uint8_t status, uint32_t length, void *buffer);

static uint16_t fillerDisconnect(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondDisconnect(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusDisconnect(uint8_t atableIndex, uint8_t status, uint32_t length, void *buffer);

static uint16_t fillerSubscribe(uint8_t *command, int16_t offset,const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondSubscribe(const AtCommands_t *atable,uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusSubscribe(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer);

static uint16_t fillerUnsubscribe(uint8_t *command, int16_t offset,const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondUnsubscribe(const AtCommands_t *atable,uint8_t atableIndex, uint32_t length, uint8_t *buffer, char status);
static int16_t  cbStatusUnsubscribe(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer);

static uint16_t fillerPublish(uint8_t *command, int16_t offset,const AtCommands_t *atable, int8_t atableIndex);
static int8_t   respondPublish(const AtCommands_t *atable,uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status);
static int16_t  cbStatusPublish(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer);

/*
 * MQTT SOCKET STATUS
 */
static int8_t getUnoccupiedSocket(void);
static int8_t isSocketFree(uint8_t id);

/*
 * MISC
 */
static void logStatus(void);
/***********************************************************************************************************************
 *                                      P R I V A T E  D A T A  D E C L A R A T I O N S
 **********************************************************************************************************************/

/*
 * SYSTEM STATUS
 */
static bool    g_statusNetwork;
static uint8_t g_statusMqttManager;

/*
 * FSM STATES
 */
static XMQTT_FsmContext_t g_fsmContextMain;
static XMQTT_FsmContext_t g_fsmContextConnect;
static XMQTT_FsmContext_t g_fsmContextDisconnect;
static XMQTT_FsmContext_t g_fsmContextSubscribe;
static XMQTT_FsmContext_t g_fsmContextUnsubscribe;
static XMQTT_FsmContext_t g_fsmContextPublish;

/*
 * LIVE REQUEST
 */
static xmqtt_contextConnect_t     g_requestConnect;
static xmqtt_contextPublish_t     g_requestPublish;
static xmqtt_contextSubscribe_t   g_requestSubscribe;
static xmqtt_contextDisconnect_t  g_requestDisconnect;
static xmqtt_contextUnsubscribe_t g_requestUnsubscribe;

/*
 * INTERNAL REQUEST QUEUES
 */
static xmqtt_contextConnect_t     g_qConnect    [SIZE_QUEUE_REQUEST_CONNECT    ];
static xmqtt_contextPublish_t     g_qPublish    [SIZE_QUEUE_REQUEST_PUBLISH    ];
static xmqtt_contextSubscribe_t   g_qSubscribe  [SIZE_QUEUE_REQUEST_SUBSCRIBE  ];
static xmqtt_contextDisconnect_t  g_qDisconnect [SIZE_QUEUE_REQUEST_DISCONNECT ];
static xmqtt_contextUnsubscribe_t g_qUnsubscribe[SIZE_QUEUE_REQUEST_UNSUBSCRIBE];

/*
 * MQTT SOCKET
 */
static xmqtt_contextConnect_t g_mqttSockets[XMQTT_MAX_SOCKETS];

/*
 * MQTT CONTAINER
 */
static xmqtt_mailBox_t g_container[XMQTT_MAX_RECEIVE_MAILBOX];

static uint8_t mailBox1[XMQTT_RECEIVE_SIZE];
static uint8_t mailBox2[XMQTT_RECEIVE_SIZE];

/*
 * MQTT URCS
 */
UrcTable_t g_mqttUrcTable[] = {
                                   {"+QMTOPEN:",  &urcMqttOpen     },
                                   {"+QMTCLOSE:", &urcMqttClose    },
                                   {"+QMTPUBEX:", &urcMqttPublish  },
                                   {"+QMTCONN:",  &urcMqttConnect  },
                                   {"+QMTRECV:",  &urcMqttReceive  },
                                   {"+QMTSUB:",   &urcMqttSubscribe},
                                   {"+QMTSTAT:",  &urcMqttStatus   }
                               };
#define URC_MQTT_COUNT         ( sizeof(g_mqttUrcTable)/sizeof(g_mqttUrcTable[0]) )

/***********************************************************************************************************************
 *                                              P U B L I C  F U N C T I O N S
 **********************************************************************************************************************/

/**************************************************
 * MAIN EXECUTE
 **************************************************/

void XMQTT_Init(void)
{
    g_container[0].lock          = 0;
    g_container[0].payload       = mailBox1;
    g_container[0].payloadLength = XMQTT_RECEIVE_SIZE;

    g_container[1].lock          = 0;
    g_container[1].payload       = mailBox2;
    g_container[1].payloadLength = XMQTT_RECEIVE_SIZE;

    AtRegisterUrc(g_mqttUrcTable, URC_MQTT_COUNT);

    fsmTransitionMain       (XMQTT_FSMS_MAIN_NO_NETWORK      );
    fsmTransitionConnect    (XMQTT_FSMS_CONNECT_SUPERVISE    );
    fsmTransitionDisconnect (XMQTT_FSMS_DISCONNECT_SUPERVISE );
    fsmTransitionSubscribe  (XMQTT_FSMS_SUBSCRIBE_SUPERVISE  );
    fsmTransitionUnsubscribe(XMQTT_FSMS_UNSUBSCRIBE_SUPERVISE);
    fsmTransitionPublish    (XMQTT_FSMS_PUBLISH_SUPERVISE    );
}

void XMQTT_Execute(void)
{    
    g_statusNetwork = ConnectionMgrIsNetAvailable();

    fsmMain();

    logStatus();
}

/**************************************************
 * MQTT APIS
 **************************************************/

int8_t XMQTT_connect(xmqtt_configuration_t mqttHandle)
{
    static uint8_t lockConnect = 0;
    uint8_t i = 0;
 
    if(!lockConnect)
    {
        lockConnect = 1;

        for(; i < SIZE_QUEUE_REQUEST_CONNECT; i++)
        {
            if(!g_qConnect[i].serviceStatus)
            {
                g_qConnect[i].config        = mqttHandle;
                g_qConnect[i].serviceStatus = 1;
                break;
            }
        }

        lockConnect = 0;

        return 0;
    }

    else
    {
        return -1;
    }
}

int8_t XMQTT_disconnect(uint8_t socketId)
{
    static uint8_t lockDisconnect = 0;
    uint8_t i = 0;
    static uint8_t topicId = 1;

    if(!lockDisconnect)
    {
        lockDisconnect = 1;
 
        for(; i < SIZE_QUEUE_REQUEST_DISCONNECT; i++)
        {
            if(g_mqttSockets[socketId].connection == 1)
            {
                if(!g_qDisconnect[i].serviceStatus)
                {
                    g_qDisconnect[i].socketId      = socketId;
                    g_qDisconnect[i].serviceStatus = 1;
                    break;
                }
            }
            else
            {
                return -3;
            }
        }
 
        lockDisconnect = 0;
 
        if (i == SIZE_QUEUE_REQUEST_DISCONNECT)
        {
            return -2;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

int8_t XMQTT_subscribe(uint8_t socket_ID, char *topic, uint16_t length, uint8_t qos)
{
    static uint8_t lockSubscribe = 0;
    uint8_t i = 0;
    static uint8_t topicId = 1;

    if(!lockSubscribe)
    {
        lockSubscribe = 1;

        for(; i < SIZE_QUEUE_REQUEST_SUBSCRIBE; i++)
        {
            if(!g_qSubscribe[i].serviceStatus)
            {
                g_qSubscribe[i].socketId      = socket_ID;
                g_qSubscribe[i].topic         = topic;
                g_qSubscribe[i].topicId       = topicId++;
                g_qSubscribe[i].length        = length;
                g_qSubscribe[i].qos           = qos;
                g_qSubscribe[i].serviceStatus = 1;
                break;
            }
        }

        lockSubscribe = 0;

        if (i == SIZE_QUEUE_REQUEST_SUBSCRIBE)
        {
            return -2;
        }
        else
        {
            return 0;
        }
    }

    else
    {
        return -1;
    }
}

int8_t XMQTT_unsubscribe(uint8_t socket_ID, char *topic, uint16_t length)
{
    return 0;
}

int8_t XMQTT_publish(uint8_t socket_ID, char * topic, char *payload, uint16_t payloadLength, uint8_t qos)
{
    static uint8_t lockPublish = 0;
    uint8_t i = 0;

    if(!lockPublish)
    {
        lockPublish = 1;

        if(g_mqttSockets[socket_ID].connection)
        {
            for(; i < SIZE_QUEUE_REQUEST_PUBLISH; i++)
            {
                if(!g_qPublish[i].serviceStatus)
                {
                    g_qPublish[i].socketId      = socket_ID;
                    g_qPublish[i].topic         = topic;
                    g_qPublish[i].payload       = payload;
                    g_qPublish[i].length        = payloadLength;
                    g_qPublish[i].qos           = qos;
                    g_qPublish[i].serviceStatus = 1;
                    break;
                }
            }
        }
        else
        {
            lockPublish = 0;
            return -3;
        }

        lockPublish = 0;

        if (i == SIZE_QUEUE_REQUEST_PUBLISH)
        {
            return -2;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

uint8_t XMQTT_isConnect(uint8_t socketId)
{
    return g_mqttSockets[socketId].connection;
}

uint8_t XMQTT_isSubscribe(uint8_t socketId, uint8_t *topic)
{
    return 0;
}

uint8_t XMQTT_isMqtt(void)
{
    return g_statusMqttManager;
}
/***********************************************************************************************************************
 *                                P U B L I C  F U N C T I O N S  B Y  R E F E R E N C E
 **********************************************************************************************************************/

/**************************************************
 * EC200 MQTT URC HANDLERS
 **************************************************/
static uint16_t urcMqttReceive(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen)
{
    char *urc = "+QMTRECV: ";
    char *cursorHead = NULL, *cursorTail = NULL;
    uint8_t  mailBoxId        = 0;
    uint16_t absPayloadLength = 0;
 
    cursorHead = strstr((const char*)buffer, urc);
    cursorTail = strstr(cursorHead, "\r\n");

    if( (NULL != cursorHead) && (NULL != cursorTail) )
    {
        cursorTail += 2;
    }
    else
    {
        while(1);
    }

    for(mailBoxId = 0 ; mailBoxId < XMQTT_MAX_RECEIVE_MAILBOX ; mailBoxId++)
    {
        if(!g_container[mailBoxId].lock)
        {
            g_container[mailBoxId].lock = 1;
            break;
        }
    }

    if(mailBoxId == XMQTT_MAX_RECEIVE_MAILBOX)
    {
        while(1);
    }
 
    cursorHead                        += strlen(urc);
    g_container[mailBoxId].socketId    = (uint8_t)atoi(cursorHead);

    cursorHead                        += 2;
    g_container[mailBoxId].topicId     = (uint8_t)atoi(cursorHead);

    cursorHead                         = strstr(cursorHead, ",\"") + 2;
    cursorTail                         = strstr(cursorHead, "\",");
    g_container[mailBoxId].topicLength = cursorTail - cursorHead;

    strncpy((char*)g_container[mailBoxId].topic , cursorHead , g_container[mailBoxId].topicLength);
    g_container[mailBoxId].topic[g_container[mailBoxId].topicLength] = '\0';

    cursorHead                           = cursorTail + 2;
    g_container[mailBoxId].payloadLength = (uint16_t)atoi(cursorHead);

    cursorHead       = strstr(cursorHead, ",\"") + 2;
    cursorTail       = strstr(cursorHead, "\"\r\n");
    absPayloadLength = cursorTail - cursorHead;
    strncpy((char*)g_container[mailBoxId].payload , cursorHead , g_container[mailBoxId].payloadLength);

    if(absPayloadLength != g_container[mailBoxId].payloadLength)
    {
        while(1);
    }

    //CALLBACK

    g_container[mailBoxId].lock = 0;

    return 0;
}

static uint16_t urcMqttStatus(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen)
{
    char *urc = "+QMTSTAT: ";
    char *start = NULL, *end = NULL;
    uint8_t socketId = 99, errorCode = 99;

    if (NULL != (start = strstr((const char*)buffer, urc)))
    {
        if (NULL != (end = strstr(start, "\r\n")))
        {
            end += 2;
        }
        start     +=  strlen(urc);
        socketId   = (uint8_t)atoi(start);
	    start     +=  2;
        errorCode  = (uint8_t)atoi(start);
    }
    
    g_mqttSockets[socketId].connection = 0;
    
    return 0;

    //DATA REQUIRED: SOCKET ID, ERROR CODE(HANDLER)
}

static uint16_t urcMqttSubscribe(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen)
{
    char *urc = "+QMTSUB: ";
    char *start = NULL, *end = NULL, *socket = NULL;
    uint8_t socketId = 99;

    if (NULL != (start = strstr((const char*)buffer, urc)))
    {
        if (NULL != (end = strstr(start, "\r\n")))
        {
            end += 2;
        }
        socket   = start + strlen(urc);
        socketId = (uint8_t)atoi(socket);
    }

    //DATA REQUIRED: SOCKET ID, MEASSAGE ID, RESULT
    g_requestSubscribe.flagSubscribe = 1;
    return 0;
}

static uint16_t urcMqttPublish(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen)
{
    char *urc = "+QMTPUBEX: ";
    char *start = NULL, *end = NULL, *socket = NULL;
    uint8_t socketId = 99;

    if (NULL != (start = strstr((const char*)buffer, urc)))
    {
        if (NULL != (end = strstr(start, "\r\n")))
        {
            end += 2;
        }
        socket   = start + strlen(urc);
        socketId = (uint8_t)atoi(socket);
    }

    g_requestPublish.flagPublish = 1;

    return 0;
}

static uint16_t urcMqttOpen(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen)
{
//MODEM DEPENDENT
    char *urc = "+QMTOPEN: ";
    char *start = NULL, *end = NULL, *socket = NULL, *result = NULL;
    uint8_t socketId = 99, resultId = 99;

    if (NULL != (start = strstr((const char*)buffer, urc)))
    {
        if (NULL != (end = strstr(start, "\r\n")))
        {
            end += 2;
        }
        socket   = start + strlen(urc);
        result   = socket + 2;
        socketId = (uint8_t)atoi(socket);
        resultId = (uint8_t)atoi(result);
    }

//MODEM INDEPENDENT 
    if(g_fsmContextConnect.state == XMQTT_FSMS_CONNECT_OPEN_TCP)
    {
        if(g_requestConnect.socketId == socketId)
        {
            if(resultId == 0)
            {
                g_requestConnect.flagOpen = 1;
            }
            else
            {

            }
        }
    }
    else
    {
    //UNINITIATED SOCKET OPEN
    }

    return 0;
}

static uint16_t urcMqttClose(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen)
{
    char *urc = "+QMTCLOSE: ";
    char *start = NULL, *end = NULL, *socket = NULL;
    uint8_t socketId = 99;

    if (NULL != (start = strstr((const char*)buffer, urc)))
    {
        if (NULL != (end = strstr(start, "\r\n")))
        {
            end += 2;
        }
        socket   = start + strlen(urc);
        socketId = (uint8_t)atoi(socket);
    }

    return 0;
}

static uint16_t urcMqttConnect(uint8_t *buffer, uint16_t len, uint8_t *destBuffer, uint16_t *respLen)
{
    char *urc = "+QMTCONN: ";
    char *start = NULL, *end = NULL, *socket = NULL;
    uint8_t socketId = 99;

    if (NULL != (start = strstr((const char*)buffer, urc)))
    {
        if (NULL != (end = strstr(start, "\r\n")))
        {
            end += 2;
        }
        socket   = start + strlen(urc);
        socketId = (uint8_t)atoi(socket);
    }

    if(g_fsmContextConnect.state == XMQTT_FSMS_CONNECT_MQTT_CONNECT)
    {
        g_requestConnect.flagConnect = 1;
    }

    return 0;
}
/**************************************************
 * AT FILLER, RESPONSE & CALLBACK
 **************************************************/

/*
 * SSL FILES
 */
static uint16_t fillerSslFiles(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);
 
    switch(atableIndex)
    {
        case (XMQTT_ATABLE_SSL_DELETE_CA) :
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "\"%s\"\r\n", g_requestConnect.config.ssl_cert_filename_ca);
        }
        break;

        case (XMQTT_ATABLE_SSL_DELETE_CC) :
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "\"%s\"\r\n", g_requestConnect.config.ssl_cert_filename_cc);
        }
        break;

        case (XMQTT_ATABLE_SSL_DELETE_CK) :
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "\"%s\"\r\n", g_requestConnect.config.ssl_cert_filename_ck);
        }
        break;
 
        case (XMQTT_ATABLE_SSL_WRITE_CA) :
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "\"%s\",%ld,200\r\n", g_requestConnect.config.ssl_cert_filename_ca, g_requestConnect.config.ssl_cert_length_ca);
        }break;
 
        case (XMQTT_ATABLE_SSL_WRITE_DATA_CA) :
        {
            memcpy( &(commandX[length]) , g_requestConnect.config.ssl_cert_ca , g_requestConnect.config.ssl_cert_length_ca );
            length += g_requestConnect.config.ssl_cert_length_ca;
 
            memcpy( &(commandX[length]) , "\r\n" , 2 );
            length += 2;
        }break;
 
        case (XMQTT_ATABLE_SSL_WRITE_CC) :
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "\"%s\",%ld,200\r\n", g_requestConnect.config.ssl_cert_filename_cc, g_requestConnect.config.ssl_cert_length_cc);
        }break;
 
        case (XMQTT_ATABLE_SSL_WRITE_DATA_CC) :
        {
            memcpy( &(commandX[length]) , g_requestConnect.config.ssl_cert_cc , g_requestConnect.config.ssl_cert_length_cc );
            length += g_requestConnect.config.ssl_cert_length_cc;
 
            memcpy( &(commandX[length]) , "\r\n" , 2 );
            length += 2;
        }break;
 
        case (XMQTT_ATABLE_SSL_WRITE_CK) :
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "\"%s\",%ld,200\r\n", g_requestConnect.config.ssl_cert_filename_ck, g_requestConnect.config.ssl_cert_length_ck);
        }break;
 
        case (XMQTT_ATABLE_SSL_WRITE_DATA_CK) :
        {
            memcpy( &(commandX[length]) , g_requestConnect.config.ssl_cert_ck , g_requestConnect.config.ssl_cert_length_ck );
            length += g_requestConnect.config.ssl_cert_length_ck;
 
            memcpy( &(commandX[length]) , "\r\n" , 2 );
            length += 2;
        }break;
 
        default :
        {
           
        }
    }
    return length;
}

static int8_t   respondSslFiles(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t  cbStatusSslFiles(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    g_requestConnect.flagSslFiles = 1;
    return 0;
}

/*
 * SSL CONFIGURATION
 */
static uint16_t fillerSslConfig(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {
        case(XMQTT_ATT_CONFIG_CA):
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "%d,\"%s\"\r\n", 2, g_requestConnect.config.ssl_cert_filename_ca);
        }break;

        case(XMQTT_ATT_CONFIG_CC):
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "%d,\"%s\"\r\n", 2, g_requestConnect.config.ssl_cert_filename_cc);
        }break;

        case(XMQTT_ATT_CONFIG_CK):
        {
            length += sprintf((char*)commandX, "AT%s", atable->command);
            length += sprintf((char*)&(commandX[length]), "%d,\"%s\"\r\n", 2, g_requestConnect.config.ssl_cert_filename_ck);
        }break;

        case(XMQTT_ATT_CONFIG_SEC_LEVEL):
        {

        }//break;

        case(XMQTT_ATT_CONFIG_SSL_VERSION):
        {

        }//break;

        case(XMQTT_ATT_CONFIG_CIPHERSUITE):
        {

        }//break;

        case(XMQTT_ATT_CONFIG_IGNORE_LOCALTIME):
        {
            length += sprintf((char*)commandX, "AT%s\r\n", atable->command);
        }break;

        default :
        {

        }
    }


    return length;
}

static int8_t   respondSslConfig(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t  cbStatusSslConfig(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    g_requestConnect.flagSslConfig = 1;
    return 0;
}

/*
 * MQTT CONFIGURATION
 */
static uint16_t fillerConfiguration(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {   
        case (XMQTT_ATT_CONFIG_KEEP_ALIVE) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%d\r\n", g_requestConnect.socketId, 30);
        }break;

        case (XMQTT_ATT_CONFIG_TIMEOUT) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%d,%d,%d\r\n", g_requestConnect.socketId, 30, 1, 1);
        }break;

        case (XMQTT_ATT_CONFIG_RECV_MODE) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%d,%d\r\n", g_requestConnect.socketId,0,1);
        }break;

        case (XMQTT_ATT_CONFIG_SSL) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            if(g_requestConnect.config.ssl)
            {
                length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%d,%d\r\n", g_requestConnect.socketId,1,2);
            }
            else
            {
                length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%d\r\n", g_requestConnect.socketId,0);
            }
        }break;

        default :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s\r\n", atable->command);
        }
    }

    return length;
}

static int8_t   respondConfiguration(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t cbStatusConfiguration(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    g_requestConnect.flagMqttConfig = 1;

    return 0;
}

/*
 * OPEN TCP
 */
static uint16_t fillerOpen(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {
        case (XMQTT_ATT_OPEN_OPEN) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,\"%s\",%s\r\n", g_requestConnect.socketId,g_requestConnect.config.ip,g_requestConnect.config.port);
        }break;

        default :
        {

        }
    }

    return length;
}

static int8_t   respondOpen(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t cbStatusOpen(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    return 0;
}

/*
 * CLOSE TCP
 */
static uint16_t fillerClose(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {
        case (XMQTT_ATT_CLOSE_CLOSE) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d\r\n", g_requestConnect.socketId);
        }break;

        default :
        {

        }
    }

    return length;
}

static int8_t   respondClose(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t cbStatusClose(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    g_requestConnect.flagClose = 1;
    return 0;
}
static int16_t cbStatusCloseDisconnect(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    g_requestDisconnect.flagClose = 1;
    return 0;
}


/*
 * CONNECT MQTT
 */
static uint16_t fillerConnect(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {
        case (XMQTT_ATT_CONNECT_CONN) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,\"%s\"\r\n", g_requestConnect.socketId, g_requestConnect.config.client);
        }break;

        default :
        {

        }
    }

    return length;
}

static int8_t   respondConnect(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t cbStatusConnect(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    return 0;
}

/*
 * DISCONNECT MQTT
 */
static uint16_t fillerDisconnect(uint8_t *command, int16_t offset, const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {
        case (XMQTT_ATT_DISCONNECT_DISC) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d\r\n", g_requestDisconnect.socketId);
        }break;

        default :
        {

        }
    }

    return length;
}

static int8_t   respondDisconnect(const AtCommands_t *atable, uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t cbStatusDisconnect(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    g_requestDisconnect.flagDisconnect = 1;
    return 0;
}

/*
 * SUBSCRIBE TOPIC
 */
static uint16_t fillerSubscribe(uint8_t *command, int16_t offset,const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {
        case (XMQTT_ATT_SUBSCRIBE_SUB) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%d,\"%s\",%d\r\n", g_requestSubscribe.socketId, g_requestSubscribe.topicId, g_requestSubscribe.topic, g_requestSubscribe.qos);
        }break;

        default :
        {

        }
    }

    return length;
}

static int8_t   respondSubscribe(const AtCommands_t *atable,uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t cbStatusSubscribe(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    return 0;
}

/*
 * UNSUBSCRIBE TOPIC
 */
static uint16_t fillerUnsubscribe(uint8_t *command, int16_t offset,const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);

    switch(atableIndex)
    {
        case (XMQTT_ATT_UNSUBSCRIBE_UNSUB) :
        {
            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%d,\"%s\"\r\n", 1, 1, "topic/sometopic");
        }break;

        default :
        {

        }
    }

    return length;
}

static int8_t   respondUnsubscribe(const AtCommands_t *atable,uint8_t atableIndex, uint32_t length, uint8_t *buffer, char status)
{
    return 1;
}

static int16_t cbStatusUnsubscribe(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    return 0;
}

/*
 * PUBLISH PAYLOAD
 */
static uint16_t fillerPublish(uint8_t *command, int16_t offset,const AtCommands_t *atable, int8_t atableIndex)
{
    uint16_t length = 0;
    uint8_t* commandX = &(command[offset]);
    //AT+QMTPUBEX=0,1,2,1,"/topic/test1",12
    switch(atableIndex)
    {
        case (XMQTT_ATT_PUBLISH_PUB) :
        {
            static uint32_t messageId = 1;

            length += (uint16_t)sprintf((char*)commandX, "AT%s", atable->command);
            length += (uint16_t)sprintf((char*)&(commandX[length]), "%d,%ld,%d,%d,\"%s\",%d\r",\
                    g_requestPublish.socketId,\
                    messageId++,\
                    g_requestPublish.qos,\
                    0,\
                    g_requestPublish.topic,\
                    g_requestPublish.length);
        }break;

        case (XMQTT_ATT_PUBLISH_PAYLOAD) :
        {
            memcpy(commandX, g_requestPublish.payload, g_requestPublish.length);
            length = g_requestPublish.length;
        }break;

        default :
        {

        }
    }

    return length;
}

static int8_t   respondPublish(const AtCommands_t *atable,uint8_t atableIndex, uint32_t length, uint8_t *buffer, int8_t status)
{
    return 1;
}

static int16_t cbStatusPublish(uint8_t atableIndex, uint8_t status, uint32_t length, void* buffer)
{
    return 0;
}

/***********************************************************************************************************************
 *                                            P R I V A T E  F U N C T I O N S
 **********************************************************************************************************************/

/**************************************************
 * FINITE STATE MACHINES
 **************************************************/

/*
 * MAIN
 */
static void fsmMain(void)
{
    if( g_statusNetwork && (XMQTT_FSMS_MAIN_ACTIVE_NETWORK != g_fsmContextMain.state) )
    {
        fsmTransitionMain(XMQTT_FSMS_MAIN_ACTIVE_NETWORK);
    }

    if( !g_statusNetwork && (XMQTT_FSMS_MAIN_NO_NETWORK != g_fsmContextMain.state) )
    {
        fsmTransitionMain(XMQTT_FSMS_MAIN_NO_NETWORK);
    }

    switch(g_fsmContextMain.state)
        {
            case (XMQTT_FSMS_MAIN_IDLE) :
                {

                }break;

            case (XMQTT_FSMS_MAIN_NO_NETWORK) :
                {

                }break;

            case (XMQTT_FSMS_MAIN_ACTIVE_NETWORK) :
                {
                    fsmConnect();

                    //fsmDisconnect();

                    fsmSubscribe();

                    //fsmUnsubscribe();

                    fsmPublish();
                }break;

            default :
                {

                }
        }
}

static void fsmTransitionMain(uint8_t switchState)
{
    g_fsmContextMain.previousState = g_fsmContextMain.state;

    switch(switchState)
    {
        case (XMQTT_FSMS_MAIN_IDLE) :
            {
                g_fsmContextMain.state = XMQTT_FSMS_MAIN_IDLE;
            }break;

        case (XMQTT_FSMS_MAIN_NO_NETWORK) :
            {
                g_statusMqttManager     = 0;
                g_fsmContextMain.state = XMQTT_FSMS_MAIN_NO_NETWORK;
            }break;

        case (XMQTT_FSMS_MAIN_ACTIVE_NETWORK) :
            {
                g_statusMqttManager     = 1;
                g_fsmContextMain.state = XMQTT_FSMS_MAIN_ACTIVE_NETWORK;
            }break;

        default :
            {

            }
    }

    NETWORK_PRINT_INFO("NET     FSM_MAIN %02d -> %02d\r\n",g_fsmContextMain.previousState, g_fsmContextMain.state);
}

/*
 * CONNECT FSM
 */
static void fsmConnect(void)
{
    switch(g_fsmContextConnect.state)
    {
        case(XMQTT_FSMS_CONNECT_IDLE) :
            {

            }break;

        case(XMQTT_FSMS_CONNECT_SUPERVISE) :
            {
                static uint8_t i = 0;

                if(g_mqttSockets[0].occupy == 1 && g_mqttSockets[0].connection == 0)
                {
                    XMQTT_connect(g_mqttSockets[0].config);
                    g_mqttSockets[0].occupy = 0;
                } 

                if(i == SIZE_QUEUE_REQUEST_CONNECT)
                {
                    i = 0;
                }

                for( ; i < SIZE_QUEUE_REQUEST_CONNECT ; i++)
                {
                    if(g_qConnect[i].serviceStatus)
                    {
                        g_requestConnect = g_qConnect[i];

                        g_requestConnect.connection     = 0;
                        g_requestConnect.occupy         = 0;
                        g_requestConnect.socketId       = 99;

                        g_requestConnect.flagSslFiles   = 0;
                        g_requestConnect.flagSslConfig  = 0;
                        g_requestConnect.flagMqttConfig = 0;
                        g_requestConnect.flagClose      = 0;
                        g_requestConnect.flagOpen       = 0;
                        g_requestConnect.flagConnect    = 0;

                        g_qConnect[i].serviceStatus     = 0;

                        if(g_requestConnect.config.ssl)
                        {
                            fsmTransitionConnect(XMQTT_FSMS_CONNECT_SSL_FILES);
                        }
                        else
                        {
                            fsmTransitionConnect(XMQTT_FSMS_CONNECT_MQTT_CONFIG);
                        }
                        
                        break;
                    }
                }
            }break;

        case(XMQTT_FSMS_CONNECT_SSL_FILES) :
            {
                int8_t status = -1;
 
                switch(g_fsmContextConnect.mode)
                {
                    case(XMQTT_FSMM_INITIATED) :
                    {
                        g_requestConnect.flagSslFiles = 0;
                        status = AtStart(atableSslFiles, (uint8_t)XMQTT_ATABLE_SSL_MAX, fillerSslFiles,respondSslFiles,cbStatusSslFiles);

                        if(status == AT_SUCCESS)
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_EXECUTED;
                        }
                        else
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                        }
                    }break;
 
                    case(XMQTT_FSMM_EXECUTED) :
                    {
                        if(g_requestConnect.flagSslFiles == 1)
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_COMPLETED;
                        }
                    }break;
 
                    case(XMQTT_FSMM_COMPLETED) :
                    {
                        fsmTransitionConnect(XMQTT_FSMS_CONNECT_SSL_CONFIG);
                    }break;
 
                    default :
                    {
                        //TODO: ???
                    }
                }
            }break;

        case(XMQTT_FSMS_CONNECT_SSL_CONFIG):
            {
                int8_t status = -1;
 
                switch(g_fsmContextConnect.mode)
                {
                    case(XMQTT_FSMM_INITIATED) :
                    {
                        g_requestConnect.flagSslConfig = 0;
                        status = AtStart(atableSslConfig, (uint8_t)XMQTT_ATT_SSL_CONFIG_MAX, fillerSslConfig, respondSslConfig, cbStatusSslConfig);

                        if(status == AT_SUCCESS)
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_EXECUTED;
                        }
                        else
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                        }
                    }break;
 
                    case(XMQTT_FSMM_EXECUTED) :
                    {
                        if(g_requestConnect.flagSslConfig == 1)
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_COMPLETED;
                        }
                    }break;
 
                    case(XMQTT_FSMM_COMPLETED) :
                    {
                        fsmTransitionConnect(XMQTT_FSMS_CONNECT_MQTT_CONFIG);
                    }break;
 
                    default :
                    {
                        //TODO: ???
                    }
                }
            }break;

        case(XMQTT_FSMS_CONNECT_MQTT_CONFIG) :
            {
                int8_t status = -1;

                switch(g_fsmContextConnect.mode)
                {
                    case(XMQTT_FSMM_INITIATED):
                    {
                        if(g_requestConnect.config.socketId == XMQTT_SOCKET_ANY)
                        {
                            int8_t socketId = getUnoccupiedSocket();

                            if(socketId < 0)
                            {
                                //RETURN ERROR STATUS BY CB
                            }
                            else
                            {
                                g_requestConnect.socketId = socketId;
                            }
                        }
                        else
                        {
                            int8_t socketId = isSocketFree(g_requestConnect.config.socketId);

                            if(socketId < 0)
                            {
                                //RETURN ERROR STATUS BY CB
                            }
                            else
                            {
                                g_requestConnect.socketId = socketId;
                            }
                        }

                        status = AtStart(atableConfig, XMQTT_ATT_CONFIG_MAX, fillerConfiguration,respondConfiguration,cbStatusConfiguration);

                        if(status == AT_SUCCESS)
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_EXECUTED;
                        }
                        else
                        {
                            //NEED TIMEOUT
                            g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                        }
                    }break;

                    case(XMQTT_FSMM_EXECUTED):
                    {
                        if(g_requestConnect.flagMqttConfig == 1)
                        {
                            g_fsmContextConnect.mode = XMQTT_FSMM_COMPLETED;
                        }
                        else
                        {
                            //TODO: TIMEOUT
                        }
                    }break;

                    case(XMQTT_FSMM_COMPLETED): //completed the by success status of at callback
                    {
                        fsmTransitionConnect(XMQTT_FSMS_CONNECT_CLOSE_TCP);
                    }break;

                    default:
                    {
                        //NEED TIMEOUT
                    }
                }
            }break;

        case(XMQTT_FSMS_CONNECT_CLOSE_TCP) :
            {
                    int8_t status = -1;

                    switch(g_fsmContextConnect.mode)
                    {
                        case(XMQTT_FSMM_INITIATED):
                            {
                                g_requestConnect.flagClose = 0;

                                status = AtStart(atableClose, XMQTT_ATT_CLOSE_MAX, fillerClose,respondClose,cbStatusClose);

                                if(status == AT_SUCCESS)
                                {
                                    g_fsmContextConnect.mode = XMQTT_FSMM_EXECUTED;

                                    RESET_TIMER(g_fsmContextConnect.timeout, 10 * 1000);
                                }
                                else
                                {
                                    g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                                }
                            }break;

                        case(XMQTT_FSMM_EXECUTED):
                            {
                                if(g_requestConnect.flagClose == 1)
                                {
                                    g_fsmContextConnect.mode = XMQTT_FSMM_COMPLETED;
                                }
                                else
                                {
                                    if(IS_TIMER_ELAPSED(g_fsmContextConnect.timeout))
                                    {
                                        g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                                    }
                                }
                            }break;

                        case(XMQTT_FSMM_COMPLETED): //completed by close urc OR CBSTATUS
                            {
                                fsmTransitionConnect(XMQTT_FSMS_CONNECT_OPEN_TCP);
                            }break;

                        default:
                            {
                                //TODO:???
                            }
                    }
            }break;

        case(XMQTT_FSMS_CONNECT_OPEN_TCP) :
            {
                int8_t status = -1;

                switch(g_fsmContextConnect.mode)
                {
                    case(XMQTT_FSMM_INITIATED):
                        {
                            g_requestConnect.flagOpen = 0;
                            status = AtStart(atableOpen, XMQTT_ATT_OPEN_MAX, fillerOpen,respondOpen,cbStatusOpen);

                            if(status == AT_SUCCESS)
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_EXECUTED;
                                RESET_TIMER(g_fsmContextConnect.timeout, 60 * 1000);
                            }
                            else
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                            }
                        }break;

                    case(XMQTT_FSMM_EXECUTED):
                        {
                            if(g_requestConnect.flagOpen == 1)
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_COMPLETED;
                            }
                            else
                            {
                                if(IS_TIMER_ELAPSED(g_fsmContextConnect.timeout))
                                {
                                    fsmTransitionConnect(XMQTT_FSMS_CONNECT_CLOSE_TCP); //TODO:MAYBE ABORT?
                                }
                            }
                        }break;

                    case(XMQTT_FSMM_COMPLETED): //completed by open urc
                        {
                            fsmTransitionConnect(XMQTT_FSMS_CONNECT_MQTT_CONNECT);
                        }

                    default:
                        {

                        }
                }

            }break;

        case(XMQTT_FSMS_CONNECT_MQTT_CONNECT) :
            {
                int8_t status = -1;

                switch(g_fsmContextConnect.mode)
                {
                    case(XMQTT_FSMM_INITIATED):
                        {
                            g_requestConnect.flagConnect = 0;
                            status = AtStart(atableConnect, XMQTT_ATT_CONNECT_MAX, fillerConnect,respondConnect,cbStatusConnect);

                            if(status == AT_SUCCESS)
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_EXECUTED;

                                RESET_TIMER(g_fsmContextConnect.timeout, 60 * 1000);
                            }
                            else
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                            }
                        }break;

                    case(XMQTT_FSMM_EXECUTED):
                        {
                            if(g_requestConnect.flagConnect == 1)
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_COMPLETED;
                            }
                            else
                            {
                                if(IS_TIMER_ELAPSED(g_fsmContextConnect.timeout))
                                {
                                    fsmTransitionConnect(XMQTT_FSMS_CONNECT_OPEN_TCP);
                                }
                            }
                        }break;

                    case(XMQTT_FSMM_COMPLETED): //completed by connect urc
                        {
                            g_mqttSockets[g_requestConnect.socketId].config     = g_requestConnect.config;
                            g_mqttSockets[g_requestConnect.socketId].occupy     = 1;
                            g_mqttSockets[g_requestConnect.socketId].connection = 1;
                            g_mqttSockets[g_requestConnect.socketId].socketId   = g_requestConnect.socketId;
                            g_requestConnect.config.cb_connect(1);
                            fsmTransitionConnect(XMQTT_FSMS_CONNECT_SUPERVISE);
                        }break;

                    default:
                        {

                        }
                }
            }break;

        default:
            {

            }
    }
}

static void fsmTransitionConnect(uint8_t switchState)
{
    g_fsmContextConnect.previousState = g_fsmContextConnect.state;

    switch(switchState)
    {
        case(XMQTT_FSMS_CONNECT_IDLE) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_IDLE;
            }break;

        case(XMQTT_FSMS_CONNECT_SUPERVISE) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_SUPERVISE;
            }break;

        case(XMQTT_FSMS_CONNECT_SSL_FILES) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_SSL_FILES;
                g_fsmContextConnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case(XMQTT_FSMS_CONNECT_SSL_CONFIG) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_SSL_CONFIG;
                g_fsmContextConnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case(XMQTT_FSMS_CONNECT_MQTT_CONFIG) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_MQTT_CONFIG;
                g_fsmContextConnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case(XMQTT_FSMS_CONNECT_CLOSE_TCP) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_CLOSE_TCP;
                g_fsmContextConnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case(XMQTT_FSMS_CONNECT_OPEN_TCP) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_OPEN_TCP;
                g_fsmContextConnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case(XMQTT_FSMS_CONNECT_MQTT_CONNECT) :
            {
                g_fsmContextConnect.state = XMQTT_FSMS_CONNECT_MQTT_CONNECT;
                g_fsmContextConnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        default:
            {

            }
    }

    NETWORK_PRINT_INFO("NET     FSM_CONNECT %02d -> %02d\r\n",g_fsmContextConnect.previousState, g_fsmContextConnect.state);
}

/*
 * DISCONNECT FSM
 */
static void fsmDisconnect(void)
{
    switch(g_fsmContextDisconnect.state)
    {
        case (XMQTT_FSMS_DISCONNECT_IDLE) :
            {

            }break;

        case (XMQTT_FSMS_DISCONNECT_SUPERVISE) :
            {
                static uint8_t i = 0;
 
                if(i == SIZE_QUEUE_REQUEST_DISCONNECT)
                {
                    i = 0;
                }
 
                for( ; i < SIZE_QUEUE_REQUEST_DISCONNECT ; i++)
                {
                    if(g_qDisconnect[i].serviceStatus)
                    {
                        if( g_mqttSockets[ (g_qDisconnect[i].socketId) ].connection )
                        {
                            g_requestDisconnect = g_qDisconnect[i];
 
                            g_qDisconnect[i].serviceStatus = 0;
 
                            fsmTransitionDisconnect(XMQTT_FSMS_DISCONNECT_MQTT_DISCONNECT);
                       
                            break;
                        }
                        else
                        {
                            //TODO: ???
                        }
                    }
                }
            }break;

        case (XMQTT_FSMS_DISCONNECT_MQTT_DISCONNECT) :
            {
                int8_t status = -1;
 
                switch(g_fsmContextDisconnect.mode)
                {
                    case(XMQTT_FSMM_INITIATED):
                    {
                        g_requestDisconnect.flagDisconnect = 0;
                        status = AtStart(atableDisconnect, XMQTT_ATT_DISCONNECT_MAX, fillerDisconnect, respondDisconnect, cbStatusDisconnect);
 
                        if(status == AT_SUCCESS)
                        {
                            g_fsmContextDisconnect.mode = XMQTT_FSMM_EXECUTED;
                        }
                        else
                        {
                            g_fsmContextDisconnect.mode = XMQTT_FSMM_INITIATED;
                        }
                    }break;
 
                    case(XMQTT_FSMM_EXECUTED):
                    {
                        if(g_requestDisconnect.flagDisconnect == 1)
                        {
                            g_fsmContextDisconnect.mode = XMQTT_FSMM_COMPLETED;
                        }
                        else
                        {
                            //TODO:TIMEOUT
                        }
                    }break;
 
                    case(XMQTT_FSMM_COMPLETED):
                    {
                        fsmTransitionDisconnect(XMQTT_FSMS_DISCONNECT_CLOSE_TCP);
                    }break;
 
                    default:
                    {
 
                    }
                }break;
            }

        case (XMQTT_FSMS_DISCONNECT_CLOSE_TCP) :
            {
                int8_t status = -1;
 
                switch(g_fsmContextConnect.mode)
                {
                    case(XMQTT_FSMM_INITIATED):
                        {
                            g_requestDisconnect.flagClose = 0;

                            status = AtStart(atableClose, XMQTT_ATT_CLOSE_MAX, fillerClose,respondClose,cbStatusCloseDisconnect);

                            if(status == AT_SUCCESS)
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_EXECUTED;

                                RESET_TIMER(g_fsmContextConnect.timeout, 10 * 1000);
                            }
                            else
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                            }
                        }break;

                    case(XMQTT_FSMM_EXECUTED):
                        {
                            if(g_requestDisconnect.flagClose == 1)
                            {
                                g_fsmContextConnect.mode = XMQTT_FSMM_COMPLETED;
                            }
                            else
                            {
                                if(IS_TIMER_ELAPSED(g_fsmContextConnect.timeout))
                                {
                                    g_fsmContextConnect.mode = XMQTT_FSMM_INITIATED;
                                }
                            }
                        }break;

                    case(XMQTT_FSMM_COMPLETED): //completed by close urc OR CBSTATUS
                        {
                            g_mqttSockets[g_requestDisconnect.socketId].connection = 0;
                            fsmTransitionConnect(XMQTT_FSMS_DISCONNECT_SUPERVISE);
                        }break;

                    default:
                        {
                            //TODO:???
                        }
                }
            }break;

        default :
            {

            }
    }
}

static void fsmTransitionDisconnect(uint8_t switchState)
{
    g_fsmContextDisconnect.previousState = g_fsmContextDisconnect.state;

    switch(switchState)
    {
        case (XMQTT_FSMS_DISCONNECT_IDLE) :
            {
                g_fsmContextDisconnect.state = XMQTT_FSMS_DISCONNECT_IDLE;
                g_fsmContextDisconnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case (XMQTT_FSMS_DISCONNECT_SUPERVISE) :
            {
                g_fsmContextDisconnect.state = XMQTT_FSMS_DISCONNECT_SUPERVISE;
                g_fsmContextDisconnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case (XMQTT_FSMS_DISCONNECT_MQTT_DISCONNECT) :
            {
                g_fsmContextDisconnect.state = XMQTT_FSMS_DISCONNECT_MQTT_DISCONNECT;
                g_fsmContextDisconnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case (XMQTT_FSMS_DISCONNECT_CLOSE_TCP) :
            {
                g_fsmContextDisconnect.state = XMQTT_FSMS_DISCONNECT_CLOSE_TCP;
                g_fsmContextDisconnect.mode  = XMQTT_FSMM_INITIATED;
            }break;

        default :
            {

            }
    }

   NETWORK_PRINT_INFO("NET     FSM_DISCONNECT %02d -> %02d\r\n",g_fsmContextDisconnect.previousState, g_fsmContextDisconnect.state);
}

/*
 * SUBSCRIBE FSM
 */

static void fsmSubscribe(void)
{
    switch(g_fsmContextSubscribe.state)
    {
        case (XMQTT_FSMS_SUBSCRIBE_IDLE):
            {

            }break;

        case (XMQTT_FSMS_SUBSCRIBE_SUPERVISE):
            {
                static uint8_t i = 0;

                if(i == SIZE_QUEUE_REQUEST_SUBSCRIBE)
                {
                    i = 0;
                }

                for( ; i < SIZE_QUEUE_REQUEST_SUBSCRIBE ; i++)
                {
                    if(g_qSubscribe[i].serviceStatus)
                    { 
                        if( g_mqttSockets[ (g_qSubscribe[i].socketId) ].connection )
                        {
                            g_requestSubscribe = g_qSubscribe[i];

                            g_qSubscribe[i].serviceStatus = 0;
 
                            fsmTransitionSubscribe(XMQTT_FSMS_SUBSCRIBE_SUB);
                        
                            break;
                        }
                        else
                        {
                            //TODO: ???
                        }
                    }
                }
            }break;

        case (XMQTT_FSMS_SUBSCRIBE_SUB):
            {
                int8_t status = -1;

                switch(g_fsmContextSubscribe.mode)
                {
                    case(XMQTT_FSMM_INITIATED):
                    {
                        g_requestSubscribe.flagSubscribe = 0;
                        status = AtStart(atableSubscribe, XMQTT_ATT_SUBSCRIBE_MAX, fillerSubscribe, respondSubscribe, cbStatusSubscribe);

                        if(status == AT_SUCCESS)
                        {
                            g_fsmContextSubscribe.mode = XMQTT_FSMM_EXECUTED;
                        }
                        else
                        {
                            g_fsmContextSubscribe.mode = XMQTT_FSMM_INITIATED;
                        }
                    }break;

                    case(XMQTT_FSMM_EXECUTED):
                    {
                        if(g_requestSubscribe.flagSubscribe == 1)
                        {
                            g_fsmContextSubscribe.mode = XMQTT_FSMM_COMPLETED;
                        }
                        else
                        {
                            //TODO:TIMEOUT
                        }
                    }break;

                    case(XMQTT_FSMM_COMPLETED):
                    {
                        fsmTransitionSubscribe(XMQTT_FSMS_SUBSCRIBE_SUPERVISE);
                    }break;

                    default:
                    {

                    }
                }
            }break;

        case (XMQTT_FSMS_SUBSCRIBE_ABORT):
            {
                XMQTT_subscribe(g_requestSubscribe.socketId, g_requestSubscribe.topic, g_requestSubscribe.length, g_requestSubscribe.qos);

                fsmTransitionSubscribe(XMQTT_FSMS_SUBSCRIBE_SUPERVISE);
            }break;

        default :
            {
                //TODO:???
            }
    }
}


static void fsmTransitionSubscribe(uint8_t switchState)
{
    g_fsmContextSubscribe.previousState = g_fsmContextSubscribe.state;

    switch(switchState)
    {
        case (XMQTT_FSMS_SUBSCRIBE_IDLE):
            {

            }break;

        case (XMQTT_FSMS_SUBSCRIBE_ABORT):
            {
                g_fsmContextSubscribe.state = XMQTT_FSMS_SUBSCRIBE_ABORT;
                g_fsmContextSubscribe.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case (XMQTT_FSMS_SUBSCRIBE_SUPERVISE):
            {
                g_fsmContextSubscribe.state = XMQTT_FSMS_SUBSCRIBE_SUPERVISE;
                g_fsmContextSubscribe.mode  = XMQTT_FSMM_INITIATED;
            }break;

        case (XMQTT_FSMS_SUBSCRIBE_SUB):
            {
                g_fsmContextSubscribe.state = XMQTT_FSMS_SUBSCRIBE_SUB;
                g_fsmContextSubscribe.mode  = XMQTT_FSMM_INITIATED;
            }break;

        default :
            {

            }
    }

    NETWORK_PRINT_INFO("NET     FSM_SUBSCRIBE %02d -> %02d\r\n",g_fsmContextSubscribe.previousState, g_fsmContextSubscribe.state);
}

/*
 * UNSUBSCRIBE FSM
 */

static void fsmUnsubscribe(void)
{
    switch(g_fsmContextUnsubscribe.state)
    {
        case (XMQTT_FSMS_UNSUBSCRIBE_IDLE) :
        {

        }break;

        case (XMQTT_FSMS_UNSUBSCRIBE_SUPERVISE) :
        {

        }break;

        case (XMQTT_FSMS_UNSUBSCRIBE_UNSUB) :
        {
            
        }break;

        default :
        {

        }
    }

}

static void fsmTransitionUnsubscribe(uint8_t switchState)
{
    g_fsmContextUnsubscribe.previousState = g_fsmContextUnsubscribe.state;

    switch(switchState)
    {
        case (XMQTT_FSMS_UNSUBSCRIBE_IDLE) :
        {

        }break;

        case (XMQTT_FSMS_UNSUBSCRIBE_SUPERVISE) :
        {

        }break;

        case (XMQTT_FSMS_UNSUBSCRIBE_UNSUB) :
        {

        }break;

        default :
        {

        }
    }

    NETWORK_PRINT_INFO("NET     FSM_UNSUBSCRIBE %02d -> %02d\r\n",g_fsmContextUnsubscribe.previousState, g_fsmContextUnsubscribe.state);
}

/*
 * PUBLISH FSM
 */

static void fsmPublish(void)
{
    switch(g_fsmContextPublish.state)
    {
        case (XMQTT_FSMS_PUBLISH_IDLE) :
            {

            }break;

        case (XMQTT_FSMS_PUBLISH_SUPERVISE) :
            {
                static uint8_t i=0;
                if(i==SIZE_QUEUE_REQUEST_PUBLISH)
                {
                    i=0;
                }
                for(;i<SIZE_QUEUE_REQUEST_PUBLISH;i++)
                {
                    if(g_qPublish[i].serviceStatus)
                    {
                        g_requestPublish = g_qPublish[i];
                        fsmTransitionPublish(XMQTT_FSMS_PUBLISH_PUB);
                        g_qPublish[i].serviceStatus=0;
                        break;
                    }
                }
            }break;

        case (XMQTT_FSMS_PUBLISH_PUB) :
            {
                int8_t status = -1;

                switch(g_fsmContextPublish.mode)
                {
                    case(XMQTT_FSMM_INITIATED):
                    {
                        status = AtStart(atablePublish, XMQTT_ATT_PUBLISH_MAX, fillerPublish, respondPublish, cbStatusPublish);

                        if(status == AT_SUCCESS)
                        {
                            g_fsmContextPublish.mode = XMQTT_FSMM_EXECUTED;

                            RESET_TIMER(g_fsmContextPublish.timeout, 60 * 1000);
                        }
                        else
                        {
                            g_fsmContextPublish.mode = XMQTT_FSMM_INITIATED;
                        }
                    }break;

                    case(XMQTT_FSMM_EXECUTED):
                    {
                        if(g_requestPublish.flagPublish == 1)
                        {
                            g_fsmContextPublish.mode = XMQTT_FSMM_COMPLETED;
                        }
                        else
                        {
                            if(IS_TIMER_ELAPSED(g_fsmContextPublish.timeout))
                            {
                                fsmTransitionPublish(XMQTT_FSMS_PUBLISH_PUB);
                            }
                        }
                    }break;

                    case(XMQTT_FSMM_COMPLETED): // SET BY +PUBEX URC
                    {
                        fsmTransitionPublish(XMQTT_FSMS_PUBLISH_SUPERVISE);
                    }break;

                    default:
                    {

                    }
                }
            }break;

        default :
            {

            }
    }
}

static void fsmTransitionPublish(uint8_t switchState)
{
    g_fsmContextPublish.previousState = g_fsmContextPublish.state;

    switch(switchState)
    {
        case (XMQTT_FSMS_PUBLISH_IDLE) :
            {
                g_fsmContextPublish.state = XMQTT_FSMS_PUBLISH_IDLE;
            }break;

        case (XMQTT_FSMS_PUBLISH_SUPERVISE) :
            {
                g_fsmContextPublish.state = XMQTT_FSMS_PUBLISH_SUPERVISE;
            }break;

        case (XMQTT_FSMS_PUBLISH_PUB) :
            {
                g_fsmContextPublish.state = XMQTT_FSMS_PUBLISH_PUB;
                g_fsmContextPublish.mode  = XMQTT_FSMM_INITIATED;
            }break;

        default :
            {

            }
    }

    NETWORK_PRINT_INFO("NET     FSM_PUBLISH %02d -> %02d\r\n",g_fsmContextPublish.previousState, g_fsmContextPublish.state);
}

/**************************************************
 * MQTT SOCKET HANDLING
 **************************************************/
static int8_t getUnoccupiedSocket(void)
{
    int8_t id;

    for(id = 0; id <= 5; id++)
    {
        if(g_mqttSockets[id].occupy == 0)
            break;
    }

    if(id <= 5)
    {
        return id;
    }
    else
    {
        return -1;
    }
}

static int8_t isSocketFree(uint8_t id)
{
    if(g_mqttSockets[id].occupy == 0)
    {
        return id;
    }
    else
    {
        return -1;
    }
}
/**************************************************
 * MISC
 **************************************************/

static void logStatus(void)
{
    static uint32_t logTimer = 0;

    if(IS_TIMER_ELAPSED(logTimer))
    {
        NETWORK_PRINT_INFO("NET: 0 | MQTT(0-5): %d.%d.%d.%d.%d.%d\r\n",\
                g_mqttSockets[0].connection,\
                g_mqttSockets[1].connection,\
                g_mqttSockets[2].connection,\
                g_mqttSockets[3].connection,\
                g_mqttSockets[4].connection,\
                g_mqttSockets[5].connection);
        RESET_TIMER(logTimer, 5 * 1000);
    }
}
