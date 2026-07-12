#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef enum
{
	FLASH_STORAGE_OK = 0,
	FLASH_STORAGE_ERROR = 1,
	FLASH_STORAGE_INVALID_DATA = 2,
	FLASH_STORAGE_EMPTY = 3
} FlashStorageStatus_t;

/* 单个电机校准数据结构 */
typedef struct
{
	float mech_zero_offset;
	int8_t direction;
	uint8_t phase_sequence;
} BrushedMotorCalibData_t;

/* 双电机校准数据结构 */
typedef struct
{
	uint32_t magic_word;
	BrushedMotorCalibData_t motor1;
	BrushedMotorCalibData_t motor2;
	uint16_t crc16;
} FlashData_t;

/* Exported constants --------------------------------------------------------*/
/* STM32F401RE FLASH配置 */
#define FLASH_START_ADDR			0x08000000U
#define FLASH_END_ADDR				0x0807FFFFU
#define FLASH_SECTOR_NUM			FLASH_SECTOR_1
#define FLASH_SECTOR_START_ADDR		0x08004000U
#define FLASH_SECTOR_SIZE			0x00004000U
#define TINYFOC_CALIB_ADDR			(FLASH_SECTOR_START_ADDR + 0x0000U)
#define FLASH_MAGIC_WORD			0x54464F43U

/* Exported functions prototypes ---------------------------------------------*/
FlashStorageStatus_t FlashStorage_Init(void);
FlashStorageStatus_t FlashStorage_WriteCalibData(const FlashData_t *calib_data);
FlashStorageStatus_t FlashStorage_ReadCalibData(FlashData_t *calib_data);
uint16_t FlashStorage_CalculateCRC(const uint8_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_STORAGE_H */
