// ============================================================================
// Module	: MCUFlash STM32G0
// Version	: 00
// Author	: AK
// Date		: 29 May 2019
// ============================================================================
#include "MCUFLASH.h"
#include "APP.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h> // Add this for uint8_t, uint32_t, uint64_t
#include "stm32g0xx_hal_flash.h"

extern UART_HandleTypeDef huart1;

// Private Proto
static uint16_t uiMCUFLASH_iFindLastBlankMemoryInCurrentAPPStorage_Exe(void);
static uint64_t ullMCUFLASH_iRead64DataFromMCUFlash_Exe(uint32_t ulAddr);
static uint8_t ucMCUFLASH_iEraseOnePageOfMCUFlash_Exe(uint32_t ulAddr);
static uint8_t ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe(uint32_t ulDestAdr, uint64_t ullData);
static uint32_t ulMCUFLASH_iGetPage_Exe(uint32_t ulAddr);
static uint32_t ulMCUFLASH_iGetBank_Exe(uint32_t ulAddr);
Enum_FLASHStatus ucMCUFLASH_eWriteMCUFlash_Exe(uint32_t ulSourceAdr, uint32_t ulDestAdr, uint16_t uiBytes);

// ============================================================================
// Name			: uiCONFIG_eReadCONFIGStatusFromMCUFlash_Exe
// Objective	: Read CONFIG Status from MCU Flash
// Input  		: None
// Output 		: None
// Return		: 	Last Valid CONFIG Status
// 					01 : Valid APP is APP1
//					02 : Valid APP is APP2
//					0xFFFFFFFFFFFFFFFF - If Error
// Version		: -
// Author		: AK
// Date			: 17 Sep 2019
// ============================================================================
uint64_t ullMCUFLASH_eReadCurrentActiveAPP_Exe(void)
{
	uint16_t uiLastBlankAddr;
	uint32_t ulAddr;
	uint64_t ullVal = 0;
	uint8_t ucResp;

	uiLastBlankAddr = uiMCUFLASH_iFindLastBlankMemoryInCurrentAPPStorage_Exe();
	if (uiLastBlankAddr != 0xFFFFU)
	{
		if (uiLastBlankAddr != 0)
		{
			uiLastBlankAddr -= 8;
		}
		else
		{
			// Load Default Value
			ullVal = VALID_APP1;
			ucResp = ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe(MCUCONFIG_MEM_START_ADDR, ullVal);
			if (ucResp == MCU_FLASH_WRITING_OK)
			{
				uiLastBlankAddr = 0;
			}
			else
			{
				ullVal = 0xFFFFFFFFFFFFFFFFU;
				return ullVal;
			}
		}

		ulAddr = MCUCONFIG_MEM_START_ADDR + uiLastBlankAddr;

		ullVal = ullMCUFLASH_iRead64DataFromMCUFlash_Exe(ulAddr);
	}
	else
	{
		ullVal = 0xFFFFFFFFFFFFFFFFU;
	}

	return ullVal;
}

// ============================================================================
// Name			: ucMCUFLASH_eWriteCONFIGStatusInMCUFlash_Exe
// Objective	: Write CONFIG Status in MCU Flash
// Input  		: 16 bit Data
// 					Lower 8 bit : 01 : Jump to Execute APP
//								: 02 : Firmware Update
// 					Upper 8 Bit : 01 : Jump to Execute APP1
//								: 02 : Jump to Execute APP2
// Output 		: MCU Flash updated
// Return		: Data Written Status
//				0 - Fail
//				1 - Success
// Version		: -
// Author		: AK
// Date			: 17 Sep 2019
// ============================================================================
uint8_t ucMCUFLASH_eWriteCONFIGStatusInMCUFlash_Exe(uint64_t ullStatus)
{
	uint16_t uiLastBlankAddr = 0;
	uint8_t ucResp = 0, ucStatus = 0;

	// Find Last blank Memory Address
	uiLastBlankAddr = uiMCUFLASH_iFindLastBlankMemoryInCurrentAPPStorage_Exe();

	if (uiLastBlankAddr > (MCUCONFIG_MEM_END_ADDR - MCUCONFIG_MEM_START_ADDR - 4))
	{
		ucStatus = ucMCUFLASH_iEraseOnePageOfMCUFlash_Exe(MCUCONFIG_MEM_START_ADDR);
		if (ucStatus == MCU_FLASH_ERASE_OK)
		{
			uiLastBlankAddr = 0;
		}
	}

	ucResp = ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe((MCUCONFIG_MEM_START_ADDR + uiLastBlankAddr), ullStatus);

	return ucResp;
}

