// ============================================================================
// Module	: Quectel GSM/GPS Module EC200
// Version	: 00
// Author	: AK
// Date		: 24 Jul 2018
// ============================================================================

//*****************************************************************************
//   INCLUDE
//*****************************************************************************
#include "stm32g0xx_hal.h"
#include "stm32g0xx.h"
#include "APP.h"
#include "GSM.H"
#include "COMM.H"
// #include "MCUFlash.h"
// #include "EEPROM.h"
// #include "FTP.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static uint8_t ucGSM_iCheckATResponse_Exe(void);
// static uint8_t ucGSM_iProcessATCmdWithNum_Exe(uint8_t ucATCmdNum, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
// static uint8_t ucGSM_iProcessATCmd_Exe(char *cATCmd, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
static uint8_t ucGSM_iProcessATCmdDualResp_Exe(char *cATCmd, uint8_t ucResp1Num, uint8_t ucLenResp1, uint8_t ucResp2Num, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
static uint8_t ucGSM_iWait4RespOrTimeOutDualResp_Exe(uint8_t ucLenResp1, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
static uint8_t ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(uint8_t ucATCmdNum, uint8_t ucResp1Num, uint8_t ucLenResp1, uint8_t ucResp2Num, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
static uint8_t ucGSM_iProcessATCmdDualResp_Exe(char *cATCmd, uint8_t ucResp1Num, uint8_t ucLenResp1, uint8_t ucResp2Num, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
static void vGSM_iRegisterAndSetModule_Exe(void);
static void vGSM_iReadSIMInsertStatus_Exe(void);
static bool bGSM_iCheckAPNSetting_Exe(void);
static uint8_t ucGSM_iReadSignalStrength_Exe(void);
static bool bGSM_iNetworkSetup_Exe(void);
static void vGSM_iSetAPN_Exe(void);
static bool bGSM_iReadNetworkRegStatus_Exe(uint8_t ucNetworkStatusType);
static bool bGSM_iCompareData_Exe(char *cData1Adr, char *cData2Adr, uint16_t uiLen);

//*****************************************************************************
// EXTERNAL VARIABLES
//*****************************************************************************
extern UART_HandleTypeDef huart3;
TsGSMData GSMData;
TsGSMStatus GSMStatus;

const char *cGSM_eATCommandTbl[] =
    {
        "AT+QFTPCLOSE\r",                 // 0
        "AT+QFTPCFG=\"contextid\",",      // 1
        "AT+QFTPCFG=\"account\",",        // 2
        "AT+QFTPCFG=\"filetype\",1\r",    // 3
        "AT+QFTPCFG=\"transmode\",1\r",   // 4
        "AT+QFTPCFG=\"rsptimeout\",90\r", // 5
        "AT+QFCLOSE=",                    // 6
        "AT+QFDEL=\"",                    // 7
        "AT+QFOPEN=",                     // 8
        "AT+QBTPWR?\r",                   // 9
        "AT+QFSEEK=",                     // 10
        "AT+QFREAD=",                     // 11
        "AT+QFTPOPEN=",                   // 12
        "AT+QFLST=",                      // 13
        "AT+QFTPGET=",                    // 14
        "AT+QFTPCWD=",                    // 15
        "AT+QFTPSIZE=",                   // 16
        "AT+CPMS=\"ME\",\"ME\",\"ME\"\r", // 17
        "AT+CMGF=1\r",                    // 18
        "AT+CSMP=17,167,0,0\r",           // 19
        "AT+CSDH=1\r",                    // 20
        "ATE0\r",                         // 21
        "AT+CPIN?\r",                     // 22
        "AT+CSQ\r",                       // 23
        "AT+CREG?\r",                     // 24
        "AT+CGREG?\r",                    // 25
        "AT+CEREG?\r",                    // 26
        "AT+QICSGP=1\r",                  // 27
        "m2m.myrevolt.com",               // 28
        "AT+CMGS=",                       // 29
        "SEND OK",                        // 30
};

const char *cGSM_eATResponseTbl[] =
    {
        "OK\r",     // 0
        "+QFOPEN:", // 1
        "CONNECT",  // 2
        "+CME ERROR:",
        "ERROR",
        "+CMS ERROR:", // 5
        "+QFTPOPEN: 0,0",
        "+QFLST:",
        "+QFTPGET:",
        "+QFTPCWD: 0,0",
        "+QFTPSIZE: 0,",    // 10
        "+CPMS:",           // 11
        "+CPIN: READY",     // 12
        "+CSQ:",            // 13
        "+CREG:",           // 14
        "+CGREG:",          // 15
        "+CEREG:",          // 16
        "+QICSGP:",         // 17
        "+CMGS:",           // 18
        ">",                // 19
        "SEND OK",          // 20
        "FW UPDATE Ok",     // 21
        "FW UPDATE FAILED", // 22
};

//*****************************************************************************
// INTERNAL VARIABLES
//*****************************************************************************
static void MX_USART3_UART_Init(void);

//*****************************************************************************
// STRUCTURE VARIABLES
//*****************************************************************************
// ============================================================================
// Name			: vGSM_eInit
// Objective	: Initialise Quectel GSM Module
// Input  		: none
// Output 		: none
// Return		: none
// Version		: -
// Author		: AK
// Date			: 25-04-2023
// ============================================================================
void vGSM_eInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // GPIO Ports Clock Enable
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GSM_RESET_Pin | GSM_PWRKEY_Pin | GSM_PWR_DRV_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GSM_RESET_Pin | GSM_PWRKEY_Pin | GSM_PWR_DRV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : GSM_RI_IN_Pin */
    GPIO_InitStruct.Pin = GSM_RI_IN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GSM_RI_IN_GPIO_Port, &GPIO_InitStruct);

    GSMData.uiGSMPowerCntr = 0;
    GSMData.ucNoResponseCntr = 0;
    GSMData.ucErrorCntr = 0;
    GSMData.ucNoSocketCntr = 0;
    GSMStatus.bPwrDownStatus = 0;
    GSMStatus.bGSMRebootRequired = FALSE;

    MX_USART3_UART_Init();
    HAL_UART_Receive_IT(GSMComPortHandle, (uint8_t *)GSM_RxOneByte, 1);

    GSM_RxCntr = 0;
    GSM_TimeOut = 0;
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

void vGSM_ePowerON_Exe(void)
{
    vAPP_eFeedTheWDT_Exe();
    HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin, GPIO_PIN_SET);
    HAL_Delay(2100);

    vAPP_eFeedTheWDT_Exe();
    HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port, GSM_PWRKEY_Pin, GPIO_PIN_RESET);

    GSM_RxCntr = 0;
    GSM_TimeOut = 0;
    GSMData.uiGSMPowerCntr = 0;

    do
    {
    } while (((GSM_RxCntr > 2) && (GSM_TimeOut > 100)) || (GSMData.uiGSMPowerCntr > 100));
    //	} while(((GSM_RxCntr > 60) && (GSM_TimeOut > 100)) || (GSMData.uiGSMPowerCntr > 100));
}

void vGSM_eDelay100mS_Exe(uint16_t uiCount)
{
    uint32_t ulCount;

    GSMData.uiGSMPowerCntr = 0;

    ulCount = uiCount * 100;

    do
    {
        vAPP_eFeedTheWDT_Exe();
    } while (GSMData.uiGSMPowerCntr < ulCount);
}

// ============================================================================
// Name			: ucGSMData_eCompareStringInGSMBuff_Exe
// Objective	: Compare Received Response at GSM Buffer with the Given Response
// Input  		: Given Response Pointer
// Output 		: none
// Return		: 	RESPONSE_MATCHING		- 9
//		  	  		RESPONSE_NOT_MATCHING	- 10
//		  	  		RESPONSE_ERROR			- 11
//		  	  		RESPONSE_CME_ERROR		- 12
//		  	  		RESPONSE_CMS_ERROR		- 13
//
//					Error Code in case of CME & CMS at
//						GSMData.cErrCode[0] - MSD
//						GSMData.cErrCode[1] - LSD
// Version		: -
// Author		: AK
// Date			: 26-07-2018
// ============================================================================
uint8_t ucGSM_eCompareStringInGSMBuff_Exe(uint8_t ucResponseNum)
{
    uint8_t ucTemp;
    char *cBuffAdd;

    cBuffAdd = strstr((char *)GSM_DataBuffer, (char *)cGSM_eATResponseTbl[ucResponseNum]);

    if (cBuffAdd != NULL)
    {
        ucTemp = RESPONSE_MATCHING;
    }
    else if (strstr((char *)GSM_DataBuffer, (char *)cGSM_eATResponseTbl[ATRESPONSE_CMEERROR]) != 0)
    {
        ucTemp = RESPONSE_CME_ERROR;
    }
    else if (strstr((char *)GSM_DataBuffer, (char *)cGSM_eATResponseTbl[ATRESPONSE_CMSERROR]) != 0)
    {
        ucTemp = RESPONSE_CMS_ERROR;
    }
    else if (strstr((char *)GSM_DataBuffer, (char *)cGSM_eATResponseTbl[ATRESPONSE_ERROR]) != 0)
    {
        ucTemp = RESPONSE_ERROR;
    }
    else
    {
        ucTemp = RESPONSE_NOT_MATCHING;
    }

    return ucTemp;
}

static bool bGSM_iCompareData_Exe(char *cData1Adr, char *cData2Adr, uint16_t uiLen)
{
    uint8_t ucVal1, ucVal2;
    uint16_t uiTemp;
    bool bStatus;

    bStatus = true;

    for (uiTemp = 0; (uiTemp < uiLen) && (bStatus == true); uiTemp++)
    {
        ucVal1 = cData1Adr[uiTemp];
        ucVal2 = cData2Adr[uiTemp];

        if (ucVal1 != ucVal2)
        {
            bStatus = false;
        }
        else
        {
            bStatus = true;
        }
    }

    return bStatus;
}

//-----------------------------------------------------------------------------

void vGSM_eStartGSM_Exe(void)
{
    uint8_t ucResp;

    ucResp = ucGSM_iCheckATResponse_Exe();
    if (ucResp == RESPONSE_MATCHING)
    {
        GSMStatus.ucGSMReadyStatus = GSM_WAS_READY;
    }
    else
    {
        vGSM_ePowerON_Exe();
        ucResp = ucGSM_iCheckATResponse_Exe();
        if (ucResp == RESPONSE_MATCHING)
        {
            GSMStatus.ucGSMReadyStatus = GSM_READY;
        }
        else
        {
            vGSM_eResetGSM_Exe();
            vGSM_ePowerON_Exe();

            ucResp = ucGSM_iCheckATResponse_Exe();
            if (ucResp == RESPONSE_MATCHING)
            {
                GSMStatus.ucGSMReadyStatus = GSM_READY;
            }
            else
            {
                GSMStatus.ucGSMReadyStatus = GSM_NOT_READY;
            }
        }
    }
    // Set SMS Storage : Fixed in Profile
    ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(ATCOMMAND_CPMS_ME, ATRESPONSE_CPMS, 28, ATRESPONSE_OK, 6, 200, 1000);

    // Set SMS in Text Mode : Fixed in Profile
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_CMGF, ATRESPONSE_OK, 200, 1000);

    // Set Result Code and use Verbose Values : Set in Profile
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_CMEE, ATRESPONSE_OK, 200, 1000);

    // Set Result code information response
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_ATV1, ATRESPONSE_OK, 200, 1000);

    // Set URC Indication
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QURCCFG, ATRESPONSE_OK, 200, 1000);

    // Set GSM as current character set : Set in Profile
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_CSCS, ATRESPONSE_OK, 200, 1000);

    // Check Message Service Centre No.
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_GETCSCA, ATRESPONSE_CSCA, 200, 1000);

    // Set SMS Parameters for Text Mode
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_CSMP, ATRESPONSE_OK, 200, 1000);

    // Show Header in Text Mode : Set in Profile
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_CSDH, ATRESPONSE_OK, 200, 1000);

    vGSM_iRegisterAndSetModule_Exe();
}

