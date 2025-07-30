// ============================================================================
// Module	: CONFIG
// Version	: A
// Author	: AK
// Date		: 06-12-2021
// ============================================================================
#include "CONFIG_EEPROM.h"
#include "APP.h"
#include "GSM.h"
#include "E2PROM_I2C.h"
#include "UTL.h"
#include "ODO.h"
#include "CAN.h"
#include "FTP.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern TsAPP_eConfig APPCONFIG;
extern TsAPP APPStatus;
extern TsGSMData GSMHealth;
extern TsOdo ODOData;
extern TuOdoVal ODOFloat2Hex;
extern TsCAN CANData;
extern TuCANStatus CANStatus;
extern TsOdo ODOData;
extern TsFTPData FTPData;

#pragma pack(push, 1)
const TsConfig CONFIG = {
    VEHICLE_RV400,

    'T', // Protocol, H - HTTP, T - TCP, U - UDP, M - MQTT

    "13.232.191.212", // FTP Server Host Name
    "21",             // FTP Server Port No.
    "ecudown",
    "haD_8A@usw",

    TRUE, // SMS OTA : Authntication: 0 - Disable, 1 - Enable
    // "+919811451380",		// SMS OTA : Master Mobile No.		// AK
    //	"+919999134462",		// SMS OTA : Master Mobile No.
    "+918373958568", // SMS OTA : Master Mobile No.		// Anil
 
    "1301",     // IOT SW VERSION - Ready for Release
    "16052025", // IOT BUILD Date
};
#pragma pack(pop)

#pragma pack(push, 1)
const TsEEPROMConfig EEPROMCONFIG = {
    0x11, 0x22, 0x33, 0x44, 0x55, // Signature
 
    "m2m.myrevolt.com", // APN

    0,                      // Status
    "13.235.101.231",       // IP address
    "1883",                 // Port number
    "RovoSync",             // Username
    "Yt7rt4hgTVhfY",         // Password
    "rev25",                 // MQTT Topic
    "ecu.revoltmotors.com", // Domain Name

    20,   // Tx Rate Ign ON (20 sec)
    20,  // Tx Rate Ign OFF (10 min)
    3600, // Battery Removed Rate (1 hr)

    "0123456789", // CCID
    FALSE,        // IMEI & CCID Not written
};
#pragma pack(pop)

void vCONFIGEEPROM_eReadConfigData_Exe(void)
{
    uint8_t ucSigBuffer[20];
    uint8_t ucTempBuffer[2];
    
    	char cBuff[20];
	int16_t iVal;

    // Read the signature from EEPROM
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_SIG, ucSigBuffer, 5);

    // Compare EEPROM signature with default
   if (!((ucSigBuffer[0] == EEPROMCONFIG.ucSig1) && \
         (ucSigBuffer[1] == EEPROMCONFIG.ucSig2) && \
         (ucSigBuffer[2] == EEPROMCONFIG.ucSig3) && \
         (ucSigBuffer[3] == EEPROMCONFIG.ucSig4) && \
         (ucSigBuffer[4] == EEPROMCONFIG.ucSig5)))
   {
        vCONFIGEEPROM_eLoadDefaultPar_Exe();
        APPStatus.uiMsgCntr = 0;
        vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_MSGCOUNTER, (uint8_t *)&APPStatus.uiMsgCntr, sizeof(APPStatus.uiMsgCntr));
   }

    // Compare EEPROM signature with default
//    if (memcmp(ucSigBuffer, (uint8_t *)&EEPROMCONFIG.ucSig1, 5) != 0)
//    {
//         vCONFIGEEPROM_eLoadDefaultPar_Exe();
//         APPStatus.uiMsgCntr = 0;
//         vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_MSGCOUNTER, (uint8_t *)&APPStatus.uiMsgCntr, sizeof(APPStatus.uiMsgCntr));
//    }

    // Read stored values from EEPROM

    //IMEI
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_IMEI, (uint8_t *)APPCONFIG.cIMEI, LEN_IMEINUM);

    //IP Address
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_DATA_SERVER_IPADDR, (uint8_t *)APPCONFIG.cDataServerIP, LEN_SERVERIP);

    //Port Number
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_DATA_SERVER_PORTNUM, (uint8_t *)APPCONFIG.cDataServerPort, LEN_PORTNUM);

    //Domain Name
    APPCONFIG.ucProtocol = ucEEPROM_eReadByte_Exe(EEPROM_ADDR_PROTOCOL);

    //APN
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_APN, (uint8_t *)APPCONFIG.cAPN, LEN_APN);

    //CCID
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_GSM_CCID, (uint8_t *)APPCONFIG.cCCID, LEN_CCID);

    APPStatus.bVehRunStatus = ucEEPROM_eReadByte_Exe(EEPROM_ADDR_VEHICLE_RUN_STATUS);

    // BATT ID
    memset(CANData.cVehBattID, 0, LEN_BATTID); // Rev 1.01 28-08-2024
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_BATTID, CANData.cVehBattID, LEN_BATTID);
    if (((CANData.cVehBattID[0] > 99) ||
         (CANData.cVehBattID[0] == 0xFF) ||
         (CANData.cVehBattID[0] == 0x0)))
    {
        CANData.cVehBattID[0] = 0;
        //		CANStatus.bBATTIDReadStatus = false;	// Rev 1.01 28-08-2024
        CANStatus.bBattCANStatus7 = false;
        CANStatus.bBattCANStatus8 = false;
    }

    // VEHICLE RUN STATUS
    APPStatus.bVehRunStatus = ucEEPROM_eReadByte_Exe(EEPROM_ADDR_VEHICLE_RUN_STATUS);

