#include "APP.h"
#include "OTA.h"
#include "COMM.h"
#include "BATT.h"
#include "GSM.h"
#include "GPS.h"
#include "UTL.h"
#include "BATT.h"
#include "GEN.h"
#include "CONFIG_EEPROM.h"
#include "E2PROM_I2C.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//        TsOTAData		OTAData;
extern	TsGSMStatus		GSMStatus;
extern	TsGSMData		GSMData;
extern	TsGPSData		GPSData;
extern	TsUARTData		UARTData;
// extern	TsAPPStatus		APPStatus;
extern 	TsAPP 			APPStatus;
extern	TsConfig		CONFIG;
extern TsEEPROMConfig EEPROMCONFIG;
extern	TsBATTData		BATTData;
extern	TsBATTStatus	BATTStatus;

const char *cOTA_eOTACommandTbl [] =
{
	"SET,VEHICLE RUN#", 		// 0
	"SET,VEHICLE STOP#",		// 1
	"SET,FIRMWARE UPDATE,",		// 2
};

const char *cOTA_eOTAResponseTbl [] =
{
	"Update OK",					// 0
	"Update Fail",
	"Firmware Update OK\n",
	"Firmware Update Fail\n",
	"Pair Key Erased",
	"Pair Key Not Erased",			// 5
	"FTP Log In Fail. Try after sometime",
	"FTP Init Fail. Try after sometime",
	"Invalid Command",
	"Signal Level is Low",			// 9
};

// static uint8_t ucGSM_iReadAndDeleteSMS_Exe(uint8_t ucTotalSMS, uint8_t ucPendingSMS);

