// ============================================================================
// FTP.c
// Author : AK
// Date   : 09-08-2018
// ============================================================================
#include "APP.h"
#include "COMM.h"
#include "main.h"
#include "GSM.h"
#include "MCUFlash.h"
#include "FTP.h"
#include "EEPROM.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

TsFTPData FTPData;
TuFTPStatus FTPStatus;
extern TsGSMData GSMData;
extern TsGSMStatus GSMStatus;
extern TsDefaultConfig DEFAULTCONFIG;

extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart3;
extern char msg[128]; // Debug message buffer
extern UART_HandleTypeDef huart1;

static bool bFTP_iInitFTP_Exe(uint8_t ucContextID);
static bool bFTP_iFTPLogin_Exe(char *pFTPAdr, char *pFTPPortNum);
static bool bFTP_iSetFolderAtFTPServer_Exe(char *cFileFolder);
static bool bFTP_iCheckFileAtFTPServer_Exe(char *pFTPFile);
static uint8_t ucFTP_iCheckFileAtGSMUFS_Exe(char *pFileName, uint32_t ulFileSize);
static bool bFTP_iGetFileFromFTPServer_Exe(char *pFTPFile, uint32_t ulFileSize);
static void vFTP_iOpenGSMUFSFile_Exe(char *pFTPFile);
static bool bFTP_iSeekGSMUFSFile_Exe(uint32_t ulFileHandle, uint32_t ulOffset, uint8_t ucPosition);
static uint32_t ulFTP_iReadGSMUFSFile_Exe(uint16_t uiFileHandle, uint16_t uiBytesToRead);
static bool bFTP_iCloseGSMUFSFile_Exe(uint32_t ulFileHandle);
static bool bFTP_iGetFTPFilePathAndFileName_Exe(uint8_t ucDevice);
static Enum_FWUpdateMsg ucFTP_iDownloadFileFromFTP2GSMUFS_Exe(char *cFolder, char *cFileName);
static bool bFTP_iWriteAndVerifyCRCOfMCUFlash_Exe(uint8_t ucAPPNum, uint32_t ulRAMDataAddr, uint16_t uiPageNum);
static bool bFTP_iWriteAndVerifyMCUFlashLastPage_Exe(uint8_t ucAPPNum, uint32_t ulRAMDataAddr, uint32_t ulBytesAlreadyWritten, uint16_t uiBytesToWrite);
static uint32_t ulFTP_iDownloadAndReadFile_Exe(char *cFolder, char *cFile);
static uint16_t uiFTP_iReadFileAndCalculateNumOfPages_Exe(char *cFileName);
static Enum_FTPVerStatus ucFTP_iCompareFWVer_Exe(char *cFolder, char *cFile, char *cSWVer);

