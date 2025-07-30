/**
  ******************************************************************************
  * @file          : APP.c
  * @brief         : Application Layer
  ******************************************************************************
  // APP
  // Version  : 1301.00,
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

 */
/* Includes ------------------------------------------------------------------*/
// #include "main.h"
#include "MCUFLASH.h"
#include "APP.h"
#include "FTP.h"
#include "EEPROM.h"
#include "GSM.h"
#include "CAN.h"
#include "COMM.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32g0xx_hal.h"
#include "stm32g0xx_hal_rcc.h"
#include <stdbool.h>
#include <string.h>
#include "setjmp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

FDCAN_HandleTypeDef hfdcan1;

I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
// Private variables ---------------------------------------------------------
uint8_t ucAPP_iRunningCntr, ucAPP_eNumOfPages, ucAPP_eNumOfPagesRecd;
volatile uint8_t ucAPP_eVerUpdate, ucAPP_eVerUpdateSMSCmd, ucAPP_iResetStatus;
uint32_t ulAPP_iResetVal = 0, ulAPP_iMCUFlashAddr, ulAPP_eBINFileSize = 0, ulAPP_eCRCVal;
uint16_t uiAPP_eCntr, uiAPP_eRemainingBytes, uiAPP_eTimer2S;

uint8_t ucAPP_eGlobalBuffer[2050];

const uint8_t HexTable[6] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
// static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_IWDG_Init(void);
void vAPP_eFeedTheWDT_Exe(void);
static void MX_TIM2_Init(void);
static void MX_TIM16_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */

// Global variables ------------------------------------------------------------
jmp_buf jmpBuffer;

typedef void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t ulJumpAddress;

TsAPPRST ResetFlag;
extern TsFTPData FTPData;
extern TsGSMData GSMData;
extern TsGSMStatus GSMStatus;

#pragma pack(push, 1)
const TsDefaultConfig DEFAULTCONFIG = {
    "13.232.191.212", // FTP Server Host Name
    "21",             // FTP Server Port No.
    "ecudown",
    "haD_8A@usw",
};
#pragma pack(pop)

char msg[128]; // Debug message buffer

int main(void)
{
  uint32_t ulFlashDestination;
  bool bResp;
  uint8_t ucResp;
  char cBuff[5];

  HAL_Init();
  SystemClock_Config();

  if (setjmp(jmpBuffer) == 1)
  {
    HAL_NVIC_SystemReset();
  }

  MX_CRC_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_IWDG_Init();
  MX_TIM2_Init();
  MX_TIM16_Init();
  MX_USART1_UART_Init();
  vEEPROM_eInit();
  vGSM_eInit();

  vEEPROM_eReadBytes_Exe(EEPROM_ADDR_FW_UPDATE_STATUS, (char *)ucAPP_eGlobalBuffer, 256);
  FTPData.ucCurrentAPPNum = ullMCUFLASH_eReadCurrentActiveAPP_Exe();
  ucAPP_eVerUpdate = ucEEPROM_eReadByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS);

  sprintf(msg, "UpdateFlag: %d\n", ucAPP_eVerUpdate);
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 1000);

  vEEPROM_eWriteByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS, NO_IOT_VERSION_CHANGE);

  if (ucAPP_eVerUpdate == SMS_IOT_FW_VERSION)
  {
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_IOT_SMS_VER_NUM, FTPData.cNewVersion, 5);
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_SMS_SENDER_NUM, GSMData.cSenderMobileNum, 25);
  }

  if ((ucAPP_eVerUpdate == CHK_VERSION_UPDATE) || (ucAPP_eVerUpdate == SMS_IOT_FW_VERSION))
  {
    vGSM_eStartGSM_Exe();
    HAL_UART_Transmit(&huart1, "GSM Started\n", strlen("GSM Started\n"), 1000);

    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_IOT_VER_NUM, FTPData.cCurrentVersion, 5);
    bResp = bFTP_eConnectAndDownloadFiles_Exe();

    if (bResp == true)
    {
      bResp = bFTP_eWriteNewHexFileInMCU_Exe(FTPData.ucNewAPPNum, FTPData.cBINFileName);

      if (bResp == true)
      {
        uiFTP_eDeleteGSMUFSFile_Exe(FTPData.cBINFileName);
        ucMCUFLASH_eWriteCONFIGStatusInMCUFlash_Exe(FTPData.ucNewAPPNum);

        vEEPROM_eWriteByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS, IOT_FW_UPDATED);
        FTPData.ucCurrentAPPNum = FTPData.ucNewAPPNum;

        vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IOT_VER_NUM, FTPData.cNewVersion, 5);

        if (ucAPP_eVerUpdate == SMS_IOT_FW_VERSION)
        {
          memset(cBuff, 0, sizeof(cBuff));
          vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IOT_SMS_VER_NUM, cBuff, 5);

          bGSM_eSendSMS_Exe(GSMData.cSenderMobileNum, (char *)cGSM_eATResponseTbl[ATRESPONSE_FWOK]);
        }
      }
      else
      {
        // HAL_UART_Transmit(&huart1, (uint8_t *)"FTP WRITE FAIL\n", 16, 1000);
        vEEPROM_eWriteByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS, NO_IOT_VERSION_CHANGE);
      }
    }
    else
    {
      HAL_UART_Transmit(&huart1, (uint8_t *)"FTP CONNECT FAIL\n", 18, 1000);
      vEEPROM_eWriteByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS, NO_IOT_VERSION_CHANGE);
    }
  }

  while (1)
  {
    // HAL_UART_Transmit(&huart1, (uint8_t *)"BOOTLOADER.........\n", strlen("BOOTLOADER.........\n"), 1000);
    switch (FTPData.ucCurrentAPPNum)
    {
    case VALID_APP1:
      ulFlashDestination = IMAGE1_MCU_FLASH_ADDR;
      break;

    case VALID_APP2:
      ulFlashDestination = IMAGE2_MCU_FLASH_ADDR;
      break;

    default:
      ucMCUFLASH_eWriteCONFIGStatusInMCUFlash_Exe(1);
      FTPData.ucCurrentAPPNum = JUMP_TO_PROGRAM_IMAGE1;

      vEEPROM_eWriteByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS, NO_FWUPDATE);
      ucAPP_eVerUpdate = CHK_VERSION_UPDATE;

      ulFlashDestination = IMAGE1_MCU_FLASH_ADDR;
    }
    vAPP_eJumpToAppAddr_Exe(ulFlashDestination);
  }
}

