// /* USER CODE BEGIN Header */
// /**
//   ******************************************************************************
//   * @file           : main.h
//   * @brief          : Header for main.c file.
//   *                   This file contains the common defines of the application.
//   ******************************************************************************
//   * @attention
//   *
//   * Copyright (c) 2025 STMicroelectronics.
//   * All rights reserved.
//   *
//   * This software is licensed under terms that can be found in the LICENSE file
//   * in the root directory of this software component.
//   * If no LICENSE file comes with this software, it is provided AS-IS.
//   *
//   ******************************************************************************
//   */
// /* USER CODE END Header */

// /* Define to prevent recursive inclusion -------------------------------------*/
// #ifndef __MAIN_H
// #define __MAIN_H

// #ifdef __cplusplus
// extern "C" {
// #endif

// /* Includes ------------------------------------------------------------------*/
// #include "stm32g0xx_hal.h"

// /* Private includes ----------------------------------------------------------*/
// /* USER CODE BEGIN Includes */

// /* USER CODE END Includes */

// /* Exported types ------------------------------------------------------------*/
// /* USER CODE BEGIN ET */

// /* USER CODE END ET */

// /* Exported constants --------------------------------------------------------*/
// /* USER CODE BEGIN EC */

// /* USER CODE END EC */

// /* Exported macro ------------------------------------------------------------*/
// /* USER CODE BEGIN EM */

// /* USER CODE END EM */

// void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

// /* Exported functions prototypes ---------------------------------------------*/
// void Error_Handler(void);

// /* USER CODE BEGIN EFP */

// /* USER CODE END EFP */

// /* Private defines -----------------------------------------------------------*/
// #define USART3TX_GSMRX_Pin GPIO_PIN_9
// #define USART3TX_GSMRX_GPIO_Port GPIOB
// #define CAN_STDBY_Pin GPIO_PIN_14
// #define CAN_STDBY_GPIO_Port GPIOC
// #define GSM_PWRKEY_Pin GPIO_PIN_0
// #define GSM_PWRKEY_GPIO_Port GPIOA
// #define GSM_RESET_Pin GPIO_PIN_1
// #define GSM_RESET_GPIO_Port GPIOA
// #define GSM_PWR_DRV_Pin GPIO_PIN_3
// #define GSM_PWR_DRV_GPIO_Port GPIOA
// #define USART1TX_DEBUGRX_Pin GPIO_PIN_9
// #define USART1TX_DEBUGRX_GPIO_Port GPIOA
// #define USART1RX_DEBUGTX_Pin GPIO_PIN_10
// #define USART1RX_DEBUGTX_GPIO_Port GPIOA
// #define FDCAN1_RX_Pin GPIO_PIN_11
// #define FDCAN1_RX_GPIO_Port GPIOA
// #define FDCAN1_TX_Pin GPIO_PIN_12
// #define FDCAN1_TX_GPIO_Port GPIOA
// #define EEPROM_WP_Pin GPIO_PIN_5
// #define EEPROM_WP_GPIO_Port GPIOB
// #define IDO_I2C1_SCL_Pin GPIO_PIN_6
// #define IDO_I2C1_SCL_GPIO_Port GPIOB
// #define IDI_I2C1_SDA_Pin GPIO_PIN_7
// #define IDI_I2C1_SDA_GPIO_Port GPIOB
// #define USART3TX_GSMRXB8_Pin GPIO_PIN_8
// #define USART3TX_GSMRXB8_GPIO_Port GPIOB

// /* USER CODE BEGIN Private defines */

// /* USER CODE END Private defines */

// #ifdef __cplusplus
// }
// #endif

// #endif /* __MAIN_H */