//======================================================================
const uint8_t ucFTP_iStartImmobFWUpdate[8] = {0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
const uint8_t ucCAN_iEndCRCFile[8] = {0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xEF, 0xFE, 0xED};
const uint8_t ucCAN_iStartBINFile[8] = {0x9A, 0xAB, 0xBC, 0xCD, 0xEF, 0xFE, 0xED, 0xDC};
const uint8_t ucAPP_iAck[8] = {'A', 'C', 'K', 0x0, 0x0, 0x0, 0x0, 0x0};

const char *cFTP_iMsgTbl[] =
    {
        "/Standalone_IoT/",         // 0                    //folder name
        "/Standalone_IoT/VERSION/", // 1                    //subfolder
        "version.txt",              // 2
        "IMG1.BIN",                 // 3
        "IMG2.BIN",                 // 4
        "IMG1_CRC.txt",             // 5
        "IMG2_CRC.txt",             // 6

        // "/Standalone_IoT/",         // 0                    //folder name
        // "/Standalone_IoT/VERSION/", // 1                    //subfolder
        // "/IMMOB/",                  // 2
        // "/IMMOB/VERSION/",          // 3
        // "version.txt",              // 4
        // "IMG1.BIN",                 // 5
        // "IMG2.BIN",                 // 6
        // "IMG1_CRC.txt",             // 7
        // "IMG2_CRC.txt",             // 8
};

bool bFTP_eConnectAndDownloadFiles_Exe(void)
{
    bool bResp = false;
    uint8_t ucResp, ucStatus = SAME_VERSION_FOUND;

    // 1) Connect to FTP server
    // sprintf(msg, "[BL:FTP] Connecting to FTP server...\r\n");
    // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    bResp = bFTP_eConnectFTPServer_Exe();
    // sprintf(msg, "[BL:FTP] bFTP_eConnectFTPServer_Exe returned: %d\r\n", bResp);
    // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

    if (bResp)
    {
        // 2) Decide if we need an update
        sprintf(msg, "[BL:FTP] Update trigger flag: %u\r\n", ucAPP_eVerUpdate);
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

        if (ucAPP_eVerUpdate == SMS_IOT_FW_VERSION)
        {
            ucStatus = DIFFERENT_VERSION_FOUND;
            sprintf(msg, "[BL:FTP] SMS-triggered update → skipping version check\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        }
        else if (ucAPP_eVerUpdate == CHK_VERSION_UPDATE)
        {
            // sprintf(msg, "[BL:FTP] Comparing current vs. server version...\r\n");
            // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

            ucStatus = ucFTP_iCompareFWVer_Exe(
                (char *)cFTP_iMsgTbl[FTP_Standalone_IoT_VERSION_FOLDER],
                (char *)cFTP_iMsgTbl[VERSION_FILE_NAME],
                FTPData.cCurrentVersion);
            // sprintf(msg, "[BL:FTP] Version compare result: %u\r\n", ucStatus);
            // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

            if (ucStatus == SAME_VERSION_FOUND)
            {
                bResp = false;
                // sprintf(msg, "[BL:FTP] Firmware up-to-date, aborting download\r\n");
                // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            }
            else
            {
                // sprintf(msg, "[BL:FTP] New firmware available\r\n");
                // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            }
        }
        else
        {
            bResp = false;
            sprintf(msg, "[BL:FTP] No valid update request, aborting\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        }

        // 3) If new version found, download and verify
        if (ucStatus == DIFFERENT_VERSION_FOUND)
        {
            // sprintf(msg, "[BL:FTP] Downloading CRC file...\r\n");
            // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            ucResp = ucFTP_eDownloadCRCDatafrmFTP2GSMUFS_Exe(Standalone_IoT);
            // sprintf(msg, "[BL:FTP] CRC download response: %u\r\n", ucResp);
            // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

            if (ucResp == CRC_FILE_READING_OK)
            {
                // sprintf(msg, "[BL:FTP] Downloading BIN file...\r\n");
                // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
                ucResp = ucFTP_eDownloadBINFilefrmFTP2GSMUFS_Exe(Standalone_IoT);
                // sprintf(msg, "[BL:FTP] BIN download response: %u\r\n", ucResp);
                // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

                if (ucResp == FILE_IN_GSM_MODULE_IS_OK)
                {
                    if (FTPData.ucNewAPPNum)
                    {
                        // sprintf(msg, "[BL:FTP] Verifying BIN checksum...\r\n");
                        // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
                        bResp = bFTP_eVerifyCheckSumOfNewBINFileInGSMModule_Exe(FTPData.cBINFileName);
                        sprintf(msg, "[BL:FTP] Checksum verify result: %d\r\n", bResp);
                        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
                    }
                    else
                    {
                        bResp = false;
                        sprintf(msg, "[BL:FTP] No new APP number set, abort\r\n");
                        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
                    }
                }
                else
                {
                    bResp = false;
                    sprintf(msg, "[BL:FTP] BIN file invalid in module, abort\r\n");
                    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
                }
            }
            else
            {
                bResp = false;
                sprintf(msg, "[BL:FTP] CRC download failed, abort\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            }
        }
    }
    else
    {
        sprintf(msg, "[BL:FTP] FTP connect failed, aborting all\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }

    // sprintf(msg, "[BL:FTP] Exit bFTP_eConnectAndDownloadFiles_Exe → %d\r\n", bResp);
    // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    return bResp;
}

bool bFTP_eConnectFTPServer_Exe(void)
{
    bool bResp;

    // Debug: closing any existing FTP session
    sprintf(msg, "[BL:FTP] Closing existing FTP session\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    vFTP_eFTPClose_Exe();

    // Debug: initializing FTP with PDP context ID
    // sprintf(msg, "[BL:FTP] Initializing FTP with context ID %d\r\n", PDP_CONTEXT_ID);
    // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    bResp = bFTP_iInitFTP_Exe(PDP_CONTEXT_ID);
    // sprintf(msg, "[BL:FTP] bFTP_iInitFTP_Exe returned: %d\r\n", bResp);
    // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

    if (bResp)
    {
        // Debug: attempting FTP login
        // sprintf(msg, "[BL:FTP] Logging in to FTP server %s:%s\r\n", DEFAULTCONFIG.cFTPIPAdd, DEFAULTCONFIG.cFTPPortNum);
        // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

        bResp = bFTP_iFTPLogin_Exe(DEFAULTCONFIG.cFTPIPAdd, DEFAULTCONFIG.cFTPPortNum);
        // sprintf(msg, "[BL:FTP] bFTP_iFTPLogin_Exe returned: %d\r\n", bResp);
        // HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }
    else
    {
        // Debug: FTP init failed
        sprintf(msg, "[BL:FTP] FTP initialization failed, aborting login\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }

    return bResp;
}

// ============================================================================
// Name 		: vFTP_eFTPClose_Exe
// Objective	: Close FTP Server
// Input		: none
// Output		: none
// Return		: none
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
void vFTP_eFTPClose_Exe(void)
{
    ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QFTCLOSE, ATRESPONSE_OK, 300, 10000);
}

// ============================================================================
// Name			: bFTP_iInitFTP_Exe
// Objective	: Initialise FTP Server
//				  1. Context ID			GPRS Context
//				  2. FTP Account 		LogIn ID & Password
//				  3. File Type 			Binary
//				  4. Transparent Mode	Passive
//				  5. Response Time Out
// Input  		: none
// Output 		: none
// Return		: 0 - Fail, 1 - Success
// Version		: -
// Author		: AK
// Date			: 12 Jul 2019
// ============================================================================
static bool bFTP_iInitFTP_Exe(uint8_t ucContextID)
{
    uint8_t ucResp = 0;
    bool bFlag = false;
    char cBuff[100];
    char dbg[128];

    // --- Debug: function entry ---
    // snprintf(dbg, sizeof(dbg), "\r\n[DEBUG] bFTP_iInitFTP_Exe start, ContextID=%u\r\n", ucContextID);
    // HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

    // Context ID
    snprintf(cBuff, sizeof(cBuff), "%s%d\r", (char *)cGSM_eATCommandTbl[ATCOMMAND_QFTPCFGCONID], ucContextID);

    // snprintf(dbg, sizeof(dbg), "[DEBUG] Sending: %s", cBuff);
    // HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

    ucResp = ucGSM_eProcessATCmdWithCmdPtr_Exe(cBuff, ATRESPONSE_OK, 300, 1000);

    snprintf(dbg, sizeof(dbg), "[DEBUG] Response for QFTPCFGCONID: %s (%u)\r\n",
             ucResp == RESPONSE_MATCHING ? "OK" : "FAIL", ucResp);
    HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

    if (ucResp == RESPONSE_MATCHING)
    {
        // FTP Account
        snprintf(cBuff, sizeof(cBuff), "%s\"%s\",\"%s\"\r",
                 (char *)cGSM_eATCommandTbl[ATCOMMAND_QFTPCFGACCOUNT],
                 DEFAULTCONFIG.cFTPUsername,
                 DEFAULTCONFIG.cFTPPassword);
        snprintf(dbg, sizeof(dbg), "[DEBUG] Sending: %s", cBuff);
        HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

        ucResp = ucGSM_eProcessATCmdWithCmdPtr_Exe(cBuff, ATRESPONSE_OK, 300, 1000);
        snprintf(dbg, sizeof(dbg), "[DEBUG] Response for QFTPCFGACCOUNT: %s (%u)\r\n",
                 ucResp == RESPONSE_MATCHING ? "OK" : "FAIL", ucResp);
        HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

        if (ucResp == RESPONSE_MATCHING)
        {
            // File Type
            snprintf(dbg, sizeof(dbg), "[DEBUG] Configuring FILETYPE\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

            ucResp = ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QFTPCFGFILETYPE, ATRESPONSE_OK, 300, 1000);
            snprintf(dbg, sizeof(dbg), "[DEBUG] Response for FILETYPE: %s (%u)\r\n",
                     ucResp == RESPONSE_MATCHING ? "OK" : "FAIL", ucResp);
            HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

            if (ucResp == RESPONSE_MATCHING)
            {
                // Transparent Mode - Passive
                // snprintf(dbg, sizeof(dbg), "[DEBUG] Configuring TRANSMODE\r\n");
                // HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

                ucResp = ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QFTPCFGTRANSMODE, ATRESPONSE_OK, 300, 1000);
                // snprintf(dbg, sizeof(dbg), "[DEBUG] Response for TRANSMODE: %s (%u)\r\n",
                //          ucResp == RESPONSE_MATCHING ? "OK" : "FAIL", ucResp);
                // HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

                if (ucResp == RESPONSE_MATCHING)
                {
                    // Response Time Out
                    // snprintf(dbg, sizeof(dbg), "[DEBUG] Configuring RSPTOUT\r\n");
                    // HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

                    ucResp = ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QFTPCFGRSPTOUT, ATRESPONSE_OK, 300, 1000);
                    // snprintf(dbg, sizeof(dbg), "[DEBUG] Response for RSPTOUT: %s (%u)\r\n",
                    //          ucResp == RESPONSE_MATCHING ? "OK" : "FAIL", ucResp);
                    // HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

                    if (ucResp == RESPONSE_MATCHING)
                    {
                        bFlag = true;
                        HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] FTP init successful\r\n", strlen("[DEBUG] FTP init successful\r\n"), HAL_MAX_DELAY);
                    }
                    else
                    {
                        HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] RSPTOUT failed\r\n", strlen("[DEBUG] RSPTOUT failed\r\n"), HAL_MAX_DELAY);
                    }
                }
                else
                {
                    HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] TRANSMODE failed\r\n", strlen("[DEBUG] TRANSMODE failed\r\n"), HAL_MAX_DELAY);
                }
            }
            else
            {
                HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] FILETYPE failed\r\n", strlen("[DEBUG] FILETYPE failed\r\n"), HAL_MAX_DELAY);
            }
        }
        else
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] ACCOUNT config failed\r\n", strlen("[DEBUG] ACCOUNT config failed\r\n"), HAL_MAX_DELAY);
        }
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] CONTEXT ID config failed\r\n", strlen("[DEBUG] CONTEXT ID config failed\r\n"), HAL_MAX_DELAY);
    }

    // snprintf(dbg, sizeof(dbg), "[DEBUG] bFTP_iInitFTP_Exe end, returning %s\r\n", bFlag ? "true" : "false");
    // HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), HAL_MAX_DELAY);

    return bFlag;
}

// ============================================================================
// Name 		: bFTP_eFTPLogin_Exe
// Objective	: Login into FTP Server
//
// Input		: FTP IP Address or Host Name, Port Num
// Output		: none
// Return		: Login Status
//					0 - fail, 1 - success
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static bool bFTP_iFTPLogin_Exe(char *pFTPAdr, char *pFTPPortNum)
{
    uint8_t ucResp;
    char cBuff[100];
    bool bFlag;

    bFlag = false;
    sprintf(cBuff, "%s\"%s\",%s\r",
            cGSM_eATCommandTbl[ATCOMMAND_QFTPOPEN],
            pFTPAdr,
            pFTPPortNum);

    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_OK, 6, ATRESPONSE_QFTPOPEN, 18, 300, 90000);
    if (ucResp == BOTH_RESPONSES_MATCHING)
    {
        bFlag = true;
    }
    else
    {
        bFlag = false;
    }
    return bFlag;
}

// ============================================================================
// Name 		: ucFTP_iDownloadCRCDatafrmFTP2GSMModule_Exe
// Objective	: Download CRC File in UFS File
// Input		: File Handle
// Output		: none
// Return		:
// 					6 - CRC_FILE_NOT_FOUND,
//					7 - CRC_FILE_OPENING_ERROR,
//					8 - CRC_FILE_READING_ERROR,
//					9 - CRC_FILE_READING_OK,
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
Enum_FWUpdateMsg ucFTP_eDownloadCRCDatafrmFTP2GSMUFS_Exe(uint8_t ucDevice)
{
    bool bResp = 0;
    char *cBuff = 0, *cBuff1 = 0;
    uint16_t uiLen = 0, uiTotalLen = 0;
    uint8_t ucCntr = 0, ucCntr1 = 0, ucHexVal = 0;
    uint32_t ulVal = 0, ulAddr = 0;

    Enum_FWUpdateMsg ucResp = NO_FWUPDATE;

    bResp = bFTP_iGetFTPFilePathAndFileName_Exe(ucDevice);
    if (bResp == false)
    {
        ucResp = VALID_IMAGE_NOT_FOUND;
    }
    else
    {
        ulAddr = ulFTP_iDownloadAndReadFile_Exe(FTPData.cFilePathAtFTP, FTPData.cCRCFileName);

        cBuff = (char *)ulAddr;
        if (cBuff != NULL)
        {
            uiTotalLen = 0;
            ucCntr = 0;
            do
            {
                cBuff1 = strchr(cBuff, ',');
                if (cBuff1 != 0)
                {
                    uiLen = cBuff1 - cBuff;
                    uiTotalLen += uiLen;
                    uiTotalLen++;
                    ulVal = 0;

                    for (ucCntr1 = 0; ucCntr1 < uiLen; ucCntr1++)
                    {
                        ulVal = ulVal << 4;
                        ucHexVal = ucAPP_eASCIIChar2HexChar_Exe(*(cBuff + ucCntr1));
                        ulVal |= ucHexVal;
                    }

                    FTPData.ulCRCValFrmFTP[ucCntr] = ulVal;

                    cBuff = ++cBuff1;
                    bResp = true;
                }
                else
                {
                    bResp = false;
                }
                ucCntr++;
            } while ((bResp == true) && (uiTotalLen < FTPData.ulFTPFileSize));

            if (bResp == true)
            {
                ucResp = CRC_FILE_READING_OK;
                FTPData.ucCRCFileItems = ucCntr;
            }
            else
            {
                ucResp = CRC_FILE_READING_ERROR;
            }

            bResp = bFTP_iCloseGSMUFSFile_Exe(FTPData.ulFileHandle);
        }
        else
        {
            ucResp = CRC_FILE_NOT_FOUND;
        }

        uiFTP_eDeleteGSMUFSFile_Exe(FTPData.cCRCFileName);
    }

    return ucResp;
}

static bool bFTP_iGetFTPFilePathAndFileName_Exe(uint8_t ucDevice)
{
    bool bResp = false;

    if (ucDevice == Standalone_IoT)
    {
        if (FTPData.ucCurrentAPPNum)
        {
            sprintf(FTPData.cFilePathAtFTP, "%s%s", cFTP_iMsgTbl[FTP_Standalone_IoT_ROOT_FOLDER], FTPData.cNewVersion);
            bResp = true;
        }
        else
        {
            bResp = false;
        }
    }
    /* else if (ucDevice == IMMOB)
     {
                 FTPData.ucCurrentAPPNum = APPCONFIG.ucImmobActiveIMG;
                 if(FTPData.ucCurrentAPPNum)
                 {
                     sprintf(FTPData.cFilePathAtFTP, "%s%s", cFTP_iMsgTbl[FTP_IMMOB_ROOT_FOLDER], FTPData.cVersion);
                     bResp = true;
                 }
                 else
                 {
                     bResp = false;
                 }
     }*/
    else
    {
        bResp = false;
    }

    if (bResp == true)
    {
        // Which is the Active IMG ?
        if (FTPData.ucCurrentAPPNum == VALID_APP1)
        {
            strcpy(FTPData.cBINFileName, cFTP_iMsgTbl[IMG2_FILE_NAME]);
            strcpy(FTPData.cCRCFileName, cFTP_iMsgTbl[IMG2_CRC_FILE_NAME]);
            FTPData.ucNewAPPNum = 2;
        }
        else if (FTPData.ucCurrentAPPNum == VALID_APP2)
        {
            strcpy(FTPData.cBINFileName, cFTP_iMsgTbl[IMG1_FILE_NAME]);
            strcpy(FTPData.cCRCFileName, cFTP_iMsgTbl[IMG1_CRC_FILE_NAME]);
            FTPData.ucNewAPPNum = 1;
        }
        else
        {
            FTPData.ucNewAPPNum = 0;
        }
    }
    else
    {
    }

    return bResp;
}

// ============================================================================
// Name			: ucFTP_iDownloadBINFilefrmFTP2GSMUFS_Exe
// Objective	: Download Updated BIN File from FTP to GSM Module
//
// Input  		: none
// Output 		: Valid IMG File Num at APPData.ucValidAPPNum
//					0 - Invalid, 1 - IMG1, 2 - IMG2
// Return 		: 	1 - FOLDER_SETTING_ERROR_AT_FTP
//					2 - FILE_SEARCH_ERROR_AT_FTP,
//					3 - FILE_DOWNLOAD_ERROR,
//					4 - FILE_ERROR_IN_GSM_MODULE,
//					5 - FILE_IN_GSM_MODULE_IS_OK,
// Author		: AK
// Date			: 18 Sep 2019
// ============================================================================
Enum_FWUpdateMsg ucFTP_eDownloadBINFilefrmFTP2GSMUFS_Exe(uint8_t ucDevice)
{
    Enum_FWUpdateMsg ucResp;
    bool bResp;

    bResp = bFTP_iGetFTPFilePathAndFileName_Exe(ucDevice);

    if (bResp == true)
    {
        ucResp = ucFTP_iDownloadFileFromFTP2GSMUFS_Exe(FTPData.cFilePathAtFTP, FTPData.cBINFileName);
    }
    else
    {
        ucResp = VALID_IMAGE_NOT_FOUND;
    }
    return ucResp;
}

// ============================================================================
// Name			: bFTP_iVerifyCheckSumOfNewBINFileInGSMModule_Exe
// Objective	: Update BIN File in MCU
//
// Input  		: none
// Output 		: none
// Return		: 0 - Fail, 1 - Success
// Author		: AK
// Date			: 18 Sep 2019
// ============================================================================
bool bFTP_eVerifyCheckSumOfNewBINFileInGSMModule_Exe(char *cBINFileName)
{
    bool bStatus;
    uint32_t ulRAMDataPointer;
    uint16_t uiPages, uiPageNum;

    uiPages = uiFTP_iReadFileAndCalculateNumOfPages_Exe(cBINFileName);

    if (uiPages != 0xFFFF)
    {
        uiPageNum = 0U;

        do
        {
            // Read Data
            ulRAMDataPointer = ulFTP_iReadGSMUFSFile_Exe(FTPData.ulFileHandle, MCU_FLASH_PAGE_SIZE);

            if (ulRAMDataPointer != NULL)
            {
                FTPData.ulCRCVal = HAL_CRC_Calculate(&hcrc, (uint32_t *)ulRAMDataPointer, MCU_FLASH_PAGE_SIZE);
                FTPData.ulCalCRCVal[uiPageNum] = FTPData.ulCRCVal;

                if (FTPData.ulCRCVal == FTPData.ulCRCValFrmFTP[uiPageNum])
                {
                    bStatus = true;
                }
                else
                {
                    bStatus = false;
                }
            }
            else
            {
                bStatus = false;
            }
            uiPageNum++;
            HAL_Delay(10);
        } while ((uiPageNum < uiPages) && (bStatus == true));
    }
    else
    {
        bStatus = false;
    }

    bFTP_iCloseGSMUFSFile_Exe(FTPData.ulFileHandle);

    return bStatus;
}

// ============================================================================
// Name			: bFTP_eWriteNewHexDataInMCU_Exe
// Objective	: Update BIN File in MCU
//
// Input  		: none
// Output 		: none
// Return		: 0 - Fail, 1 - Success
// Author		: AK
// Date			: 18 Sep 2019
// ============================================================================

#if 0
bool bFTP_eWriteNewHexFileInMCU_Exe(uint8_t ucAPPNum, char *cFileName)
{
    bool bStatus = true;
    uint32_t fileSize = FTPData.ulFTPFileSize;
    uint32_t pageSz = MCU_FLASH_PAGE_SIZE;                // 2048
    uint16_t fullPages = fileSize / pageSz;               // floor
    uint16_t totalPg = (fileSize + pageSz - 1U) / pageSz; // ceil
    uint16_t pendBytes = fileSize - (fullPages * pageSz);

    uint16_t uiPageNum = 0;
    uint32_t ulRAMDataPtr = 0; // keep as uint32_t since APIs expect that
    uint32_t bytesWritten;

    // (Optional) still call helper, but ignore if different
    if (uiFTP_iReadFileAndCalculateNumOfPages_Exe(cFileName) == 0xFFFF)
        return false;

    // -------- full pages --------
    for (uiPageNum = 0; uiPageNum < fullPages && bStatus; uiPageNum++)
    {
        ulRAMDataPtr = ulFTP_iReadGSMUFSFile_Exe(FTPData.ulFileHandle, pageSz);
        if (!ulRAMDataPtr)
        {
            bStatus = false;
            break;
        }

        bStatus = bFTP_iWriteAndVerifyCRCOfMCUFlash_Exe(
            ucAPPNum,
            ulRAMDataPtr, // uint32_t OK
            uiPageNum);
    }

    // -------- last partial --------
    if (bStatus && pendBytes)
    {
        // read only remainder
        ulRAMDataPtr = ulFTP_iReadGSMUFSFile_Exe(FTPData.ulFileHandle, pendBytes);
        if (!ulRAMDataPtr)
        {
            bStatus = false;
        }
        else
        {
            // pad to 8-byte boundary for DW programming
            uint32_t padded = (pendBytes + 7U) & ~7U; // round up
            static uint8_t padBuf[MCU_FLASH_PAGE_SIZE];
            memcpy(padBuf, (void *)ulRAMDataPtr, pendBytes);
            memset(padBuf + pendBytes, 0xFF, padded - pendBytes);

            bytesWritten = uiPageNum * pageSz; // pages actually done!

            bStatus = bFTP_iWriteAndVerifyMCUFlashLastPage_Exe(
                ucAPPNum,
                (uint32_t)padBuf, // cast pointer → uint32_t
                bytesWritten,
                padded); // write padded length
        }
    }

    bFTP_iCloseGSMUFSFile_Exe(FTPData.ulFileHandle);
    return bStatus;
}
#endif

#if 1
bool bFTP_eWriteNewHexFileInMCU_Exe(uint8_t ucAPPNum, char *cFileName)
{
    bool bStatus = false;
    uint16_t uiPages = 0, uiPageNum = 0, uiPendingBytes = 0;
    uint32_t ulBytesAlreadyWritten = 0, ulRAMDataPointer = 0;
    char dbgBuf[64];

    // 1) how many pages?
    uiPages = uiFTP_iReadFileAndCalculateNumOfPages_Exe(cFileName);
    snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG: Total pages to write = %u\r\n", uiPages);
    HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);

    if (uiPages != 0xFFFF)
    {
        uiPageNum = 0U;

        do
        {
            // 2) read next chunk
            // snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG: Reading page %u...\r\n", uiPageNum);
            // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);

            ulRAMDataPointer = ulFTP_iReadGSMUFSFile_Exe(FTPData.ulFileHandle, MCU_FLASH_PAGE_SIZE);

            if (ulRAMDataPointer != NULL)
            {
                // snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG: Write & CRC check page %u\r\n", uiPageNum);
                // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);

                bStatus = bFTP_iWriteAndVerifyCRCOfMCUFlash_Exe(ucAPPNum, ulRAMDataPointer, uiPageNum);
            }
            else
            {
                snprintf(dbgBuf, sizeof(dbgBuf), "ERROR: Failed to read page %u\r\n", uiPageNum);
                HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);
                bStatus = false;
            }
            uiPageNum++;
            // HAL_Delay(10);
            snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG:uiPageNum=%u uiPages=%u\r\n", uiPageNum, uiPages);
            HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);

            vAPP_eFeedTheWDT_Exe();
        } while ((uiPageNum < (uiPages)) && (bStatus == true));

        // 3) after full pages
        if (bStatus == true)
        {
            ulBytesAlreadyWritten = uiPages * MCU_FLASH_PAGE_SIZE;
            uiPendingBytes = FTPData.ulFTPFileSize - ulBytesAlreadyWritten;

            snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG: Full pages done, bytesWritten=%lu, pending=%u\r\n",
                     ulBytesAlreadyWritten, uiPendingBytes);
            HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);

            if (uiPendingBytes)
            {
                snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG: Reading last %u bytes\r\n", uiPendingBytes);
                HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);

                ulRAMDataPointer = ulFTP_iReadGSMUFSFile_Exe(FTPData.ulFileHandle, uiPendingBytes);
                if (ulRAMDataPointer != NULL)
                {
                    bStatus = bFTP_iWriteAndVerifyMCUFlashLastPage_Exe(
                        ucAPPNum, ulRAMDataPointer,
                        ulBytesAlreadyWritten, uiPendingBytes);
                }
                else
                {
                    HAL_UART_Transmit(&huart1,
                                      (uint8_t *)"ERROR: Read last page failed\r\n",
                                      27, 100);
                    bStatus = false;
                }
            }
            else
            {
                bStatus = true;
            }
        }
        else
        {
            HAL_UART_Transmit(&huart1,
                              (uint8_t *)"ERROR: Page loop aborted\r\n",
                              23, 100);
            bStatus = false;
        }
    }
    else
    {
        HAL_UART_Transmit(&huart1,
                          (uint8_t *)"ERROR: Invalid page count\r\n",
                          26, 100);
        bStatus = false;
    }

    bFTP_iCloseGSMUFSFile_Exe(FTPData.ulFileHandle);

    // 4) final status
    snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG: bFTP_eWriteNewHexFileInMCU_Exe returning %s\r\n",
             bStatus ? "TRUE" : "FALSE");
    HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), 100);

    return bStatus;
}
#endif
// ============================================================================
// Name 		: bFTP_eDeleteGSMModuleUFSFile_Exe
// Objective	: Delete UFS File
// Input		: File Handle
// Output		: CMS or CME Error Code at GSMData.cErrCode[2]
// Return		: 0 - Fail, 1 - Success, Error code in case of CME Error, CMS Error
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
uint16_t uiFTP_eDeleteGSMUFSFile_Exe(char *pFTPFile)
{
    uint8_t ucResp;
    uint16_t uiErrVal;
    char cBuff[100];

    sprintf(cBuff, "%s%s\"\r", cGSM_eATCommandTbl[ATCOMMAND_QFDEL], pFTPFile);
    ucResp = ucGSM_eProcessATCmdWithCmdPtr_Exe(cBuff, ATRESPONSE_OK, 300, 5000);
    if (ucResp == RESPONSE_MATCHING)
    {
        uiErrVal = 1;
    }
    else
    {
        uiErrVal = 0;
    }
    return uiErrVal;
}