// ============================================================================
// Name			: uiCONFIG_iFindLastBlankMemoryAddrInOneSectorOf2kb_Exe
// Objective	: Find Last Blank Location in a Sector 0f 2kb MCU to Store the Config Data
// Input  		: none
// Output 		: none
// Return		: 16 bit Index Number pointing to Blank Location
//					0xFFFF - Incase of any error
// Version		: -
// Author		: AK
// Date			: 19 Sep 2023
// ============================================================================
static uint16_t uiMCUFLASH_iFindLastBlankMemoryInCurrentAPPStorage_Exe(void)
{
	uint16_t uiCntr;
	bool bStatus;
	uint32_t ulAddr;
	uint64_t ullData;
	uint8_t ucStatus;
	char debugStr[128];

	bStatus = false;
	for (uiCntr = 0; (uiCntr < 0x800U) && (bStatus == false); uiCntr += 8)
	{
		ulAddr = MCUCONFIG_MEM_START_ADDR + uiCntr;

		ullData = ullMCUFLASH_iRead64DataFromMCUFlash_Exe(ulAddr);

		if (ullData == 0xFFFFFFFFFFFFFFFFU)
		{
			bStatus = true;
		}
		else
		{
		}
	}

	if (bStatus == false)
	{
		// Read Last Data, Erase the Page and Store in beginning
		ullData = ullMCUFLASH_iRead64DataFromMCUFlash_Exe(MCUCONFIG_MEM_END_ADDR - 8 + 1);
		ucStatus = ucMCUFLASH_iEraseOnePageOfMCUFlash_Exe(MCUCONFIG_MEM_START_ADDR);
		if (ucStatus == MCU_FLASH_ERASE_OK)
		{
			ucStatus = ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe(MCUCONFIG_MEM_START_ADDR, ullData);
			if (ucStatus == MCU_FLASH_WRITING_OK)
			{
				uiCntr = 8;
			}
			else
			{
				uiCntr = 0xFFFFU;
			}
		}
		else
		{
			uiCntr = 0xFFFFU;
		}
	}
	else
	{
		uiCntr -= 8U;
	}
	// sprintf(debugStr, "[DEBUG] Exit %s: LastBlankAddr=0x%04X\r\n", __func__, uiCntr);
	// HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
	return uiCntr;
}

