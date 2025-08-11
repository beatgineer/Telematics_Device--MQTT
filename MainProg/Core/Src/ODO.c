#include "ODO.h"
#include "APP.h"
#include "E2PROM_I2C.h"
#include "CONFIG_EEPROM.h"
#include "CAN.h"

#include <stdio.h>
#include <string.h>

TsOdo ODOData; 
TuOdoVal ODOFloat2Hex;
extern TsAPP APPStatus;
extern TsAPP APPStatus;
extern TsCAN CANData;

// const uint8_t ucODO_eRV400RangeFactor[] = {14, 9, 7};		//Rev 1.03
const uint8_t ucODO_eRV400RangeFactor[] = {15, 11, 8}; // Rev 1.03

// ----------------------------------------------------------------------------
// Name			: vODO_eInit
// Objective	: Init Odo module
//
// Input  		: none
// Output 		: none
// Return		: none
// Version		: -
// ----------------------------------------------------------------------------
void vODO_eInit(void)
{
    //	ODOData.bOdoAndTripValChanged	= false;
    ODOData.ulOdoValInKmPrev = ODOData.ulOdoValInKm;
    //	ODOData.fCumWh 					= 0;
    //	ODOData.bWhPerKmCalculated		= false;
}

// ============================================================================
// Name			: vODO_eCalculateOdo_Exe
// Objective	: Calculate Odometer & Trip Meter
//					Addup Speed 5 times at every 300mS
// Input  		: Speed in kmph
// Output 		: 	1. Odometer Value @ 1km Resolution 	- max 999999 km
//			 		2. Trip meter - A Value @ 0.1km Resolution - max 999.9 km
//			 		3. Trip meter - B Value @ 0.1km Resolution - max 999.9 km
//					4. Cumulative Speed at every 300ms
// Return		: none
// Version		: -
// ============================================================================
void vODO_eCalculateTripAndOdo_Exe(uint8_t ucSpeed)
{
    // uint8_t ucTempBuffer[2];
    char cBuff[5];

    if (ucSpeed)
    {
        // Conversion Into Distance
        // Speed in meter per hr = Speed in kmph * 1000
        // Speed in meter per Sec = (Speed in kmph * 1000) / 3600
        // Distance in meter in 360mS = ((Speed in kmph * 1000)/ 3600) * 0.360)
        // Distance in 0.1 meter Resolution (in 360mS) = ((Speed in kmph * 1000)/ 3600) * 0.360) * 10
        // Distance in 0.1 meter Resolution (in 360mS) = Speed in kmph

        ODOData.uiTripADist += ucSpeed;
        ODOData.uiTripBDist += ucSpeed;
        ODOData.uiOdoDistVal += ucSpeed;

        if (ODOData.uiTripADist >= 1000)
        {
            ODOData.uiTripAValInKm++;
            ODOData.uiTripADist -= 1000;
            APPStatus.bValChanged = true;

            if (ODOData.uiTripADist > 9999) // Rev 1.021 19-09-2024
            {
                ODOData.uiTripADist = 0; // Rev 1.021 19-09-2024
            }
        }
        else
        {
        }

        if (ODOData.uiTripBDist >= 1000)
        {
            ODOData.uiTripBValInKm++;
            ODOData.uiTripBDist -= 1000;
            APPStatus.bValChanged = true;

            if (ODOData.uiTripBDist > 9999) // Rev 1.021 19-09-2024
            {
                ODOData.uiTripBDist = 0; // Rev 1.021 19-09-2024
            }
        }
        else
        {
        }

        if (ODOData.uiOdoDistVal >= 1000)
        {
            ODOData.ulOdoValInKm++;
            ODOData.uiOdoDistVal -= 1000;
            APPStatus.bValChanged = true;

            if (ODOData.ulOdoValInKm > 9999999) // Rev 1.021 19-09-2024
            {
                ODOData.ulOdoValInKm = 0; // Rev 1.021 19-09-2024
            }

            // char cODO[10];
            // sprintf(cODO, "%d", (ODOData.ulOdoValInKm / 10));

            // vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_ODO_DIST, (char *)ODOData.cStoreODO, LEN_ODO_DIST);


            // sprintf(ODOData.cStoreODO, "%d", (ODOData.ulOdoValInKm / 10));

            // vEEPROM_eReadBytes_Exe(EEPROM_ADDR_ODO_DIST, ODOData.cStoreODO, LEN_ODO_DIST);

        }
        else
        {
        }
    }
    else
    {
    }
}