// ============================================================================
// Name 		: ucFTP_eCloseUFSFile_Exe
// Objective	: Close Opened UFS File
// Input		: File Handle
// Output		: none
// Return		: 0 - Fail, 1 - Success
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static bool bFTP_iCloseGSMUFSFile_Exe(uint32_t ulFileHandle)
{
    char cBuff[100];
    uint8_t ucResp;
    bool bFlag;

    sprintf(cBuff, "%s%d\r", cGSM_eATCommandTbl[ATCOMMAND_QFCLOSE], ulFileHandle);
    ucResp = ucGSM_eProcessATCmdWithCmdPtr_Exe(cBuff, ATRESPONSE_OK, 300, 1000);
    if (ucResp == RESPONSE_MATCHING)
    {
        bFlag = true;
    }
    else
    {
        bFlag = false;
    }
    return bFlag;
}

// ============================================================================
// Name			: ulFTP_iDownloadAndReadFile_Exe
// Objective	: Download File from FTP to GSM Module UFS and Read
// Input  		: Folder & File Name Pointer
// Output 		: none
// Return 		: 0 - Fail in reading File from FTP,
//				  Non Zero : Ok
// Author		: AK
// Date			: 18 Sep 2019
// ============================================================================
static uint32_t ulFTP_iDownloadAndReadFile_Exe(char *cFolder, char *cFile)
{
    bool bResp = false;
    uint8_t ucResp = 0;
    uint32_t ulAddr = 0;

    ucResp = ucFTP_iDownloadFileFromFTP2GSMUFS_Exe(cFolder, cFile);

    if (ucResp == FILE_IN_GSM_MODULE_IS_OK)
    {
        vFTP_iOpenGSMUFSFile_Exe(cFile);

        if (FTPData.ulFileHandle != 0)
        {
            bResp = bFTP_iSeekGSMUFSFile_Exe(FTPData.ulFileHandle, 0, 0);

            if (bResp == true)
            {
                ulAddr = ulFTP_iReadGSMUFSFile_Exe(FTPData.ulFileHandle, FTPData.ulFTPFileSize);
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
    return ulAddr;
}

// ============================================================================
// Name 		: ucFTP_iGetFileFromFTP_Exe
// Objective	: Download File from FTP to GSM UFS
// Input		: File Name
// Output		: File Size at FTPData.ulFTPFileSize
// Return 		: 	1 - FOLDER_SETTING_ERROR_AT_FTP
//					2 - FILE_SEARCH_ERROR_AT_FTP,
//					3 - FILE_DOWNLOAD_ERROR,
//					4 - FILE_ERROR_IN_GSM_MODULE,
//					5 - FILE_IN_GSM_MODULE_IS_OK,
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static Enum_FWUpdateMsg ucFTP_iDownloadFileFromFTP2GSMUFS_Exe(char *cFolder, char *cFileName)
{
    bool bResp;
    uint8_t ucStatus;

    Enum_FWUpdateMsg ucResp = NO_FWUPDATE;

    bResp = bFTP_iSetFolderAtFTPServer_Exe(cFolder);
    if (true == bResp)
    {
        bResp = bFTP_iCheckFileAtFTPServer_Exe(cFileName);
        if (true == bResp)
        {
            ucStatus = ucFTP_iCheckFileAtGSMUFS_Exe(cFileName, FTPData.ulFTPFileSize);
            uiFTP_eDeleteGSMUFSFile_Exe(cFileName);

            bResp = bFTP_iGetFileFromFTPServer_Exe(cFileName, FTPData.ulFTPFileSize);
            if (true == bResp)
            {
                ucStatus = ucFTP_iCheckFileAtGSMUFS_Exe(cFileName, FTPData.ulFTPFileSize);
                if (ucStatus == FILE_FOUND)
                {
                    ucResp = FILE_IN_GSM_MODULE_IS_OK;
                }
                else
                {
                    ucResp = FILE_ERROR_IN_GSM_MODULE;
                }
            }
            else
            {
                ucResp = FILE_DOWNLOAD_ERROR;
            }
        }
        else
        {
            ucResp = FILE_SEARCH_ERROR_AT_FTP;
        }
    }
    else
    {
        ucResp = FOLDER_SETTING_ERROR_AT_FTP;
    }
    return ucResp;
}

// ============================================================================
// Name 		: vFTP_eOpenGSMModuleUFSFile_Exe
// Objective	: Open File Available in GSM UFS for Read operation
// Input		: File Name
// Output		: 32 Bit File Handle at APPData.ulFileHandle
// Return		: none
// Version	: -
// Author	: AK
// Date 		: 27-07-2019
// ============================================================================
static void vFTP_iOpenGSMUFSFile_Exe(char *pFTPFile)
{
    uint8_t ucResp;
    char cBuff[100], *pAddr1, *pAddr2;

    sprintf(cBuff, "%s\"%s\",0\r", cGSM_eATCommandTbl[ATCOMMAND_QFOPEN], pFTPFile);
    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_QFOPEN, 13, ATRESPONSE_OK, 6, 300, 1000);
    if ((ucResp == BOTH_RESPONSES_MATCHING) || (ucResp == RESPONSE1_MATCHING))
    {
        pAddr1 = strchr((char *)GSM_DataBuffer, ':');
        if (pAddr1 != NULL)
        {
            pAddr1++;
            pAddr2 = strchr(pAddr1, '\r');
            if (pAddr2 != NULL)
            {
                pAddr2 = 0;
                FTPData.ulFileHandle = atol(pAddr1);
            }
            else
            {
                FTPData.ulFileHandle = 0;
            }
        }
        else
        {
            FTPData.ulFileHandle = 0;
        }
    }
    else if (ucResp == RESPONSE_CME_ERROR)
    {
        ucResp = ucGSM_eProcessATCmdWithCmdNum_Exe(ATCOMMAND_QFOPENQUERY, ATRESPONSE_QFOPEN, 200, 1000);
        if (ucResp == RESPONSE_MATCHING)
        {
            pAddr1 = strchr((char *)GSM_DataBuffer, ',');
            if (pAddr1 != NULL)
            {
                pAddr1++;
                pAddr2 = strchr(pAddr1, ',');
                if (pAddr2 != NULL)
                {
                    pAddr2 = 0;
                    FTPData.ulFileHandle = atol(pAddr1);
                }
                else
                {
                    FTPData.ulFileHandle = 0;
                }
            }
            else
            {
                FTPData.ulFileHandle = 0;
            }
        }
        else
        {
            FTPData.ulFileHandle = 0;
        }
    }
    else
    {
        FTPData.ulFileHandle = 0;
    }
}

// ============================================================================
// Name 		: bFTP_eSeekUFSFile_Exe
// Objective	: Set the Current Position of File Pointer
// Input		: File Name, Offset and Position
// Output		: none
// Return		: 0 - Fail, 1 - Success
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static bool bFTP_iSeekGSMUFSFile_Exe(uint32_t ulFileHandle, uint32_t ulOffset, uint8_t ucPosition)
{
    uint8_t ucResp;
    bool bFlag;

    char cBuff[100];

    sprintf(cBuff, "%s%d,%d,%d\r", cGSM_eATCommandTbl[ATCOMMAND_QFSEEK], ulFileHandle, ulOffset, ucPosition);
    ucResp = ucGSM_eProcessATCmdWithCmdPtr_Exe(cBuff, ATRESPONSE_OK, 200, 1000);

    if (ucResp == RESPONSE_MATCHING)
    {
        bFlag = true;
    }
    else
    {
        bFlag = false;
    }
    return bFlag;
}

// ============================================================================
// Name 		: ulFTP_iReadGSMUFSFile_Exe
// Objective	: Read No. of Bytes from the Current Position of File Pointer
// Input		: File Name, No. of Bytes
// Output		: none
// Return		: Pointer of the Data Read
//				  0 - Fail, Non Zero - 32 bit Start Address of RAM Buffer
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static uint32_t ulFTP_iReadGSMUFSFile_Exe(uint16_t uiFileHandle, uint16_t uiBytesToRead)
{
    char cBuff[100], *cBuff1 = 0, *cBuff2 = 0, *cBuff3 = 0;
    uint8_t ucResp;
    uint16_t uiLen;

    sprintf(cBuff, "%s%d,%d\r", cGSM_eATCommandTbl[ATCOMMAND_QFREAD], uiFileHandle, uiBytesToRead);
    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_CONNECT, 8, ATRESPONSE_OK, 6, 200, 1000);
    if ((ucResp == RESPONSE1_MATCHING) ||
        (ucResp == BOTH_RESPONSES_MATCHING))
    {
        cBuff1 = strchr(GSM_DataBuffer, ' ');
        if (cBuff1 != 0)
        {
            cBuff1++;
            cBuff2 = strchr(cBuff1, 0x0D);
            if (cBuff2 != 0)
            {
                *cBuff2 = 0;
                uiLen = atoi(cBuff1);
                if (uiLen == uiBytesToRead)
                {
                    *cBuff2 = 0x0D;
                    cBuff2++;
                    cBuff2++;

                    cBuff1 = cBuff2 + uiBytesToRead;
                    cBuff3 = strstr((char *)cBuff1, cGSM_eATResponseTbl[ATRESPONSE_OK]);
                    if (cBuff3 == 0)
                    {
                        cBuff2 = 0;
                    }
                    else
                    {
                    }
                }
                else
                {
                    cBuff2 = 0;
                }
            }
            else
            {
                cBuff2 = 0;
            }
        }
        else
        {
            cBuff2 = 0;
        }
    }
    else
    {
        cBuff2 = 0;
    }
    return (uint32_t)cBuff2;
}

// ============================================================================
// Name			: uiFTP_iReadAndCalculateNumOfPages_Exe
// Objective	: Read File and Calculate Num of Pages
//
// Input  		: none
// Output 		: none
// Return		: 0xFFFF - Error in File, 1 - 100 : Num of Pages
// Author		: AK
// Date			: 18 Sep 2019
// ============================================================================
static uint16_t uiFTP_iReadFileAndCalculateNumOfPages_Exe(char *cFileName)
{
    bool bResp = false;
    uint16_t uiPages = 0;

    vFTP_iOpenGSMUFSFile_Exe(cFileName);
    if (FTPData.ulFileHandle != 0)
    {
        bResp = bFTP_iSeekGSMUFSFile_Exe(FTPData.ulFileHandle, 0, 0);
        if (true == bResp)
        {
            // Calculate No. of Pages
            uiPages = FTPData.ulFTPFileSize / MCU_FLASH_PAGE_SIZE;
        }
        else
        {
            uiPages = 0xFFFF;
        }
    }
    else
    {
        uiPages = 0xFFFF;
    }
    return uiPages;
}

// ============================================================================
// Name			: bFTP_iWriteAndVerifyCRCOfMCUFlash_Exe
// Objective	: Write and Verify CRC of MCU Flash
//
// Input  		: none
// Output 		: none
// Return		: 0 - Fail, 1 - Success
// Author		: AK
// Date			: 18 Sep 2019
// ============================================================================
static bool bFTP_iWriteAndVerifyCRCOfMCUFlash_Exe(uint8_t ucAPPNum, uint32_t ulRAMDataAddr, uint16_t uiPageNum)
{
    bool bStatus = false;
    uint32_t ulFlashDataAddr = 0, ulMCUFlashAddress;
    uint8_t ucResp = 0;
    char dbgBuf[100];
    int dbgLen;

    // Entry debug
    // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
    //                   "\r\n[DEBUG] Enter bFTP_iWriteAndVerifyCRC... APPNum=%u, RAMAddr=0x%08lX, Page=%u\r\n",
    //                   ucAPPNum, ulRAMDataAddr, uiPageNum);
    // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);

    if (ucAPPNum == VALID_APP1)
    {
        ulMCUFlashAddress = IMAGE1_MCU_FLASH_ADDR;
        bStatus = true;
        // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
        //                   "[DEBUG] Selected VALID_APP1, MCUFlashAddr=0x%08lX\r\n",
        //                   ulMCUFlashAddress);
        // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);
    }
    else if (ucAPPNum == VALID_APP2)
    {
        ulMCUFlashAddress = IMAGE2_MCU_FLASH_ADDR;
        bStatus = true;
        // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
        //                   "[DEBUG] Selected VALID_APP2, MCUFlashAddr=0x%08lX\r\n",
        //                   ulMCUFlashAddress);
        // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);
    }
    else
    {
        ulMCUFlashAddress = 0;
        bStatus = false;
        // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
        //                   "[DEBUG] Invalid APPNum! ulMCUFlashAddress=0x%08lX\r\n",
        //                   ulMCUFlashAddress);
        // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);
    }

    ulFlashDataAddr = ulMCUFlashAddress + (uiPageNum * MCU_FLASH_PAGE_SIZE);
    // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
    //                   "[DEBUG] Calculated FlashDataAddr=0x%08lX\r\n",
    //                   ulFlashDataAddr);
    // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);

    if (bStatus == true)
    {
        ucResp = ucMCUFLASH_eWriteMCUFlash_Exe(ulRAMDataAddr, ulFlashDataAddr, MCU_FLASH_PAGE_SIZE);
        // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
        //                   "[DEBUG] WriteMCUFlash response=%u\r\n", ucResp);
        // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);

        if (ucResp == MCU_FLASH_WRITING_OK)
        {
            // Check CRC
            // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
            //                   "[DEBUG] Starting CRC calculation for %u words at 0x%08lX\r\n",
            //                   MCU_FLASH_PAGE_SIZE, ulFlashDataAddr);
            // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);

            FTPData.ulCRCVal = HAL_CRC_Calculate(&hcrc, (uint32_t *)ulFlashDataAddr, MCU_FLASH_PAGE_SIZE);

            // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
            //                   "[DEBUG] Calculated CRC=0x%08lX, Expected CRC=0x%08lX\r\n",
            //                   FTPData.ulCRCVal, FTPData.ulCRCValFrmFTP[uiPageNum]);
            // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);

            if (FTPData.ulCRCVal == FTPData.ulCRCValFrmFTP[uiPageNum])
            {
                bStatus = true;
                // HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] CRC match\r\n", strlen("[DEBUG] CRC match\r\n"), HAL_MAX_DELAY);
            }
            else
            {
                bStatus = false;
                HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] CRC mismatch\r\n", strlen("[DEBUG] CRC mismatch\r\n"), HAL_MAX_DELAY);
            }
        }
        else
        {
            bStatus = false;
            HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] Write failed\r\n", strlen("[DEBUG] Write failed\r\n"), HAL_MAX_DELAY);
        }
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)"[DEBUG] Skipping write/CRC due to invalid APPNum\r\n",
                          strlen("[DEBUG] Skipping write/CRC due to invalid APPNum\r\n"), HAL_MAX_DELAY);
    }

    // Final status debug
    // dbgLen = snprintf(dbgBuf, sizeof(dbgBuf),
    //                   "[DEBUG] Exiting bFTP_iWriteAndVerifyCRC: bStatus=%s\r\n",
    //                   bStatus ? "true" : "false");
    // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, dbgLen, HAL_MAX_DELAY);

    return bStatus;
}