// ============================================================================
// Name			: ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe
// Objective	: Write 64 bit Value into MCU Flash without Erasing Flash
// Input  		: 1. Source Address of Buffer
//		  		  2. Byte to Write
// Output 		: Data in MCU Flash
// Return		: Status of Flash Writing
// 					1 = MCU_FLASH_UNLOCK_ERROR,
//					2 = MCU_FLASH_LOCK_ERROR,
//					3 = MCU_FLASH_WRITING_ERROR,
//					4 = MCU_FLASH_WRITING_OK,
// Author		: AK
// Date			: 19 Sep 2023
// ============================================================================
static uint8_t ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe(uint32_t ulDestAdr, uint64_t ullData)
{
	uint8_t ucResp;
	char debugStr[128];

	ucResp = HAL_FLASH_Unlock();
	if (ucResp == HAL_OK)
	{
		// Program Flash
		__disable_irq();
		ucResp = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ulDestAdr, ullData);
		__enable_irq();

		if (ucResp == HAL_OK)
		{
			ucResp = HAL_FLASH_Lock();

			if (ucResp == HAL_OK)
			{
				ucResp = MCU_FLASH_WRITING_OK;
				sprintf(debugStr, "[DEBUG] Flash locked after writing at 0x%08lX\r\n", ulDestAdr);
				HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			}
			else
			{
				ucResp = MCU_FLASH_LOCK_ERROR;
				sprintf(debugStr, "[ERROR] FLASH Lock failed! HAL_Status=%d\r\n", ucResp);
				HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			}
		}
		else
		{
			ucResp = MCU_FLASH_WRITING_ERROR;
			sprintf(debugStr, "[ERROR] FLASH write failed! HAL_Status=%d\r\n", ucResp);
			HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
		}
	}
	else
	{
		ucResp = MCU_FLASH_UNLOCK_ERROR;
		sprintf(debugStr, "[ERROR] FLASH Unlock failed! HAL_Status=%d\r\n", ucResp);
		HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
	}
	return ucResp;
}

#if 0
static uint8_t ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe(uint32_t ulDestAdr, uint64_t ullData)
{
	uint8_t ucResp;
	char debugStr[128];

	sprintf(debugStr, "[DEBUG] Enter %s: Addr=0x%08lX, Data=0x%016llX\r\n", __func__, ulDestAdr, ullData);
	HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);

	// Unlock Flash
	ucResp = HAL_FLASH_Unlock();
	if (ucResp != HAL_OK)
	{
		sprintf(debugStr, "[ERROR] FLASH Unlock failed! HAL_Status=%d\r\n", ucResp);
		HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
		return MCU_FLASH_UNLOCK_ERROR;
	}

	// Program 64-bit (Double Word)
	ucResp = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ulDestAdr, ullData);
	sprintf(debugStr, "[DEBUG] FlashProg Addr=0x%08lX, Data=0x%016llX, HAL_Status=%d\r\n", ulDestAdr, ullData, ucResp);
	HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);

	if (ucResp == HAL_OK)
	{
		ucResp = HAL_FLASH_Lock();
		if (ucResp == HAL_OK)
		{
			sprintf(debugStr, "[DEBUG] Flash locked after writing at 0x%08lX\r\n", ulDestAdr);
			HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			sprintf(debugStr, "[DEBUG] WriteMCUFlash response=%d\r\n", MCU_FLASH_WRITING_OK);
			HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			return MCU_FLASH_WRITING_OK;
		}
		else
		{
			sprintf(debugStr, "[ERROR] FLASH Lock failed! HAL_Status=%d\r\n", ucResp);
			HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			return MCU_FLASH_LOCK_ERROR;
		}
	}
	else
	{
		uint32_t flashErr = HAL_FLASH_GetError();
		sprintf(debugStr, "[ERROR] Flash write FAILED at 0x%08lX | HAL_Status=%d | FLASH_ERR=0x%lX\r\n",
				ulDestAdr, ucResp, flashErr);
		HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
		HAL_FLASH_Lock();
		return MCU_FLASH_WRITING_ERROR;
	}
}
#endif

