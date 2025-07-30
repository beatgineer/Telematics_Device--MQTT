// ============================================================================
// FTP.h
// Author : AK
// Date   : 10-04-2014
// ============================================================================

#ifndef __FTP_H
#define __FTP_H

#include "stm32g0xx_hal.h"
#include "stm32g0xx.h"
#include <stdbool.h>

extern uint32_t ulFTP_eCRCVal, ulFTP_eCRCValFrmFTP[];

#define FW_UPDATE_HRS_MIN 02U
#define FW_UPDATE_HRS_MAX 03U

#define CANID_IMMOM_FW_UPDATE 0x300

typedef enum
{
    FTP_Standalone_IoT_ROOT_FOLDER = 0, // 0
    FTP_Standalone_IoT_VERSION_FOLDER,  // 1
    VERSION_FILE_NAME,                  // 2
    IMG1_FILE_NAME,                     // 3
    IMG2_FILE_NAME,                     // 4
    IMG1_CRC_FILE_NAME,                 // 5
    IMG2_CRC_FILE_NAME,                 // 6

    // FTP_Standalone_IoT_ROOT_FOLDER = 0, // 0
    // FTP_Standalone_IoT_VERSION_FOLDER,  // 1
    // FTP_IMMOB_ROOT_FOLDER,              // 2
    // FTP_IMMOB_VERSION_FOLDER,           // 3
    // VERSION_FILE_NAME,                  // 4
    // IMG1_FILE_NAME,                     // 5
    // IMG2_FILE_NAME,                     // 6
    // IMG1_CRC_FILE_NAME,                 // 7
    // IMG2_CRC_FILE_NAME,                 // 8

    // IMMOB_REGULAR_DATA = 0,
    // IMMOB_READY_FOR_FW_UPDATE,
    // IMMOB_CRC_FILE_RECEIVED,
    // IMMOB_START_BIN_FILE,
    // IMMOB_BIN_FILE_RECEIVING,
    // IMMOB_BIN_FILE_RECEIVED,
    // IMMOB_FW_UPDATE_FAILED,

    Standalone_IoT = 0,
} Enum_FTPMsg;

typedef enum
{
    VERSION_NOT_CHECKED = 0,
    SAME_VERSION_FOUND,
    DIFFERENT_VERSION_FOUND,
} Enum_FTPVerStatus;

typedef enum
{
    NO_FWUPDATE = 0,
    CHECKSUM_NOT_MATCHING,
    CHECKSUM_MATCHING,
    IMMOB_FIRMWARE_UPDATE_OK,
    IMMOB_FIRMWARE_UPDATE_OK_BUT_BIN_FILE_CLOSING_ERROR,
    IMMOB_FIRMWARE_UPDATE_OK_BUT_BIN_FILE_DELETE_ERROR, // 5
    FOLDER_SETTING_ERROR_AT_FTP,
    FILE_SEARCH_ERROR_AT_FTP,
    FILE_DOWNLOAD_ERROR,
    FILE_ERROR_IN_GSM_MODULE,
    FILE_IN_GSM_MODULE_IS_OK, // 10
    CRC_FILE_NOT_FOUND,
    CRC_FILE_OPENING_ERROR,
    CRC_FILE_READING_ERROR,
    CRC_FILE_READING_OK,
    CRC_FILE_END_NOT_OK,
    BIN_FILE_RECEIVED_OK,
    BIN_FILE_RECEIVED_FAIL,
    BIN_FILE_READING_ERROR,
    VALID_IMAGE_NOT_FOUND,
    DATA_TIME_OUT_ERROR,
    IMMOB_NOT_READY_FOR_FW,
} Enum_FWUpdateMsg;

typedef enum
{
    FW_UPDATE_NOT_REQD = 0,
    FW_UPDATE_REQD,

    FW_UPDATE_STATUS_NOT_CHECKED = 0,
    FW_UPDATE_STATUS_READY_TO_CHECK,
    FW_UPDATE_STATUS_CHECKED_TODAY,
    FW_UPDATE_FOUND,

    VERSION_TYPE_FILE = 0,
    CRC_TYPE_FILE,
    BIN_TYPE_FILE,

    NO_FW_STATUS = 0,
    CRC_AND_BIN_FILES_DOWNLOAD_OK,
    CRC_AND_BIN_FILES_DOWNLOAD_FAIL,
} Enum_FTPFileMsg;

#pragma pack(push, 1)
typedef struct
{
    uint32_t ulCRCVal;
    uint32_t ulCRCValFrmFTP[52];
    uint32_t ulCalCRCVal[52];
    uint32_t ulFileHandle;
    uint32_t ulFTPFileSize;

    uint8_t ucCRCFileStatusInGSM; // 0 - No File, 1 - File Stored and CRC Checked
    char cCurrentVersion[5];
    char cNewVersion[5];

    uint8_t ucFWCheckedDate;
    uint8_t ucNewAPPNum;
    uint8_t ucCurrentAPPNum;
    char cFilePathAtFTP[35];
    char cBINFileName[15];
    char cCRCFileName[15];
    uint8_t ucCRCFileItems;

    uint8_t GSMTime;
    uint8_t GSMDate;
} TsFTPData;
#pragma pack(pop)

typedef union
{
    struct
    {
        bool bFTPLogIn : 1;
        bool bFTPReadVerFile : 1;
        uint8_t ucVersionStatus : 2;         // 0 - VERSION_NOT_CHECKED
                                             // 1- SAME_VERSION_FOUND,
                                             // 2- DIFFERENT_VERSION_FOUND
        bool bCRCFileStatus : 1;             // 0 - error, 1 - Ok
        bool bClusterBINFileStatusInGSM : 1; // 0 - No File, 1 - File Stored and CRC Checked
        bool bImmobBINFileStatusInGSM : 1;   // 0 - No File, 1 - File Stored and CRC Checked

        uint8_t ucFWStatus : 2; // 0 - VERSION_NOT_CHECKED
    };
    uint8_t ucData[2];
} TuFTPStatus;

// void vFTP_eInit(void);
// void vFTP_eRunFTP_Exe();
// void vFTP_eCheckFTPServerForFWUpdate_Exe(void);
// void vFTP_eCheckFileAvailableInGSMModuleUFS_Exe(char *cFileName, uint32_t ulFileSize);
// bool bFTP_eUpdateClusterFirmware_Exe(void);
bool bFTP_eConnectFTPServer_Exe(void);
Enum_FWUpdateMsg ucFTP_eUpdateImmobFirmware_Exe(void);
void vFTP_eFTPClose_Exe(void);
Enum_FWUpdateMsg ucFTP_eDownloadCRCDatafrmFTP2GSMUFS_Exe(uint8_t ucDevice);
Enum_FWUpdateMsg ucFTP_eDownloadBINFilefrmFTP2GSMUFS_Exe(uint8_t ucDevice);
bool bFTP_eVerifyCheckSumOfNewBINFileInGSMModule_Exe(char *cBINFileName);
bool bFTP_eConnectAndDownloadFiles_Exe(void);
bool bFTP_eWriteNewHexFileInMCU_Exe(uint8_t ucAPPNum, char *cFileName);
uint16_t uiFTP_eDeleteGSMUFSFile_Exe(char *pFTPFile);

#endif

/***** END OF FILE ****/
