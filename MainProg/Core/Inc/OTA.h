// // ============================================================================
// // OTA.h
// // Author : AK
// // Date   : 05-05-2014
// // ============================================================================

// #ifndef __OTA_H_
// #define __OTA_H_

// #include <stdbool.h>

// #define TRY_COUNTS 3

// extern const char *cOTA_eOTAResponseTbl[];

// // typedef enum
// // {
// // 	NO_OTA_COMMAND_AVAILABLE = 0,
// // 	SOCKET_OTA_COMMAND_AVAILABLE,
// // 	BLE_OTA_COMMAND_AVAILABLE,
// // 	SMS_OTA_COMMAND_AVAILABLE,

// // 	SMS_OTA_CMD = 0,
// // 	SOCKET_OTA_CMD,
// // 	BLE_OTA_CMD,

// // 	VERSION_NOT_CHECKED = 0,
// // 	SAME_VERSION_FOUND,
// // 	DIFFERENT_VERSION_FOUND,
// // } OTACmd;

// #pragma pack(push, 1)
// typedef struct
// {
// 	//	uint8_t		ucOTASMSIndex;
// 	//	uint8_t		ucOTASMSSentStatus;
// 	//	char			cOTABuffer[200];
// 	//	uint8_t 		ucOTACmdIndex;

// 	//	uint8_t		ucOTACmdStatus;
// 	//	uint8_t		ucOTASocketID;

// 	//	char			cVersion[8];

// 	uint8_t ucOTARespPending;
// 	uint8_t ucOTAPairDonePending;
// 	uint32_t ulOTARespPendingTimer;
// } TsOTAData;
// #pragma pack(pop)

// typedef enum
// {
// 	OTACOMMAND_SETVEHICLERUN,	  // 0
// 	OTACOMMAND_SETVEHICLESTOP,	  // 1
// 	OTACOMMAND_SETFIRMWAREUPDATE, // 2
// } OTACommand;

// #define TOTAL_OTA_COMMANDS OTACOMMAND_SETFIRMWAREUPDATE + 1
// #define INVALID_OTA_COMMAND 0x07

// typedef enum
// {
// 	OTARESPONSE_UPDATEOK, // 0
// 	OTARESPONSE_UPDATEFAIL,
// 	OTARESPONSE_FWUPDATEOK,
// 	OTARESPONSE_FWUPDATEFAIL,
// 	OTARESPONSE_PAIRKEY_ERASED,
// 	OTARESPONSE_PAIRKEY_NOT_ERASED, // 5
// 	OTARESPONSE_FTPLOGINFAILMSG,
// 	OTARESPONSE_FTPINITFAILMSG,
// 	OTARESPONSE_INVALIDCOMMAND,
// 	OTARESPONSE_SIGNALLOW, // 9
// } OTAResponse;

// extern const char *cOTA_eOTACommandTbl[];

// void vOTA_eInit(void);
// // void vOTA_eExecuteOTACmd_Exe(uint8_t ucOTACmdIndex);
// // void vOTA_eReadSMSOTACmdFromSIM_Exe(void);
// // void vOTA_eReadSMSOTACmdFromME_Exe(void);

// #endif

// /***** END OF FILE ****/
