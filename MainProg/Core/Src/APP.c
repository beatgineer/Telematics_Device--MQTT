/**
  ******************************************************************************
  * @file          : APP.c
  * @brief         : Application Layer
  ******************************************************************************
  // APP
  // Version  : 1301,
  // Author   : Brij Z
  // Date	  : 23-03-2025
  ******************************************************************************
  MCU : STM32G0B1RC 256kB Flash, RAM 144kB

  MCU FLASH SIZE : 0x0800 0000  to  0x0803 FFFF : 256kB

	_________________________28-07-2025 MEMORY MAP_________________________________________________
  //  ---------------------------------------------------------------------------------
  //  BOOT LOADER         0x0800 0000 - 0x0800 A800   42 KB   Pages: 0-20
  //  ---------------------------------------------------------------------------------
  //  IMG1                0x0800 B000 - 0x0801 4800   46 KB    Pages: 21-43
  //  ---------------------------------------------------------------------------------
  //  IMG2                0x0801 6000 - 0x0801 F800   40 KB    Pages: 44-63
  //  ---------------------------------------------------------------------------------
  //  CONFIG              0x0803 F800 - 0x0803 FFFF   2 KB     Page: 127
  //  ---------------------------------------------------------------------------------

  // 64 Bit Data
  //  0 - Invalid
  //  1 � Updated Application in Flash 1
  //  2 � Updated Application in Flash 2
  //  3 onwards.. - Invalid
 * APP.c
 *
 *  Created on: Mar 1, 2025
 *      Author: Brij.Z
 */

// ============================================================================
// STANDARD LIBRARIES
// ============================================================================
#include "setjmp.h"
#include <string.h>
#include <stdlib.h>
#include <stm32g0b1xx.h>
// ============================================================================
// HEADER FILES
// ============================================================================
#include "main.h"
#include "APP.h"
#include "ADC.h"
#include "CAN.h"
#include "COMM.h"
#include "GSM.h"
#include "GPS.h"
#include "GEN.h"
#include "E2PROM_I2C.h"
#include "CONFIG_EEPROM.h"
#include "UTL.h"
#include "BATT.h"
#include "OTA.h"
#include "ODO.h"
#include "FTP.h"

// ============================================================================
// FUNCTION PROTOTYPE
// ============================================================================
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin);
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin);
static void vAPP_iReadLiveData_Exe(void);
static void vAPP_iSendLiveModeData_Exe(void);
bool SendDataOverMQTT_Exe(void);
void vAPP_eSendLiveDataAtTxRate_Exe(void);
void vAPP_eWriteMsgCntrInEEPROM_Exe(void);
static void vAPP_iReadIgnStatus_Exe(void);
void SystemClock_Config(void);
static void MX_IWDG_Init(void);
void vAPP_eFeedTheWDT_Exe(void);
static void vAPP_iInit(void);
void MX_TIM2_Init(void);
static void MX_CRC_Init(void);
static void vAPP_iReadOTACmdFrmSMS_Exe(void);
void vAPP_iVehicleRunStatusAction_Exe(void);

// ============================================================================
jmp_buf jmpBuffer;
uint16_t uiDelayCounts;

// ============================================================================
// BUFFERS
// ============================================================================
char cAPP_eGlobalBuffer[500];
char cAPP_eGPSDataBuffer[900]; // increased buffer size from 300 to 900 in order to add other parameters into the buffer
// ============================================================================
// HANDLES
// ============================================================================
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef htim2;
CRC_HandleTypeDef hcrc;
// ============================================================================
// STRUCTURES
// ============================================================================
TsAPP_eTimer TIMERData;
TsAPP APPStatus;
TsAPP_eConfig APPCONFIG;
extern TsUARTData UARTData;
extern TsGPSData GPSData;
extern TsGSMStatus GSMStatus;
extern TsGSMData GSMData;
extern TsCAN CANData;
extern TuCANStatus CANStatus;
extern TsOTAData OTAData;
extern TsOdo ODOData;
extern TsBATTData BATTData;
extern TsBATTStatus BATTStatus;
// =============================================================================