#if 0
static uint8_t ucMCUFLASH_iWrite64bitDataInMCUFlash_Exe(uint32_t ulDestAdr, uint64_t ullData)
{
	uint8_t ucResp;
	char debugStr[128];

	// Unlock Flash
	ucResp = HAL_FLASH_Unlock();
	if (ucResp != HAL_OK)
	{
		sprintf(debugStr, "[ERROR] FLASH Unlock failed! HAL_Status=%d\r\n", ucResp);
		HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
		return MCU_FLASH_UNLOCK_ERROR;
	}

	// Program 64-bit (Double Word)
	ucResp = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ulDestAdr, ullData);
	if (ucResp == HAL_OK)
	{
		ucResp = HAL_FLASH_Lock();
		if (ucResp == HAL_OK)
		{
			sprintf(debugStr, "[DEBUG] Flash write OK at 0x%08lX\r\n", ulDestAdr);
			HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			return MCU_FLASH_WRITING_OK;
		}
		else
		{
			sprintf(debugStr, "[ERROR] FLASH Lock failed! HAL_Status=%d\r\n", ucResp);
			HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			return MCU_FLASH_LOCK_ERROR;
		}
	}
	else
	{
		uint32_t flashErr = HAL_FLASH_GetError();
		sprintf(debugStr,
				"[ERROR] Flash write FAILED at 0x%08lX | HAL_Status=%d | FLASH_ERR=0x%lX\r\n",
				ulDestAdr, ucResp, flashErr);
		HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
		HAL_FLASH_Lock(); // Always lock flash
		return MCU_FLASH_WRITING_ERROR;
	}
}
#endif

static uint64_t ullMCUFLASH_iRead64DataFromMCUFlash_Exe(uint32_t ulAddr)
{
	uint64_t ullVal = 0;

	ullVal |= *((char *)ulAddr + 7);
	ullVal = ullVal << 8;
	ullVal |= *((char *)ulAddr + 6);
	ullVal = ullVal << 8;
	ullVal |= *((char *)ulAddr + 5);
	ullVal = ullVal << 8;
	ullVal |= *((char *)ulAddr + 4);
	ullVal = ullVal << 8;
	ullVal |= *((char *)ulAddr + 3);
	ullVal = ullVal << 8;
	ullVal |= *((char *)ulAddr + 2);
	ullVal = ullVal << 8;
	ullVal |= *((char *)ulAddr + 1);
	ullVal = ullVal << 8;
	ullVal |= *((char *)ulAddr + 0);

	return ullVal;
}

static uint8_t ucMCUFLASH_iEraseOnePageOfMCUFlash_Exe(uint32_t ulAddr)
{
	uint32_t ulPageError = 0, ulFirstPage = 0, ulNbOfPages = 0, ulBankNumber = 0;
	uint8_t ucResp;
	char debugStr[128];
	static FLASH_EraseInitTypeDef EraseInitStruct;

	// sprintf(debugStr, "[DEBUG] Enter %s: EraseAddr=0x%08lX\r\n", __func__, ulAddr);
	// HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);

	HAL_FLASH_Unlock();

	ulFirstPage = ulMCUFLASH_iGetPage_Exe(ulAddr);
	ulNbOfPages = ulMCUFLASH_iGetPage_Exe(ulAddr) - ulFirstPage + 1;
	ulBankNumber = ulMCUFLASH_iGetBank_Exe(ulAddr);

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = ulBankNumber;
	EraseInitStruct.Page = ulFirstPage;
	EraseInitStruct.NbPages = ulNbOfPages;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &ulPageError) == HAL_OK)
	{
		ucResp = HAL_FLASH_Lock();
		// sprintf(debugStr, "[DEBUG] Page Erased OK at 0x%08lX\r\n", ulAddr);
		// HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);

		if (ucResp == HAL_OK)
		{
			ucResp = MCU_FLASH_ERASE_OK;
		}
		else
		{
			// sprintf(debugStr, "[ERROR] FLASH Lock failed after erase! HAL_Status=%d\r\n", ucResp);
			// HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
			ucResp = MCU_FLASH_LOCK_ERROR;
		}
	}
	else
	{
		sprintf(debugStr, "[ERROR] Page Erase FAILED at 0x%08lX, PageError=0x%08lX\r\n", ulAddr, ulPageError);
		HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);
		ucResp = MCU_FLASH_ERASE_ERROR;
	}

	// sprintf(debugStr, "[DEBUG] Exit %s -> %d\r\n", __func__, ucResp);
	// HAL_UART_Transmit(&huart1, (uint8_t *)debugStr, strlen(debugStr), 100);

	return ucResp;
}

