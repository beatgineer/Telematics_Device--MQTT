// ============================================================================
// ODO.h
// Author : AK
// Date   : 01 Jun 2022
// ============================================================================

#ifndef __ODO_H
#define __ODO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define km2MiFactor 0.621371F

#define SAMPLES_FOR_ODO_CAL 10U

#define SIG1 0x55
#define SIG2 0x66
#define SIG3 0x77
#define SIG4 0x88

    extern const uint8_t ucODO_eRV400RangeFactor[];

#pragma pack(push, 1)
    typedef struct
    {
        uint8_t ucSig[4];

        uint16_t uiOdoDistVal;
        uint32_t ucOdoVal[8]; 
        uint32_t ulOdoValInKm; 
        uint32_t ulOdoValInKmPrev;
        // uint32_t ulOdoValInMi;

        char cStoreODO[10];

        uint16_t uiTripADist;
        uint8_t ucTripA[5];
        uint16_t uiTripAValInKm;
        uint16_t uiTripAValInMi;

        uint16_t uiTripBDist;
        uint8_t ucTripB[5];
        uint16_t uiTripBValInKm;
        // uint16_t uiTripBValInMi;

        //	bool		bOdoAndTripValChanged;	// 0 - Odo & Trip Val not Changed, Do not Store, 1 - Odo & Trip Val Changed, Store

        uint8_t ucRange[5];
        uint8_t ucRange1[5];
        uint16_t uiCalRangeInKm;  // Calculated Range
        uint16_t uiCalRangeInKm1; // Calculated Range : AK:04102023 Just for Testing
        uint16_t uiCalRangeInMi;  // Calculated Range

        bool bWhPerKmCalculated; // 0 - Not, 1 - Calculated
        uint8_t ucWhPerKm;
        // uint8_t ucWhPerMi;
        //	float		fCumWh;
        char cWh[3];
        //	float		fTotalConsumedWh;
        //	float		fPerkmWhConsumed;
        //	uint16_t	uiRemainingWH;
    } TsOdo;
#pragma pack(pop)

    typedef union
    {
        float fValue;
        char cVal[4];
    } TuOdoVal;

    void vODO_eInit(void);
    void vODO_eCalculateTripAndOdo_Exe(uint8_t ucSpeed);
    void vODO_eUpdateRangeBuff_Exe(uint8_t ucUnit);
    void vODO_eUpdateRPMBuff_Exe(void);
    // void vODO_eCalculateWhPerkmOrMi_Exe(void);
    // void vODO_eCalculateRealTimeRange_Exe(void);

    // void vODO_eUpdateOdo_Exe(void);
    // void vODO_eUpdateTripA_Exe(void);
    // void vODO_eUpdateTripB_Exe(void);
    // void vODO_eUpdateRange_Exe(uint16_t uiRange);
    // void vODO_eStoreOdoAndTripValInE2PROM_Exe(void);
    // void vODO_eConvertODOinASCII_Exe(char *cOdoVal);
    // void vODO_eConvertTripAinASCII_Exe(char *cTripAVal);
    // void vODO_eConvertTripBinASCII_Exe(char *cTripBVal);

#ifdef __cplusplus
}
#endif

#endif

/***** END OF FILE ****/