int main(void)
{
	uint8_t ucResp;

	HAL_Init();
	SystemClock_Config();
	vAPP_iInit();

	HAL_GPIO_WritePin(MCU_LED_GPIO_Port, MCU_LED_Pin, GPIO_PIN_SET);

	TIMERData.uiLEDCntr = 0;

	vBATT_eInit();

	MX_IWDG_Init();
	vAPP_eFeedTheWDT_Exe();
	vCOMM_eUSART1Init();

	vEEPROM_eInit();
	vODO_eInit();

	vCAN_eInit();

	// for (ucResp = 0; ucResp < 10; ucResp++)
	// {
	// 	vBATT_eReadMainBATTVolt_Exe();
	// 	vBATT_eReadIntBATTVolt_Exe();
	// 	vAPP_eFeedTheWDT_Exe();
	// }
	// BATTStatus.bBATTVoltStatusPrev = BATTStatus.bBATTStatus;

	vGSM_eInit();
	vGPS_eInit();

	vAPP_eFeedTheWDT_Exe();

	vGEN_eONBattChg_Exe();

	vGSM_eStartGSM_Exe();
	vGPS_eStartGPS_Exe();
	vCONFIGEEPROM_eReadConfigData_Exe();

	/*if (GSMStatus.bSIMInsertStatus == SIM_NOT_INSERTED)
{
  vGSM_eReStartGSM_Exe();
}*/
	// for (ucResp = 0; ucResp < 20; ucResp++)
	// {
	// 	vBATT_eReadMainBATTVolt_Exe();
	// 	vBATT_eReadIntBATTVolt_Exe();
	// 	vAPP_eFeedTheWDT_Exe();
	// }
	vBATT_Start_ADC();
	vAPP_eFeedTheWDT_Exe();

	vGSM_eInitaliseGSM_Exe();

	vBATT_eDetectBatteryPresence();
	vGEN_eMakeIntBattChargingOnOff_Exe(ON);

	GSM_RxCntr = 0;
	GSM_TimeOut = 0;

	vAPP_iReadIgnStatus_Exe();
	vGEN_eLoadTxRate_Exe();

	APPStatus.bIgnStatusPrev = 0;
	// APPStatus.bIgnStatusPrev = APPStatus.bIgnStatus;
	TIMERData.ulPresentTxRate = TIMERData.ulTxRate;

	TIMERData.ul18SecCntr = 0;
	TIMERData.ul30SecCntr = 0;
	TIMERData.ul65SecCntr = 0;

	// =============================================================================

	while (1)
	{
		vAPP_eFeedTheWDT_Exe();
		vCAN_eReadCANData_Exe();
		vCAN_eGetBattID_Exe();
		vAPP_iReadIgnStatus_Exe();
		vAPP_eSendLiveDataAtTxRate_Exe();
		// vBATT_eReadIntBATTVolt_Exe();

		//-----------------------------------
		// Task based Jobs
		//-----------------------------------
		if (TIMERData.uiTimer360mS >= (360 - 1))
		{
			TIMERData.uiTimer360mS = 0;
			vODO_eCalculateTripAndOdo_Exe(CANData.ucSpeedInkmph);
			vAPP_iReadIgnStatus_Exe();
                        vBATT_eDetectBatteryPresence();
		}
		else
		{
		}

		if (TIMERData.ul18SecCntr > 18000)
		{
			TIMERData.ul18SecCntr = 0;
			vGEN_eMakeIntBattChargingOnOff_Exe(ON);
		}

		if (TIMERData.ul30SecCntr > 30000)
		{
			TIMERData.ul30SecCntr = 0;

			if ((GSMStatus.bSIMInsertStatus == SIM_NOT_INSERTED) || (GSMData.ucNoSocketCntr > 10) || (GSMData.ucNoResponseCntr > 10) ||
				(GSMStatus.bCREGStatus == NETWORK_REGISTRATION_FAILED) ||
				(GSMStatus.bCGREGStatus == NETWORK_REGISTRATION_FAILED) ||
				(GSMStatus.bGSMRebootRequired == 1))
			{
				vGSM_eReStartGSM_Exe();
				GSMStatus.bGSMRebootRequired = 0;
			}
		}

		if (TIMERData.ul65SecCntr >= 15000) // replaced 65sec to 15sec for testing purpose
		{
			TIMERData.ul65SecCntr = 0;
			// vGEN_eMakeGSMPowerDownIfBattNotPresent_Exe();
			vAPP_iVehicleRunStatusAction_Exe();
			vAPP_iReadOTACmdFrmSMS_Exe();
		}
		else
		{
		}

		//-----------------------------------
		// Firmware Update (300sec Timer)
		//-----------------------------------
		if (TIMERData.ul300SecCntr >= 3000) // 1sec = 1000ms.
		{
			TIMERData.ul300SecCntr = 0;
			vFTP_eCheckFTPServerForFWUpdate_Exe();
			HAL_UART_Transmit(&huart1, (uint8_t *)"OTA not done\n", 13, 500);
		}

		if (TIMERData.uiLEDCntr > 200)
		{
			HAL_GPIO_TogglePin(MCU_LED_GPIO_Port, MCU_LED_Pin);
			TIMERData.uiLEDCntr = 0;
		}
		vGPS_eOperateGPSLED_Exe();
	}
}

