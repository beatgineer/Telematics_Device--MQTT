/*
 * CAN.c
 *
 *  Created on: Mar 2, 2025
 *      Author: Brij.Z
 */
#include "APP.h"
#include "main.h"
#include "CAN.h"
#include "stm32g0xx_hal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/*function prototype*/
static void MX_FDCAN1_Init(void);
static void vCAN_iCANConfig(void);
void vCAN_eInit(void);
void vCAN_eReadCANData_Exe(void);
void vCAN_eSendCANCommands_Exe(uint32_t CAN_ID);
void vCAN_eChargerStatus_Exe(void);
void vCAN_eGetBattID_Exe(void);
void vCAN_eUpdateBattSOC_Exe(void);
void vCAN_eUpdateBattTemp_Exe(void);
void vCAN_eReadErrCode_Exe(void);
void vCAN_eChargerStatus_Exe(void);
static void vCAN_iReadBattErr1_Exe(void);
static void vCAN_iReadBattErr2_Exe(void);
static void vCAN_iReadBattErr3_Exe(void);
static void vCAN_iReadBattErr4_Exe(void);
static void vCAN_iReadBattErr5_Exe(void);
static void vCAN_iReadBattErr6_Exe(void);
static void vCAN_iReadBattErr7_Exe(void);
static void vCAN_iReadBattErr8_Exe(void);
static void vCAN_iReadBattErr9_Exe(void);
static void vCAN_iReadContErr1_Exe(void);
static void vCAN_iReadContErr2_Exe(void);

FDCAN_HandleTypeDef hfdcan1;

extern FDCAN_HandleTypeDef hfdcan1;

FDCAN_FilterTypeDef sFilterConfig;
FDCAN_TxHeaderTypeDef TxHeader;
FDCAN_RxHeaderTypeDef RxHeader;
FDCAN_RxHeaderTypeDef RxHeader1;

//*****************************************************************************
// INTERNAL VARIABLES
//*****************************************************************************

TsCAN CANData;
TuCANStatus CANStatus;
TsCell CELLData; // Rev 1.021 19-09-2024
extern TsAPP APPStatus;

uint8_t ucCAN_eRxData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t ucCAN_eTxData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
// uint32_t ulFTP_eTxCANID;

static void MX_FDCAN1_Init(void)
{

	/* USER CODE BEGIN FDCAN1_Init 0 */

	/* USER CODE END FDCAN1_Init 0 */

	/* USER CODE BEGIN FDCAN1_Init 1 */

	/* USER CODE END FDCAN1_Init 1 */
	hfdcan1.Instance = FDCAN1;
	hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
	hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
	hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
	hfdcan1.Init.AutoRetransmission = ENABLE;
	hfdcan1.Init.TransmitPause = ENABLE;
	hfdcan1.Init.ProtocolException = DISABLE;
	hfdcan1.Init.NominalPrescaler = 8;
	hfdcan1.Init.NominalSyncJumpWidth = 16;
	hfdcan1.Init.NominalTimeSeg1 = 12;
	hfdcan1.Init.NominalTimeSeg2 = 3;
	hfdcan1.Init.DataPrescaler = 1;
	hfdcan1.Init.DataSyncJumpWidth = 4;
	hfdcan1.Init.DataTimeSeg1 = 5;
	hfdcan1.Init.DataTimeSeg2 = 4;
	hfdcan1.Init.StdFiltersNbr = 1;
	hfdcan1.Init.ExtFiltersNbr = 0;
	hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
	if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN FDCAN1_Init 2 */

	/* USER CODE END FDCAN1_Init 2 */
}