// ============================================================================
// Name 		: ucFTP_eDownloadFileFromFTPServer_Exe
// Objective	: Download File From FTP Server to GSM Module
// Input		: File Name
// Output		: none
// Return		: 	0 - File download failed,
//			  		1 - File downloaded successfully,
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static bool bFTP_iGetFileFromFTPServer_Exe(char *pFTPFile, uint32_t ulFileSize)
{
    uint8_t ucResp = 0;
    char cBuff[100];
    bool bFlag = false;

    sprintf(cBuff, "%s\"%s\",\"UFS:%s\",0\r", cGSM_eATCommandTbl[ATCOMMAND_QFTPGET], pFTPFile, pFTPFile);
    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_OK, 6, ATRESPONSE_QFTPGET, 13, 300, 90000);
    if (ucResp == BOTH_RESPONSES_MATCHING)
    {
        if (FTPData.ulFTPFileSize == ulFileSize)
        {
            bFlag = true;
        }
        else
        {
            bFlag = false;
        }
    }
    else
    {
    }
    return bFlag;
}

// ============================================================================
// Name 		: ucFTP_iCheckFileAvailableInGSMModuleUFS_Exe
// Objective	: Check File Available in GSM UFS
// Input		: File Name & File Size
// Output		: none
// Return		: 	0 - Unknown Error
//				1 - File Not Found (+CME ERROR: Fail to list the file)
//				2 - Same File Found, Size Not Found
//				3 - Same File Found, Size Same
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static uint8_t ucFTP_iCheckFileAtGSMUFS_Exe(char *pFileName, uint32_t ulFileSize)
{
    uint8_t ucResp, ucFlag;
    char cBuff[100], *pAddr, *pAddr1;
    uint32_t ulBytes;

    sprintf(cBuff, "%s\"%s\"\r", cGSM_eATCommandTbl[ATCOMMAND_QFLST], pFileName);
    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_QFLST, 10, ATRESPONSE_OK, 6, 300, 5000);

    if (ucResp == BOTH_RESPONSES_MATCHING)
    {
        pAddr = strchr((char *)GSM_DataBuffer, ',');
        if (pAddr != NULL)
        {
            pAddr++;
            pAddr1 = strchr(pAddr, '\r');
            if (pAddr1 != NULL)
            {
                pAddr1 = 0;
                ulBytes = atol(pAddr);
                if (ulBytes)
                {
                    FTPData.ulFTPFileSize = ulBytes;

                    char dbgBuf[64];

                    // snprintf(dbgBuf, sizeof(dbgBuf), "[DEBUG] FTP file size: %lu\r\n", (unsigned long)FTPData.ulFTPFileSize);
                    // HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), HAL_MAX_DELAY);

                    ucFlag = FILE_FOUND;
                }
                else
                {
                    ucFlag = ZERO_SIZE_FILE_FOUND;
                }
            }
            else
            {
                ucFlag = NO_FILE_FOUND;
            }
        }
        else
        {
            ucFlag = NO_FILE_FOUND;
        }
    }
    else if (ucResp == RESPONSE_CME_ERROR)
    {
        ucFlag = NO_FILE_FOUND;
    }
    else
    {
        ucFlag = UNKNOWN_ERROR;
    }
    return ucFlag;
}