//    uint8_t ucFWCheckedDate = 0;
//    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_FW_CHECKED_DATE, &ucFWCheckedDate, 1);
    // FW Checked Date
    FTPData.ucFWCheckedDate = ucEEPROM_eReadByte_Exe(EEPROM_ADDR_FW_CHECKED_DATE);

    // FW Version Check Status
    FTPData.ucFWCheckStatus = ucEEPROM_eReadByte_Exe(EEPROM_ADDR_FW_UPDATE_STATUS);

    // Read Version written in E2PROM
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_IOT_VER_NUM, cBuff, 5);
    iVal = strncmp((char *)CONFIG.cSWRev, cBuff, 4);
    if (iVal != 0)
    {
        // Cluster Current FW Ver
        vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IOT_VER_NUM, (char *)CONFIG.cSWRev, 5);
    }
    else
    {
    }

    // vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_ODO_DIST, (char *)ucTempBuffer, LEN_ODO_DIST);
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_ODO_DIST, ODOData.cStoreODO, LEN_ODO_DIST);

    // Read Message Counter
    vEEPROM_eReadBytes_Exe(EEPROM_ADDR_MSGCOUNTER, ucTempBuffer, 2);
    APPStatus.uiMsgCntr = (ucTempBuffer[0] << 8) | ucTempBuffer[1];
}

void vCONFIGEEPROM_eLoadDefaultPar_Exe(void)
{
    uint8_t ucTempBuffer[2];
    uint8_t ucIMEIBuffer[LEN_IMEINUM] = {0};

    // Write default signature
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_SIG, (uint8_t *)&EEPROMCONFIG.ucSig1, 5);

    // IMEI
    if (bGSM_eGetIMEINum_Exe((char *)APPCONFIG.cIMEI))
    {
        // vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IMEI, ucIMEIBuffer, LEN_IMEINUM);
        vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IMEI, APPCONFIG.cIMEI, LEN_IMEINUM);
    }
    
    // if (bGSM_eGetIMEINum_Exe((char *)ucIMEIBuffer))
    // {
    //     vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IMEI, ucIMEIBuffer, LEN_IMEINUM);  
    // }
    // else
    // {
    //     vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IMEI, (uint8_t *)DEFAULTCONFIG.cIMEI, LEN_IMEINUM);
    // }

    // CCID
    if (bGSM_eGetICCIDNum_Exe((char *)ucIMEIBuffer))
    {
        vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_GSM_CCID, ucIMEIBuffer, LEN_CCID);
    }
    // else
    // {
    //     vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_GSM_CCID, (uint8_t *)DEFAULTCONFIG.cIMEI, LEN_IMEINUM);
    // }

    // APN Details
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_APN, (uint8_t *)EEPROMCONFIG.cAPN, LEN_APN);

    // Protocol
    // vEEPROM_eWriteByte_Exe(EEPROM_ADDR_PROTOCOL, DEFAULTCONFIG.ucProtocol);

    // Server IP and Port
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_DATA_SERVER_IPADDR, (uint8_t *)EEPROMCONFIG.cLocationIPAdd, LEN_SERVERIP);
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_DATA_SERVER_PORTNUM, (uint8_t *)EEPROMCONFIG.cLocationPortNum, LEN_PORTNUM);

    // IoT Current FW Ver
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_IOT_VER_NUM, (char *)CONFIG.cSWRev, 5);

    // Ignition ON Tx Rate
    ucTempBuffer[0] = (EEPROMCONFIG.uiTxRateIgnON >> 8) & 0xFF;
    ucTempBuffer[1] = (EEPROMCONFIG.uiTxRateIgnON) & 0xFF;
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_TXRATE_IGNON, ucTempBuffer, 2);

    // Ignition OFF Tx Rate
    ucTempBuffer[0] = (EEPROMCONFIG.uiTxRateIgnOFF >> 8) & 0xFF;
    ucTempBuffer[1] = (EEPROMCONFIG.uiTxRateIgnOFF) & 0xFF;
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_TXRATE_IGNOFF, ucTempBuffer, 2);

    // BATT ID
   /* ucTempBuffer[0] = 0;
    ucTempBuffer[1] = 0;
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_BATTID, (char *)ucTempBuffer, LEN_BATTID);*/
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_BATTID, CANData.cVehBattID, LEN_BATTID);

    // CANData.cVehBattID
        CANData.cVehBattID[0] = 0;
    //	CANStatus.bBATTIDReadStatus = false;	// Rev 1.01 28-08-2024
    CANStatus.bBattCANStatus7 = false;
    CANStatus.bBattCANStatus8 = false;

    // Vehicle Run Status
    vEEPROM_eWriteByte_Exe(EEPROM_ADDR_VEHICLE_RUN_STATUS, VEHICLE_RUN);

    // // Batt Removed Tx Rate
    // ucTempBuffer[0] = (DEFAULTCONFIG.uiTxRateBattRemove >> 8) & 0xFF;
    // ucTempBuffer[1] = (DEFAULTCONFIG.uiTxRateBattRemove) & 0xFF;
    // vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_TXRATE_BATTREMOVE, ucTempBuffer, 2);

    // Initialize Msg Counter
    APPStatus.uiMsgCntr = 0;
    vEEPROM_eWriteBytes_Exe(EEPROM_ADDR_MSGCOUNTER, (uint8_t *)&APPStatus.uiMsgCntr, sizeof(APPStatus.uiMsgCntr));
}