// ============================================================================
// Name			: vCAN_eInit
// Objective	: Initialise RS485
// Input  		: none
// Output 		: none
// Return		: none
// Version		: -
// ============================================================================
void vCAN_eInit(void)
{
	MX_FDCAN1_Init();

	CANData.ucCurrentCANId = 0;

	CANData.uiBattVolt = 0;
	CANData.bCurrPol = 0; // 0 - Positive, 1 - Negative
	CANData.uiBattCurrent = 0;
	CANData.ucBattStateFlag = 0;
	CANData.ucBattType = 3;
	CANData.ucBattSOC = 0;
	CANData.cBattSOC[0] = 0;
	CANData.ucBattSOH = 0;
	CANData.uiNumOfCycles = 0;

	CANData.ucVehMode = 1;
	CANData.ucVehModePrev = 0;
	CANData.uiRPM = 0;
	CANData.cRPM[0] = 0;
	CANData.ucSpeedInkmph = 0;
	CANData.ucSpeedInmiph = 0;
	CANData.cSpeed[0] = 0;
	CANData.ucControllerTemp = 0;
	CANData.ucControllerErr1 = 0;
	CANData.ucControllerErr2 = 0;

	CANData.uiFullChgAH = 0;
	CANData.uiResidualAH = 0;
	//	CANData.uiResidualEnergy	= 0;
	CANData.uiTimeToCharge = 0;

	CELLData.uiCellMinVolt = 0;
	CELLData.uiCellMaxVolt = 0;

	CANData.ucBattCellMinTemp = 0;
	CANData.ucBattCellMaxTemp = 0;
	CANData.ucBattMOSTemp = 0;
	CANData.ucBattAmbientTemp = 0;

	CANData.ucBattErr1 = 0;
	CANData.ucBattErr2 = 0;
	CANData.ucBattErr3 = 0;
	CANData.ucBattErr4 = 0;
	CANData.ucBattErr5 = 0;
	CANData.ucBattErr6 = 0;
	CANData.ucBattErr7 = 0;
	CANData.ucBattErr8 = 0;
	CANData.ucBattErr9 = 0;

	CANData.ucClusterErr1 = 0;
	CANData.ucChargerErr1 = 0;

	CANStatus.ucImmobErr = 0;

	CANStatus.bImmobCANStatus = false;
	CANStatus.bContCANStatus1 = false;
	CANStatus.bContCANStatus2 = false;
	CANStatus.bBattCANStatus1 = false;
	CANStatus.bBattCANStatus2 = false;
	CANStatus.bBattCANStatus3 = false;
	CANStatus.bBattCANStatus4 = false;
	CANStatus.bBattCANStatus5 = false;
	CANStatus.bBattCANStatus6 = false;
	CANStatus.bBattCANStatus7 = false;
	CANStatus.bBattCANStatus8 = false;
	CANStatus.bBattChgCANStatus = false;

	CANStatus.bCANFrame413Recd = false;
	CANStatus.bCANFrame414Recd = false;
	CANStatus.bCANFrame415Recd = false;

	//ulFTP_eTxCANID = CANID_CLUSTER_TO_IMMOB;
	CANData.ucCANTxCntr = 3;

	vCAN_iCANConfig();

	CANData.ucBattCANDataTimer = 0;
	CANData.ucContCANDataTimer = 0;
}

/**
 * @brief  Rx FIFO 0 callback.
 * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
 *         the configuration information for the specified FDCAN.
 * @param  RxFifo0ITs indicates which Rx FIFO 0 interrupts are signaled.
 *         This parameter can be any combination of @arg FDCAN_Rx_Fifo0_Interrupts.
 * @retval None
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		/* Retrieve Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, ucCAN_eRxData) != HAL_OK)
		{
			Error_Handler();
		}

		vCAN_eReadCANData_Exe();
	}
}

static void vCAN_iCANConfig(void)
{
	/* Configure Rx filter */
	sFilterConfig.IdType = FDCAN_EXTENDED_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE_NO_EIDM;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x1011FFA7; // 0x1FFFFFFF;	//0x18900140;
	sFilterConfig.FilterID2 = 0x1028FFA7; // 0x1FFFFFFF;	//0x189FFFFF;

	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}

	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 1;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE; //	 FDCAN_FILTER_RANGE_NO_EIDM;//FDCAN_FILTER_DUAL;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x300 << 16;
	sFilterConfig.FilterID2 = 0x410;

	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/* Start the FDCAN module */
	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
	{
		Error_Handler();
	}

	/* Prepare Tx message Header */
	TxHeader.Identifier = 0x409;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;
}

void vCAN_eSendCANCommands_Exe(uint32_t CAN_ID)
{
	TxHeader.Identifier = CAN_ID;

	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, (uint8_t *)ucCAN_eTxData) != HAL_OK)
	{
		Error_Handler();
	}
	else
	{
	}
}

