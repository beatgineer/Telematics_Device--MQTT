#ifndef __CAN_H
#define __CAN_H

#ifdef __cplusplus
extern "C"
{
#endif

// #include "CONFIG_EEPROM.H"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// FUNCTION PROTOTYPE
// ============================================================================
void vCAN_eInit(void);
void vCAN_eReadCANData_Exe(void);
void vCAN_eSendCANCommands_Exe(uint32_t CAN_ID);
void vCAN_eChargerStatus_Exe(void);
void vCAN_eGetBattID_Exe(void);
void vCAN_eUpdateBattSOC_Exe(void);
void vCAN_eUpdateBattTemp_Exe(void);
void vCAN_eReadErrCode_Exe(void);
void vCAN_eChargerStatus_Exe(void);

// ============================================================================
// PIN DEFINITION
// ============================================================================
#define FDCAN1_RX_Pin GPIO_PIN_11
#define FDCAN1_RX_GPIO_Port GPIOA
#define FDCAN1_TX_Pin GPIO_PIN_12
#define FDCAN1_TX_GPIO_Port GPIOA
// ============================================================================
#define TOTAL_CAN_FRAMES 7U
#define TOTAL_PROTECTION_VALUES 20U
#define CAN_TX_FRAMES 2U
#define CFAC_RPM_SPEED_RV400 33U // RV400
#define BATT_OVER_TEMP 60
#define BATT_THERMAL_RUNAWAY_TEMP 65
#define LEN_BATTID 14U
// ============================================================================
// CAN MSG-IDs
// ============================================================================
// #define CANID_IMMOM_FW_UPDATE 0x300
// #define CANID_CLUSTER_TO_IMMOB 0x409
#define CANID_IoT_TO_MOTORCONTROLLER 0x400
// ============================================================================
// BATTERY ERROR CODE
// ============================================================================
#define CELL_OVER_VOLTAGE_ALARM 0x100U
#define CELL_OVER_VOLTAGE_PROTECTION 0x101U

#define CELL_UNDER_VOLTAGE_ALARM 0x102U
#define CELL_UNDER_VOLTAGE_PROTECTION 0x103U

#define PACK_OVER_VOLTAGE_ALARM 0x104U
#define PACK_OVER_VOLTAGE_PROTECTION 0x105U

#define PACK_UNDER_VOLTAGE_ALARM 0x106U
#define PACK_UNDER_VOLTAGE_PROTECTION 0x107U

#define DISCHARGE_OVER_TEMPERATURE_ALARM 0x108U
#define DISCHARGE_OVER_TEMPERATURE_PROTECTION 0x109U

#define DISCHARGE_LOW_TEMPERATURE_ALARM 0x110U
#define DISCHARGE_LOW_TEMPERATURE_PROTECTION 0x111U

#define CHARGE_OVER_TEMPERATURE_ALARM 0x112U
#define CHARGE_OVER_TEMPERATURE_PROTECTION 0x113U

#define CHARGE_LOW_TEMPERATURE_ALARM 0x114U
#define CHARGE_LOW_TEMPERATURE_PROTECTION 0x115U

#define HIGH_DIFF_CELL_TEMP_ALARM 0x116U
#define HIGH_DIFF_CELL_TEMP_PROTECTION 0x117U

#define CHARGING_OVER_CURRENT_ALARM 0x118U
#define CHARGING_OVER_CURRENT_PROTECTION 0x119U

#define REGEN_OVER_CURRENT_ALARM 0x120U
#define REGEN_OVER_CURRENT_PROTECTION 0x121U

#define DISCHARGE_OVER_CURRENT_ALARM 0x122U
#define DISCHARGE_OVER_CURRENT_PROTECTION 0x123U

#define DISCHARGE_OVER_PLUS_CURRENT_ALARM 0x124U
#define DISCHARGE_OVER_PLUS_CURRENT_PROTECTION 0x125U

#define LOW_INSULATION_ALARM 0x126U
#define LOW_INSULATION_PROTECTION 0x127U

#define LOW_SOC_ALARM 0x128U
#define LOW_SOC_PROTECTION 0x129U

#define BATT_ID_CONFLICT_ALARM 0x130U
#define BATT_ID_CONFLICT_PROTECTION 0x131U
// ============================================================================

#define AFE_COMM_ERR 0x132U
#define CELL_VOLTAGE_ACQ_ERR 0x133U
#define TEMP_ACQ_ERR 0x134U
#define BALANCE_RESISTOR_OVER_TEMP_ERR 0x135U
#define MOSFET_OVER_TEMP_ERR 0x136U
#define PRE_DISCHARGE_MOSFET_OVER_TEMP_ERR 0x137U
#define PRE_DISCHARGE_TIMEOUT_ERR 0x138U
#define PRE_DISCHARGE_OVER_CURRENT_ERR 0x139U
#define AFE_OVER_VOLTAGE_ERR 0x140U
#define AFE_UNDER_VOLTAGE_ERR 0x141U
#define AFE_DISCHARGE_OVER_CURRENT_ERR 0x142U
#define AFE_CHARGE_OVER_CURRENT_ERR 0x143U
#define AFE_SHORT_CIRCUIT_ERR 0x144U
#define AFE_CHARGING_OVER_TEMP_ERR 0x145U
#define AFE_CHARGING_LOW_TEMP_ERR 0x146U
#define AFE_DISCHARGE_OVER_TEMP_ERR 0x147U
#define AFE_DISCHARGE_LOW_TEMP_ERR 0x148U
#define FULLY_CHARGE_ERR 0x149U
#define ZERO_VOLT_PROTECTION_ERR 0x150U
#define PRE_DISCHARGE_MOSFET_FAIL_ERR 0x151U
#define DISCHARGE_MOSFET_FAIL_ERR 0x152U
#define CHARGE_MOSFET_FAIL_ERR 0x153U
#define PRE_DISHARGE_MOSFET_DRIVER_FAIL_ERR 0x154U
#define DISCHARGE_MOSFET_DRIVE_FAIL_ERR 0x155U
#define CHARGE_MOSFET_DRIVE_FAIL_ERR 0x156U
#define EOL_ERR 0x157U
#define SEVERE_OVER_CELL_VOLTAGE_ERR 0x158U
#define CHARGE_PROTECTION_STATE_ERR 0x159U
#define DISCHARGE_PROTECTION_STATE_ERR 0x160U
#define CONT_MOSFET_FAIL_ERR 0x161U
#define CONT_DRIVER_FAIL_ERR 0x162U
#define CONT_OVER_CURRENT_ERR 0x163U
#define CONT_OVER_VOLTAGE_ERR 0x164U
#define CONT_OVER_TEMPERATURE_ERR 0x165U
#define CONT_UNDER_VOLTAGE_ERR 0x166U
#define CONT_MOTOR_PHASE_ERR 0x167U
#define CONT_MOTOR_HALL_ERR 0x168U
#define CONT_MOTOR_OVER_TEMPERATURE_ERR 0x169U
#define CONT_MOTOR_STALL_ERR 0x170U
#define CHARGER_NOT_CHARGING_ERR 0x171U
#define BATT_OVER_TEMPERATURE_ERR 0x172U
#define BATT_THERMAL_RUNAWAY_TEMPERATURE_ERR 0x173U
#define IMMOB_DC_DC_MOSFET_ERR 0x174U
#define IMMOB_CONTROLLER_MOSFET_ERR 0x175U
#define IMMOB_INDICATOR_MOSFET_ERR 0x176U
#define IMMOB_VEHICLE_TRYING_TO_START_BUT_NOT_ALLOWED 0x177U
#define IMMOB_IMMOBILIZATION_ERR 0x178U
#define CONT_THROTTLE_ERR 0x184U
// ============================================================================

/* Not Implemented
179	CAN Bus Failure
180	GSM Socket not made
181	SIM Not inserted
182	GPS error
183	GPRS Not Active*/

//----------------------------------------------------------------

	extern uint8_t ucCAN_eRxData[];
	extern uint8_t ucCAN_eTxData[];
//----------------------------------------------------------------

typedef enum
{
	NO_CAN_CMD = 0x0,
	CAN_START_VEHICLE = 0x01,
	CAN_STOP_VEHICLE = 0x02,
	CAN_UNLOCK_VEHICLE = 0x04,
	CAN_FIND_VEHICLE = 0x08,
	CAN_START_IND_BUZZER = 0x20,

	CAN_IMMOB_PAIRING_ENABLE = 0x01,
	CAN_IMMOB_ERASE_KEYS = 0x02,
	CAN_IMMOB_READ_KEYS = 0x04,
	CAN_IMMOB_GET_KEYID = 0x08,

	CAN_HAZARD_LIGHT_OFF = 0x01,
	CAN_HAZARD_LIGHT_ON = 0x02,

} EnumVehicleCANCmd;

typedef enum
{
	POLARITY_PLUS = 0,
	POLARITY_MINUS,
} EnumPol;

//----------------------------------------------------------------

#pragma pack(push, 1)
	typedef struct
	{
		//	uint8_t		ucRxData[10];
		uint8_t ucCurrentCANId;
		uint16_t uiCANRxTimeOut;

		uint8_t ucProdYear;
		uint8_t ucProdMonth;
		uint8_t ucProdDay;
		//	uint8_t		ucSNum0;				// Rev 1.01 28-08-2024
		//	uint8_t		ucSNum1;				// Rev 1.01 28-08-2024
		//	uint8_t		ucSNum2;				// Rev 1.01 28-08-2024
		//	uint8_t		ucSNum3;				// Rev 1.01 28-08-2024
		uint8_t ucBattSNum[3]; // Rev 1.01 28-08-2024
		char cVehBattID[LEN_BATTID];

		uint16_t uiBattVolt;
		bool bCurrPol; // 0 - Positive, 1 - Negative
		uint16_t uiBattCurrent;
		uint8_t ucBattStateFlag;
		uint8_t ucBattType;

		uint8_t ucVehMode;
		uint8_t ucVehModePrev;
		uint16_t uiRPM;
		char cRPM[4];
		uint8_t ucSpeedInkmph;
		uint8_t ucSpeedInmiph;
		char cSpeed[4];
		uint16_t uiMotorCurrent;
		uint8_t ucControllerTemp;
		uint8_t ucControllerErr1;
		uint8_t ucControllerErr2;
		uint16_t uiContVoltage;

		uint8_t ucBattSOC;
		char cBattSOC[3];
		uint8_t ucBattSOH;
		uint16_t uiNumOfCycles;

		uint16_t uiFullChgAH;
		uint16_t uiResidualAH;
		uint16_t uiResidualEnergy;
		uint16_t uiResidualEnergyPrev;
		uint16_t uiTimeToCharge;

		uint16_t uiBattCellMinVolt;
		uint16_t uiBattCellMaxVolt;

		uint8_t ucBattCellMinTemp;
		uint8_t ucBattCellMaxTemp;
		char cBattTemp[3];
		uint8_t ucBattMOSTemp;
		uint8_t ucBattAmbientTemp;

		uint8_t ucBattErr1;
		uint8_t ucBattErr2;
		uint8_t ucBattErr3;
		uint8_t ucBattErr4;
		uint8_t ucBattErr5;
		uint8_t ucBattErr6;
		uint8_t ucBattErr7;
		uint8_t ucBattErr8;
		uint8_t ucBattErr9;

		uint8_t ucClusterErr1;
		uint8_t ucChargerErr1;

		uint8_t ucChgConnectCntr;
		uint8_t ucChgConnectCntrPrev;

		uint8_t ucCANTxCntr;

		uint8_t ucBattCANDataTimer;
		uint8_t ucContCANDataTimer;

		uint8_t ucTotalErrors;
		uint8_t ucAllErrorDispStatus;
		uint16_t uiCurrentErrors[20];
		uint8_t ucErrDispTimer;
		uint8_t ucErrDispCntr;
		char cErrCodeBuff[5];
	} TsCAN;
#pragma pack(pop)

#pragma pack(push, 1)
	typedef struct
	{
		uint8_t ucIndexNum;
		uint8_t ucNumOfCellData;
		uint16_t uiCellVoltage[20];

		uint16_t uiCellMinVolt;
		uint16_t uiCellMaxVolt;
	} TsCell;
#pragma pack(pop)

	typedef union
	{
		struct
		{
			//		bool	bBATTIDReadStatus		: 1;	// Rev 1.01 28-08-2024
			bool bCANDataRecdStatus : 1; // 0 - Not, 1 - Recd
			bool bCANCmdSent : 1;		 // 0 - Not, 1 - Sent
			bool bChargerStatus : 1;
			bool bONStatus : 1;
			bool bACCStatus : 1;
			bool bCRGStatus : 1;
			bool bBrakeStatus : 1;
			bool bAntiTheftStatus : 1;
			bool bSpeedLimitStatus : 1;
			bool bCruiseStatus : 1;
			bool bReverseStatus : 1;
			bool bReadyStatus : 1;
			bool bChgProtection : 1;
			bool bChargerErr : 1;
			bool bVehTryingON : 1;	  // 0 - No, 1 - Error
			bool bImmobCANStatus : 1; // 0 - No, 1 - data Recd

			uint8_t ucKeyFOBCmd : 4;		// 0 - No, 1 - Start, 2 - Unlock, 4 - Lock, 8 - Find
			uint8_t ucImmobStatus : 2;		// 0 - No, 1- Buzzer Status, 2 - Ind Status
			bool bImmobilizationStatus : 1; // 0 - No, 1 - Active
			uint8_t ucImmobErr : 8;
			bool bContCANStatus1 : 1; // 0 - No, 1 - data Recd

			bool bContCANStatus2 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus1 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus2 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus3 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus4 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus5 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus6 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus7 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus8 : 1; // 0 - No, 1 - data Recd
			bool bBattCANStatus9 : 1;
			bool bBattCANStatus10 : 1;	// 0 - No, 1 - data Recd
			bool bBattChgCANStatus : 1; // 0 - No, 1 - data Recd
			bool bDataFromImmob : 1;	// 0 - No, 1 - data Recd
			bool bCANFrame413Recd : 1;	// 1 - CAN Frame on 0x413 Recd
			bool bCANFrame414Recd : 1;	// 1 - CAN Frame on 0x414 Recd
			bool bCANFrame415Recd : 1;	// 1 - CAN Frame on 0x415 Recd
		};
		uint16_t uiData[3];
	} TuCANStatus;

	//----------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H */