#if 0
/*Commented as the SMS feature is not needed in MQTT based IOT project
// ============================================================================
// Name			: vOTA_eInit
// Objective	: Initialise OTA Module
// Input  		: none
// Output 		: none
// Return		: none
// ============================================================================
void vOTA_eInit(void)
{
//	OTAData.ucOTACmdStatus[SMS_OTA_CMD] 	= NO_OTA_COMMAND_AVAILABLE;
//	OTAData.ucOTACmdStatus[SOCKET_OTA_CMD]	= NO_OTA_COMMAND_AVAILABLE;
//	OTAData.ucOTACmdIndex  					= 0xFF;
//	OTAData.ucOTASMSSentStatus				= FALSE;
}


// ============================================================================
// Name			: ucOTA_eReadOTACommandFromSMS_Exe
// Objective	: Read OTA SMS and extract following info, if SMS Request received
//			  		from valid Mobile Num.
//					1. Sender Mobile Num
//					2. SMS Index
//					3. SMS Data
//			  		Verify the Sender Mobile present in the authorized list
// Input  		: none
// Output 		: If Valid Command
//					1. Sender Mobile Num : GSMData.cMobileNum
//					2. OTA Cmd Index Num : OTAData.ucSMSIndex
//					3. OTA Cmd Status    : OTAData.ucOTACmdStatus
//
// Return		: FALSE - Not A Valid Sender or Invalid Command
//			  	  TRUE  - Valid Sender and Valid Command
// ============================================================================
void vOTA_eReadSMSOTACmdFromSIM_Exe(void)
{
	bool bResp;

	bResp = bGSM_eCheckPendingSMSFromSIM_Exe();
	if (bResp == SMS_PRESENT)
	{
		GSMData.ucPendingSMSMS = ucGSM_iReadAndDeleteSMS_Exe(GSMData.ucTotalSMSMS, GSMData.ucPendingSMSMS);
	}
}

void vOTA_eReadSMSOTACmdFromME_Exe(void)
{
	bool bResp;

	bResp = bGSM_eCheckPendingSMSFromME_Exe();
	if (bResp == SMS_PRESENT)
	{
		GSMData.ucPendingMESMS = ucGSM_iReadAndDeleteSMS_Exe(GSMData.ucTotalMESMS, GSMData.ucPendingMESMS);
	}
	else
	{
	}
}


#if 0
void vOTA_eReadOTACommandFromSMS_Exe(uint8_t ucSMSStorage)
{
	uint8_t ucResp, ucCntr, ucCntr1, ucTotalSMS, ucPendingSMS;
	bool bResp, bStatus;
	char *cBuff;

	bResp = bGSM_eCheckPendingSMS_Exe(ucSMSStorage);
	if (bResp == SMS_PRESENT)
	{
		if (ucSMSStorage == ME_SMS_STORAGE)
		{
			ucTotalSMS	= GSMData.ucTotalMESMS;
			ucPendingSMS= GSMData.ucPendingMESMS;
		}
		else if (ucSMSStorage == SM_SMS_STORAGE)
		{
			ucTotalSMS	= GSMData.ucTotalSMSMS;
			ucPendingSMS= GSMData.ucPendingSMSMS;
		}
		else
		{
		}

		for (ucCntr = 0; (ucCntr < ucTotalSMS) && (ucPendingSMS != 0); ucCntr++)
		{
			ucResp = ucGSM_eReadSMS_Exe(ucCntr);
			if (ucResp == SMS_PRESENT)
			{
				bStatus = bGSM_eDeleteSMS_Exe(ucCntr);
				if(bStatus == TRUE)
				{
					if (ucSMSStorage == ME_SMS_STORAGE)
					{
						GSMData.ucPendingMESMS--;
						ucPendingSMS--;
					}
					else if (ucSMSStorage == SM_SMS_STORAGE)
					{
						GSMData.ucPendingSMSMS--;
						ucPendingSMS--;
					}
					else
					{
					}
				}
				else
				{
				}

				// Search OTA Command
				ucCntr1 = 0;
				do
				{
					cBuff = strstr((char *)GSMData.cSMSData, (char *)cOTA_eOTACommandTbl[ucCntr1]);
					ucCntr1++;
				} while ((cBuff == NULL) && (ucCntr1 < TOTAL_OTA_COMMANDS));

				if (cBuff)
				{
					// Valid Command
					ucCntr1--;
					OTAData.ucOTACmdIndex = ucCntr1;

					if (ucSMSStorage == ME_SMS_STORAGE)
					{
						GSMData.ucCurrentMESMSIndex = ucCntr;
					}
					else if (ucSMSStorage == SM_SMS_STORAGE)
					{
						GSMData.ucCurrentSMSMSIndex = ucCntr;
					}
					else
					{
					}

					vOTA_eExecuteOTACmd_Exe();
				}
				else
				{
					strcpy (cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_INVALIDCOMMAND]);
					bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, cAPP_eGlobalBuffer);
				}
			}
			else
			{
			}
		}
	}
	else
	{
	}
}
#endif

static uint8_t ucGSM_iReadAndDeleteSMS_Exe(uint8_t ucTotalSMS, uint8_t ucPendingSMS)
{
	uint8_t ucCntr, ucOTACmdIndex, ucResp;
	bool bStatus;
	char *cBuff;

	for (ucCntr = 0; (ucCntr < ucTotalSMS) && (ucPendingSMS != 0); ucCntr++)
	{
		ucResp = ucGSM_eReadSMS_Exe(ucCntr);
		if (ucResp == SMS_PRESENT)
		{
			bStatus = bGSM_eDeleteSMS_Exe(ucCntr);
			if(bStatus == TRUE)
			{
				ucPendingSMS--;
			}
			else
			{
			}

			// Search OTA Command
			ucOTACmdIndex = 0;
			do
			{
				cBuff = strstr((char *)GSMData.cSMSData, (char *)cOTA_eOTACommandTbl[ucOTACmdIndex]);
				ucOTACmdIndex++;
			} while ((cBuff == NULL) && (ucOTACmdIndex < TOTAL_OTA_COMMANDS));

			if (cBuff)
			{
				// Valid Command
				ucOTACmdIndex--;
			}
			else
			{
			}

			vOTA_eExecuteOTACmd_Exe(ucOTACmdIndex);
		}
		else
		{
			strcpy (cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_INVALIDCOMMAND]);
			bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, cAPP_eGlobalBuffer);
		}
	}

	return ucPendingSMS;
}

// ============================================================================
// Name			: vOTA_eExecuteOTACmd_Exe
// Objective	: 1. Execute OTA Command received from SMS / GPRS
//	    	  	  2. Send Response of OTA Command
// Input  		: Command no. at OTAData.ucOTACmdIndex
// Output 		: Response String at cAPP_eGlobalBuffer[0]
// Return		: 0 - FAIL, 1 - SUCCESS
// ============================================================================
void vOTA_eExecuteOTACmd_Exe(uint8_t ucOTACmdIndex)
{
	uint8_t ucResp;
	char *cBuff1, *cBuff2;
	bool bResp;

	switch (ucOTACmdIndex)
	{
		case OTACOMMAND_SETVEHICLERUN:
			APPStatus.bVehRunStatus = VEHICLE_RUN;
			vEEPROM_eWriteByte_Exe(EEPROM_ADDR_VEHICLE_RUN_STATUS, VEHICLE_RUN);
			strcpy(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_UPDATEOK]);
			bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, cAPP_eGlobalBuffer);
			break;

		case OTACOMMAND_SETVEHICLESTOP:
			APPStatus.bVehRunStatus = VEHICLE_STOP;
			vEEPROM_eWriteByte_Exe(EEPROM_ADDR_VEHICLE_RUN_STATUS, VEHICLE_STOP);
			strcpy(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_UPDATEOK]);
			bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, cAPP_eGlobalBuffer);
			break;

            /*
		case OTACOMMAND_SETFIRMWAREUPDATE:

			if((BATTStatus.bIntBATTVoltStatus == BATTERY_PRESENT) && (GSMStatus.bRTCStatus == TRUE) && (GSMStatus.ucGSMSignalStatus == GSM_SIGNAL_LEVEL_OK))
			{
				bResp = bFTP_eConnectFTPServer_Exe();
				if(bResp == TRUE)
				{
					cBuff1 = pucUTL_eSearchChar_Exe(',', 2, GSMData.cSMSData);
					if(cBuff1 != 0)
					{
						cBuff1++;
						cBuff2 = pucUTL_eSearchChar_Exe('#', 1, cBuff1) ;
						if(cBuff2 != 0)
						{
							*cBuff2 = 0;
							strcpy(APPStatus.cVersion, cBuff1);

							bResp = bFTP_eUpdateFirmware_Exe();
							if (TRUE == bResp)
							{
								ucResp = 1;
								GSMStatus.bFWSMSPending = FALSE;
							}
							else
							{
								ucResp = 2;
								GSMStatus.bFWSMSPending = TRUE;
							}
						}
						else
						{
							ucResp = 2;
						}
					}
					else
					{
						ucResp = 2;
					}
				}
				else
				{
					ucResp = 3;
				}
			}
			else
			{
				ucResp = 4;
			}

			switch (ucResp)
			{
				case 1:
					strcpy(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_FWUPDATEOK]);
					break;
				case 2:
					strcpy(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_FWUPDATEFAIL]);
					break;
				case 3:
					strcpy(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_FWUPDATEFAIL]);
//					ucLen = strlen (cAPP_eGlobalBuffer);
					strcat(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_FTPLOGINFAILMSG]);
					break;
				case 4:
					strcpy(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_FWUPDATEFAIL]);
//					ucLen = strlen (cAPP_eGlobalBuffer);
//					sprintf(&cAPP_eGlobalBuffer[ucLen], cOTA_eOTAResponseTbl[OTARESPONSE_SIGNALLOW]);
					strcat(cAPP_eGlobalBuffer, cOTA_eOTAResponseTbl[OTARESPONSE_SIGNALLOW]);
					break;
			}

			bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, cAPP_eGlobalBuffer);

			if(ucResp == 1)
			{
				vGSM_eCloseSocket_Exe(LOCATION_SOCKET_ID);
				HAL_Delay(100);
				NVIC_SystemReset();
			}
			else
			{
			}

			break;
            */

		// case OTACOMMAND_ERASEPAIRKEY:
		// 	BLEStatus.ucBLEUniqueIdStoreStatus = ERASE_BLE_UNIQUE_ID;
		// 	vAPP_eEraseUID_Exe();

		// 	strcpy(cAPP_eGlobalBuffer, (char *)cOTA_eOTAResponseTbl[OTARESPONSE_PAIRKEY_ERASED]);
		// 	bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, cAPP_eGlobalBuffer);
		// 	break;
		// default:
		// 	strcpy(cAPP_eGlobalBuffer, (char *)cOTA_eOTAResponseTbl[OTARESPONSE_INVALIDCOMMAND]);
		// 	bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, cAPP_eGlobalBuffer);
		// 	break;
	}
}

#endif