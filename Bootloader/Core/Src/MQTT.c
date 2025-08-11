/**
  ******************************************************************************
  * @file          : MQTT.c
  * @brief         : MQTT protocol logics
  ******************************************************************************
  // APP
  // Version  : 1301,
  // Author   : Brij Z
  // Date	  : 05-08-2025
  ******************************************************************************
**/
#if 0
#include "stm32g0b1xx.h"
#include "stm32g0xx.h"
#include "APP.h"
#include "GSM.H"
#include "COMM.H"
#include "CAN.h"
#include "FTP.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "MQTT.h"
//*****************************************************************************
// FUNCTION PROTOTYPE
//*****************************************************************************
bool bMQTT_CheckAndConnect_Exe(const char *topic);
bool bMQTT_SubscribeTopic_Exe(const char *topic);
bool bMQTT_SendPublishCmd_Exe(const char *topic, const char *payload);
void vMQTT_Incoming_Msg(void);
void parse_and_transmit_qmtrecv(const char *msg);
void handle_payload(const char *payload);
bool bMQTT_PublishPayload_Exe(void);
bool bMQTT_Publish_VehicleState(void);
bool bMQTT_Publish_CriticalData(void);
bool bMQTT_Publish_ResetFWcheckkDateData(void);
void vMQTT_Disconnect_Exe(void);
//*****************************************************************************
// Buffer
//*****************************************************************************
extern uint8_t ucCAN_eRxData[8];
//*****************************************************************************
// Structures
//*****************************************************************************
extern TsGSMData GSMData;
extern TsGSMStatus GSMStatus;
extern TsFTPData FTPData;
extern TsCAN CANData;
extern TsAPP APPStatus;
extern const TsEEPROMConfig EEPROMCONFIG;
extern TsAPP_eTimer TIMERData;
extern TsGPSData GPSData;
extern TsAPP_eConfig APPCONFIG;
//*****************************************************************************
// Handles
//*****************************************************************************
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
//*****************************************************************************

//---------------------------------------------
// MQTT Authentication and Connection Setup
//---------------------------------------------
bool bMQTT_CheckAndConnect_Exe(const char *topic)
{
    bool bStatus = false;
    uint8_t resp;
    char atCmd[150];

    if (GSMStatus.bMQTTConnected)
        return true;

    // Step 1: Configure authentication (client ID, username, password)
    sprintf(atCmd, "AT+QMTCFG=\"SSL\",0,0\r\n"); // Disable SSL for MQTT connection
    resp = ucGSM_eProcessATCmd_Exe(atCmd, ATRESPONSE_OK, 500, 3000);
    if (resp != RESPONSE_MATCHING)
        return false;

    // Step 2: Open TCP connection to broker
    sprintf(atCmd, "AT+QMTOPEN=0,\"%s\",%s\r", EEPROMCONFIG.cLocationIPAdd, EEPROMCONFIG.cLocationPortNum);
    resp = ucGSM_eProcessATCmd_Exe(atCmd, ATRESPONSE_OK, 500, 3000);
    if (resp != RESPONSE_MATCHING)
        return false;

    // Step 2.1: Wait for unsolicited +QMTOPEN: 0,0
    bStatus = ucGSM_eWaitForATResponse("+QMTOPEN: 0,0", 500, 5000);
    if (!bStatus)
        return false;

    // Step 3: Send CONNECT packet (Client ID, Username, Password)
    snprintf(atCmd, sizeof(atCmd), "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n", APPCONFIG.cIMEI, EEPROMCONFIG.cMQTT_Username, EEPROMCONFIG.cMQTT_Password);

    resp = ucGSM_eProcessATCmd_Exe(atCmd, ATRESPONSE_OK, 7000, 3000);
    if (resp != RESPONSE_MATCHING)
        return false;

    // Step 3.1: Wait for unsolicited +QMTCONN: 0,0,0 (success)
    bStatus = ucGSM_eWaitForATResponse("+QMTCONN: 0,0,0", 500, 3000);
    if (resp != RESPONSE_MATCHING)
        return false;

    vAPP_eFeedTheWDT_Exe();

    sprintf(atCmd, "AT+QMTSUB=0,1,\"%s\",0\r", topic);
    resp = ucGSM_eProcessATCmd_Exe(atCmd, ATRESPONSE_OK, 500, 3000); // Wait for '>' prompt
    if (resp != RESPONSE_MATCHING)
        return false;

    vAPP_eFeedTheWDT_Exe();

    bMQTT_SubscribeTopic_Exe(APPCONFIG.cIMEI);

    // Update connection status and return
    GSMStatus.bMQTTConnected = bStatus;
    return bStatus;
}