static uint8_t ucGSM_iCheckATResponse_Exe(void)
{
    uint8_t ucResp;

    ucResp = ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_ATE0, ATRESPONSE_OK, 300, 1000);
    if (ucResp != RESPONSE_MATCHING)
    {
        ucResp = ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_ATE0, ATRESPONSE_OK, 300, 1000);
        if (ucResp != RESPONSE_MATCHING)
        {
            ucResp = ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_ATE0, ATRESPONSE_OK, 300, 1000);
        }
        else
        {
        }
    }
    else
    {
    }

    return ucResp;
}

// ============================================================================
// Name		: vGSM_eResetGSM_Exe
// Objective	: Hardware Reset GSM Module by giving a Low pulse to RESET_N Pin
//				  for 150mS to 460mS.
//				  RESETTING TO BE DONE WHEN TURNING OFF THE MODULE BY AT+QPOWD COMMAND
//				  AND PWRKEY PIN FAILED.
// Input  		: none
// Output 	: none
// Return		: none
// Version	: -
// ============================================================================
void vGSM_eResetGSM_Exe(void)
{
    HAL_GPIO_WritePin(GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(460);
    HAL_GPIO_WritePin(GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_RESET);
    vAPP_eFeedTheWDT_Exe();
    HAL_Delay(2000);
}

// ============================================================================
// Name			: ucFTP_iProcessATCmdWithCmdNum_Exe
// Objective	:	1. Forward AT Commands to GSM Module whose pointer is inputed.
//					2. Wait for the Response up to Given Time out.
//					3. Compare Received Response with the Given Response
// Input		:	1. AT Command pointer
//					2. Desired Response Pointer
//					3. Time Out, if Character Received
//					4. Time Out, if No Character Received
// Output		: GSMHealth.ucCMEErrorCntr
// Return		: 	RESPONSE_MATCHING		- 0
//			  		RESPONSE_NOT_MATCHING	- 1
//			  		RESPONSE_ERROR			- 2
//			  		RESPONSE_CME_ERROR		- 3
//			  		RESPONSE_CMS_ERROR		- 4
//			  		NO_RESPONSE				- 5
// Version	: -
// Author	: AK
// Date		: 26-07-2018
// ============================================================================

uint8_t ucGSM_eProcessATCmdWithCmdNum_Exe(uint8_t ucATCmdNum, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    char *cATCmd;
    uint8_t ucResp;

    cATCmd = (char *)cGSM_eATCommandTbl[ucATCmdNum];
    ucResp = ucGSM_iProcessATCmd_Exe((char *)cATCmd, ucRespNum, ulTimeOutCharRecd, ulTimeOutNoCharRecd);
    return ucResp;
}

uint8_t ucGSM_eProcessATCmdWithCmdPtr_Exe(char *cATCmd, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    uint8_t ucResp;

    ucResp = ucGSM_iProcessATCmd_Exe(cATCmd, ucRespNum, ulTimeOutCharRecd, ulTimeOutNoCharRecd);
    return ucResp;
}

uint8_t ucGSM_iProcessATCmd_Exe(char *cATCmd, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    uint8_t ucTemp;
    bool bResp;

    GSM_RxCntr = 0;
    GSM_TimeOut = 0;

    ucTemp = NO_RESPONSE;
    vCOMM_eTxString_Exe(GSMComPortHandle, (char *)cATCmd, 200);
    HAL_UART_Receive_IT(GSMComPortHandle, (uint8_t *)GSM_RxOneByte, 1);

    bResp = bGSM_iWait4RespOrTimeOut_Exe(ulTimeOutCharRecd, ulTimeOutNoCharRecd);

    if (bResp == FALSE)
    {
        GSMData.ucNoResponseCntr = 0;
        ucTemp = ucGSM_eCompareStringInGSMBuff_Exe(ucRespNum);
        if ((ucTemp == RESPONSE_ERROR) || (ucTemp == RESPONSE_CME_ERROR) || (ucTemp == RESPONSE_CMS_ERROR))
        {
            GSMData.ucErrorCntr++;
        }
        else
        {
        }
    }
    else
    {
        if (GSM_RxCntr == 0)
        {
            GSMData.ucNoResponseCntr++;
        }
        else
        {
        }
    }
    return ucTemp;
}

// ============================================================================
// Name			: vGSM_eWait4RespOrTimeOut_Exe
// Objective	:	1. Wait for Response from GSM Module
//	    		 	2. Wait for Character Received Time out, if any character is received or
//	     		 	3. Wait for No Character Received Time Out
//
// Output 		: none
// Return		: 	0 - Character recd and its Timeout over
//		 			1 - No char recd, Full Timeout Over
// Author		: AK
// Date			: 04-12-2021
// ============================================================================
bool bGSM_iWait4RespOrTimeOut_Exe(uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    bool bTemp;

    bTemp = false;
    do
    {
        if (GSM_TimeOut > ulTimeOutNoCharRecd)
        {
            bTemp = true;
        }
        else
        {
        }
        vAPP_eFeedTheWDT_Exe();
    } while ((GSM_RxCntr <= 1) && (bTemp == false));

    if (bTemp == false)
    {
        do
        {
            vAPP_eFeedTheWDT_Exe();
        } while (GSM_TimeOut < ulTimeOutCharRecd);
    }
    else
    {
    }

    return bTemp;
}

// ============================================================================
// Name			: ucFTP_iProcessATCmdDualRespWithPtr_Exe
// Objective	:	1. Forward AT Commands to GSM Module whose pointer is inputed.
//			 		2. Wait for the 2 Responses up to Given Time out.
//			 		3. Compare both Received Response with the Given Response
// Input		:	1. AT Command pointer
//			 		2. Desired Response Pointer
//			 		3. Time Out, if Character Received
//			 		4. Time Out, if No Character Received
// Output		: GSMHealth.ucCMEErrorCntr
// Return		: 	RESPONSE_MATCHING		- 0
//			  		RESPONSE_NOT_MATCHING	- 1
//			  		RESPONSE_ERROR			- 2
//			  		RESPONSE_CME_ERROR		- 3
//			  		RESPONSE_CMS_ERROR		- 4
//			  		NO_RESPONSE				- 5
//			  		BOTH_RESPONSES_MATCHING	- 6
//			  		RESPONSE1_MATCHING		- 7
//			  		RESPONSE2_MATCHING		- 8
// Version		: -
// Author		: AK
// Date			: 26-07-2018
// ============================================================================
uint8_t ucGSM_eProcessATCmdDualRespWithPtr_Exe(char *cATCmd, uint8_t ucResp1Num, uint8_t ucLenResp1, uint8_t ucResp2Num, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    uint8_t ucResp;

    ucResp = ucGSM_iProcessATCmdDualResp_Exe((char *)cATCmd, ucResp1Num, ucLenResp1, ucResp2Num, ucLenResp2, ulTimeOutCharRecd, ulTimeOutNoCharRecd);
    return ucResp;
}

// ============================================================================
// Name			: bGSM_eWait4RespOrTimeOutDualResp_Exe
// Objective	: Wait for Response
// Input		:	1. Length of Response 1
//			 		2. Length of Response 2
//			 		3. Time Out, if Character Received
//			 		4. Time Out, if No Character Received
// Output		: None
// Return		: 	NO_CHAR_RECD_TIMEOUT		- 0
//			  		SOME_CHAR_RECD_TIMEOUT		- 2
//					BOTH_RESPONSES_OR_ERROR_RECD- 3
// Version		: -
// Author		: AK
// Date			: 26-07-2018
// ============================================================================
static uint8_t ucGSM_iWait4RespOrTimeOutDualResp_Exe(uint8_t ucLenResp1, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    bool bTemp;
    uint8_t ucResp;

    bTemp = false;
    ucResp = SOME_CHAR_RECD_TIMEOUT;

    do
    {
        if (GSM_TimeOut >= ulTimeOutNoCharRecd)
        {
            bTemp = true;
            ucResp = NO_CHAR_RECD_TIMEOUT;
        }
        else
        {
        }
#if WDT
        WDT_REFRESH();
#endif
    } while ((GSM_RxCntr <= 1) && (bTemp == false));

    if (bTemp == false)
    {
        // Some Char Recd
        do
        {
            if (GSM_RxCntr >= (ucLenResp1 + ucLenResp2))
            {
                bTemp = true;
                ucResp = BOTH_RESPONSES_OR_ERROR_RECD;

                do
                {
                } while (GSM_TimeOut < ulTimeOutCharRecd);
            }
            else
            {
            }
#if WDT
            WDT_REFRESH();
#endif
        } while ((GSM_TimeOut < ulTimeOutNoCharRecd) && (bTemp == false));
    }
    else
    {
        if ((GSM_TimeOut > ulTimeOutNoCharRecd) && (GSM_RxCntr > 1))
        {
            ucResp = SOME_CHAR_RECD_TIMEOUT;
        }
        else
        {
            ucResp = NO_CHAR_RECD_TIMEOUT;
        }
    }

    return ucResp;
}

static uint8_t ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(uint8_t ucATCmdNum, uint8_t ucResp1Num, uint8_t ucLenResp1, uint8_t ucResp2Num, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    uint8_t ucResp;
    char *cATCmd;

    cATCmd = (char *)cGSM_eATCommandTbl[ucATCmdNum];
    ucResp = ucGSM_iProcessATCmdDualResp_Exe((char *)cATCmd, ucResp1Num, ucLenResp1, ucResp2Num, ucLenResp2, ulTimeOutCharRecd, ulTimeOutNoCharRecd);
    return ucResp;
}

static uint8_t ucGSM_iProcessATCmdDualResp_Exe(char *cATCmd, uint8_t ucResp1Num, uint8_t ucLenResp1, uint8_t ucResp2Num, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd)
{
    uint8_t ucTemp, ucResp;
    char *cBuff1, *cBuff2;

    GSM_RxCntr = 0;
    GSM_TimeOut = 0;
    ucTemp = NO_RESPONSE;
    vCOMM_eTxString_Exe(GSMComPortHandle, (char *)cATCmd, 200);
    HAL_UART_Receive_IT(GSMComPortHandle, (uint8_t *)GSM_RxOneByte, 1);

    ucResp = ucGSM_iWait4RespOrTimeOutDualResp_Exe(ucLenResp1, ucLenResp2, ulTimeOutCharRecd, ulTimeOutNoCharRecd);
    if ((ucResp == BOTH_RESPONSES_OR_ERROR_RECD) || (ucResp == SOME_CHAR_RECD_TIMEOUT))
    {
        cBuff1 = strstr(GSM_DataBuffer, (char *)cGSM_eATResponseTbl[ucResp1Num]);
        cBuff2 = strstr(GSM_DataBuffer, (char *)cGSM_eATResponseTbl[ucResp2Num]);

        if ((cBuff1 != 0) && (cBuff2 != 0))
        {
            ucTemp = BOTH_RESPONSES_MATCHING;
        }
        else if (cBuff1 != 0)
        {
            ucTemp = RESPONSE1_MATCHING;
        }
        else if (cBuff2 != 0)
        {
            ucTemp = RESPONSE2_MATCHING;
        }
        else
        {
            if (strstr(GSM_DataBuffer, (char *)ATRESPONSE_CMEERROR) != 0)
            {
                ucTemp = RESPONSE_CME_ERROR;
            }
            else if (strstr(GSM_DataBuffer, (char *)ATRESPONSE_CMSERROR) != 0)
            {
                ucTemp = RESPONSE_ERROR;
            }
            else if (strstr(GSM_DataBuffer, (char *)ATRESPONSE_ERROR) != 0)
            {
                ucTemp = RESPONSE_ERROR;
            }
            else
            {
                ucTemp = RESPONSE_NOT_MATCHING;
            }
        }
    }
    else
    {
        ucTemp = RESPONSE_NOT_MATCHING;
    }
    return ucTemp;
}

static void vGSM_iRegisterAndSetModule_Exe(void)
{
    uint8_t ucResp, ucTemp;
    bool bStatus;

    vGSM_iReadSIMInsertStatus_Exe();

    if (GSMStatus.bSIMInsertStatus == SIM_INSERTED)
    {
        ucTemp = 0;
        do
        {
            ucResp = ucGSM_iReadSignalStrength_Exe(); // Read Signal Strength
            ucTemp++;
            HAL_Delay(1000);
            vAPP_eFeedTheWDT_Exe();

        } while (!(ucResp == GSM_SIGNAL_LEVEL_OK) && (ucTemp < 15));

        GSM_TimeOut = 0;
        do
        {
            bStatus = bGSM_iNetworkSetup_Exe();
            if (bStatus == false)
            {
                HAL_Delay(1000);
            }
            else
            {
            }
            vAPP_eFeedTheWDT_Exe();

        } while ((bStatus == false) && (GSM_TimeOut < 20000));

        bStatus = bGSM_iCheckAPNSetting_Exe();
        if (bStatus == false)
        {
            vGSM_iSetAPN_Exe();
        }
        else
        {
            ;
        }
    }
    else
    {
    }
}

// ============================================================================
// Name			: vGSM_eReadSIMInsertStatus_Exe
// Objective	: Read SIM Insert Status
// 		  		   Check SIM Inserted	: AT+CPIN?\r
// Input  		: none
// Output 		: SIM Status at GSMHealth.ucSIMInsertStatus
//				  SIM Not Present	0
//		  		  SIM Present		1
//
// Return		: // Version		: -
// Author		: AK
// Date			: 26-07-2018
// ============================================================================
static void vGSM_iReadSIMInsertStatus_Exe(void)
{
    uint8_t ucResp;

    // Read SIM PIN Status
    ucResp = ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(ATCOMMAND_CPIN, ATRESPONSE_CPINREADY, 14, ATRESPONSE_OK, 6, 300, 5000);

    if (ucResp == BOTH_RESPONSES_MATCHING)
    {
        GSMStatus.bSIMInsertStatus = SIM_INSERTED;
    }
    else
    {
        GSMStatus.bSIMInsertStatus = SIM_NOT_INSERTED;
    }
}

// ============================================================================
// Name			: ucGSM_eReadSignalStrength_Exe
// Objective	: Read GSM Signal Quality
//					0 -113 dBm or less
//					1 -111 dBm
//					2...30 -109... -53 dBm
//					31 -51 dBm or greater
//					99 		Not known or not detectable
//
// Input  		: none
// Output 		: GSM Signal Level at GSMDHealth.cSignalLevel
//					-99 if Invalid
//
// Return		:	GSM_SIGNAL_LEVEL_OK		0
//	    	  		GSM_SIGNAL_NOT_IN_RANGE	1
//		  			NO_GSM_SIGNAL			2
//		  			JAMMING_IN_GSM			3
//		  			GSM_NO_RESPONSE			4
//		  			GSM_SIGNAL_ERROR		5
// Version		: -
// Author		: AK
// Date			: 26-07-2018
// ============================================================================
static uint8_t ucGSM_iReadSignalStrength_Exe(void)
{
    uint8_t ucTemp, ucFlag;
    char *cBuffAdd1, *cBuffAdd2;

    ucFlag = NO_GSM_SIGNAL;
    ucTemp = ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(ATCOMMAND_CSQ, ATRESPONSE_CSQ, 10, ATRESPONSE_OK, 6, 300, 1000);

    if (ucTemp == BOTH_RESPONSES_MATCHING)
    {
        // Check for Strength Level
        cBuffAdd1 = strchr((char *)GSM_DataBuffer, ':');
        if (cBuffAdd1 != NULL)
        {
            cBuffAdd1++;
            cBuffAdd1++;
            cBuffAdd2 = strchr(cBuffAdd1, ',');
            if (cBuffAdd2 != NULL)
            {
                *cBuffAdd2 = '\0';
                GSMData.ucGSMSignalVal = atoi(cBuffAdd1);

                if (GSMData.ucGSMSignalVal == 99)
                {
                    ucFlag = NO_GSM_SIGNAL;
                }
                else if (GSMData.ucGSMSignalVal < GSM_SIGNAL_THRESHOLD_LOW)
                {
                    ucFlag = GSM_SIGNAL_LEVEL_LOW;
                }
                else
                {
                    ucFlag = GSM_SIGNAL_LEVEL_OK;
                }
                *cBuffAdd2 = ',';

                GSMStatus.ucGSMSignalStatus = ucFlag;
            }
        }
        else
        {
            GSMData.ucGSMSignalVal = 99;
            ucFlag = GSM_SIGNAL_ERROR;
        }
    }
    else if (ucTemp == NO_RESPONSE)
    {
        ucFlag = NO_RESPONSE;
    }
    else
    {
        ;
    }
    return ucFlag;
}

// ============================================================================
// Name		: ucGSM_eNetworkSetup_Exe
// Objective	: Make GSM Module registered on Network
// 		 	1. Check SIM Inserted				: AT+CPIN?\r
//		 	If SIM Present then Check followings :
//			2. Read Signal Quality				: AT+CSQ\r
//				If Signal Quality is Ok then Check followings :
// 			3. Get Network Registration Status	: AT+CREG?\r
// 			4. Get Network Registration Status	: AT+CGREG?\r
//			5. Read Network Registration 		: AT+COPS?\r
//			6. Read EPS Network Registration 	: AT+CEREG?\r
// Input  		: none
// Output 	:  GSMHealth.bGSMNetworkRegStatus and GSMHealth.bGPRSNetworkRegStatus
//				NETWORK_REGISTRATION_FAILED		0
//		  	 	NETWORK_REGISTRATION_SUCCESS	1
// Return		: NETWORK_REGISTRATION_FAILED	0
//		 	 NETWORK_REGISTRATION_SUCCESS	1
// Author		: AK
// Date		: 06-12-2021
// ============================================================================
static bool bGSM_iNetworkSetup_Exe(void)
{
    uint8_t ucRetry = 0;
    bool bFlag, bResp;

    bFlag = NETWORK_REGISTRATION_FAILED;

    do
    {
        bResp = bGSM_iReadNetworkRegStatus_Exe(GSM_NETWORK_REGISTRATION);
        if (bResp != NETWORK_REGISTRATION_SUCCESS)
        {
            HAL_Delay(1000);
        }
        else
        {
        }
        ucRetry++;
#if WDT
        WDT_REFRESH();
#endif
    } while ((bResp != NETWORK_REGISTRATION_SUCCESS) && (ucRetry < 90));

    if (bResp == NETWORK_REGISTRATION_SUCCESS)
    {
        GSMStatus.bCREGStatus = NETWORK_REGISTRATION_SUCCESS;

        ucRetry = 0;
        do
        {
            bResp = bGSM_iReadNetworkRegStatus_Exe(GPRS_NETWORK_REGISTRATION);
            if (bResp != NETWORK_REGISTRATION_SUCCESS)
            {
                HAL_Delay(1000);
            }
            else
            {
            }
            ucRetry++;
#if WDT
            WDT_REFRESH();
#endif
        } while ((bResp != NETWORK_REGISTRATION_SUCCESS) && (ucRetry < 60));

        if (bResp == NETWORK_REGISTRATION_SUCCESS)
        {
            GSMStatus.bCGREGStatus = NETWORK_REGISTRATION_SUCCESS;

            bFlag = NETWORK_REGISTRATION_SUCCESS;
            bResp = bGSM_iReadNetworkRegStatus_Exe(EPS_NETWORK_REGISTRATION);

            if (bResp == NETWORK_REGISTRATION_SUCCESS)
            {
                bFlag = NETWORK_REGISTRATION_SUCCESS;
            }
            else
            {
                ;
            }
        }
        else
        {
            GSMStatus.bCGREGStatus = NETWORK_REGISTRATION_FAILED;
        }
    }
    else
    {
        GSMStatus.bCREGStatus = NETWORK_REGISTRATION_FAILED;
    }
    return bFlag;
}

// ============================================================================
// Name		: bGSM_eReadNetworkRegStatus_Exe
// Objective	: Read Network Registration Status by :
//				AT+CREG?\r
//			Response :
//				+CREG:<n>,<stat>
//				OK
//			NETWORK_SEARCHING			0
//		  	NETWORK_REGISTERED_HOME	1
//		  	NETWORK_NOT_REGISTERED		2
//		  	NETWORK_REGISTRATION_DENIED	3
//		  	NETWORK_UNKNOWN			4
//		  	NETWORK_REGISTERED_ROAMING	5
//
// Input  		: Network Type :
//			1 - GSM Network Registration
//			2 - GPRS Network Registration
//			3 - EPS Network Registration
// Output 	:  none
// Return		: NETWORK_REGISTRATION_FAILED	0
//		  	 NETWORK_REGISTRATION_SUCCESS	1
// Version	: -
// Author		: AK
// Date		: 06-12-2021
// ============================================================================
static bool bGSM_iReadNetworkRegStatus_Exe(uint8_t ucNetworkStatusType)
{
    uint8_t ucResp, ucVal;
    char *cBuffAdd;
    bool bFlag;

    bFlag = NETWORK_REGISTRATION_FAILED;
    if (ucNetworkStatusType == GSM_NETWORK_REGISTRATION)
    {
        ucResp = ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(ATCOMMAND_GETCREG, ATRESPONSE_CREG, 10, ATRESPONSE_OK, 6, 300, 9000);
    }
    else if (ucNetworkStatusType == GPRS_NETWORK_REGISTRATION)
    {
        ucResp = ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(ATCOMMAND_GETCGREG, ATRESPONSE_CGREG, 10, ATRESPONSE_OK, 6, 300, 9000);
    }
    else if (ucNetworkStatusType == EPS_NETWORK_REGISTRATION)
    {
        ucResp = ucGSM_iProcessATCmdDualRespWithCmdNum_Exe(ATCOMMAND_GETCEREG, ATRESPONSE_CEREG, 10, ATRESPONSE_OK, 6, 300, 9000);
    }
    else
    {
        ;
    }

    if (ucResp == BOTH_RESPONSES_MATCHING)
    {
        cBuffAdd = strchr((char *)GSM_DataBuffer, ',');
        if (cBuffAdd != NULL)
        {
            cBuffAdd++;
            ucVal = *cBuffAdd;

            if ((ucVal == '1') || (ucVal == '5'))
            {
                bFlag = NETWORK_REGISTRATION_SUCCESS;
            }
            else
            {
                ;
            }
        }
        else
        {
            ;
        }
    }
    else
    {
        ;
    }
    return bFlag;
}

// ============================================================================
// Name		: bGSM_eCheckAPNSetting_Exe
// Objective	: Get APN
//			 Command:
//			 AT+CGDCONT?<CR>
//			Response, if Set :
//				+CGDCONT: 1,"IP","www","",0,0,,,,<CR><LF>
//				+CGDCONT: 8,"IPV4V6","IMS","254.128.0.0.0.0.0.0.0.0.0.89.102.239.246.1",0,0,0,2,1,1<CR><LF>
//				OK<CR><LF>
//
//			Response, if Not Set :
//			+CGDCONT: 1,"IP","www.mnc011.mcc404.gprs","10.126.160.82",0,0,,,,<CR><LF>
//			+CGDCONT: 8,"IPV4V6","IMS","254.128.0.0.0.0.0.0.0.0.0.89.102.239.246.1",0,0,0,2,1,1<CR><LF>
//			OK<CR><LF>
//
// Input  		: none
// Output 	: none
// Return		: 0 - Not Set
//			  1 - APN Set
// Version	: -
// Author		: AK
// Date		: 7-12-21
// ============================================================================
static bool bGSM_iCheckAPNSetting_Exe(void)
{
    uint8_t ucLen, ucResp;
    char *cBuff1;
    bool bStatus;
    char cBuff[50];

    // query
    sprintf(cBuff, "%s%d\r", cGSM_eATCommandTbl[ATCOMMAND_QICSGP], PDP_CONTEXT_ID);
    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_GETQICSGP, 21, ATRESPONSE_OK, 6, 300, 150000);
    if (ucResp == BOTH_RESPONSES_MATCHING)
    {
        sprintf(cBuff, "%s %d,\"%s\",", (char *)cGSM_eATResponseTbl[ATRESPONSE_GETQICSGP], PDP_CONTEXT_ID, cGSM_eATCommandTbl[ATCOMMAND_APNNAME]);
        cBuff1 = strchr(GSM_DataBuffer, '+');
        if (cBuff1 != 0)
        {
            ucLen = strlen(cBuff);
            bStatus = bGSM_iCompareData_Exe(cBuff, cBuff1, ucLen);
        }
        else
        {
            bStatus = false;
        }
    }
    else
    {
        bStatus = false;
    }
    GSMStatus.bAPNStatus = bStatus;
    return bStatus;
}

