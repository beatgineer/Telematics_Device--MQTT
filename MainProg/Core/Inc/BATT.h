// Define to prevent recursive inclusion -------------------------------------
#ifndef __BATT_H
#define __BATT_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32g0xx_hal.h"
#include <stdbool.h>

#define TOTAL_ADC_CHANNEL 3

extern ADC_HandleTypeDef hadc;
extern uint8_t ucBATTFlag;

#define ADC_CFAC_MAINBATTVOL (float)1.129     //0.02497            //0.03304     Date:10-07-2025
#define ADC_CFAC_INTBATTVOL (float)1.129	// Date:10-07-2025
#define ADC_CFAC_INTBATTCHGSTATUS (float)1.129	// Date:10-07-2025

#define INT_BATT_PRESENT_THRESHOLD_MV 1100 // Safe threshold ~1.0V
#define MAIN_BATT_PRESENT_THRESHOLD_MV 1100 // Safe threshold ~1.0V

#define MAINBATT_ADCIN6_Pin 			GPIO_PIN_6
#define MAINBATT_ADCIN6_GPIO_Port 		GPIOA

#define INTBATT_ADC1IN7_Pin 			GPIO_PIN_7
#define INTBATT_ADC1IN7_GPIO_Port 		GPIOA

#define INTBATT_CHG_DRV_Pin 			GPIO_PIN_3
#define INTBATT_CHG_DRV_GPIO_Port 		GPIOB

#define INTBATT_CHGSTATUS_ADCIN8_Pin 		GPIO_PIN_0
#define INTBATT_CHGSTATUS_ADCIN8_GPIO_Port 	GPIOB

typedef enum
{
    RAW_MAIN_BATT_ADC_VAL = 0,
    RAW_INT_BATT_ADC_VAL = 1,
    RAW_INT_BATT_CHG_VAL = 2,
} Enum_ADCVAL_Mode;

typedef enum
{
	MCB_OFF = 0,
	MCB_ON,

	NO_BATTERY_VOLTAGE = 0,
	BATTERY_VOLTAGE_OK,
		
	NO_BATT_RS485_RESPONSE = 0,
	BATT_RS485_RESPONSE_OK,
	
	BATTERY_NOT_PRESENT = 0,
	BATTERY_PRESENT,
} Enum_BATTDataType;

typedef struct
{
	uint32_t	ulRawADCVal[TOTAL_ADC_CHANNEL];

	volatile uint16_t	uMainBATTVolt;
	volatile uint16_t	uiIntBATTVolt;
} TsBATTData; 

typedef union
{
	struct
	{
		bool		bMainBATTVoltStatus			: 1;
		bool 		bIntBATTVoltStatus 			: 1;
		bool 		bBATTVoltStatus 				: 1;
		bool		bBATTVoltStatusPrev		: 1;
		bool		bBATTStatus				: 1;
		bool		bMCBStatus				: 1;
		bool		bIntBattMOSFETChgStatus	: 1;		// 0 - Chg Off, 1 - Chg ON
		uint8_t		ucSpare					: 3;
	};
	uint8_t	ucData[1];
}TsBATTStatus;

extern void _Error_Handler(char *, int);
void vBATT_eInit(void);
// void vBATT_eReadMainBATTVolt_Exe(void);
void vBATT_Start_ADC(void);
void vBATT_eDetectBatteryPresence(void);
// void vBATT_eReadIntBATTChgStatus_Exe(void);
// uint8_t ucBATT_eReadBATTPresence_Exe(void);
// void vBATT_eReadVehicleStatus_Exe(void);

#ifdef __cplusplus
}
#endif
#endif

/***** END OF FILE ****/
