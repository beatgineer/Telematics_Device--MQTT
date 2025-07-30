#include "main.h"
#include "APP.h"
#include "EEPROM.h"
#include "stm32g0xx_hal.h"
#include <stdio.h>
#include <stm32g0xx_hal_rcc.h>
#include <stm32g0b1xx.h>

extern I2C_HandleTypeDef hi2c1;

static void vEEPROM_iMakeWPDisable_Exe(void);
static void vEEPROM_iMakeWPEnable_Exe(void);
void MX_I2C1_Init(void);

/*****************************************************************************************************************************************/

void vEEPROM_eInit(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   MX_I2C1_Init();

   /* Enable GPIO Clock */
   __HAL_RCC_GPIOB_CLK_ENABLE();

   /* Configure GPIO pin Output Level */
   HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);

   /* Configure GPIO Pin */
   GPIO_InitStruct.Pin = EEPROM_WP_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(EEPROM_WP_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
void MX_I2C1_Init(void)
{
   hi2c1.Instance = I2C1;
   hi2c1.Init.Timing = 0x10707DBC; // Corrected double semicolon
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

   /** Configure Analogue filter */
   if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
   {
      Error_Handler();
   }

   /** Configure Digital filter */
   if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
   {
      Error_Handler();
   }
}

void vEEPROM_eWriteByte_Exe(uint16_t uiMemAddress, uint8_t ucData)
{
   vEEPROM_iMakeWPDisable_Exe();
   HAL_Delay(2);

   if (HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, uiMemAddress, I2C_MEMADD_SIZE_16BIT, &ucData, 1, 1000) != HAL_OK)
   {
      Error_Handler();
   }

   HAL_Delay(5); // Write cycle delay (EEPROM typically needs ~5ms)
   vEEPROM_iMakeWPEnable_Exe();
}

void vEEPROM_eWriteBytes_Exe(uint16_t uiAddr, char *pBuffer, uint16_t uiBytesToWrite)
{
   uint16_t uiCntr;

   for (uiCntr = 0; uiCntr < uiBytesToWrite; uiCntr++)
   {
      vEEPROM_eWriteByte_Exe((uiAddr + uiCntr), *(pBuffer + uiCntr));
   }
}

uint8_t ucEEPROM_eReadByte_Exe(uint16_t uiMemAddress)
{
   uint8_t ucVal = 0;

   vEEPROM_iMakeWPEnable_Exe();

   if (HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, uiMemAddress, I2C_MEMADD_SIZE_16BIT, &ucVal, 1, 1000) != HAL_OK)
   {
      Error_Handler();
   }

   return ucVal;
}

void vEEPROM_eReadBytes_Exe(uint16_t uiMemAddress, uint8_t *ucData, uint16_t uiSize)
{
   vEEPROM_iMakeWPEnable_Exe();

   if (HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, uiMemAddress, I2C_MEMADD_SIZE_16BIT, ucData, uiSize, 1000) != HAL_OK)
   {
      Error_Handler();
   }
}

static void vEEPROM_iMakeWPDisable_Exe(void)
{
   HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
}

static void vEEPROM_iMakeWPEnable_Exe(void)
{
   HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);
}

#define EEPROM_TOTAL_BYTES 4096U
#define PAGE_SIZE          32U

void vEEPROM_Reset(void)
{
    char eraseBuffer[PAGE_SIZE];
    uint16_t addr;

    memset(eraseBuffer, 0xFF, PAGE_SIZE); // 0xFF is standard erased value

    for (addr = 0; addr < EEPROM_TOTAL_BYTES; addr += PAGE_SIZE)
    {
        // Ensure last page doesn't exceed memory size
        uint16_t bytesToWrite = PAGE_SIZE;
        if ((EEPROM_TOTAL_BYTES - addr) < PAGE_SIZE)
            bytesToWrite = EEPROM_TOTAL_BYTES - addr;

        vEEPROM_eWriteBytes_Exe(addr, eraseBuffer, bytesToWrite);
        // Wait 5ms for EEPROM write cycle (optional but recommended)
        // vDelay_ms(5);
    }
}


/*
END OF FILE
*/
