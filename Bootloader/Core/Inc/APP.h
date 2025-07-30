#if 1
/**
 ******************************************************************************
 * @file           : APP.h
 * @brief          : Header for APP.c file.
 *                   This file contains the common defines of the application.
 *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_H__
#define __APP_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
// *****************************************************************************/

/* Private defines -----------------------------------------------------------*/

// #define IMAGE1_MCU_FLASH_ADDR  ((uint32_t)0x0800A800U)  // IMG1 start (page 25)
// #define IMAGE2_MCU_FLASH_ADDR  ((uint32_t)0x08026000U)  // IMG2 start (page 76)

#define IMAGE1_MCU_FLASH_ADDR  ((uint32_t)0x0800B000)   //page: 22
#define IMAGE2_MCU_FLASH_ADDR  ((uint32_t)0x08016000)  //Page: 42

/*
  EEPROM Address Map Table

  | Name                    | Start   | End     | Length |
  |-------------------------|---------|---------|--------|
  | SIG                     | 0x0000  | 0x0004  |   5    |
  | IMEINUM                 | 0x0005  | 0x0016  |  18    |
  | APN                     | 0x0017  | 0x0027  |  17    |
  | APNUSERNAME             | 0x0028  | 0x002D  |   6    |
  | APNPASSWORD             | 0x002E  | 0x0033  |   6    |
  | PROTOCOL                | 0x0034  | 0x0034  |   1    |
  | DATA_SERVER_IPADDR      | 0x0035  | 0x0048  |  20    |
  | DATA_SERVER_PORTNUM     | 0x0049  | 0x004E  |   6    |
  | TXRATE_IGNON            | 0x004F  | 0x0050  |   2    |
  | TXRATE_IGNOFF           | 0x0051  | 0x0052  |   2    |
  | GSM_CCID                | 0x0053  | 0x0067  |  21    |
  | FTPHOSTNAME             | 0x0068  | 0x0085  |  30    |
  | FTPUSERNAME             | 0x0086  | 0x0094  |  15    |
  | FTPPASSWORD             | 0x0095  | 0x00A3  |  15    |
  | SWVER                   | 0x00A4  | 0x00A8  |   5    |
  | CCID2                   | 0x00A9  | 0x00BD  |  21    |
  | AUTHMOBILENUM           | 0x00BE  | 0x00D1  |  20    |
  | REV                     | 0x00D2  | 0x00D9  |   8    |
  | REVDATE                 | 0x00DA  | 0x00E2  |   9    |
  | DOMAIN                  | 0x00E3  | 0x010A  |  40    |
  | VEHICLE_RUN_STATUS      | 0x010B  | 0x010B  |   1    |
  | BATTID                  | 0x010C  | 0x0119  |  14    |
  | ODO_DIST                | 0x011A  | 0x0121  |   8    |
  | MSGCOUNTER              | 0x0122  | 0x0125  |   4    |

  // Firmware-update flags (separate block)
  | FW_UPDATE_STATUS        | 0x0300  | 0x0300  |   1    |
  | FW_CHECKED_DATE         | 0x0301  | 0x0301  |   1    |
  | IMMOB_VER_NUM           | 0x0302  | 0x0306  |   5    |
  | IOT_VER_NUM             | 0x0307  | 0x030B  |   5    |
  | IOT_SMS_VER_NUM         | 0x030C  | 0x0310  |   5    |
  | IMMOB_SMS_VER_NUM       | 0x0311  | 0x0315  |   5    |
  | SMS_SENDER_NUM          | 0x0316  | 0x032E  |  25    |

  // Total config block usage: 0x0126 bytes (294 in decimal)
*/


// Lengths
#define LEN_SIG 5U                // signature
#define LEN_IMEINUM 18U           // IMEI
#define LEN_APN 17U               // APN
#define LEN_USERNAME 6U           // APN user
#define LEN_PASSWORD 6U           // APN pass
#define LEN_PROTOCOL 1U           // protocol flag
#define LEN_SERVERIP 20U          // data server IP
#define LEN_PORTNUM 6U            // data server port
#define LEN_TXRATE 2U             // tx rate
#define LEN_CCIDNUM 21U           // GSM CCID
#define LEN_FTPHostName 30U       // FTP host
#define LEN_FTPUSERNAME 15U       // FTP user
#define LEN_FTPPASSWRD 15U        // FTP pass
#define LEN_SWVER 5U              // software version
#define LEN_CCIDNUM2 21U          // (reuse CCIDNUM for something else)
#define LEN_AUTHMOBILENUM 20U     // SMS sender
#define LEN_REV 8U                // revision
#define LEN_REVDATE 9U            // revision date
#define LEN_DOMAIN 40U            // domain name
#define LEN_VEHICLE_RUN_STATUS 1U // run flag
#define LEN_BATTID 14U            // battery ID
#define LEN_ODO_DIST 8U           // odometer
#define LEN_MSGCOUNTER 4U         // <-- pick your length

