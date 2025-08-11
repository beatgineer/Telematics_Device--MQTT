// ============================================================================
// FTP.c
// Author : B.Zaveri
// Date   : 09-06-2025
// ============================================================================
#include "APP.h"
#include "GEN.h"
#include "COMM.h"
#include "UTL.h"
#include "main.h"
#include "CONFIG_EEPROM.h"
#include "GPS.h"
#include "OTA.h"
#include "GSM.h"
#include "MCUFlash.h"
#include "FTP.h"
#include "E2PROM_I2C.h"
#include "CAN.h"
#include "BATT.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// ============================================================================
// FUNCTION PROTOTYPE
// ============================================================================
void vFTP_eInit(void);
void vFTP_eCheckFTPServerForFWUpdate_Exe(void);

// ============================================================================
// Structures
// ============================================================================
TsFTPData FTPData;
extern TsAPP_eTimer TIMERData;
// extern TsConfig APPCONFIG;
extern TsAPP_eConfig APPCONFIG;
extern TsAPP APPStatus;
extern TsGSMData GSMData;
extern TsGSMStatus GSMStatus;
extern TsGPSData GPSData;
extern TsBATTStatus BATTStatus;
//extern TsOTAData OTAData;
extern TsEEPROMConfig EEPROMCONFIG;
extern TsCAN CANData;
extern TuCANStatus CANStatus;
extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart3;

//===========================================================================
// Constant Definitions
//===========================================================================
const uint8_t ucFTP_iStartImmobFWUpdate[8] = {0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
const uint8_t ucCAN_iEndCRCFile[8] = {0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xEF, 0xFE, 0xED};
const uint8_t ucCAN_iStartBINFile[8] = {0x9A, 0xAB, 0xBC, 0xCD, 0xEF, 0xFE, 0xED, 0xDC};
const uint8_t ucAPP_iAck[8] = {'A', 'C', 'K', 0x0, 0x0, 0x0, 0x0, 0x0};
extern char msg[128]; // Debug message buffer
extern UART_HandleTypeDef huart1;

const char *cFTP_iMsgTbl[] =
	{
		"/Standalone_IoT/",			// 0
		"/Standalone_IoT/VERSION/", // 1
		"/IMMOB/",					// 2
		"/IMMOB/VERSION/",			// 3
		"version.txt",				// 4
		"IMG1.BIN",					// 5
		"IMG2.BIN",					// 6
		"IMG1_CRC.txt",				// 7
		"IMG2_CRC.txt",				// 8
};

// uint8_t ucAddr[2048];
// ========================================================================================================

void vFTP_eInit(void)
{
	FTPData.ucFWCheckedDate = 0;
}

// ========================================================================================================
/*
Step	Condition / Action
1		Read date and time from RTC
2		Reset 300s timer
3		If date ≠ last update date
4		If time is between FW_UPDATE_HRS_MIN and MAX
5		If battery is present, GSM is OK, and ignition is OFF
6		Save status and date in EEPROM
7		Close GSM socket, reset system to begin firmware update
*/

void vFTP_eCheckFTPServerForFWUpdate_Exe(void)
{
	// uint8_t ucTime = 0, ucDate = 0;
	bool bResp = false;

	bResp = ucGSM_eReadRTC_Exe();
	if (bResp == true)
	{
		TIMERData.ul300SecCntr = 0;

		// Get Date in BCD: DD = GSMData.cRTC[8..9]
		/*ucDate*/ FTPData.GSMDate = ((GSMData.cRTC[8] - '0') << 4) | (GSMData.cRTC[9] - '0');

		if (/*ucDate*/ FTPData.GSMDate != FTPData.ucFWCheckedDate)
		{
			// Get Hour in BCD: HH = GSMData.cRTC[11..12]
			FTPData.GSMTime = ((GSMData.cRTC[11] - '0') << 4) | (GSMData.cRTC[12] - '0');

			// if (((FTPData.GSMTime >= FW_UPDATE_HRS_MIN) && (FTPData.GSMTime <= FW_UPDATE_HRS_MAX)) || FTPData.ucFWCheckedDate == 0x00)
			if (((FTPData.GSMTime >= FW_UPDATE_HRS_MIN) && (FTPData.GSMTime <= FW_UPDATE_HRS_MAX)))
			{
				if ((BATTStatus.bMainBATTVoltStatus == BATTERY_PRESENT) &&   //BATTStatus.bBATTStatus == BATTERY_PRESENT
					(GSMStatus.ucGSMSignalStatus == GSM_SIGNAL_LEVEL_OK) &&
					(APPStatus.bIgnStatus == IGN_OFF))
				{
					FTPData.ucFWCheckedDate = FTPData.GSMDate; // ucDate;

					FTPData.ucFWCheckStatus = CHK_VERSION_UPDATE;
					vEEPROM_eWriteByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS, CHK_VERSION_UPDATE);

					vEEPROM_eWriteByte_Exe(EEPROM_ADDR_FW_CHECKED_DATE, FTPData.ucFWCheckedDate);
					bMQTT_SendPublishCmd_Exe(APPCONFIG.cIMEI, "Starting FOTA update");
					// vGSM_eCloseSocket_Exe(LOCATION_SOCKET_ID);
					vMQTT_Disconnect_Exe();
					// vAPP_eFeedTheWDT_Exe();
					HAL_Delay(100);
					// HAL_UART_Transmit(&huart1, (uint8_t *)"Starting FOTA update\n", strlen("Starting FOTA update\n"), 500);
					NVIC_SystemReset();
				}
			}
		}
	}
}

/***** END OF FILE ****/