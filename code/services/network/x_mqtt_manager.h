/**
 * @file        x_mqtt_manager.h
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

#ifndef TCU_SERVICES_NETWORK_STACK_MQTT_X_MQTT_MANAGER_H_
#define TCU_SERVICES_NETWORK_STACK_MQTT_X_MQTT_MANAGER_H_

#include <stdint.h>

typedef enum
{
    XMQTT_SOCKET_0   =  0,
    XMQTT_SOCKET_1   =  1,
    XMQTT_SOCKET_2   =  2,
    XMQTT_SOCKET_3   =  3,
    XMQTT_SOCKET_4   =  4,
    XMQTT_SOCKET_5   =  5,
    XMQTT_SOCKET_ANY = 99
}XMQTT_SOCKET_IDS;

typedef struct
{
    char *ip;
    char *port;
    char *client;

    uint8_t ssl;
    uint8_t autoReconnect;
    uint8_t socketId;

    void (*cb_connect)    (uint8_t);
    void (*cb_disconnect) (uint8_t);
    void (*cb_subscribe)  (uint8_t);
    void (*cb_unsubscribe)(uint8_t);
    void (*cb_publish)    (uint8_t);

    uint8_t  *ssl_cert_filename_ca;
    uint8_t  ssl_cert_filename_length_ca;
    const char  *ssl_cert_ca;
    uint16_t ssl_cert_length_ca;

    uint8_t  *ssl_cert_filename_cc;
    uint8_t  ssl_cert_filename_length_cc;
    const char  *ssl_cert_cc;
    uint16_t ssl_cert_length_cc;

    uint8_t  *ssl_cert_filename_ck;
    uint8_t  ssl_cert_filename_length_ck;
    const char  *ssl_cert_ck;
    uint16_t ssl_cert_length_ck;
    
}xmqtt_configuration_t;

typedef struct {
    uint8_t lock;

    uint8_t socketId;
    uint8_t topicId;

    uint8_t topic[100];
    uint8_t topicLength;
    uint8_t *payload;
    uint16_t payloadLength;
}xmqtt_mailBox_t;

void XMQTT_Init   (void);
void XMQTT_Execute(void);

int8_t  XMQTT_connect    (xmqtt_configuration_t mqttHandle);
int8_t  XMQTT_disconnect (uint8_t socketId);
int8_t  XMQTT_subscribe  (uint8_t socket_ID, char *topic, uint16_t length, uint8_t qos);
int8_t  XMQTT_unsubscribe(uint8_t socket_ID, char *topic, uint16_t length);
int8_t  XMQTT_publish    (uint8_t socket_ID, char *topic, char *payload, uint16_t payloadLenth, uint8_t qos);

uint8_t XMQTT_isConnect  (uint8_t socketId);
uint8_t XMQTT_isSubscribe(uint8_t socketId, uint8_t *topic);
uint8_t XMQTT_isMqtt     (void);

#endif
