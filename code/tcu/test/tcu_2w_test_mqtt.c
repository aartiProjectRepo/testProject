/**
 * @file        tcu_2w_test_mqtt.c
 *
 * @copyright   Accolade Electronics Pvt Ltd, 2023-24
 *              All Rights Reserved
 *              UNPUBLISHED, LICENSED SOFTWARE.
 *              Accolade Electronics, Pune
 *              CONFIDENTIAL AND PROPRIETARY INFORMATION
 *              WHICH IS THE PROPERTY OF M/s Accolade Electronics.
 *
 * @date        12 December 2023
 * @author      Diksha J     <diksha.jadhav@accoladeelectronics.com>
 * 
 * @brief       TEST APPLICATION FOR EC200 MQTT STACK MANAGER
 */

#include <stdio.h>

#include "x_mqtt_manager.h"
#include "x_mqtt_certificates.h"

#include "modem_port.h"


xmqtt_configuration_t tempHandle, tempHandle2;

//char *ip = "tmlcvpdpdsqa01.inservices.tatamotors.com";
//char *port = "9993";


char *ip = "iot-prod-p-ap.sibros.tech";
char *port = "8883";

char *ip2 = "broker.hivemq.com";
char *port2 = "1883";


char *client = "nitin123";
char *topicX[6]={"$aws/certificates/create/json","/aepl/2w/internal-demo","/topic/test2","/topic/test3","/topic/test4","/topic/test5"};
char *payloadX[6]={"{}","Hello World!1","Hello World!2","Hello World!3","Hello World!4","Hello World!5"};
uint16_t lengthX[6]={2,13,13,13,13,13};

char *topicSub[6] = {"$aws/certificates/create/json/rejected","subtopic/test1","subtopic/test2","subtopic/test3","subtopic/test4","subtopic/test5"};
uint16_t topicLen[6] = {38,14,14,14,14,14};

void cbConnect(uint8_t status);
void cb_subscribe(uint8_t status);
void cb_publish(uint8_t status);

void cbConnect(uint8_t status)
{
    NETWORK_PRINT_INFO("CONNECTED:%d\r\n",status);
}

void cb_subscribe(uint8_t status)
{
    NETWORK_PRINT_INFO("SUBCRIBED:%d\r\n",status);
}

void cb_publish(uint8_t status)
{
    NETWORK_PRINT_INFO("PUBLISHED:%d\r\n",status);
}

appMqtt_Init()
{
    tempHandle.socketId     = XMQTT_SOCKET_ANY;
    tempHandle.ip           = ip2;
    tempHandle.port         = port2;
    tempHandle.client       = "nitin121";
    tempHandle.ssl          = 0;

    tempHandle.cb_connect   = cbConnect;
    tempHandle.cb_subscribe = cb_subscribe;
    tempHandle.cb_publish   = cb_publish;
    
    tempHandle.ssl_cert_filename_ca        = "cacert.pem";
    tempHandle.ssl_cert_filename_length_ca = 10;
    tempHandle.ssl_cert_ca                 = ca;
    tempHandle.ssl_cert_length_ca          = size_ca;

    tempHandle.ssl_cert_filename_cc        = "client.pem";
    tempHandle.ssl_cert_filename_length_cc = 10;
    tempHandle.ssl_cert_cc                 = cc;
    tempHandle.ssl_cert_length_cc          = size_cc;

    tempHandle.ssl_cert_filename_ck        = "user_key.pem";
    tempHandle.ssl_cert_filename_length_ck = 12;
    tempHandle.ssl_cert_ck                 = ck;
    tempHandle.ssl_cert_length_ck          = size_ck;
    
    XMQTT_connect(tempHandle);

    // tempHandle2.socketId = XMQTT_SOCKET_ANY;
    // tempHandle2.ip       = ip2;
    // tempHandle2.port     = port2;
    // tempHandle2.ssl      = 0;

    // tempHandle2.cb_connect   = cbConnect;
    // tempHandle2.cb_subscribe = cb_subscribe;
    // tempHandle2.cb_publish   = cb_publish;

    // tempHandle2.client   = "nitin122";
    // XMQTT_connect(tempHandle2);

    // tempHandle2.client   = "nitin123";
    // XMQTT_connect(tempHandle2);

    // tempHandle2.client   = "nitin124";
    // XMQTT_connect(tempHandle2);

    // tempHandle2.client   = "nitin125";
    // XMQTT_connect(tempHandle2);

    // tempHandle2.client   = "nitin126";
    // XMQTT_connect(tempHandle2);
}

appMqtt_Exe()
{
    static uint32_t publishTimer = 0;
    
    if(IS_TIMER_ELAPSED(publishTimer))
    {
        if(XMQTT_isMqtt())
        {
            XMQTT_publish(0,topicX[1],payloadX[0],lengthX[0], 1); 
            static uint8_t once0 = 0;
            if(once0 == 0)
            {
                //XMQTT_subscribe(0, topicSub[0], topicLen[0], 1);
                once0 = 1;
            }
        }
    
        //        if( 0 == XMQTT_publish(1,topicX[1],payloadX[1],lengthX[1]) )
//        {
//            static uint8_t once1 = 0;
//            if(once1 == 0)
//            {
//                XMQTT_subscribe(1, topicSub[1], topicLen[1]);
//                once1 = 1;
//            }
//        }
//        if( 0 == XMQTT_publish(2,topicX[2],payloadX[2],lengthX[2]) )
//        {
//            static uint8_t once2 = 0;
//            if(once2 == 0)
//            {
//                XMQTT_subscribe(2, topicSub[2], topicLen[2]);
//                once2 = 1;
//            }
//        }
//        if( 0 == XMQTT_publish(3,topicX[3],payloadX[3],lengthX[3]) )
//        {
//            static uint8_t once3 = 0;
//            if(once3 == 0)
//            {
//                XMQTT_subscribe(3, topicSub[3], topicLen[3]);
//                once3 = 1;
//            }
//        }
//        if( 0 == XMQTT_publish(4,topicX[4],payloadX[4],lengthX[4]) )
//        {
//            static uint8_t once4 = 0;
//            if(once4 == 0)
//            {
//                XMQTT_subscribe(4, topicSub[4], topicLen[4]);
//                once4 = 1;
//            }
//        }
//        if( 0 == XMQTT_publish(5,topicX[5],payloadX[5],lengthX[5]) )
//        {
//            static uint8_t once5 = 0;
//            if(once5 == 0)
//            {
//                XMQTT_subscribe(5, topicSub[5], topicLen[5]);
//                once5 = 1;
//            }
//        }
        
        RESET_TIMER(publishTimer, 10 * 1000);
    }
}