// ----------------------------------------------------------------------------
// Name			: vODO_eUpdateRangeBuff_Exe
// Objective	: Update Range Buffer
// Input  		: ucUnit	: 0 - km
//							: 1 - mi
// Output 		: none
// Return		: none 
// Version		: -
// ----------------------------------------------------------------------------
void vODO_eUpdateRangeBuff_Exe(uint8_t ucUnit)
{
    ODOData.uiCalRangeInKm = (CANData.ucBattSOC * ucODO_eRV400RangeFactor[CANData.ucVehMode - 1]) / 10;
    // switch (ucUnit)
    // {
    // case KM:
    //     sprintf((char *)ODOData.ucRange, "%04d", ODOData.uiCalRangeInKm);

    //     vLCD_eUpdateDisplayKmSymbolInRange_Exe(ON);  // Display km
    //     vLCD_eUpdateDisplayMiSymbolInRange_Exe(OFF); // remove mi
    //     break;

    // case MI:
    //     ODOData.uiCalRangeInMi = (uint16_t)((float)ODOData.uiCalRangeInKm * km2MiFactor);

    //     sprintf((char *)ODOData.ucRange, "%04d", ODOData.uiCalRangeInMi);

    //     vLCD_eUpdateDisplayKmSymbolInRange_Exe(OFF); // Remove km
    //     vLCD_eUpdateDisplayMiSymbolInRange_Exe(ON);  // Display mi
    //     break;
    // }

    // vLCD_eUpdateDisplayRPMSymbol_Exe(OFF);  // Remove rpm Symbol
    // vLCD_eUpdateDisplayRangeSymbol_Exe(ON); // Display Range Symbol

    // ODOData.ucRange[0] &= 0x0F;
    // ODOData.ucRange[1] &= 0x0F;
    // ODOData.ucRange[2] &= 0x0F;
    // ODOData.ucRange[3] &= 0x0F;

    // vLCD_eUpdateDisplayRPMRangeBuff_Exe((char *)ODOData.ucRange);
}

void vODO_eUpdateRPMBuff_Exe(void)
{
    sprintf(CANData.cRPM, "%04d", CANData.uiRPM);
    CANData.cRPM[0] &= 0x0F;
    CANData.cRPM[1] &= 0x0F;
    CANData.cRPM[2] &= 0x0F;
    CANData.cRPM[3] &= 0x0F;
}

/*
void vODO_eCalculateWhPerkmOrMi_Exe(void)
{
    uint8_t ucVal;

    if (CANData.ucSpeedInkmph)
    {
        ucVal = ODOData.ulOdoValInKm - ODOData.ulOdoValInKmPrev;

        if (APPStatus.bUnitDisplay == KM_DISPLAY)
        {
            if (ucVal >= 10)
            {
                // 1 km distance travelled, Calculate Wh/km
                ODOData.ucWhPerKm = CANData.uiResidualEnergyPrev - CANData.uiResidualEnergy;
                CANData.uiResidualEnergyPrev = CANData.uiResidualEnergy;
                ODOData.ulOdoValInKmPrev = ODOData.ulOdoValInKm;
            }
            else
            {
            }
        }
        else if (APPStatus.bUnitDisplay == MI_DISPLAY)
        {
            if (ucVal >= 16)
            {
                // 1.6 km distance travelled, Calculate Wh/mi
                ODOData.ucWhPerMi = CANData.uiResidualEnergyPrev - CANData.uiResidualEnergy;
                CANData.uiResidualEnergyPrev = CANData.uiResidualEnergy;
                ODOData.ulOdoValInKmPrev = ODOData.ulOdoValInKm;
            }
            else
            {
            }
        }
        else
        {
        }
    }
    else
    {
    }
}*/