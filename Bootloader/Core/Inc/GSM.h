// ============================================================================
// Module	: Quectel GSM / GPS Module EC20
// Version	: 00
// Author	: AK
// Date		: 24 Jul 2018
// ============================================================================

#ifndef __GSM_H
#define __GSM_H

//*****************************************************************************
//   INCLUDE
//*****************************************************************************
#include "stdbool.h"

//*****************************************************************************
//   DEFINITION
//*****************************************************************************

#define TRUE 1
#define FALSE 0

typedef enum
{
    ATCOMMAND_QFTCLOSE, // 0
    ATCOMMAND_QFTPCFGCONID,
    ATCOMMAND_QFTPCFGACCOUNT, // 2
    ATCOMMAND_QFTPCFGFILETYPE,
    ATCOMMAND_QFTPCFGTRANSMODE,
    ATCOMMAND_QFTPCFGRSPTOUT, // 5
    ATCOMMAND_QFCLOSE,        // 6
    ATCOMMAND_QFDEL,          // 7
    ATCOMMAND_QFOPEN,         // 8
    ATCOMMAND_QFOPENQUERY,    // 9
    ATCOMMAND_QFSEEK,         // 10
    ATCOMMAND_QFREAD,         // 11
    ATCOMMAND_QFTPOPEN,       // 12
    ATCOMMAND_QFLST,          // 13
    ATCOMMAND_QFTPGET,        // 14
    ATCOMMAND_QFTPCWD,        // 15
    ATCOMMAND_QFTPSIZE,       // 16
    ATCOMMAND_CPMS_ME,        // 17
    ATCOMMAND_CMGF,           // 18
    ATCOMMAND_CSMP,           // 19
    ATCOMMAND_CSDH,           // 20
    ATCOMMAND_ATE0,           // 21
    ATCOMMAND_CPIN,           // 22
    ATCOMMAND_CSQ,            // 23
    ATCOMMAND_GETCREG,        // 24
    ATCOMMAND_GETCGREG,       // 25
    ATCOMMAND_GETCEREG,       // 26
    ATCOMMAND_QICSGP,         // 27
    ATCOMMAND_APNNAME,        // 28
    ATCOMMAND_CMGS,           // 29
    ATCOMMAND_CMEE,           // 30
    ATCOMMAND_ATV1,           // 31
    ATCOMMAND_QURCCFG,        // 32
    ATCOMMAND_CSCS,           // 33
    ATCOMMAND_GETCSCA,        // 34
    ATRESPONSE_CSCA,          // 35
} Enum_ATCmd;

typedef enum
{
    ATRESPONSE_OK = 0,
    ATRESPONSE_QFOPEN,  // 1
    ATRESPONSE_CONNECT, // 2
    ATRESPONSE_CMEERROR,
    ATRESPONSE_ERROR,
    ATRESPONSE_CMSERROR, // 5
    ATRESPONSE_QFTPOPEN,
    ATRESPONSE_QFLST,
    ATRESPONSE_QFTPGET,
    ATRESPONSE_QFTPCWD,
    ATRESPONSE_QFTPSIZE,  // 10
    ATRESPONSE_CPMS,      // 11
    ATRESPONSE_CPINREADY, // 12
    ATRESPONSE_CSQ,       // 13
    ATRESPONSE_CREG,      // 14
    ATRESPONSE_CGREG,     // 15
    ATRESPONSE_CEREG,     // 16
    ATRESPONSE_GETQICSGP, // 17
    ATRESPONSE_CMGS,      // 18
    ATRESPONSE_SEND,      // 19
    ATRESPONSE_SENDOK,    // 20
    ATRESPONSE_FWOK,      // 21
    ATRESPONSE_FWFAIL,    // 22
} Enum_ATResp;

typedef enum
{
    GSM_NOT_READY = 0,
    GSM_READY,
    GSM_WAS_READY,

    NO_RESP,
    RESPONSE_NOT_MATCHING,
    RESPONSE_MATCHING,
    RESPONSE_ERROR,
    RESPONSE_CME_ERROR,
    RESPONSE_CMS_ERROR,      // 5
    BOTH_RESPONSES_MATCHING, // 6
    RESPONSE1_MATCHING,      // 7
    RESPONSE2_MATCHING,      // 8
    NO_RESPONSE,
    BOTH_RESPONSE_RECD, // 10
    NO_CHAR_RECD_TIMEOUT,
    CHAR_RECD_TIMEOUT,
    SOME_CHAR_RECD_TIMEOUT,
    BOTH_RESPONSES_OR_ERROR_RECD, // 14

} EnumDataString;

