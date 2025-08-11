/*
 * APP.c
 *
 *  Created on: Mar 1, 2025
 *      Author: B.Zaveri
 */

/* Includes ------------------------------------------------------------------*/
#include "BATT.h"
#include "APP.h"
#include "COMM.h"
#include "GEN.h"
// #include "OTA.h"
#include "GPS.h"
#include "CAN.h"
#include <string.h>


// ============================================================================
// FUNCTION PROTOTYPE
// ============================================================================
static void MX_ADC1_Init(void);
static void MX_DMA_Init(void);
void vBATT_eInit(void);
void vBATT_eReadMainBATTVolt_Exe(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void vBATT_Start_ADC(void);
void vBATT_eDetectBatteryPresence(void);

// ============================================================================
// Handles
// ============================================================================
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc1;
// ============================================================================
//  Structures
// ============================================================================
TsBATTData BATTData;
TsBATTStatus BATTStatus;
extern TsCAN CANData;
extern TsUARTData UARTData;
extern TsAPP_eTimer TIMERData;
extern TsAPPData APPData;
extern TsGSMStatus GSMStatus;
extern TsGSMData GSMData;

void vBATT_eInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // GPIO Ports Clock Enable
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    // __HAL_RCC_GPIOC_CLK_ENABLE();
    // __HAL_RCC_GPIOF_CLK_ENABLE();

    MX_DMA_Init();
    MX_ADC1_Init();

    GPIO_InitStruct.Pin = INTBATT_CHG_DRV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(INTBATT_CHG_DRV_GPIO_Port, &GPIO_InitStruct);

    BATTStatus.bIntBattMOSFETChgStatus = OFF;
    BATTStatus.bMainBATTVoltStatus = BATTERY_PRESENT;
    // BATTStatus.bIntBATTVoltStatus = BATTERY_PRESENT;
    BATTStatus.bBATTStatus = BATTERY_PRESENT;
    BATTStatus.bBATTStatus = BATTERY_PRESENT;
}

// ============================================================================
// Name		: vBATT_eReadMainBATTVolt_Exe
// Objective	: Read Main Battery Voltage & Battery Presence
//			  Update previous status of Battery
// Input  		: none
// Output 	: 1. Main Batt Voltage  BATTData.ucBATTVolt
//			  2. Battery Presence BATTData.ucBATTStatus
// Return		: none
// Version	: -
// ============================================================================
void vBATT_eReadMainBATTVolt_Exe(void)
{
    // HAL_ADC_Start_DMA(&hadc, (uint32_t*)BATTData.ulRawADCVal, TOTAL_ADC_CHANNEL);
    // BATTData.MainBATTVolt = (uint8_t)(BATTData.ulRawADCVal[RAW_MAIN_BATT_ADC_VAL] * ADC_CFAC_MAINBATTVOL);

    // if (BATTData.ucBATTVolt < 10)
    // {
    //     BATTStatus.bBATTVoltStatus = NO_BATTERY_VOLTAGE;
    // }
    // else
    // {
    //     BATTStatus.bBATTVoltStatus = BATTERY_VOLTAGE_OK;
    // }
}

// ============================================================================
// Name		: vBATT_eReadIntBATTVolt_Exe
// Objective	: Read Internal Battery Voltage & Charging Status
//
// Input  		: none
// Output 	: Int Batt Voltage in mV at BATTData.uiBATTVolt
//
// Return		: none
// Version	: -
// ============================================================================
void vBATT_Start_ADC(void)
{
    HAL_ADC_Start_DMA(&hadc, (uint32_t *)BATTData.ulRawADCVal, TOTAL_ADC_CHANNEL);
    // BATTData.uiIntBATTVolt = (uint16_t)(BATTData.ulRawADCVal[RAW_INT_BATT_ADC_VAL] * ADC_CFAC_INTBATTVOL);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    uint16_t raw_mainbatt = BATTData.ulRawADCVal[RAW_MAIN_BATT_ADC_VAL];
    uint16_t raw_intbatt = BATTData.ulRawADCVal[RAW_INT_BATT_ADC_VAL];

    BATTData.uMainBATTVolt = (uint16_t)(raw_mainbatt * ADC_CFAC_MAINBATTVOL); // in V
    BATTData.uiIntBATTVolt = (uint16_t)(raw_intbatt * ADC_CFAC_INTBATTVOL);   // in mV
}

void vBATT_eDetectBatteryPresence(void)
{
    if (BATTData.uiIntBATTVolt >= INT_BATT_PRESENT_THRESHOLD_MV)
    {
        BATTStatus.bIntBATTVoltStatus = BATTERY_PRESENT;
    }
    else
    {
        BATTStatus.bIntBATTVoltStatus = BATTERY_NOT_PRESENT;
    }

    if (BATTData.uMainBATTVolt >= MAIN_BATT_PRESENT_THRESHOLD_MV)
    {
        BATTStatus.bMainBATTVoltStatus = BATTERY_PRESENT;
    }
    else
    {
        BATTStatus.bMainBATTVoltStatus = BATTERY_NOT_PRESENT;
    }
    vAPP_eFeedTheWDT_Exe();
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */
    //
    /* USER CODE END ADC1_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC1_Init 1 */
    //
    /* USER CODE END ADC1_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
     */
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV8;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_SEQ_FIXED;
    hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.LowPowerAutoPowerOff = DISABLE;
    hadc.Init.ContinuousConvMode = ENABLE;
    hadc.Init.NbrOfConversion = 1;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = ENABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
    hadc.Init.OversamplingMode = DISABLE;
    hadc.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    if (HAL_ADC_Init(&hadc) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_7;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_8;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC1_Init 2 */
    //
    /* USER CODE END ADC1_Init 2 */
}

/***** END OF FILE ****/
