#ifndef __CALIBRATION_H
#define __CALIBRATION_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32g0xx_hal.h"
#include <stdbool.h>

/* Version Information */
#define IOT_SW_VERSION "0002"
#define IOT_BUILD_DATE "16052025"

/* FTP Server Configuration */
#define FTP_SERVER_IPADDRESS "13.232.191.212"
#define FTP_SERVER_PORTNUM "21"
#define FTP_SERVER_USERNAME "ecudown"
#define FTP_SERVER_PASSWORD "haD_8A@usw"
#define DOMAIN_NAME "ecu.revoltmotors.com"

/* MQTT Server Configuration */
#define MQTT_SERVER_IPADDRESS "13.235.101.231"
#define MQTT_SERVER_PORTNUM "1883"
#define MQTT_APN "m2m.myrevolt.com"
#define PROTOCOL 'M'       // M - MQTT, H - HTTP, T - TCP, U - UDP
#define MQTT_TOPIC "rev25" 
#define MQTT_USERNAME "RovoSync"
#define MQTT_PASSWORD "Yt7rt4hgTVhfY"

/*Transmission Rates*/
#define TX_RATE_IGN_ON 10       
#define TX_RATE_IGN_OFF 20     
#define TX_RATE_BATT_REMOVED 30 

/*SMS Configuration*/
#define SMS_MOBILE_NUMBER "+918373958568"

#endif
