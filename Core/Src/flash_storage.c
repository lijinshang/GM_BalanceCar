/*****************************************************************************
程序名称：STM32_FLASH驱动(STM32F401RE)
程 序 员：李锦上 Email：lijinshang@126.com 
使用说明：
修改历史：
2025-11-1：基本完成
******************************************************************************/
#include "flash_storage.h"
#include <string.h>

FlashStorageStatus_t FlashStorage_Init(void)
{
	return FLASH_STORAGE_OK;
}

uint16_t FlashStorage_CalculateCRC(const uint8_t *data, uint32_t length)
{
	uint16_t crc = 0xFFFF;
	uint32_t i, j;

	for (i = 0; i < length; i++)
	{
		crc ^= (uint16_t)data[i] << 8;

		for (j = 0; j < 8; j++)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc = crc << 1;
			}
		}
	}

	return crc;
}

FlashStorageStatus_t FlashStorage_WriteCalibData(const FlashData_t *flash_data)
{
	HAL_StatusTypeDef hal_status;
	uint32_t sector_error = 0;
	FLASH_EraseInitTypeDef erase_init;
	FlashData_t temp_data;

	memcpy(&temp_data, flash_data, sizeof(FlashData_t));
	temp_data.magic_word = FLASH_MAGIC_WORD;

	temp_data.crc16 = 0;
	temp_data.crc16 = FlashStorage_CalculateCRC((uint8_t*)&temp_data, sizeof(FlashData_t));

	hal_status = HAL_FLASH_Unlock();
	if (hal_status != HAL_OK)
	{
		return FLASH_STORAGE_ERROR;
	}

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                       FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
	erase_init.Sector = FLASH_SECTOR_NUM;
	erase_init.NbSectors = 1;
	erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	hal_status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
	if (hal_status != HAL_OK)
	{
		HAL_FLASH_Lock();
		return FLASH_STORAGE_ERROR;
	}

	uint32_t *data_ptr = (uint32_t*)&temp_data;
	for (uint32_t i = 0; i < sizeof(FlashData_t) / 4; i++)
	{
		hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, TINYFOC_CALIB_ADDR + (i * 4), data_ptr[i]);
		if (hal_status != HAL_OK)
		{
			HAL_FLASH_Lock();
			return FLASH_STORAGE_ERROR;
		}
	}

	HAL_FLASH_Lock();

	FlashData_t verify_data;
	if (FlashStorage_ReadCalibData(&verify_data) == FLASH_STORAGE_OK)
	{
		if (memcmp(&temp_data, &verify_data, sizeof(FlashData_t)) == 0)
		{
			return FLASH_STORAGE_OK;
		}
	}

	return FLASH_STORAGE_ERROR;
}

FlashStorageStatus_t FlashStorage_ReadCalibData(FlashData_t *flash_data)
{
	FlashData_t temp_data;
	uint16_t calculated_crc;

	uint32_t *flash_ptr = (uint32_t*)TINYFOC_CALIB_ADDR;
	uint32_t *data_ptr = (uint32_t*)&temp_data;

	for (uint32_t i = 0; i < sizeof(FlashData_t) / 4; i++)
	{
		data_ptr[i] = flash_ptr[i];
	}

	uint8_t *byte_ptr = (uint8_t*)&temp_data;
	uint32_t ff_count = 0;
	for (uint32_t i = 0; i < sizeof(FlashData_t); i++)
	{
		if (byte_ptr[i] == 0xFF)
		{
			ff_count++;
		}
	}

	if (ff_count >= sizeof(FlashData_t))
	{
		return FLASH_STORAGE_EMPTY;
	}

	if (temp_data.magic_word != FLASH_MAGIC_WORD)
	{
		return FLASH_STORAGE_EMPTY;
	}

	uint16_t stored_crc = temp_data.crc16;
	temp_data.crc16 = 0;

	calculated_crc = FlashStorage_CalculateCRC((uint8_t*)&temp_data, sizeof(FlashData_t));

	temp_data.crc16 = stored_crc;

	if (calculated_crc != stored_crc)
	{
		return FLASH_STORAGE_INVALID_DATA;
	}

	memcpy(flash_data, &temp_data, sizeof(FlashData_t));

	return FLASH_STORAGE_OK;
}