// ============================================================================
// Name			: vAPP_eSendLiveDataAtTxRate_Exe
// Objective	: Send Live Data at Tx Rate based on Ignition Status (20sec IGN-ON, 10min IGN-OFF, 60min Battery Removed)
// ============================================================================
void vAPP_eSendLiveDataAtTxRate_Exe(void)
{
	if (GSMStatus.ucGSMReadyStatus == GSM_READY || GSMStatus.ucGSMReadyStatus == GSM_WAS_READY)
	{
		if (TIMERData.ulPresentTxRate >= TIMERData.ulTxRate)
		{
			vAPP_iSendLiveModeData_Exe();
			TIMERData.ulPresentTxRate = 0;
			vGEN_eLoadTxRate_Exe();
		}
		if (APPStatus.bIgnStatus != APPStatus.bIgnStatusPrev)
		{
			vGEN_eLoadTxRate_Exe();
		}
		if (BATTStatus.bMainBATTVoltStatus == BATTERY_NOT_PRESENT)
		{
			vGEN_eLoadTxRate_Exe();
		}
		else if (BATTStatus.bMainBATTVoltStatus == BATTERY_PRESENT)
		{
			vGEN_eLoadTxRate_Exe();
		}
	}
	else
	{
		vGSM_eStartGSM_Exe();
	}
}

// ============================================================================
// Name			: vAPP_iReadOTAFrmSMS_Exe
// Objective	: Read OTA Command from SMS
// ============================================================================
static void vAPP_iReadOTACmdFrmSMS_Exe(void)
{
	vOTA_eReadSMSOTACmdFromSIM_Exe();
	vOTA_eReadSMSOTACmdFromME_Exe();
}

// ============================================================================
// Name			: ucAPP_iSetVehicleRunStatus_Exe
// Objective	: Enable-Disable Motor Controller Based On Vehicle Run Status
// ============================================================================

void vAPP_iVehicleRunStatusAction_Exe(void)
{
	uint8_t ucTemp;
	Enum_VehState_Mode ucStatus = MOTOR_POWER_OFF;

	// To Disable Motor Controller
	if ((APPStatus.bVehRunStatus == VEHICLE_STOP) && (APPStatus.bIgnStatus == IGN_ON) && (CANData.uiRPM == 0))
	{
		ucCAN_eTxData[0] = 0x00;
		ucCAN_eTxData[1] = 0x00;
		ucCAN_eTxData[2] = 0x00;
		ucCAN_eTxData[3] = 0x00;
		ucCAN_eTxData[4] = 0x00;
		ucCAN_eTxData[5] = 0x00;
		ucCAN_eTxData[6] = 0x00;
		ucCAN_eTxData[7] = 0x00;

		for (ucTemp = 0; ucTemp < 3; ucTemp++)
		{
			// Make Motor Controller Disable
			vCAN_eSendCANCommands_Exe(CANID_IoT_TO_MOTORCONTROLLER);
			HAL_Delay(100);
		}
	}
	else
	{
		// To Enable Motor Controller
		if (APPStatus.bVehRunStatus == VEHICLE_RUN)
		{
			ucCAN_eTxData[0] = 0x10;
			ucCAN_eTxData[1] = 0x00;
			ucCAN_eTxData[2] = 0x00;
			ucCAN_eTxData[3] = 0x00;
			ucCAN_eTxData[4] = 0x00;
			ucCAN_eTxData[5] = 0x00;
			ucCAN_eTxData[6] = 0x00;
			ucCAN_eTxData[7] = 0x00;

			for (ucTemp = 0; ucTemp < 3; ucTemp++)
			{
				// Make Motor Controller Disable
				vCAN_eSendCANCommands_Exe(CANID_IoT_TO_MOTORCONTROLLER);
				HAL_Delay(100);
			}
		}
		else
		{
			ucStatus = ALREADY_DISABLE;
		}
	}
	// return ucStatus;
}