// Return		: Status of Flash Erase
// 					1 = MCU_FLASH_UNLOCK_ERROR,
//					2 = MCU_FLASH_LOCK_ERROR,
//					3 = MCU_FLASH_WRITING_ERROR,
//					4 = MCU_FLASH_WRITING_OK,
//					5 = MCU_FLASH_ERASE_OK,
//					6 = MCU_FLASH_ERASE_ERROR,
Enum_FLASHStatus ucMCUFLASH_eEraseOnePageOfMCUFlash_Exe(uint32_t ulAddr)
{
	uint32_t ulPageError = 0, ulFirstPage = 0, ulNbOfPages = 1;
	uint32_t ulBankNumber = 0, ulStatus;

	Enum_FLASHStatus ucResp;

	static FLASH_EraseInitTypeDef EraseInitStruct;

	// __disable_irq();

	/* USER CODE BEGIN 2 */
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Clear OPTVERR bit set on virgin samples */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	/* Get the 1st page to erase */
	ulFirstPage = ulMCUFLASH_iGetPage_Exe(ulAddr);

	/* Get the number of pages to erase from 1st page */
	ulNbOfPages = ulMCUFLASH_iGetPage_Exe(ulAddr) - ulFirstPage + 1;

	/* Get the bank */
	ulBankNumber = ulMCUFLASH_iGetBank_Exe(ulAddr);

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = ulBankNumber;
	EraseInitStruct.Page = ulFirstPage;
	EraseInitStruct.NbPages = ulNbOfPages;

	/* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	   you have to make sure that these data are rewritten before they are accessed during code
	   execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	   DCRST and ICRST bits in the FLASH_CR register. */

	ulStatus = HAL_FLASHEx_Erase(&EraseInitStruct, &ulPageError);
	if (ulStatus == HAL_OK)
	//	if (HAL_FLASHEx_Erase(&EraseInitStruct, &ulPageError) == HAL_OK)
	{
		ucResp = MCU_FLASH_ERASE_OK;

		HAL_FLASH_Lock();
	}
	else
	{
		ucResp = MCU_FLASH_ERASE_ERROR;

		HAL_FLASH_Lock();
	}

	//	ulErrCode = HAL_FLASH_GetError();

	//	__enable_irq();

	return ucResp;
}

/**
 * @brief  Gets the page of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The page of a given address
 */
static uint32_t ulMCUFLASH_iGetPage_Exe(uint32_t Addr)
{
	uint32_t page = 0;

	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
	{
		/* Bank 1 */
		page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	}
	else
	{
		/* Bank 2 */
		page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	}

	return page;
}

/**
 * @brief  Gets the bank of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The bank of a given address
 */
static uint32_t ulMCUFLASH_iGetBank_Exe(uint32_t Addr)
{
	return FLASH_BANK_1;
}

