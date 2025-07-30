// ============================================================================
// COMM.h
// Author : AK
// Date   : 01 Dec 2021
// ============================================================================

#ifndef __COMM_H
#define __COMM_H

//*****************************************************************************
//   INCLUDE
//*****************************************************************************
#include <stdint.h>        // uint8_t, uint16_t, uint32_t
#include <stdbool.h>       // bool
#include "stm32g0xx_hal.h" // HAL GPIO, UART_HandleTypeDef, etc.
#include "GSM.h"           // for GSM_BUFF_SIZE, if you still use it

//*****************************************************************************
//   DEFINITION
//*****************************************************************************
#define USART1TX_DEBUGRX_Pin GPIO_PIN_9
#define USART1TX_DEBUGRX_GPIO_Port GPIOA

#define USART1RX_DEBUGTX_Pin GPIO_PIN_10
#define USART1RX_DEBUGTX_GPIO_Port GPIOA

#define GSMTX_USART3RX_Pin GPIO_PIN_9
#define GSMTX_USART3RX_GPIO_Port GPIOB

#define GSMRX_USART3TX_Pin GPIO_PIN_8
#define GSMRX_USART3TX_GPIO_Port GPIOB

#define UART1RxCntr UARTData.uiUART1RxCntr
#define UART1TimeOut UARTData.ulUART1TimeOut
#define UART1RxOneByte UARTData.cUART1RxData
#define UART1Buffer UARTData.cUART1Buff
#define UART1TxComplete UARTData.bUART1TxComplete
#define UART1RxComplete UARTData.ucUART1RxComplete

//---------------------------------------------------------

//*****************************************************************************
//   VARIABLES
//*****************************************************************************

typedef enum
{
    CHAR_RECEIVED_TIMEOUT = 0,
    CHAR_NOT_RECEIVED_TIMEOUT,
} eNUM_UARTDATASTATUS;

#pragma pack(push, 1)
typedef struct
{
    uint16_t uiUART3RxCntr;
    uint32_t ulUART3TimeOut;
    bool bUART3TxComplete;
    bool bUART3RxComplete;
    char cUART3RxData[2];
    // char		cUART3Buff[GSM_BUFF_SIZE];
    uint16_t uiUARTTxCntr;
} TsUARTData;
#pragma pack(pop)

//*****************************************************************************
//   PROTO
//*****************************************************************************
// void vCOMM_eUSART1Init(void);
// void vCOMM_eUSART3Init(void);
void vCOMM_eTxString_Exe(UART_HandleTypeDef *USARTx, char *pcMessage, uint16_t uiTimeOut);
#endif

/***** END OF FILE ****/