// EEPROM Address Map
#define EEPROM_ADDR_SIG 0x0000                           // [0x0000..0x0004]
#define EEPROM_ADDR_IMEI (EEPROM_ADDR_SIG + LEN_SIG)     // 0x0005 [..0x0016]
#define EEPROM_ADDR_APN (EEPROM_ADDR_IMEI + LEN_IMEINUM) // 0x0017 [..0x0027]
#define EEPROM_ADDR_APNUSERNAME (EEPROM_ADDR_APN + LEN_APN)               // 0x0028 [..0x002D]
#define EEPROM_ADDR_APNPASSWORD (EEPROM_ADDR_APNUSERNAME + LEN_USERNAME)  // 0x002E [..0x0033]
#define EEPROM_ADDR_PROTOCOL (EEPROM_ADDR_APNPASSWORD + LEN_PASSWORD)     // 0x0034 [..0x0034]
#define EEPROM_ADDR_DATA_SERVER_IPADDR (EEPROM_ADDR_PROTOCOL + LEN_PROTOCOL) // 0x0035 [..0x0048]
#define EEPROM_ADDR_DATA_SERVER_PORTNUM (EEPROM_ADDR_DATA_SERVER_IPADDR + LEN_SERVERIP) // 0x0049 [..0x004E]
#define EEPROM_ADDR_TXRATE_IGNON (EEPROM_ADDR_DATA_SERVER_PORTNUM + LEN_PORTNUM)       // 0x004F [..0x0050]
#define EEPROM_ADDR_TXRATE_IGNOFF (EEPROM_ADDR_TXRATE_IGNON + LEN_TXRATE)              // 0x0051 [..0x0052]
#define EEPROM_ADDR_GSM_CCID (EEPROM_ADDR_TXRATE_IGNOFF + LEN_TXRATE)                  // 0x0053 [..0x0067]
#define EEPROM_ADDR_FTPHOSTNAME (EEPROM_ADDR_GSM_CCID + LEN_CCIDNUM)                   // 0x0068 [..0x0085]
#define EEPROM_ADDR_FTPUSERNAME (EEPROM_ADDR_FTPHOSTNAME + LEN_FTPHOSTNAME)            // 0x0086 [..0x0094]
#define EEPROM_ADDR_FTPPASSWORD (EEPROM_ADDR_FTPUSERNAME + LEN_FTPUSERNAME)            // 0x0095 [..0x00A3]
#define EEPROM_ADDR_SWVER (EEPROM_ADDR_FTPPASSWORD + LEN_FTPPASSWORD)                  // 0x00A4 [..0x00A8]
#define EEPROM_ADDR_CCID2 (EEPROM_ADDR_SWVER + LEN_SWVER)                              // 0x00A9 [..0x00BD]
#define EEPROM_ADDR_AUTHMOBILENUM (EEPROM_ADDR_CCID2 + LEN_CCIDNUM2)                   // 0x00BE [..0x00D1]
#define EEPROM_ADDR_REV (EEPROM_ADDR_AUTHMOBILENUM + LEN_AUTHMOBILENUM)                // 0x00D2 [..0x00D9]
#define EEPROM_ADDR_REVDATE (EEPROM_ADDR_REV + LEN_REV)                                // 0x00DA [..0x00E2]
#define EEPROM_ADDR_DOMAIN (EEPROM_ADDR_REVDATE + LEN_REVDATE)                         // 0x00E3 [..0x010A]
#define EEPROM_ADDR_VEHICLE_RUN_STATUS (EEPROM_ADDR_DOMAIN + LEN_DOMAIN)               // 0x010B [..0x010B]
#define EEPROM_ADDR_BATTID (EEPROM_ADDR_VEHICLE_RUN_STATUS + LEN_VEHICLE_RUN_STATUS)   // 0x010C [..0x0119]
#define EEPROM_ADDR_ODO_DIST (EEPROM_ADDR_BATTID + LEN_BATTID)                         // 0x011A [..0x0121]
#define EEPROM_ADDR_MSGCOUNTER (EEPROM_ADDR_ODO_DIST + LEN_ODO_DIST)                   // 0x0122 [..0x0125]

