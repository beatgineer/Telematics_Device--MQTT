#ifndef __CAN_H
#define __CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

//----------------------------------------------------------------

#define FDCAN1_RX_Pin GPIO_PIN_11
#define FDCAN1_RX_GPIO_Port GPIOA
#define FDCAN1_TX_Pin GPIO_PIN_12
#define FDCAN1_TX_GPIO_Port GPIOA

#define CANID_IMMOM_FW_UPDATE				0x300
#define CANID_CLUSTER_TO_IMMOB				0x409
#define CANID_CLUSTER_TO_MOTORCONTROLLER	0x400

#define UDS_TOOL_CMD_CLUSTER_FW_UPDATE		0x01
#define UDS_CLUSTER_RESP_CLUSTER_FW_UPDATE	0x01

#define UDS_TOOL_CMD_TESTER_PRESENT			0x3E
#define UDS_CLUSTER_RESP_TESTER_PRESENT		0x7E
 
#define UDS_TOOL_CMD_SECURITY_ACCESS		0x27
#define UDS_CLUSTER_RESP_SECURITY_ACCESS	0x67
//#define UDS_CLUSTER_RESP_SECURITY_ACCESS1	0x67
//#define UDS_CLUSTER_RESP_SECURITY_ACCESS2	0x67

#define UDS_TOOL_CMD_DIAG_SESSION_CONTROL	0x10
#define UDS_CLUSTER_RESP_DIAG_SESSION_CONTROL	0x50

#define UDS_TOOL_CMD_REQUEST_DOWNLOAD		0x34
#define UDS_CLUSTER_RESP_REQUEST_DOWNLOAD	0x74

#define UDS_TOOL_CMD_ROUTINE_CONTROL		0x31
#define UDS_CLUSTER_RESP_ROUTINE_CONTROL	0x71

#define UDS_TOOL_CMD_TRANSFER_DATA			0x36
#define UDS_CLUSTER_RESP_TRANSFER_DATA		0x76

#define UDS_TOOL_CMD_TRANSFER_EXIT			0x37
#define UDS_CLUSTER_RESP_TRANSFER_EXIT		0x77

//----------------------------------------------------------------

extern uint8_t ucCAN_eRxData[], ucCAN_eRxData1[];
extern uint8_t ucCAN_eTxData[];
extern uint32_t ulFTP_eTxCANID;

//----------------------------------------------------------------

typedef enum
{
	NO_CAN_CMD 				= 0x0,
	CAN_START_VEHICLE 		= 0x01,
	CAN_STOP_VEHICLE		= 0x02,
	CAN_UNLOCK_VEHICLE		= 0x04,
	CAN_FIND_VEHICLE		= 0x08,
	CAN_START_IND_BUZZER	= 0x20,

	CAN_IMMOB_PAIRING_ENABLE= 0x01,
	CAN_IMMOB_ERASE_KEYS	= 0x02,
	CAN_IMMOB_READ_KEYS 	= 0x04,
	CAN_IMMOB_GET_KEYID		= 0x08,

	CAN_HAZARD_LIGHT_OFF	= 0x01,
	CAN_HAZARD_LIGHT_ON		= 0x02,

} EnumVehicleCANCmd;

typedef enum
{
	NO_RESPN = 0,
	TESTER_PRESENT,
	SECURITY_ACCESS1,			// 2
	SECURITY_ACCESS2,
	DIAG_SESSION_CONTROL,
	REQUEST_DOWNLOAD,			// 5
	ROUTINE_CONTROL,
	TRANSFER_DATA,				// 7
	REQUEST_TRANSFER_EXIT,
	UDS_ERROR,					// 9
	FW_UPDATED,					// 10
	DATA_RECEIVED,				// 11
	READY_TO_RECEIVE_DATA,		// 12
	FIRMWARE_UPDATE_DONE,		// 13
	FIRMWARE_UPDATE_WRITING_ERROR,	// 14
} EnumResp;

//----------------------------------------------------------------

#pragma pack(push,1)
typedef struct
{
	uint8_t		ucCANFWStatus;
	uint32_t	ulBINFileSize;
	uint8_t		uc2kPages;
}TsCAN;
#pragma pack(pop)

typedef union
{
	struct
	{
		bool	bRdyToSend		: 1;
		bool	bSecurityAccess	: 1;
		bool	bSecurityPassed	: 1;

		bool	bClusterPresent	: 1;
		bool	bCANDataRecd	: 1;
		bool	bFileDataRecd	: 1;
	};
	uint8_t	ucData[1];
}TuCANStatus;

//----------------------------------------------------------------

void vCAN_eInit(void);
void vCAN_eReadCANData_Exe(void);
void vCAN_eSendCANCommands_Exe(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H */


