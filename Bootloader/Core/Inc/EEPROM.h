#ifndef __E2PROM_I2C_H
#define __E2PROM_I2C_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "stm32g0xx_hal.h"

// EEPROM ADDRESS (8bits)
#define EEPROM_ADDR 0xA0
#define EEPROM_I2C &hi2c1

// E2PROM 24C032 - 32 kilobits = 32,768 bits = 4,096 Bytes (32bytes page size X 128 Page Num)
#define PAGE_SIZE 32 // in Bytes
#define PAGE_NUM 128 // number of pages

#define EEPROM_WP_Pin GPIO_PIN_5
#define EEPROM_WP_GPIO_Port GPIOB

#define IDO_I2C1_SCL_Pin GPIO_PIN_6
#define IDO_I2C1_SCL_GPIO_Port GPIOB

#define IDI_I2C1_SDA_Pin GPIO_PIN_7
#define IDI_I2C1_SDA_GPIO_Port GPIOB

    void vEEPROM_eInit(void);
    void vEEPROM_eWriteBytes_Exe(uint16_t uiAddr, char *pBuffer, uint16_t uiBytesToWrite);
    void vEEPROM_eWriteByte_Exe(uint16_t uiMemAddress, uint8_t ucData);
    void vEEPROM_eReadBytes_Exe(uint16_t uiMemAddress, uint8_t *ucData, uint16_t uiSize);
    uint8_t ucEEPROM_eReadByte_Exe(uint16_t uiMemAddress);
    void MX_I2C1_Init(void);
    void vEEPROM_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __E2PROM_H */