// after MSGCOUNTER you can reserve more if needed up to your next block�
 
// Firmware-update flags (keep these well apart from the config block)
#define EEPROM_ADDR_FW_UPDATE_STATUS 0x300  // 1 byte
#define EEPROM_ADDR_FW_CHECKED_DATE  0x314  // 1 byte
// #define EEPROM_ADDR_IMMOB_VER_NUM    0x0124  // 5 bytes [0x0124..0x0128]
#define EEPROM_ADDR_IOT_VER_NUM      0x302  // 5 bytes [0x0129..0x012D]
#define EEPROM_ADDR_IOT_SMS_VER_NUM  0x303 // 5 bytes [0x012E..0x0132]
#define EEPROM_ADDR_SMS_SENDER_NUM   0x316  // 25 bytes [0x0138..0x0150]


// #define EEPROM_ADDR_FW_UPDATE_STATUS 0x0300  // 1 byte
// #define EEPROM_ADDR_FW_CHECKED_DATE 0x0301   // 1 byte
// #define EEPROM_ADDR_IMMOB_VER_NUM 0x0302     // 5 bytes [0x302..0x306]
// #define EEPROM_ADDR_IOT_VER_NUM 0x0307       // 5 bytes [0x307..0x30B]
// #define EEPROM_ADDR_IOT_SMS_VER_NUM 0x030C   // 5 bytes [0x30C..0x310]
// #define EEPROM_ADDR_IMMOB_SMS_VER_NUM 0x0311 // 5 bytes [0x311..0x315]
// #define EEPROM_ADDR_SMS_SENDER_NUM 0x0316    // 25 bytes[0x316..0x32E]

// Total size of the config block (up through MSGCOUNTER)
#define TOTAL_CONFIG_SIZE ((EEPROM_ADDR_MSGCOUNTER + LEN_MSGCOUNTER) - EEPROM_ADDR_SIG)

    // *****************************************************************************/
    extern volatile uint8_t ucAPP_eVerUpdate;
    extern uint16_t uiAPP_eTimer2S;

    // *****************************************************************************/
    typedef enum
    {
        JUMP_TO_PROGRAM_IMAGE1 = 1,
        JUMP_TO_PROGRAM_IMAGE2 = 2,
        FLASH_PROGRAM_IMAGE = 3,
        SET_DEFAULT_IMAGE = 4,
    } TsActivity;

    typedef enum
    {
        NO_VERSION_CHECK = 0,
        CHK_VERSION_UPDATE = 1,
        NO_IOT_VERSION_CHANGE = 2,
        IOT_FW_UPDATED = 3,
        NO_IMMOB_VERSION_CHANGE = 4,
        IMMOB_FW_UPDATED = 5,
        SMS_IOT_FW_VERSION = 6,
        SMS_IMMOB_FW_VERSION = 7,
    } Enum_FTPFWMsg;

#pragma pack(push, 1)
    typedef struct
    {
        char cFTPIPAdd[LEN_SERVERIP];
        char cFTPPortNum[LEN_PORTNUM];
        char cFTPUsername[LEN_FTPUSERNAME];
        char cFTPPassword[LEN_FTPPASSWRD];
    } TsDefaultConfig;
#pragma pack(pop)

    typedef union
    {
        struct
        {
            bool bLPWRRST_FLAG : 1;
            bool bWWDGRST_FLAG : 1;
            bool bIWDGRST_FLAG : 1;
            bool bSFTRST_FLAG : 1;
            bool bPWRRST_FLAG : 1;
            bool bPINRST_FLAG : 1;
            bool bOBLRST_FLAG : 1;
        };
        uint8_t ucData[1];
    } TsAPPRST;

    // *****************************************************************************/
    //   Structure
    // *****************************************************************************/

    //---------------------------------------------------------------------------

    void SystemClock_Config(void);
    void Error_Handler();
    void vAPP_eJumpToAppAddr_Exe(uint32_t ulAppAddr);
    uint8_t ucAPP_eASCIIChar2HexChar_Exe(uint8_t ucVal);
    void vAPP_eFeedTheWDT_Exe(void);


#ifdef __cplusplus
    extern "C"
    {
#endif

#endif
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
    }
#endif

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
#endif