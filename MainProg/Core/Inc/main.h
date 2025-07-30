// /* USER CODE BEGIN Header */
// ///**
// //  ******************************************************************************
// //  * @file           : main.h
// //  * @brief          : Header for main.c file.
// //  *                   This file contains the common defines of the application.
// //  ******************************************************************************
// //  * @attention
// //  *
// //  * Copyright (c) 2025 STMicroelectronics.
// //  * All rights reserved.
// //  *
// //  * This software is licensed under terms that can be found in the LICENSE file
// //  * in the root directory of this software component.
// //  * If no LICENSE file comes with this software, it is provided AS-IS.
// //  *
// //  ******************************************************************************
// //  */
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
// //
// /* USER CODE END Includes */

// /* Exported types ------------------------------------------------------------*/
// /* USER CODE BEGIN ET */
// //
// /* USER CODE END ET */

// /* Exported constants --------------------------------------------------------*/
// /* USER CODE BEGIN EC */
// //
// /* USER CODE END EC */

// /* Exported macro ------------------------------------------------------------*/
// /* USER CODE BEGIN EM */
// //
// /* USER CODE END EM */

// void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

// /* Exported functions prototypes ---------------------------------------------*/
// void Error_Handler(void);

// /* USER CODE BEGIN EFP */
// //
// /* USER CODE END EFP */

// /* Private defines -----------------------------------------------------------*/
// #define GSMTX_USART3RX_Pin GPIO_PIN_9
// #define GSMTX_USART3RX_GPIO_Port GPIOB
// #define CAN_STDBY_Pin GPIO_PIN_14
// #define CAN_STDBY_GPIO_Port GPIOC
// #define GSM_PWRKEY_Pin GPIO_PIN_0
// #define GSM_PWRKEY_GPIO_Port GPIOA
// #define GSM_RESET_Pin GPIO_PIN_1
// #define GSM_RESET_GPIO_Port GPIOA
// #define DSCH_CKT_CONTROL_Pin GPIO_PIN_2
// #define DSCH_CKT_CONTROL_GPIO_Port GPIOA
// #define GSM_SUPPLY_CONTROL_Pin GPIO_PIN_3
// #define GSM_SUPPLY_CONTROL_GPIO_Port GPIOA
// #define GPS_STATUS_LED_Pin GPIO_PIN_5
// #define GPS_STATUS_LED_GPIO_Port GPIOA
// #define MAINBATT_ADCIN6_Pin GPIO_PIN_6
// #define MAINBATT_ADCIN6_GPIO_Port GPIOA
// #define INTBATT_ADC1IN7_Pin GPIO_PIN_7
// #define INTBATT_ADC1IN7_GPIO_Port GPIOA
// #define INTBATT_CHGSTATUS_ADCIN8_Pin GPIO_PIN_0
// #define INTBATT_CHGSTATUS_ADCIN8_GPIO_Port GPIOB
// #define GSM_RI_IN_Pin GPIO_PIN_2
// #define GSM_RI_IN_GPIO_Port GPIOB
// #define GSM_RI_IN_EXTI_IRQn EXTI2_3_IRQn
// #define Reserved_Pin GPIO_PIN_8
// #define Reserved_GPIO_Port GPIOA
// #define USART1TX_DEBUGRX_Pin GPIO_PIN_9
// #define USART1TX_DEBUGRX_GPIO_Port GPIOA
// #define USART1RX_DEBUGTX_Pin GPIO_PIN_10
// #define USART1RX_DEBUGTX_GPIO_Port GPIOA
// #define FDCAN1_RX_Pin GPIO_PIN_11
// #define FDCAN1_RX_GPIO_Port GPIOA
// #define FDCAN1_TX_Pin GPIO_PIN_12
// #define FDCAN1_TX_GPIO_Port GPIOA
// #define MCU_LED_Pin GPIO_PIN_15
// #define MCU_LED_GPIO_Port GPIOA
// #define INTBATT_CHG_DRV_Pin GPIO_PIN_3
// #define INTBATT_CHG_DRV_GPIO_Port GPIOB
// #define IGN_IN_Pin GPIO_PIN_4
// #define IGN_IN_GPIO_Port GPIOB
// #define IGN_IN_EXTI_IRQn EXTI4_15_IRQn
// #define EEPROM_WP_Pin GPIO_PIN_5
// #define EEPROM_WP_GPIO_Port GPIOB
// #define IDO_I2C1_SCL_Pin GPIO_PIN_6
// #define IDO_I2C1_SCL_GPIO_Port GPIOB
// #define IDI_I2C1_SDA_Pin GPIO_PIN_7
// #define IDI_I2C1_SDA_GPIO_Port GPIOB
// #define GSMRX_USART3TX_Pin GPIO_PIN_8
// #define GSMRX_USART3TX_GPIO_Port GPIOB

// /* USER CODE BEGIN Private defines */
// //
// /* USER CODE END Private defines */

// #ifdef __cplusplus
// }
// #endif

// #endif /* __MAIN_H */