typedef enum
{
    UNKNOWN_ERROR = 0,
    NO_FILE_FOUND,
    SAME_FILE_FOUND_MATCHING_SIZE,
    SAME_FILE_FOUND_NOT_MATCHING_SIZE,
    ZERO_SIZE_FILE_FOUND,
    FILE_FOUND,

    SIM_NOT_INSERTED = 0,
    SIM_INSERTED,

    NO_GSM_SIGNAL = 0,
    GSM_SIGNAL_ERROR,
    GSM_SIGNAL_LEVEL_LOW,
    GSM_SIGNAL_LEVEL_OK,

    NETWORK_REGISTRATION_FAILED = 0,
    NETWORK_REGISTRATION_SUCCESS,

    GSM_NETWORK_REGISTRATION = 1,
    GPRS_NETWORK_REGISTRATION,
    EPS_NETWORK_REGISTRATION,
} Enum_Msg;

//----------------------------------------------------------------

#define GSMComPortHandle &huart3

#define GSM_BuffSize 2100
#define GSM_BaudRate 115200
#define GSM_RxCntr GSMData.uiUART3RxCntr
#define GSM_TimeOut GSMData.ulUART3TimeOut
#define GSM_RxComplete GSMData.ucUART3RxComplete
#define GSM_RxStart GSMData.ucUART3RxStart
#define GSM_RxOneByte GSMData.cUART3RxData
#define GSM_DataBuffer GSMData.cUART3Buff

#define USART3TX_GSMRX_Pin GPIO_PIN_8
#define USART3TX_GSMRX_GPIO_Port GPIOB

#define USART3RX_GSMTX_Pin GPIO_PIN_9
#define USART3RX_GSMTX_GPIO_Port GPIOB

#define GSM_RESET_Pin GPIO_PIN_1
#define GSM_RESET_GPIO_Port GPIOA

#define GSM_PWRKEY_Pin GPIO_PIN_0
#define GSM_PWRKEY_GPIO_Port GPIOA

#define GSM_PWR_DRV_Pin GPIO_PIN_3
#define GSM_PWR_DRV_GPIO_Port GPIOA

#define GSM_RI_IN_Pin GPIO_PIN_2
#define GSM_RI_IN_GPIO_Port GPIOB
#define GSM_RI_EXTI1_EXTI_IRQn EXTI2_3_IRQn

#define CONTROL_Z 0x1A

#define GSM_NO_SOCKET_COUNTER 50
#define GSM_ERROR_COUNTER 10
#define GSM_NO_RESPONSE_COUNTER 5

#define TOTAL_SOCKET 1

#define PDP_CONTEXT_ID 1
#define LOCATION_SOCKET_ID 0

#define GSM_SIGNAL_THRESHOLD_LOW 7
#define GSM_SIGNAL_THRESHOLD_HIGH 12

extern const char *cGSM_eATCommandTbl[];
extern const char *cGSM_eATResponseTbl[];

#pragma pack(push, 1)
typedef struct
{
    uint16_t uiUART3RxCntr;
    uint32_t ulUART3TimeOut;
    uint8_t ucUART3RxComplete;
    uint8_t ucUART3RxStart;
    char cUART3RxData[2];
    char cUART3Buff[GSM_BuffSize];
    uint8_t ucNoResponseCntr;
    uint8_t ucErrorCntr;
    uint8_t ucNoSocketCntr;

    uint8_t ucATCmdNum;
    uint8_t ucResponseNum;
    uint8_t ucResponseNum1;
    uint8_t ucResponseNum2;
    uint8_t ucLenResp1;
    uint8_t ucLenResp2;
    uint32_t ulTimeOutCharRecd;
    uint32_t ulTimeOutNoCharRecd;

    uint8_t ucGSMSignalVal;

    char cSenderMobileNum[25];

    uint16_t uiGSMPowerCntr;
} TsGSMData;
#pragma pack(pop)