void vCAN_eReadCANData_Exe(void)
{
	uint16_t uiTemp = 0;
	uint32_t ulTemp;
	uint8_t ucTemp = 0;

	switch (RxHeader.Identifier)
	{
	case 0x600:
		CANData.ucContCANDataTimer = 0;

		CANData.ucControllerErr1 = ucCAN_eRxData[0];
		CANData.ucControllerErr2 = ucCAN_eRxData[1];
		CANStatus.bReadyStatus = (ucCAN_eRxData[2] & 0x08) >> 3;
		CANStatus.bChgProtection = (ucCAN_eRxData[2] & 0x20) >> 5;
		CANStatus.bBrakeStatus = (ucCAN_eRxData[3] & 0x08) >> 3;
		CANStatus.bAntiTheftStatus = (ucCAN_eRxData[3] & 0x10) >> 4;

		CANStatus.bReverseStatus = (ucCAN_eRxData[3] & 0x80) >> 7;
		if (CANStatus.bReverseStatus == true)
		{
			CANData.ucVehMode = 4;
		}
		else
		{
			CANData.ucVehMode = ucCAN_eRxData[3] & 0x07;
		}

		CANData.ucControllerTemp = ucCAN_eRxData[6] - 40;

		CANStatus.bContCANStatus1 = true;
		break; 

	case 0x601:
		CANData.ucContCANDataTimer = 0;

		CANData.uiRPM = (ucCAN_eRxData[0] << 8) | ucCAN_eRxData[1];
		CANData.uiContVoltage = (ucCAN_eRxData[4] << 8) | ucCAN_eRxData[5];
		CANData.uiMotorCurrent = (ucCAN_eRxData[6] << 8) | ucCAN_eRxData[7];

		CANData.ucSpeedInkmph = CANData.uiRPM / CFAC_RPM_SPEED_RV400;

		CANStatus.bContCANStatus2 = true;
		break;

	case 0x1011FFA7:
		CANData.ucBattCANDataTimer = 0;

		CANData.uiBattVolt = ((ucCAN_eRxData[1] << 8) | ucCAN_eRxData[0]) / 10;
		uiTemp = ((ucCAN_eRxData[3] << 8) | ucCAN_eRxData[2]) / 10;
		if (uiTemp < 1000)
		{
			CANData.bCurrPol = POLARITY_MINUS;
			CANData.uiBattCurrent = 1000 - uiTemp;
		}
		else
		{
			CANData.bCurrPol = POLARITY_PLUS;
			CANData.uiBattCurrent = uiTemp - 1000;
		}

		CANStatus.bBattCANStatus1 = true;
		break;

	case 0x1020FFA7:
		CANData.ucBattCANDataTimer = 0;

		CANStatus.bACCStatus = ucCAN_eRxData[6] & 0x01;
		CANStatus.bONStatus = ucCAN_eRxData[6] & 0x02;
		CANStatus.bCRGStatus = ucCAN_eRxData[6] & 0x04;

		CANStatus.bBattCANStatus2 = true;
		break;

	case 0x1023FFA7:
		CANData.ucBattCANDataTimer = 0;

		CANData.ucBattSOC = ((ucCAN_eRxData[1] << 8) | ucCAN_eRxData[0]) / 10;
		CANData.ucBattSOH = ((ucCAN_eRxData[3] << 8) | ucCAN_eRxData[2]) / 10;
		CANData.uiNumOfCycles = (ucCAN_eRxData[5] << 8) | ucCAN_eRxData[4];

		CANStatus.bBattCANStatus3 = true;
		break;

	case 0x1024FFA7:
		CANData.uiFullChgAH = (ucCAN_eRxData[1] << 8) | ucCAN_eRxData[0];
		CANData.uiResidualAH = (ucCAN_eRxData[3] << 8) | ucCAN_eRxData[2];
		CANData.uiResidualEnergy = (ucCAN_eRxData[5] << 8) | ucCAN_eRxData[4];
		CANData.uiTimeToCharge = (ucCAN_eRxData[7] << 8) | ucCAN_eRxData[6];

		CANStatus.bBattCANStatus4 = true;
		break;

	case 0x1025FFA7:
		CELLData.uiCellMinVolt = (ucCAN_eRxData[1] << 8) | ucCAN_eRxData[0];
		CELLData.uiCellMaxVolt = (ucCAN_eRxData[3] << 8) | ucCAN_eRxData[2];
		CANStatus.bBattCANStatus9 = true;
		break;

	case 0x1027FFA7:
		CANData.ucBattCANDataTimer = 0; 

		CANData.ucBattCellMaxTemp = ucCAN_eRxData[0] - 40;
		CANData.ucBattCellMinTemp = ucCAN_eRxData[1] - 40;
		CANData.ucBattMOSTemp = ucCAN_eRxData[2] - 40;
		CANData.ucBattAmbientTemp = ucCAN_eRxData[3] - 40;

		if ((CANData.ucBattCellMaxTemp >= BATT_OVER_TEMP) &&
			(CANData.ucBattCellMaxTemp < BATT_THERMAL_RUNAWAY_TEMP))
		{
			CANData.ucBattErr9 &= 0b11111001; 
			CANData.ucBattErr9 |= 0b00000010; 
		}
		else if (CANData.ucBattCellMaxTemp >= BATT_THERMAL_RUNAWAY_TEMP)
		{
			CANData.ucBattErr9 &= 0b11111001; 
			CANData.ucBattErr9 |= 0b00000100; 
		}
		else
		{
			
			CANData.ucBattErr9 &= 0b11111001; 
		}

		CANStatus.bBattCANStatus5 = true;
		break;

	case 0x1028FFA7:
		CANData.ucBattErr1 = ucCAN_eRxData[0];
		CANData.ucBattErr2 = ucCAN_eRxData[1];
		CANData.ucBattErr3 = ucCAN_eRxData[2];
		CANData.ucBattErr4 = ucCAN_eRxData[3];
		CANData.ucBattErr5 = ucCAN_eRxData[4];
		CANData.ucBattErr6 = ucCAN_eRxData[5];
		CANData.ucBattErr7 = ucCAN_eRxData[6];
		CANData.ucBattErr8 = ucCAN_eRxData[7];

		CANStatus.bBattCANStatus6 = true;
		break;

	case 0x1030FFA7:
		CELLData.ucIndexNum = ucCAN_eRxData[0] & 0b00011111;
		CELLData.ucNumOfCellData = (ucCAN_eRxData[0] & 0b11100000) >> 5;
		ucTemp = CELLData.ucIndexNum * 4;

		ulTemp = ((ucCAN_eRxData[2] << 8) | ucCAN_eRxData[1]);
		CELLData.uiCellVoltage[ucTemp++] = ulTemp & 0x3FFF;

		ulTemp = ((ulTemp & 0b1100000000000000) >> 14) | (ucCAN_eRxData[3] << 2) | (ucCAN_eRxData[4] << 10);
		CELLData.uiCellVoltage[ucTemp++] = ulTemp & 0x00003FFF;

		ulTemp = (ulTemp >> 14) | (ucCAN_eRxData[5] << 4) | (ucCAN_eRxData[6] << 12);
		CELLData.uiCellVoltage[ucTemp++] = ulTemp & 0x00003FFF;

		ulTemp = ((ulTemp & 0xFFFFC000) >> 14) | (ucCAN_eRxData[7] << 6);
		CELLData.uiCellVoltage[ucTemp] = ulTemp & 0x00003FFF;

		CANStatus.bBattCANStatus10 = true;
		break;

	case 0x1002FFA7:
		CANData.ucProdYear = ucCAN_eRxData[0];
		CANData.ucProdMonth = ucCAN_eRxData[1];
		CANData.ucProdDay = ucCAN_eRxData[2];

		CANStatus.bBattCANStatus7 = true;
		break;

	case 0x1003A7A5:
		if (ucCAN_eRxData[0] == 0)
		{
			CANStatus.bChargerStatus = true;
			CANData.ucBattErr9 &= 0b11111110;
			CANData.ucChgConnectCntr++;
		}
		else
		{
			CANStatus.bChargerStatus = false;
			CANData.ucBattErr9 |= 0b00000001;
		}

		CANStatus.bBattChgCANStatus = true;
		break;

	case 0x1033FFA7: // Rev 1.01 28-08-2024
		if (ucCAN_eRxData[0] == 3)
		{
			CANData.ucBattSNum[0] = ucCAN_eRxData[3] & 0x0F;
			CANData.ucBattSNum[1] = ((ucCAN_eRxData[4] & 0x0F) << 4) |
									(ucCAN_eRxData[5] & 0x0F);
			CANData.ucBattSNum[2] = ((ucCAN_eRxData[6] & 0x0F) << 4) |
									(ucCAN_eRxData[7] & 0x0F); // Rev 1.01 28-08-2024
		}
		else
		{
		}

		CANStatus.bBattCANStatus8 = true;
		break;
	}
}