// =============================================================================

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == IGN_IN_Pin)
	{
		vAPP_iReadIgnStatus_Exe();

		// vAPP_iSendIgnEventData_Exe();
		for (int iCntr = 0; iCntr < 10000; iCntr++)
		{
			;
		}
	}
	/*
		if (GPIO_Pin == GSM_RI_IN_Pin)
		{
			if (GSMStatus.uiRITimer > 110) // 120mS pulse
			{
				GSMStatus.bRIDetected = TRUE;
			}
		}*/
}

// =============================================================================

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == IGN_IN_Pin)
	{
		APPStatus.bIgnStatus = OFF;
		//	vAPP_iSendIgnEventData_Exe();

		for (int iCntr = 0; iCntr < 10000; iCntr++)
		{
			;
		}
	}

	/*if (GPIO_Pin == GSM_RI_IN_Pin)
	{
		GSMStatus.uiRITimer = 0;
	}*/
}

// ============================================================================
// Name		: vAPP_iReadLiveModeData_Exe
// Objective	: Read Live Mode Data
// Input  		: Data Type :
//			  0 - Location Data
//			  1 onwards  - Event data
// Output 	: none
// Return		: 0 - DATA_SENT_FAIL / ERROR
//		  	1 - DATA_SENT
// ============================================================================

static void vAPP_iReadLiveData_Exe(void)
{
	ucGSM_eReadSignalStrength_Exe();
	vGPS_eReadGPSData_Exe();
	ucGSM_eReadRTC_Exe();
	vGSM_eReadNetworkInfo_Exe();
	vAPP_iReadIgnStatus_Exe();
}

static void vAPP_iSendLiveModeData_Exe(void)
{
	static uint8_t prevIgnStatus = 0xFF; // Initialize to an invalid state so first run is detected

	vAPP_eFeedTheWDT_Exe();
	bool bResp;

	vAPP_iReadLiveData_Exe();
	memset(cAPP_eGPSDataBuffer, 0, sizeof(cAPP_eGPSDataBuffer));

	// Detect ignition status change
	if (APPStatus.bIgnStatus != prevIgnStatus)
	{
		// ON → OFF
		if (prevIgnStatus == IGN_ON && APPStatus.bIgnStatus == IGN_OFF)
		{
			vGEN_eLoadAndParseData_Exe(EVENT_DATA, IGN_OFF_ALERT);
		}
		// OFF → ON
		else if (prevIgnStatus == IGN_OFF && APPStatus.bIgnStatus == IGN_ON)
		{
			vGEN_eLoadAndParseData_Exe(EVENT_DATA, IGN_ON_ALERT); // you can define this alert
		}

		// Update previous status
		prevIgnStatus = APPStatus.bIgnStatus;
	}
	else
	{
		// No change → send location data
		vGEN_eLoadAndParseData_Exe(LOCATION_DATA, NO_ALERT);
	}

	// Transmit the prepared data
	bResp = SendDataOverMQTT_Exe();

	if (bResp == TRUE)
	{
		APPStatus.uiMsgCntr++;
	}
}