// ============================================================================
// Name			: ucMCUFLASH_eWriteBuffer2MCUFlash_Exe
// Objective	: Write Buffer Data into MCU Flash
// Input  		: 1. Source Address of Buffer
//		  		  2. Destination Address of MCU Flash
//		  		  3. No. of Bytes to Write
// Output 		: Data in MCU Flash
// Return		: Status of Flash Writing
//					HAL_OK		- 0 - Flash Writing Successful
//					HAL_ERROR	- 1 - Error
//					HAL_BUSY	- 2 - Error
//					HAL_TIMEOUT - 3 - Error
// Author		: AK
// Date			: 3 Jan 2019
// ============================================================================
#if 0
Enum_FLASHStatus ucMCUFLASH_eWriteMCUFlash_Exe(uint32_t ulSourceAdr, uint32_t ulDestAdr, uint16_t uiBytes)
{
	char dbgStr[128];
	uint64_t ullVal = 0;
	uint16_t uiTemp = 0;
	uint8_t ucStatus;
	Enum_FLASHStatus ucResp = MCU_FLASH_NO_STATUS;

	// sprintf(dbgStr, "[DEBUG] Start Write Page: FlashDst=0x%08lX, Src=0x%08lX, Size=%u\r\n", ulDestAdr, ulSourceAdr, uiBytes);
	// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

	ucStatus = ucMCUFLASH_eEraseOnePageOfMCUFlash_Exe(ulDestAdr);

	if (ucStatus == MCU_FLASH_ERASE_OK)
	{
		sprintf(dbgStr, "[DEBUG] Page Erased OK at 0x%08lX\r\n", ulDestAdr);
		HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

		HAL_FLASH_Unlock();

		uiTemp = 0;
		while (uiTemp < uiBytes)
		{
			ullVal = 0;
			// Defensive: collect up to 8 bytes (handle last partial write)
			uint8_t byteCount = ((uiTemp + 8) <= uiBytes) ? 8 : (uiBytes - uiTemp);
			for (uint8_t k = 0; k < byteCount; k++)
			{
				ullVal |= ((uint64_t)(*((uint8_t *)ulSourceAdr + uiTemp + k))) << (8 * k);
			}

			// sprintf(dbgStr, "[DEBUG] FlashProg Addr=0x%08lX, Data=0x%016llX, Offset=%u, Bytes=%u\r\n",
			//         ulDestAdr, ullVal, uiTemp, byteCount);
			// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

			__disable_irq();
			ucStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ulDestAdr, ullVal);
			__enable_irq();

			if (ucStatus != HAL_OK)
			{
				uint32_t flashErr = HAL_FLASH_GetError();
				sprintf(dbgStr, "[ERROR] Flash Write Error at Addr=0x%08lX, Offset=%u, HAL_Status=%u, FlashErr=0x%lX\r\n",
						ulDestAdr, uiTemp, ucStatus, flashErr);
				HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);
				break;
			}

			ulDestAdr += 8;
			uiTemp += 8;
		}

		HAL_FLASH_Lock();
		sprintf(dbgStr, "[DEBUG] Flash locked after writing page at 0x%08lX\r\n", ulDestAdr - uiBytes);
		HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

		ucResp = (ucStatus == HAL_OK) ? MCU_FLASH_WRITING_OK : MCU_FLASH_WRITING_ERROR;
	}
	else
	{
		sprintf(dbgStr, "[ERROR] Erase Failed at 0x%08lX, Status=%u\r\n", ulDestAdr, ucStatus);
		HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);
		ucResp = MCU_FLASH_ERASE_ERROR;
	}

	// sprintf(dbgStr, "[DEBUG] Write Page RETURN status=%u\r\n", ucResp);
	// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

	return ucResp;
}
#endif

#if 0
Enum_FLASHStatus ucMCUFLASH_eWriteMCUFlash_Exe(
	uint32_t ulSourceAdr,
	uint32_t ulDestAdr,
	uint16_t uiBytes)
{
	char dbgStr[128];
	uint8_t *p = (uint8_t *)ulSourceAdr;
	uint32_t end = ulDestAdr + uiBytes;
	HAL_StatusTypeDef hal_ret;
	uint64_t data64;
	uint32_t addr;

	// 1) Erase the page
	if (ucMCUFLASH_eEraseOnePageOfMCUFlash_Exe(ulDestAdr) != MCU_FLASH_ERASE_OK)
	{
		return MCU_FLASH_ERASE_ERROR;
	}

	// 2) Program double-words
	HAL_FLASH_Unlock();

	for (addr = ulDestAdr; addr < end; addr += 8)
	{
		// pack 8 bytes (little-endian)
		data64 = ((uint64_t)p[0] << 0) | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);

		hal_ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, data64);
		if (hal_ret != HAL_OK)
		{
			uint32_t err = HAL_FLASH_GetError();

			snprintf(dbgStr, sizeof(dbgStr), "[ERROR] Program failed at 0x%08lX, HAL=%u, ERR=0x%lX\r\n", addr, hal_ret, err);
			HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

			HAL_FLASH_Lock();
			return MCU_FLASH_WRITING_ERROR;
		}

		p += 8;
	}
	HAL_FLASH_Lock();

	// snprintf(dbgStr, sizeof(dbgStr), "0x%08lX\r\n", addr);
	// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);
	// 3) Final status
	return MCU_FLASH_WRITING_OK;
}
#endif