void vCAN_eChargerStatus_Exe(void)
{/*
	if ((CANStatus.bChargerStatus == true) &&
		(APPStatus.ucTimer200mS > 5))
	{
		APPStatus.ucTimer200mS = 0;
		if (CANData.ucChgConnectCntr == CANData.ucChgConnectCntrPrev)
		{
			CANStatus.bChargerStatus = false;
			CANData.ucBattErr9 |= 0b00000001;
		}
		else
		{
			CANData.ucBattErr9 &= 0b11111110;
			CANData.ucChgConnectCntrPrev = CANData.ucChgConnectCntr;
		}
	}
	else
	{
	}
*/
}

void vCAN_eGetBattID_Exe(void)
{
	char cBuff[20]; 
	int16_t iVal;	

	if ((CANStatus.bBattCANStatus7 == true) &&
		(CANStatus.bBattCANStatus8 == true))
	{
		if (CANData.ucProdYear >= 24)
		{
			strcpy(cBuff, CANData.cVehBattID);

			sprintf(CANData.cVehBattID, "20%02d%02d%02d%x%02x%02x",
					CANData.ucProdYear,
					CANData.ucProdMonth,
					CANData.ucProdDay,
					CANData.ucBattSNum[0],
					CANData.ucBattSNum[1],
					CANData.ucBattSNum[2]); 

			CANData.cVehBattID[13] = 0; 

			iVal = strncmp(cBuff, CANData.cVehBattID, LEN_BATTID); 

			if (iVal != 0) 
			{
				vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_BATTID, (char *)CANData.cVehBattID, LEN_BATTID);
			} 
		}
		else
		{
			CANData.cVehBattID[0] = 0;
		}

		CANStatus.bBattCANStatus7 = false;
		CANStatus.bBattCANStatus8 = false;
	}
	else
	{
	}
}

