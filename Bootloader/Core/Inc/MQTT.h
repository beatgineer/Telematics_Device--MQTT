#if 0
bool bMQTT_CheckAndConnect_Exe(const char *topic);
bool bMQTT_PublishPayload_Exe(void);
bool bMQTT_Publish_VehicleState(void);
bool bMQTT_Publish_CriticalData(void);
bool bMQTT_SendPublishCmd_Exe(const char *topic, const char *payload);
bool bMQTT_SubscribeTopic_Exe(const char *topic);
// void vGSM_ParseMQTTMessage(const char *msg);
void vMQTT_Incoming_Msg();
void parse_and_transmit_qmtrecv(const char *input);
void handle_payload(const char *payload);
bool bMQTT_Publish_ResetFWcheckkDateData(void);
void vMQTT_Disconnect_Exe(void);
#endif