typedef union
{
    struct
    {
        uint8_t ucGSMCmdStatus : 8;   // 0 - No Cmd Sent, 1 - Cmd Sent Wait for Time Out, 2 - TimeOut Over, Check resp ....
        uint8_t ucGSMReadyStatus : 2; // 0 - Not Ready, 1 - Ready, 2 - Was Ready

        uint8_t ucGSMPower : 3; // 0 - Off, 1 - PWM Started, 2 - Power ON Delay ,
        uint8_t ucATCmdCntr : 2;
        bool bPWRKey : 1;            // 0 - No PwrKey used, 1 - PWRKey Issued
        bool bPwrDownStatus : 1;     // 0 - GSM Active, 1 - Power Down,
        bool bGSMReadyStatus : 1;    // 0 - Not Ready, 1 - Ready
        bool bGSMRebootRequired : 1; // 0 - no, 1 - Required

        uint8_t ucGSMSignalStatus : 2; // 0 - No Signal, 1 - Error, 2 - Low, 3 - Ok
        bool bCREGStatus : 1;          // 0 - Not Reg., 1 - Reg
        bool bCGREGStatus : 1;         // 0 - Not Reg., 1 - Reg
        bool bCEREGStatus : 1;         // 0 - Not Reg., 1 - Reg

        uint8_t ucNetworkInfo : 3; // 0 - No Network, 1 -, 2 - GSM, 3 - WCDMA, 4 - LTE
        bool bGSMInitStatus : 1;   // 0 - Not Initialise, 1 - Init
        bool bSIMInsertStatus : 1; // 0 - Not Inserted, 1 - Inserted
        bool bAPNStatus : 1;       // 0 - Not Set, 1 - APN Set
        bool bGSMSetting : 1;      // 0 - Not Set, 1 - Set
        bool bGPRSStatus : 1;      // 0 - Not Active, 1 - Active

        bool bSocketStatus : 1;      // 0 - Not Active, 1 - Active
        uint8_t ucGSMCmdType : 2;    // 0 - Init, 1 - AT Command, 2 - Data being Sent
        bool bRTCStatus : 1;         // 0 - Not Valid, 1 - Valid
        bool bReadyToSendData : 1;   // 0 - Not Ready, 1 - Ready to Send
        bool bFTPLogIn : 1;          // 0 - Not Logged In, 1 - Logged In
        uint8_t ucFTPFileStatus : 2; // 0 - UNKNOWN_ERROR, 1 - NO_FILE_FOUND, 2 -SAME_FILE_FOUND_MATCHING_SIZE, 3 - SAME_FILE_FOUND_NOT_MATCHING_SIZE,

        bool bMESMSStatus : 1;     // 0 - NO_SMS_PRESENT, 1 - SMS_PRESENT
        bool bDataSentPending : 1; // 0 - Not pending, 1 - Pending
        bool bFWSMSPending : 1;
        bool bSMSInit : 1;       // 0 - Not, 1 - Present
        bool bMusicInit : 1;     // 0 - Not Initialise, 1 - Init
        bool bMusicBTStatus : 1; // 0 - Disconnected, 1 - connected

        bool bBTStatus : 1;   // 0 - Not Checked, 1 - Checked
        bool bSMSRequest : 1; // 0 - No, 1 - Send Immob Key data
    };
    uint8_t ucData[7];
} TsGSMStatus;

void vGSM_eInit(void);
void vGSM_ePowerON_Exe(void);
void vGSM_eResetGSM_Exe(void);
// void vGSM_ePWRKeyStartGSM_Exe(void);
uint8_t ucGSM_iProcessATCmd_Exe(char *cATCmd, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
void vGSM_eStartGSM_Exe(void);
uint8_t ucGSM_eCompareStringInGSMBuff_Exe(uint8_t ucResponseNum);
uint8_t ucGSM_eProcessATCmdWithCmdNum_Exe(uint8_t ucATCmdNum, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
uint8_t ucGSM_eProcessATCmdWithCmdPtr_Exe(char *cATCmd, uint8_t ucRespNum, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
uint8_t ucGSM_eProcessATCmdDualRespWithPtr_Exe(char *cATCmd, uint8_t ucResp1Num, uint8_t ucLenResp1, uint8_t ucResp2Num, uint8_t ucLenResp2, uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);
void vGSM_eDelay100mS_Exe(uint16_t uiCount);
bool bGSM_eSendSMS_Exe(char *cReceiverMobile, char *cMessage);
bool bGSM_iWait4RespOrTimeOut_Exe(uint32_t ulTimeOutCharRecd, uint32_t ulTimeOutNoCharRecd);

#endif

/***** END OF FILE ****/