//---------------------------------------------
// MQTT Subscribe Wrapper
//---------------------------------------------
bool bMQTT_SubscribeTopic_Exe(const char *topic)
{
    char atCmd[128];
    bool bStatus;

    sprintf(atCmd, "AT+QMTSUB=0,1,\"%s\",0\r", topic);

    bStatus = ucGSM_eProcessATCmd_Exe(atCmd, ATRESPONSE_OK, 500, 3000); // Wait for '>' prompt
    if (bStatus != RESPONSE_MATCHING)
        return false;

    else
    {
        return true;
    }
    return bStatus;
}

//---------------------------------------------
// MQTT Publish Wrapper
//---------------------------------------------
bool bMQTT_SendPublishCmd_Exe(const char *topic, const char *payload)
{
    char atCmd[128];
    bool bStatus;

    int len = strlen(payload);

    // Step 1: Send the AT command to initiate publish
    sprintf(atCmd, "AT+QMTPUB=0,0,0,1,\"%s\"\r\n", topic); // QOS=0, Retain=0

    bStatus = ucGSM_eProcessATCmd_Exe(atCmd, ATRESPONSE_SEND, 200, 3000); // Wait for '>' prompt
    if (bStatus != RESPONSE_MATCHING)
        return false;

    // Step 2: Send payload
    vGSM_SendString(payload);
    vGSM_SendByte(0x1A); // End of message (CTRL+Z)

    // Step 3: Wait for publish confirmation
    bStatus = ucGSM_eWaitForATResponse("+QMTPUB: 0,0,0", 200, 5000);
    return bStatus;
}

void vMQTT_Incoming_Msg()
{
    if (GSMStatus.GSM_MessageReady == 1)
    {
        GSMStatus.GSM_MessageReady = 0;
        for (int try = 1; try < 50; try++)
        {
            parse_and_transmit_qmtrecv((char *)GSM_DataBuffer);
        }
    }
}

void parse_and_transmit_qmtrecv(const char *input)
{
    const char *prefix = "+QMTRECV:";
    if (strstr(input, prefix) == NULL)
        return;

    char buffer[RX_BUFFER_SIZE];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token;
    int field_index = 0;

    token = strtok(buffer, ",");
    while (token != NULL)
    {
        field_index++;

        if (field_index == 5)
        { // 5th token includes message
            char *msg = token;

            // Trim surrounding quotes
            if (msg[0] == '"')
                msg++;
            char *end = strchr(msg, '"');
            if (end)
                *end = '\0';

            // Transmit over UART1
            //            HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            //            HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

            // Handle the message
            handle_payload(msg);
            break;
        }

        token = strtok(NULL, ",");
    }
}