// ============================================================================
// Name 		: bFTP_iSetCurrentFolderAtFTPServer_Exe
// Objective	: Set Current Folder on FTP Folder to retrive the File
//
// Input		: None
// Output		: none
// Return		: Login Status
//					0 - fail, 1 - success
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static bool bFTP_iSetFolderAtFTPServer_Exe(char *cFileFolder)
{
    char cBuff[100];
    uint8_t ucResp;
    bool bResp;

    sprintf(cBuff, "%s\"%s\"\r", (char *)cGSM_eATCommandTbl[ATCOMMAND_QFTPCWD], cFileFolder);
    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_OK, 6, ATRESPONSE_QFTPCWD, 13, 300, 90000);
    if (ucResp == BOTH_RESPONSES_MATCHING)
    {
        bResp = true;
    }
    else
    {
        bResp = false;
    }
    return bResp;
}

// ============================================================================
// Name 		: ucFTP_eCheckFileAtFTPServer_Exe
// Objective	: Check File available at FTP Server and Match File Size
// Input		: File Name
// Output		: File Size at APPData.ulFTPFileSize
// Return		: 0 - File Not Found, 1 - File Available
// Version		: -
// Author		: AK
// Date 		: 27-07-2019
// ============================================================================
static bool bFTP_iCheckFileAtFTPServer_Exe(char *pFTPFile)
{
    char cBuff[100], *cBuffAdr = 0, *cBuffAdr1 = 0;
    uint8_t ucResp = 0;
    uint16_t uiStatus = 0;
    bool bFlag = false;

    sprintf(cBuff, "%s\"%s\"\r", (char *)cGSM_eATCommandTbl[ATCOMMAND_QFTPSIZE], pFTPFile);

    ucResp = ucGSM_eProcessATCmdDualRespWithPtr_Exe(cBuff, ATRESPONSE_OK, 6, ATRESPONSE_QFTPSIZE, 14, 300, 90000);
    if ((ucResp == BOTH_RESPONSES_MATCHING) || (ucResp == RESPONSE1_MATCHING))
    {
        cBuffAdr = strchr((char *)GSM_DataBuffer, ':');
        if (cBuffAdr != NULL)
        {
            cBuffAdr++;
            cBuffAdr++;
            cBuffAdr1 = strchr(cBuffAdr, ',');
            if (cBuffAdr1 != NULL)
            {
                *cBuffAdr1 = 0;
                uiStatus = atoi(cBuffAdr);
                if (uiStatus == 0)
                {
                    cBuffAdr1++;
                    cBuffAdr = strchr(cBuffAdr1, 0x0D);
                    if (cBuffAdr != NULL)
                    {
                        *cBuffAdr = 0;
                        FTPData.ulFTPFileSize = atol(cBuffAdr1);

                        char dbgBuf[64];
                        snprintf(dbgBuf, sizeof(dbgBuf), "[DEBUG] FTP file size: %lu\r\n", (unsigned long)FTPData.ulFTPFileSize);
                        HAL_UART_Transmit(&huart1, (uint8_t *)dbgBuf, strlen(dbgBuf), HAL_MAX_DELAY);

                        bFlag = true;
                    }
                    else
                    {
                        bFlag = false;
                    }
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
    }
    else
    {
    }
    return bFlag;
}

// Return : 0 - Fail, 1 - Success
static bool bFTP_iWriteAndVerifyMCUFlashLastPage_Exe(uint8_t ucAPPNum, uint32_t ulRAMDataAddr, uint32_t ulBytesAlreadyWritten, uint16_t uiBytesToWrite)
{
    bool bStatus = false;
    uint32_t ulFlashDataAddr = 0, ulMCUFlashAddress = 0;
    uint8_t ucResp = 0, ucRAMVal = 0, ucFlashVal = 0;
    uint16_t uiCntr = 0;
    char dbg[128];

    // Write Data
    if (ucAPPNum == VALID_APP1)
    {
        ulMCUFlashAddress = IMAGE1_MCU_FLASH_ADDR;
    }
    else if (ucAPPNum == VALID_APP2)
    {
        ulMCUFlashAddress = IMAGE2_MCU_FLASH_ADDR;
    }
    else
    {
        bStatus = false;
    }

    ulFlashDataAddr = ulMCUFlashAddress + ulBytesAlreadyWritten;
    snprintf(dbg, sizeof(dbg), "DBG-LASTPAGE: APP=%u base=0x%08lX offset=0x%lX -> writeAddr=0x%08lX len=%u\r\n", ucAPPNum, (unsigned long)ulMCUFlashAddress, (unsigned long)ulBytesAlreadyWritten, (unsigned long)ulFlashDataAddr, (unsigned)uiBytesToWrite);
    HAL_UART_Transmit(&huart1, (uint8_t *)dbg, strlen(dbg), 200);

    ucResp = ucMCUFLASH_eWriteMCUFlash_Exe(ulRAMDataAddr, ulFlashDataAddr, uiBytesToWrite);

    if (ucResp == MCU_FLASH_WRITING_OK)
    {
        bStatus = true;

        // Read & Compare Data
        uiCntr = 0;
        do
        {
            // vAPP_eFeedTheWDT_Exe();

            ucRAMVal = *((char *)(ulRAMDataAddr + uiCntr));
            ucFlashVal = *((char *)(ulFlashDataAddr + uiCntr));
            if (ucRAMVal == ucFlashVal)
            {
                bStatus = true;
            }
            else
            {
                bStatus = false;
            }
            uiCntr++;
        } while ((uiCntr < uiBytesToWrite) && (bStatus == true));
    }
    else
    {
        bStatus = false;
    }
    return bStatus;
}

// ============================================================================
// Name			: bFTP_iCompareClusterFWVer_Exe
// Objective	: Read Version.txt File to detect any Version Change
//
// Input  		: none
// Output 		: 	FTPStatus.ucVersionStatus = SAME_VERSION_FOUND
//					FTPStatus.ucVersionStatus = DIFFERENT_VERSION_FOUND
// Return		: 0 - Version Not Checked, 1 - Same Version Found, 2 - Different Version Found
// Author		: AK
// Date			: 18 Sep 2019
// ============================================================================
static Enum_FTPVerStatus ucFTP_iCompareFWVer_Exe(char *cFolder, char *cFile, char *cSWVer)
{
    uint32_t ulAddr = 0;
    int16_t iVal = 0;
    Enum_FTPVerStatus ucResp = VERSION_NOT_CHECKED;

    ulAddr = ulFTP_iDownloadAndReadFile_Exe(cFolder, cFile);

    if (ulAddr != NULL)
    {
        strncpy(FTPData.cNewVersion, (char *)ulAddr, FTPData.ulFTPFileSize);
        iVal = strncmp(FTPData.cNewVersion, cSWVer, FTPData.ulFTPFileSize);

        if (iVal == 0)
        {
            // No Change in Version
            ucResp = SAME_VERSION_FOUND;
        }
        else
        {
            // FW Update Request received, Go for FW Update
            ucResp = DIFFERENT_VERSION_FOUND;
        }

        bFTP_iCloseGSMUFSFile_Exe(FTPData.ulFileHandle);
        uiFTP_eDeleteGSMUFSFile_Exe((char *)cFTP_iMsgTbl[VERSION_FILE_NAME]);
    }
    else
    {
    }

    return ucResp;
}

/***** END OF FILE ****/