// ============================================================================
// Name		: vGSM_eSetAPN_Exe
// Objective	: Set APN
//			AT+CGDCONT=1,"IP","www"<CR>
//			OK<CR><LF>
// Input  		: none
// Output 	: none
// Return		: none
// Version	: -
// Author		: AK
// Date		: 7-12-21
// ============================================================================
static void vGSM_iSetAPN_Exe(void)
{
    uint8_t ucStatus;
    char cBuff[50];

    sprintf(cBuff, "%s%d,1,\"%s\",\"\",\"\",0\r", (char *)cGSM_eATCommandTbl[ATCOMMAND_QICSGP], PDP_CONTEXT_ID, cGSM_eATCommandTbl[ATCOMMAND_APNNAME]);
    ucStatus = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_GETQICSGP, 21, ATRESPONSE_OK, 6, 300, 150000);
    if (ucStatus == BOTH_RESPONSES_MATCHING)
    {
        GSMStatus.bAPNStatus = true;
    }
    else
    {
        GSMStatus.bAPNStatus = false;
    }
}

// ============================================================================
// Name		: bGSM_eSendSMS_Exe
// Objective	: Send SMS, use AT+CMGS for upto 160 Char.
//					AT+QCMGS for more than 160 Char
//					Command
//					 AT+CMGS="+919811451380"<CR><LF>
//					Response
//					 >
//					 Write Data with Control-Z
//					 +CMGS: 245
//					 OK
// Input  		: 1. Receiver Phone No.
//			  	Data Stored at cAPP_eGlobalBuffer without
//			  	- Write Control Z in this function
// Output 	: none
// Return		: Status
//	    			0 - FAIL
//	    			1 - SUCCESS
// Version	: -
// Author	: AK
// Date		: 12 Aug 2018
// ============================================================================
bool bGSM_eSendSMS_Exe(char *cReceiverMobile, char *cMessage)
{
    uint8_t ucStatus;
    char cBuff[180];
    bool bStatus;

    bStatus = false;
    sprintf(cBuff, "%s\"%s\"\r", cGSM_eATCommandTbl[ATCOMMAND_CMGS], cReceiverMobile);
    ucStatus = ucGSM_eProcessATCmdWithCmdPtr_Exe(cBuff, ATRESPONSE_SEND, 300, 1000);
    if (ucStatus == RESPONSE_MATCHING)
    {
        sprintf(cBuff, "%s%c", cMessage, CONTROL_Z);

        ucStatus = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_CMGS, 8, ATRESPONSE_SENDOK, 6, 300, 150000);
        if ((ucStatus == BOTH_RESPONSES_MATCHING) || (RESPONSE1_MATCHING))
        {
            bStatus = true;
        }
        else
        {
            bStatus = false;
        }
    }
    else
    {
        bStatus = false;
    }
    return bStatus;
}
