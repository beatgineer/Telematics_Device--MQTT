// ============================================================================
// COMM
// Version	: A
// Author	: AK
// Date		: 01-12-2021
// ============================================================================

//*****************************************************************************
//   INCLUDE
//*****************************************************************************
#include "main.h"
#include "COMM.H"
#include "APP.h"
#include "GSM.h"

#include <string.h>
#include <stdlib.h>

#include "stm32g0xx_hal.h"

extern TsGSMData GSMData;

//*****************************************************************************
// DEFINE and MACRO
//*****************************************************************************

//*****************************************************************************
// EXTERNAL VARIABLES
//*****************************************************************************
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
// ----------------------------------------------------------------------------
// Name		: vCOMM_eTxString_Exe
// Objective: SEND ASCII STRING
// Input  	: USART Num
//			  Pointer of String to be sent
// Output 	: none
// Return	: none
// Author	: AK
// Date		: 01 Dec 2021
// ----------------------------------------------------------------------------
void vCOMM_eTxString_Exe(UART_HandleTypeDef *USARTx, char *pcMessage, uint16_t uiTimeOut)
{
    uint16_t uiLen;
    uiLen = strlen(pcMessage);
    if (HAL_UART_Transmit(USARTx, (uint8_t *)pcMessage, uiLen, uiTimeOut) != HAL_OK)
    {
        Error_Handler();
    }

    //	UARTData.uiUARTTxCntr = uiLen;
}

// ----------------------------------------------------------------------------
// Name		: vCOMM_eTxHexBytes_Exe
// Objective: SEND Bytes
// Input  	: USART Num
//			  Pointer of String to be sent, Num of Hex Bytes
// Output 	: none
// Return	: none
// Author	: AK
// Date		: 01 Dec 2021
// ----------------------------------------------------------------------------
void vCOMM_eTxHexBytes_Exe(UART_HandleTypeDef *USARTx, char *pcMessage, uint8_t ucBytes, uint16_t uiTimeOut)
{
    uint8_t ucCntr;
    uint8_t cBuff[5];

    cBuff[1] = 0;

    for (ucCntr = 0; ucCntr < ucBytes; ucCntr++)
    {
        cBuff[0] = *(pcMessage + ucCntr);
        HAL_UART_Transmit(USARTx, cBuff, 1, uiTimeOut);
    }
}

// ----------------------------------------------------------------------------
// Name		: HAL_UART_RxCpltCallback
// Objective: UART Data Receive in Interrupt Mode
//			  Interrupt callback routine
// Input  	: UART Handle Type
// Output 	: UART Data at Data Buffer
//			  UART1 : UARTData.ucUART4Buff[]
//			  UART3 : UARTData.ucUART3Buff[] 
// Return	: none
// Version	: -
// Author	: AK
// Date		: 01 Dec 2021
// ----------------------------------------------------------------------------
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        if (GSM_RxCntr < GSM_BuffSize)
        {
            GSM_DataBuffer[GSM_RxCntr++] = GSM_RxOneByte[0];
            GSM_DataBuffer[GSM_RxCntr] = 0;
        }
        else
        {
            GSM_RxCntr = GSM_BuffSize;
        }

        GSM_TimeOut = 0;
        HAL_UART_Receive_IT(GSMComPortHandle, (uint8_t *)GSM_RxOneByte, 1);
    }
    else
    {
    }
}

// void vCOMM_eUSART1Init(void)
// {
//   huart1.Instance = USART1;
//   huart1.Init.BaudRate = 115200;
//   huart1.Init.WordLength = UART_WORDLENGTH_8B;
//   huart1.Init.StopBits = UART_STOPBITS_1;
//   huart1.Init.Parity = UART_PARITY_NONE;
//   huart1.Init.Mode = UART_MODE_TX_RX;
//   huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//   huart1.Init.OverSampling = UART_OVERSAMPLING_16;
//   huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//   huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
//   huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
//   if (HAL_UART_Init(&huart1) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
//   {
//     Error_Handler();
//   }
// }

/**
 * @brief  UART error callbacks
 * @param  UartHandle: UART handle
 * @note   This example shows a simple way to report transfer error, and you can
 *         add your own implementation.
 * @retval None
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  if (UartHandle == &huart3)
  {
    __HAL_UART_CLEAR_PEFLAG(&huart3);
    HAL_UART_MspDeInit(&huart3);
    HAL_UART_MspInit(&huart3);
  }
}
#if 0
/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @note   This example shows a simple way to report end of IT Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}


/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @note   This example shows a simple way to report end of IT Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		RS485_TxComplete = 1;
	}
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
	Error_Handler();
}

#if 0

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
/*
//	if (UartHandle == &huart1)
//	{
//		__HAL_UART_CLEAR_PEFLAG(&huart1);
//		HAL_UART_MspDeInit(&huart1);
//		HAL_UART_MspInit(&huart1);
//		// FRAMING, overrun or HIGH noise ERROR
//		// Restart USART and continue communicating
//	}
	if (UartHandle == &huart2)
	{
		__HAL_UART_CLEAR_PEFLAG(&huart2);
		HAL_UART_MspDeInit(&huart2);
		HAL_UART_MspInit(&huart2);
		// FRAMING, overrun or HIGH noise ERROR
		// Restart USART and continue communicating
	}
//	if (UartHandle == &huart3)
//	{
//		__HAL_UART_CLEAR_PEFLAG(&huart3);
//		HAL_UART_MspDeInit(&huart3);
//		HAL_UART_MspInit(&huart3);
//		// FRAMING, overrun or HIGH noise ERROR
//		// Restart USART and continue communicating
//	}*/
}
#endif
/***** END OF FILE ****/

#endif