void handle_payload(const char *payload)
{
    if (strcmp(payload, "Start") == 0)
    {
        APPStatus.bVehRunStatus = VEHICLE_RUN;
        GSMStatus.bMQTTVehicleCmd = 1;
        // HAL_UART_Transmit(&huart1, (uint8_t *)"VEHICLE START\n", 13, 500);
    }
    else if (strcmp(payload, "Stop") == 0)
    {
        APPStatus.bVehRunStatus = VEHICLE_STOP;
        GSMStatus.bMQTTVehicleCmd = 1;
        // HAL_UART_Transmit(&huart1, (uint8_t *)"VEHICLE STOP\n", 13, 500);
    }
    else if (strcmp(payload, "FWUpdate") == 0)
    {
        GSMStatus.bMQTTFWchkStatus = 1;
    }
    else if (strcmp(payload, "Diagnose") == 0)
    {
        GSMStatus.bMQTTCriticalDataCmd = 1;
    }
    else
    {
        const char *unknown = "Unknown command\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)unknown, strlen(unknown), HAL_MAX_DELAY);
    }
}

//---------------------------------------------
// Main MQTT Payload Publisher
//---------------------------------------------
bool bMQTT_PublishPayload_Exe(void)
{
    return bMQTT_SendPublishCmd_Exe(EEPROMCONFIG.cTopic, cAPP_eGlobalBuffer);
}

//---------------------------------------------
// Main MQTT Vehicle Status Publisher
//---------------------------------------------
bool bMQTT_Publish_VehicleState(void)
{
    bool status = true;

    if (GSMStatus.bMQTTVehicleCmd == 1 && APPStatus.bVehRunStatus == VEHICLE_RUN)
    {
        GSMStatus.bMQTTVehicleCmd = 0;
        status = bMQTT_SendPublishCmd_Exe(APPCONFIG.cIMEI, "VEHICLE STARTED");
        vEEPROM_eWriteByte_Exe(EEPROM_ADDR_VEHICLE_RUN_STATUS, VEHICLE_RUN);
    }
    else if (GSMStatus.bMQTTVehicleCmd == 1 && APPStatus.bVehRunStatus == VEHICLE_STOP)
    {
        GSMStatus.bMQTTVehicleCmd = 0;
        status = bMQTT_SendPublishCmd_Exe(APPCONFIG.cIMEI, "VEHICLE STOPPED");
        vEEPROM_eWriteByte_Exe(EEPROM_ADDR_VEHICLE_RUN_STATUS, VEHICLE_STOP);
    }

    return status;
}

//---------------------------------------------
// Main MQTT Critical Data Publisher
//---------------------------------------------
bool bMQTT_Publish_CriticalData(void)
{
    bool status = true;
    char CAN_Data[256];

    if (GSMStatus.bMQTTCriticalDataCmd == 1)
    {
        GSMStatus.bMQTTCriticalDataCmd = 0;
        snprintf(CAN_Data, sizeof(CAN_Data), "V:%u, SOC:%u, SOH:%u, C_Temp:%u, B_Temp:%u, Veh_State:%u, FW_Date:%u",
                 CANData.uiBattVolt,
                 CANData.ucBattSOC,
                 CANData.ucBattSOH,
                 CANData.ucControllerTemp,
                 CANData.ucBattCellMaxTemp,
                 APPStatus.bVehRunStatus ? 1 : 0,
                FTPData.ucFWCheckedDate);

        bMQTT_SendPublishCmd_Exe(APPCONFIG.cIMEI, CAN_Data);
    }
    // return status;
}

//---------------------------------------------
// MQTT FW checked date reset publisher
//---------------------------------------------
bool bMQTT_Publish_ResetFWcheckkDateData(void)
{
    bool status = true;

    if (GSMStatus.bMQTTFWchkStatus == 1)
    {
        GSMStatus.bMQTTFWchkStatus = 0;
        FTPData.ucFWCheckedDate = 0; // Firmware checked date resets to Zero
        status = bMQTT_SendPublishCmd_Exe(APPCONFIG.cIMEI, "FW checked date = 0");
    }
    return status;
}

//---------------------------------------------
// Disconnect MQTT
//---------------------------------------------
void vMQTT_Disconnect_Exe(void)
{
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QMTDISC, ATRESPONSE_QMTDISC, 1000, 2000);
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QMTCLOSE, ATRESPONSE_QMTCLOSE, 1000, 2000);
    GSMStatus.bMQTTConnected = false;
}
#endif