static void vAPP_iSendIgnEventData_Exe(void)
{
	vAPP_eFeedTheWDT_Exe();
	bool bResp;

	vAPP_iReadLiveData_Exe();
	memset(cAPP_eGPSDataBuffer, 0, sizeof(cAPP_eGPSDataBuffer));

	if (APPStatus.bIgnStatus == IGN_ON)
	{
		vGEN_eLoadAndParseData_Exe(EVENT_DATA, IGN_ON_ALERT);
	}
	// else
	// {
	// 	vGEN_eLoadAndParseData_Exe(EVENT_DATA, IGN_OFF_ALERT);
	// }
	else if (APPStatus.bIgnStatus == IGN_OFF)
	{
		vGEN_eLoadAndParseData_Exe(EVENT_DATA, IGN_OFF_ALERT);
	}

	bResp = SendDataOverMQTT_Exe();

	if (bResp == TRUE)
	{
		APPStatus.uiMsgCntr++;
	}
	else
	{
		;
	}
}
// ============================================================================
bool SendDataOverMQTT_Exe(void)
{
	bool bStatus = FALSE;
	uint8_t ucTry = 0;

	do
	{
		bStatus = bMQTT_CheckAndConnect_Exe(); // Check broker connection
		if (bStatus == TRUE)
		{
			bStatus = bMQTT_PublishData_Exe(); // Publish live data
		}

		ucTry++;

		if (bStatus == FALSE)
		{
			vMQTT_Disconnect_Exe(); // Optional: disconnect on failure
		}

		vAPP_eFeedTheWDT_Exe();

	} while ((bStatus == FALSE) && (ucTry < 3));

	if (bStatus == TRUE)
	{
		GSMData.ucNoResponseCntr = 0;
		GSMData.ucErrorCntr = 0;
		GSMData.ucNoSocketCntr = 0;

		APPStatus.uiMsgCntr++;
		vGEN_eLoadTxRate_Exe();
	}

	return bStatus;
}

// ============================================================================
// Name		: vAPP_eWriteMsgCntr_Exe
// Objective	: Write Message Counter in EEPROM
// Input  		: None
// Output 	: none
// Return		: None
// Version	: -
// ============================================================================
void vAPP_eWriteMsgCntrInEEPROM_Exe(void)
{
	vEEPROM_eWriteByte_Exe(EEPROM_ADDR_MSGCOUNTER, (APPStatus.uiMsgCntr >> 8)); // Write Upper 8 Bit
	vEEPROM_eWriteByte_Exe(EEPROM_ADDR_MSGCOUNTER + 1, APPStatus.uiMsgCntr);	// Write Lower 8 Bit
}

static void vAPP_iReadIgnStatus_Exe(void)
{
	APPStatus.bIgnStatus = HAL_GPIO_ReadPin(IGN_IN_GPIO_Port, IGN_IN_Pin);
	// HAL_UART_Transmit(&huart1, (uint8_t *)&APPStatus.bIgnStatus, 1, 1000);
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
	RCC_OscInitStruct.PLL.PLLN = 8;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}

	/* Fix SysTick for 1ms tick */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
}

// =============================================================================

static void MX_IWDG_Init(void)
{
	/* USER CODE END IWDG_Init 1 */
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_64; // IWDG clock = 32,000/64 = 500 Hz (Each tick = 2 ms)
	hiwdg.Init.Window = 4095;				  // Timeout = (Reload Value + 1) x Tick Time
	hiwdg.Init.Reload = 4095;				  //(4095 + 1) x 2ms = 8192ms = 8.192s.  If the software does not refresh the watchdog within 8.192 seconds, the MCU will reset.
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
	{
		Error_Handler();
	}
}

// =============================================================================

void vAPP_eFeedTheWDT_Exe(void)
{
	HAL_IWDG_Refresh(&hiwdg);
}

// =============================================================================

static void MX_CRC_Init(void)
{

	/* USER CODE BEGIN CRC_Init 0 */
	//
	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */
	//
	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	if (HAL_CRC_Init(&hcrc) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */
	//
	/* USER CODE END CRC_Init 2 */
}

// =============================================================================

static void vAPP_iInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// GPIO Ports Clock Enable
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// Configure GPIO pin Output Level
	HAL_GPIO_WritePin(MCU_LED_GPIO_Port, MCU_LED_Pin, GPIO_PIN_RESET);

	// Configure GPIO pins : MCU_LED_Pin
	GPIO_InitStruct.Pin = MCU_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(MCU_LED_GPIO_Port, &GPIO_InitStruct);

	// Configure GPIO pins : IGN_INP_Pin
	GPIO_InitStruct.Pin = IGN_IN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(IGN_IN_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

// =============================================================================

void MX_TIM2_Init(void)
{

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 64 - 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1000 - 1;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 10;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */
	HAL_TIM_MspPostInit(&htim2);
}

// =============================================================================

void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	//__disable_irq();
	// while (1)
	//{
	//}
	/* USER CODE END Error_Handler_Debug */
}

// =============================================================================

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