void vAPP_eJumpToAppAddr_Exe(uint32_t ulAppAddr)
{
  if (((*(__IO uint32_t *)ulAppAddr) & 0x2FFE0000) == 0x20000000)
  {
    ulJumpAddress = *(__IO uint32_t *)(ulAppAddr + 4); // Get Address
    JumpToApplication = (pFunction)ulJumpAddress;      // Jump to application

    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t *)ulAppAddr);
    JumpToApplication();
  }
  else
  {
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
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
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

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

  /* USER CODE END CRC_Init 2 */
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
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10B17DB5;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
   */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
   */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */
}

void vAPP_eFeedTheWDT_Exe(void)
{
  HAL_IWDG_Refresh(&hiwdg);
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
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
  sConfigOC.Pulse = 0;
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

/**
 * @brief TIM16 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 64000 - 1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 500 - 1;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

// ****************************************************************************
// NAME   : ucUTL_eASCIIChar2HexChar_Exe
// ROLE   : Convert ASCII Char to Hex char.
//
// INPUT  : ASCII Char
// OUTPUT : None
//
// GLOBAL VARIABLE USED : None
//
// FUNCTION USED : None
//
// HIST   :
// ----------------------------------------------------------------------------
// |Version | Date  | Author |                 Description                    |
// ----------------------------------------------------------------------------
// | 01.00  |080711 | Arvind | Creation of Header                             |
// ****************************************************************************
uint8_t ucAPP_eASCIIChar2HexChar_Exe(uint8_t ucVal)
{
  uint8_t ucTemp;

  if ((ucVal - '0') > 0x10)
  {
    if ((ucVal - 'A') > 0x10)
    {
      ucTemp = HexTable[(ucVal - 'a')];
    }
    else
    {
      ucTemp = HexTable[(ucVal - 'A')];
    }
  }
  else
  {
    ucTemp = ucVal - '0';
  }

  return (ucTemp);
}

// /**
//   * @brief GPIO Initialization Function
//   * @param None
//   * @retval None
//   */
// static void MX_GPIO_Init(void)
// {
//   GPIO_InitTypeDef GPIO_InitStruct = {0};
//   /* USER CODE BEGIN MX_GPIO_Init_1 */

//   /* USER CODE END MX_GPIO_Init_1 */

//   /* GPIO Ports Clock Enable */
//   __HAL_RCC_GPIOB_CLK_ENABLE();
//   __HAL_RCC_GPIOC_CLK_ENABLE();
//   __HAL_RCC_GPIOA_CLK_ENABLE();

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(CAN_STDBY_GPIO_Port, CAN_STDBY_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(GPIOA, GSM_PWRKEY_Pin|GSM_RESET_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin Output Level */
//   HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);

//   /*Configure GPIO pin : CAN_STDBY_Pin */
//   GPIO_InitStruct.Pin = CAN_STDBY_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(CAN_STDBY_GPIO_Port, &GPIO_InitStruct);

//   /*Configure GPIO pins : GSM_PWRKEY_Pin GSM_RESET_Pin */
//   GPIO_InitStruct.Pin = GSM_PWRKEY_Pin|GSM_RESET_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//   /*Configure GPIO pin : EEPROM_WP_Pin */
//   GPIO_InitStruct.Pin = EEPROM_WP_Pin;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//   HAL_GPIO_Init(EEPROM_WP_GPIO_Port, &GPIO_InitStruct);

//   /* USER CODE BEGIN MX_GPIO_Init_2 */

//   /* USER CODE END MX_GPIO_Init_2 */
// }

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
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
