// ============================================================================
// Module	: CAN
// Version	: A
// Author	: AK
// Date		: 03 Apr 2024
// ============================================================================

//*****************************************************************************
//   INCLUDE
//*****************************************************************************

#include "main.h"
#include "APP.h"
#include "CAN.h"

#include "stm32g0xx_hal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//*****************************************************************************

static void MX_FDCAN1_Init(void);
static void vCAN_iCANConfig(void);

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
// extern	TsAPPStat	APPStatus;

// uint32_t ulCANID[7] 		 = {0x1002FFA7, 0x1003FFA7, 0x1011FFA7, 0x1023FFA7, 0x1024FFA7, 0x1027FFA7, 0x1028FFA7};
// const uint8_t ucCANTxData[7] = {0, 1, 2, 3, 4, 5, 6};

uint8_t ucCAN_eRxData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t ucCAN_eRxData1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t ucCAN_eTxData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
// uint32_t ulFTP_eTxCANID;

// ============================================================================
// Name			: vCAN_eInit
// Objective	: Initialise RS485
// Input  		: none
// Output 		: none
// Return		: none
// Version		: -
// Author		: AK
// Date			: 05-12-2018
// ============================================================================
void vCAN_eInit(void)
{
    MX_FDCAN1_Init();
    vCAN_iCANConfig();

    CANData.ucCANFWStatus = NO_RESPN;
    CANStatus.bRdyToSend = false;
    CANStatus.bSecurityAccess = false;
    CANStatus.bSecurityPassed = false;
    CANStatus.bClusterPresent = false;
}

/**
 * @brief FDCAN1 Initialization Function
 * @param None
 * @retval None
 */
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

/**
 * @brief  Configures the FDCAN.
 * @param  None
 * @retval None
 */
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
    TxHeader.Identifier = 0x301;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;
}

void vCAN_eSendCANCommands_Exe(void)
{
    TxHeader.Identifier = 0x301;

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
    switch (RxHeader.Identifier)
    {
    case 0x300:
        if (ucCAN_eRxData[0] == UDS_TOOL_CMD_TESTER_PRESENT)
        {
            if (CANStatus.bClusterPresent == false)
            {
                CANData.ucCANFWStatus = TESTER_PRESENT;
                CANStatus.bClusterPresent = true;
            }
            else
            {
                CANStatus.bCANDataRecd = true;
            }
        }
        else if (ucCAN_eRxData[0] == UDS_TOOL_CMD_ROUTINE_CONTROL)
        {
            if (CANStatus.bFileDataRecd == false)
            {
                CANStatus.bFileDataRecd = true;
            }
            else
            {
                CANStatus.bCANDataRecd = true;
            }
        }
        else
        {
            CANStatus.bCANDataRecd = true;
        }
        break;
    }
}

/*
    End of File
*/