// ============================================================================
// Name			: vCAN_eUpdateBattSOC_Exe
// Objective	: Update Battery SOC
//
// Input  		: none
// Output 		: SOC in % at CANData.cBattSOC in BCD
// Return		: none
// Version		: -
// ============================================================================
void vCAN_eUpdateBattSOC_Exe(void)
{
	uint8_t ucLen = 0;
	char cBuff[6];

	sprintf(cBuff, "%d", CANData.ucBattSOC);
	ucLen = strlen(cBuff);
	switch (ucLen)
	{
	case 1:
		CANData.cBattSOC[2] = cBuff[0] & 0x0F;
		CANData.cBattSOC[1] = 10;
		CANData.cBattSOC[0] = 10;
		break;

	case 2:
		CANData.cBattSOC[2] = cBuff[1] & 0x0F;
		CANData.cBattSOC[1] = cBuff[0] & 0x0F;
		CANData.cBattSOC[0] = 10;
		break;

	case 3:
		CANData.cBattSOC[2] = cBuff[2] & 0x0F;
		CANData.cBattSOC[1] = cBuff[1] & 0x0F;
		CANData.cBattSOC[0] = cBuff[0] & 0x0F;
		break;

	default:
		CANData.cBattSOC[2] = 0;
		CANData.cBattSOC[1] = 10;
		CANData.cBattSOC[0] = 10;
		break;
	}
}

// ============================================================================
// Name			: vCAN_eUpdateBattTemp_Exe
// Objective	: Update Battery Temp
//
// Input  		: none
// Output 		: Temp in % at CANData.cBattTemp in BCD
// Return		: none
// Version		: -
// ============================================================================
void vCAN_eUpdateBattTemp_Exe(void)
{
	uint8_t ucLen = 0;
	char cBuff[6];

	sprintf(cBuff, "%d", CANData.ucBattCellMaxTemp);
	ucLen = strlen(cBuff);
	switch (ucLen)
	{
	case 1:
		CANData.cBattTemp[2] = cBuff[0] & 0x0F;
		CANData.cBattTemp[1] = 10;
		CANData.cBattTemp[0] = 10;
		break;

	case 2:
		CANData.cBattTemp[2] = cBuff[1] & 0x0F;
		CANData.cBattTemp[1] = cBuff[0] & 0x0F;
		CANData.cBattTemp[0] = 10;
		break;

	case 3:
		CANData.cBattTemp[2] = cBuff[2] & 0x0F;
		CANData.cBattTemp[1] = cBuff[1] & 0x0F;
		CANData.cBattTemp[0] = cBuff[0] & 0x0F;
		break;

	default:
		CANData.cBattTemp[2] = 0;
		CANData.cBattTemp[1] = 10;
		CANData.cBattTemp[0] = 10;
		break;
	}
}

