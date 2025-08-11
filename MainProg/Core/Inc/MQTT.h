#ifndef __MQTT_H
#define __MQTT_H

typedef enum
{
    CONNECTION_RESET_CLOSED = 1,      /**< Connection is closed or reset by peer.
                                          Action: Execute AT+QMTOPEN command and reopen MQTT connection. */
    PINGREQ_FAILED = 2,               /**< Sending PINGREQ packet timed out or failed.
                                           Action: Deactivate PDP, activate PDP, and then reopen MQTT connection. */
    CONNECT_PACKET_FAILED = 3,        /**< Sending CONNECT packet timed out or failed.
                                           Actions:
                                           1. Verify username and password correctness.
                                           2. Ensure client ID is unique.
                                           3. Reopen MQTT connection and resend CONNECT packet. */
    CONNACK_PACKET_FAILED = 4,        /**< Receiving CONNACK packet timed out or failed.
                                           Actions:
                                           1. Verify username and password correctness.
                                           2. Ensure client ID is unique.
                                           3. Reopen MQTT connection and resend CONNECT packet. */
    SERVER_INITIATED_DISCONNECT = 5,  /**< Client sent DISCONNECT but server closed connection first.
                                           This is a normal process. */
    CLIENT_PACKET_FAILURE_CLOSE = 6,  /**< Client initiates closing connection due to continuous packet send failure.
                                           Actions:
                                           1. Verify data correctness.
                                           2. Try reopening MQTT connection; network congestion or error may exist. */
    LINK_NOT_ALIVE_SERVER_UNAVAIL = 7 /**< Link is not alive or server is unavailable.
                                           Action: Ensure link is alive and server is currently available. */
} MQTTerrCode;

#pragma pack(push, 1)
typedef struct
{
    // Use uint8_t for bitfields, 1 bit each for flags
    uint8_t bMQTTConnected        : 1; // 0 - Not Connected, 1 - Connected
    uint8_t bMQTTDisconnected     : 1; // 0 - Not Disconnected, 1 - Disconnected
    uint8_t IncomingMsg           : 1; // 0 - No, 1 - Ready
    uint8_t bMQTTErrorCode        : 1; // 0 - No error, 1 - Error
    
    uint8_t bMQTTVehicleCmd       : 1; // 0 - STOP, 1 - START
    uint8_t bMQTTCriticalDataCmd : 1; // 0 - STOP, 1 - START
    uint8_t bMQTTFWchkStatus      : 1; // 0 - STOP, 1 - START
    uint8_t bMQTTIncomingMsg      : 1; // 0 - Not Sent, 1 - Sent
    uint8_t bMQTTErrorGenerated   :1;

    uint8_t MQTT_ErrCode          : 7; // 7 bits for error code (0-127)
} TsMQTTStatus;
#pragma pack(pop)


typedef enum
{
    MQTTCOMMAND_SETVEHICLESTOP, // 0
    MQTTCOMMAND_SETVEHICLERUN,  // 1
} MQTTCommand;

bool bMQTT_CheckAndConnect_Exe(const char *topic);
bool bMQTT_PublishPayload_Exe(void);
bool bMQTT_Publish_VehicleState(void);
bool bMQTT_Publish_CriticalData(void);
bool bMQTT_SendPublishCmd_Exe(const char *topic, const char *payload);
bool bMQTT_SubscribeTopic_Exe(const char *topic);
void vMQTT_Incoming_Msg();
void parse_and_transmit_qmtrecv(const char *input);
void parse_and_transmit_qmtstat(const char *input);
void handle_payload(const char *payload);
void handle_errcode(const char *err_code);
bool bMQTT_Publish_ResetFWcheckkDateData(void);
void vMQTT_Disconnect_Exe(void);

#endif