#if 1
Enum_FLASHStatus ucMCUFLASH_eWriteMCUFlash_Exe(uint32_t ulSourceAdr, uint32_t ulDestAdr, uint16_t uiBytes)
{
	char dbgStr[128];
	uint64_t ullVal = 0;
	uint16_t uiTemp;
	uint8_t ucStatus;
	Enum_FLASHStatus ucResp = MCU_FLASH_NO_STATUS;

	// sprintf(dbgStr, "[DEBUG] Start Write Page: FlashDst=0x%08lX, Src=0x%08lX, Size=%u\r\n", ulDestAdr, ulSourceAdr, uiBytes);
	// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

	ucStatus = ucMCUFLASH_eEraseOnePageOfMCUFlash_Exe(ulDestAdr);

	if (ucStatus == MCU_FLASH_ERASE_OK)
	{
		// sprintf(dbgStr, "[DEBUG] Page Erased OK at 0x%08lX\r\n", ulDestAdr);
		// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

		HAL_FLASH_Unlock();

		uiTemp = 0;
		// __disable_irq();
		do
		{
			ullVal = 0;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 7);
			ullVal = ullVal << 8;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 6);
			ullVal = ullVal << 8;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 5);
			ullVal = ullVal << 8;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 4);
			ullVal = ullVal << 8;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 3);
			ullVal = ullVal << 8;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 2);
			ullVal = ullVal << 8;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 1);
			ullVal = ullVal << 8;
			ullVal |= *((char *)ulSourceAdr + uiTemp + 0);

			ucStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ulDestAdr, ullVal);

			// if (ucStatus != HAL_OK)
			// {
			// 	sprintf(dbgStr, "[ERROR] Flash Write Error at Addr=0x%08lX, Offset=%u, HAL_Status=%u, FlashErr=0x%lX\r\n",
			// 			ulDestAdr, uiTemp, ucStatus, HAL_FLASH_GetError());
			// 	HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);
			// 	break;
			// }

			ulDestAdr += 8;
			uiTemp += 8;
		} while ((uiTemp < uiBytes) && (ucStatus == HAL_OK));
		// __enable_irq();

		// sprintf(dbgStr, "[DEBUG] FlashProg Addr=0x%08lX, Data=0x%016llX, Offset=%u, HAL_Status=%u\r\n",
		// 		ulDestAdr, ullVal, uiTemp, ucStatus);
		// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

		if (ucStatus == HAL_OK)
		{
			ucResp = MCU_FLASH_WRITING_OK;
		}
		else
		{
			ucResp = MCU_FLASH_WRITING_ERROR;
		}

		HAL_FLASH_Lock();
		sprintf(dbgStr, "[DEBUG] Flash locked after writing page at 0x%08lX\r\n", ulDestAdr - uiBytes);
		HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);
	}
	else
	{
		// sprintf(dbgStr, "[ERROR] Erase Failed at 0x%08lX, Status=%u\r\n", ulDestAdr, ucStatus);
		// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);
		ucResp = MCU_FLASH_ERASE_ERROR;
	}

	// sprintf(dbgStr, "[DEBUG] Write Page RETURN status=%u\r\n", ucResp);
	// HAL_UART_Transmit(&huart1, (uint8_t *)dbgStr, strlen(dbgStr), 100);

	return ucResp;
}
#endif