void vCAN_eReadErrCode_Exe(void)
{
	//	if( (CANData.ucAllErrorDispStatus == DISP_ODO_TRIP) ||\
//		(CANData.ucAllErrorDispStatus == NO_ERROR))
	//	{
	//		CANData.ucTotalErrors	= 0;
	//		CANData.ucErrDispCntr	= 0;
	//		CANData.ucErrDispTimer	= 0;

#if WDT
	WDT_REFRESH();
#endif

	// Check No. of Errors
	vCAN_iReadBattErr1_Exe();
	vCAN_iReadBattErr2_Exe();
	vCAN_iReadBattErr3_Exe();
	vCAN_iReadBattErr4_Exe();
	vCAN_iReadBattErr5_Exe();
	vCAN_iReadBattErr6_Exe();
	vCAN_iReadBattErr7_Exe();
	vCAN_iReadBattErr8_Exe();
	vCAN_iReadBattErr9_Exe();

	// Immobilizer Errors
	//		vCAN_iReadImmobErr_Exe();

	vCAN_iReadContErr1_Exe();
	vCAN_iReadContErr2_Exe();

	// Cluster Errors
	//	}
	//	else
	//	{
	//	}
}

static void vCAN_iReadContErr2_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucControllerErr2)
	{
		//		vIND_eOperateLED_Exe(MALFUNCTION_LED, ON);

		//----------------------------------
		// Bit 0 : MOSFET failure
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x01;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_MOSFET_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 1: Driver failure
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x02;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_DRIVER_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2: Over Current fault
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x04;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_OVER_CURRENT_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 3: Over Voltage fault
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x08;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_OVER_VOLTAGE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 4: Over Temperature fault
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x10;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_OVER_TEMPERATURE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 5: Under Voltage fault
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x20;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_UNDER_VOLTAGE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6: Motor phase failure
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x40;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_MOTOR_PHASE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 7: Motor Hall failure
		//----------------------------------
		ucVal = CANData.ucControllerErr2 & 0x80;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_MOTOR_HALL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadContErr1_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucControllerErr1 & 0B11110111)
	{
		//		vIND_eOperateLED_Exe(MALFUNCTION_LED, ON);

		//----------------------------------
		// Bit 0 : Motor Over Temperature fault
		//----------------------------------
		ucVal = CANData.ucControllerErr1 & 0x01;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_MOTOR_OVER_TEMPERATURE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 1: Motor stall failure
		//----------------------------------
		ucVal = CANData.ucControllerErr1 & 0x02;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_MOTOR_STALL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2: Throttle Error
		//----------------------------------
		ucVal = CANData.ucControllerErr1 & 0x04;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CONT_THROTTLE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr1_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr1)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 & 1: Cell Over Voltage
		//----------------------------------
		ucVal = CANData.ucBattErr1 & 0x03;
		if (ucVal == 0x01)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CELL_OVER_VOLTAGE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x02)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CELL_OVER_VOLTAGE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2 & 3: Cell Under Voltage
		//----------------------------------
		ucVal = CANData.ucBattErr1 & 0x0C;
		if (ucVal == 0x04)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CELL_UNDER_VOLTAGE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x08)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CELL_UNDER_VOLTAGE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 4 & 5: Pack Over Voltage
		//----------------------------------
		ucVal = CANData.ucBattErr1 & 0x30;
		if (ucVal == 0x10)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PACK_OVER_VOLTAGE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x20)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PACK_OVER_VOLTAGE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6 & 7: Pack Under Voltage
		//----------------------------------
		ucVal = CANData.ucBattErr1 & 0xC0;
		if (ucVal == 0x40)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PACK_UNDER_VOLTAGE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x80)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PACK_UNDER_VOLTAGE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr2_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr2)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 & 1: Discharge Over Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr2 & 0x03;
		if (ucVal == 0x01)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_OVER_TEMPERATURE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x02)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_OVER_TEMPERATURE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2 & 3: Discharge Low Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr2 & 0x0C;
		if (ucVal == 0x04)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_LOW_TEMPERATURE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x08)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_LOW_TEMPERATURE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 4 & 5: Charging Over Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr2 & 0x30;
		if (ucVal == 0x10)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGE_OVER_TEMPERATURE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x20)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGE_OVER_TEMPERATURE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6 & 7: Charging Low Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr2 & 0xC0;
		if (ucVal == 0x40)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGE_LOW_TEMPERATURE_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x80)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGE_LOW_TEMPERATURE_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr3_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr3)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 & 1: Differential Cell Temperature Protection
		//----------------------------------
		ucVal = CANData.ucBattErr3 & 0x03;
		if (ucVal == 0x01)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = HIGH_DIFF_CELL_TEMP_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x02)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = HIGH_DIFF_CELL_TEMP_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2 & 3: Over Current Charging
		//----------------------------------
		ucVal = CANData.ucBattErr3 & 0x0C;
		if (ucVal == 0x04)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGING_OVER_CURRENT_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x08)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGING_OVER_CURRENT_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 4 & 5: Regen Over Current
		//----------------------------------
		ucVal = CANData.ucBattErr3 & 0x30;
		if (ucVal == 0x10)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = REGEN_OVER_CURRENT_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x20)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = REGEN_OVER_CURRENT_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6 & 7: Discharge Over Plus Current
		//----------------------------------
		ucVal = CANData.ucBattErr3 & 0xC0;
		if (ucVal == 0x40)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_OVER_CURRENT_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x80)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_OVER_CURRENT_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr4_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr4 & 0B11001111)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 & 1: DISCHARGE_OVER_PLUS_CURRENT_ERR
		//----------------------------------
		ucVal = CANData.ucBattErr4 & 0x03;
		if (ucVal == 0x01)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_OVER_PLUS_CURRENT_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x02)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_OVER_PLUS_CURRENT_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2 & 3: Low Insulation
		//----------------------------------
		ucVal = CANData.ucBattErr4 & 0x0C;
		if (ucVal == 0x04)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = LOW_INSULATION_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x08)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = LOW_INSULATION_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		/*		//----------------------------------
				// Bit 4 & 5: Low SOC
				//----------------------------------
				ucVal = CANData.ucBattErr4 & 0x30;
				if(ucVal == 0x10)
				{
					CANData.uiCurrentErrors[CANData.ucTotalErrors] = LOW_SOC_ALARM;
					CANData.ucTotalErrors++;
				}
				else if(ucVal == 0x20)
				{
					CANData.uiCurrentErrors[CANData.ucTotalErrors] = LOW_SOC_PROTECTION;
					CANData.ucTotalErrors++;
				}
				else
				{
				}*/

		//----------------------------------
		// Bit 6 & 7: BATT_ID_CONFLICT_ERR
		//----------------------------------
		ucVal = CANData.ucBattErr4 & 0xC0;
		if (ucVal == 0x40)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = BATT_ID_CONFLICT_ALARM;
			CANData.ucTotalErrors++;
		}
		else if (ucVal == 0x80)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = BATT_ID_CONFLICT_PROTECTION;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr5_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr5)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 : AFE Comm Error
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x01;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_COMM_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 1: CELL_VOLTAGE_ACQ_ERR
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x02;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CELL_VOLTAGE_ACQ_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2: TEMP_ACQ_ERR
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x04;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = TEMP_ACQ_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 3: BALANCE_RESISTOR_OVER_TEMP_ERR
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x08;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = BALANCE_RESISTOR_OVER_TEMP_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 4: MOS Over Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x10;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = MOSFET_OVER_TEMP_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 5: PRE_DISCHARGE_MOSFET_OVER_TEMP_ERR
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x20;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PRE_DISCHARGE_MOSFET_OVER_TEMP_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6: Pre Discharge TimeOut
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x40;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PRE_DISCHARGE_TIMEOUT_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 7: Pre DisCharge Over Current
		//----------------------------------
		ucVal = CANData.ucBattErr5 & 0x80;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PRE_DISCHARGE_OVER_CURRENT_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr6_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr6)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 : AFE Comm Error
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x01;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_OVER_VOLTAGE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 1: AFE Under Voltage
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x02;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_UNDER_VOLTAGE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2: AFE Discharge Over Current
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x04;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_DISCHARGE_OVER_CURRENT_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 3: AFE charge Over Current
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x08;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_CHARGE_OVER_CURRENT_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 4: AFE Short Circuit
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x10;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_SHORT_CIRCUIT_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 5: AFE Charging Over Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x20;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_CHARGING_OVER_TEMP_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6: AFE Charging Low Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x40;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_CHARGING_LOW_TEMP_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 7: AFE Discharge OVER Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr6 & 0x80;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_DISCHARGE_OVER_TEMP_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr7_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr7 & 0B11111101)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 : AFE Discharge Low Temp
		//----------------------------------
		ucVal = CANData.ucBattErr7 & 0x01;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = AFE_DISCHARGE_LOW_TEMP_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		/*		//----------------------------------
				// Bit 1: FULLY_CHARGE_ERR
				//----------------------------------
				ucVal = CANData.ucBattErr7 & 0x02;
				if(ucVal)
				{
					CANData.uiCurrentErrors[CANData.ucTotalErrors] = FULLY_CHARGE_ERR;
					CANData.ucTotalErrors++;
				}
				else
				{
				}*/

		//----------------------------------
		// Bit 2: Zero Volt Charge Inhibiion
		//----------------------------------
		ucVal = CANData.ucBattErr7 & 0x04;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = ZERO_VOLT_PROTECTION_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 3: Pre Placed MOS Adhesion
		//----------------------------------
		ucVal = CANData.ucBattErr7 & 0x08;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PRE_DISCHARGE_MOSFET_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 4: Discharge MOS Adhesion
		//----------------------------------
		ucVal = CANData.ucBattErr7 & 0x10;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_MOSFET_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 5: Charge MOS fail
		//----------------------------------
		ucVal = CANData.ucBattErr7 & 0x20;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGE_MOSFET_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6: Pre Placed MOS Adhesion
		//----------------------------------
		ucVal = CANData.ucBattErr7 & 0x40;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = PRE_DISHARGE_MOSFET_DRIVER_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 7: Discharge MOS DRIVER FAIL
		//----------------------------------
		ucVal = CANData.ucBattErr7 & 0x80;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_MOSFET_DRIVE_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr8_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr8 & 0B10111111) // Rev 1.021 19-09-2024
										 //	if(CANData.ucBattErr8)							// Rev 1.021 19-09-2024
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 : charge MOSFET DRIVER FAIL
		//----------------------------------
		ucVal = CANData.ucBattErr8 & 0x01;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGE_MOSFET_DRIVE_FAIL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 1: EOL
		//----------------------------------
		ucVal = CANData.ucBattErr8 & 0x02;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = EOL_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2: Severe Over Voltage
		//----------------------------------
		ucVal = CANData.ucBattErr8 & 0x04;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = SEVERE_OVER_CELL_VOLTAGE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 6: Charge Protection Error
		//----------------------------------
		//		ucVal = CANData.ucBattErr8 & 0x40;
		//		if(ucVal)
		//		{
		//			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGE_PROTECTION_STATE_ERR;
		//			CANData.ucTotalErrors++;
		//		}
		//		else
		//		{
		//		}

		//----------------------------------
		// Bit 7: AFE Discharge OVER Temperature
		//----------------------------------
		ucVal = CANData.ucBattErr8 & 0x80;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = DISCHARGE_PROTECTION_STATE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}

static void vCAN_iReadBattErr9_Exe(void)
{
	uint8_t ucVal;

	if (CANData.ucBattErr9)
	{
		//		vIND_eOperateLED_Exe(BATT_ERR_LED, ON);

		//----------------------------------
		// Bit 0 : Charger Connected but Not Charging
		//----------------------------------
		ucVal = CANData.ucBattErr9 & 0b00000001;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = CHARGER_NOT_CHARGING_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 1 : Over Temperature > 60degC
		//----------------------------------
		ucVal = CANData.ucBattErr9 & 0b00000010;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = BATT_OVER_TEMPERATURE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}

		//----------------------------------
		// Bit 2: Thermal Runaway > 65degC
		//----------------------------------
		ucVal = CANData.ucBattErr9 & 0b00000100;
		if (ucVal)
		{
			CANData.uiCurrentErrors[CANData.ucTotalErrors] = BATT_THERMAL_RUNAWAY_TEMPERATURE_ERR;
			CANData.ucTotalErrors++;
		}
		else
		{
		}
	}
	else
	{
	}
}


/*
	End of File
*/
