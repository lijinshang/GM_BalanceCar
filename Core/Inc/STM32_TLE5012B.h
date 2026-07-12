#ifndef _STM32_TLE5012_
#define _STM32_TLE5012_

#include "stdint.h"
#include "math.h"
#include "main.h"
#include "spi.h"
#include "STM32_TLE5012_Config.h"


#define SYSTEM_ERROR_MASK           0x4000
#define INTERFACE_ERROR_MASK        0x2000
#define INV_ANGLE_ERROR_MASK        0x1000

#define READ_STA_CMD_NOSAFETY       0x8000
#define READ_STA_CMD                0x8001
#define READ_ACTIV_STA_CMD          0x8011
#define READ_ANGLE_VAL_CMD          0x8021
#define READ_ANGLE_SPD_CMD          0x8031
#define READ_ANGLE_REV_CMD          0x8041
#define READ_TEMP_CMD               0x8051
#define READ_INTMODE_1              0x8061
#define READ_SIL                    0x8071
#define READ_INTMODE_2              0x8081
#define READ_INTMODE_3              0x8091
#define READ_OFFSET_X               0x80A1
#define READ_OFFSET_Y               0x80B1
#define READ_SYNCH                  0x80C1
#define READ_IFAB                   0x80D1
#define READ_INTMODE_4              0x80E1
#define READ_TEMP_COEFF             0x80F1
#define READ_RAW_X_CMD              0x8101
#define READ_RAW_Y_CMD              0x8111

#define READ_UPD_STA_CMD            0x8401
#define READ_UPD_ANGLE_VAL_CMD      0x8421
#define READ_UPD_ANGLE_SPD_CMD      0x8431
#define READ_UPD_ANGLE_REV_CMD      0x8441

#define READ_BLOCK_CRC              0x8088

#define WRITE_ACTIV_STA             0x0011
#define WRITE_INTMODE_1             0x5061
#define WRITE_SIL                   0x5071
#define WRITE_INTMODE_2             0x5081
#define WRITE_INTMODE_3             0x5091
#define WRITE_OFFSET_X              0x50A1
#define WRITE_OFFSET_Y              0x50B1
#define WRITE_SYNCH                 0x50C1
#define WRITE_IFAB                  0x50D1
#define WRITE_INTMODE_4             0x50E1
#define WRITE_TEMP_COEFF            0x50F1

#define CHECK_CMD_UPDATE            0x0400

#define CRC_POLYNOMIAL              0x1D
#define CRC_SEED                    0xFF
#define CRC_NUM_REGISTERS           8

#define DELETE_BIT_15               0x7FFF
#define CHANGE_UINT_TO_INT_15       32768
#define CHECK_BIT_14                0x4000

#define DELETE_7BITS                0x01FF
#define CHANGE_UNIT_TO_INT_9        512
#define CHECK_BIT_9                 0x0100

#define POW_2_15                    32768.0
#define POW_2_7                     128.0

#define TEMP_OFFSET                 152.0
#define TEMP_DIV                    2.776

#define GET_BIT_14_4                0x7FF0


#define DUMMY                   0xFFFF

#define ANGLE_360_VAL			360.0f

typedef struct
{
	SPI_HandleTypeDef *spi;
	GPIO_TypeDef *cs_gpio_port;
	uint32_t cs_pin;
	GPIO_TypeDef *sck_gpio_port;
	uint32_t sck_pin;
	GPIO_TypeDef *mosi_gpio_port;
	uint32_t mosi_pin;
	uint16_t registers[CRC_NUM_REGISTERS];

	float Temperature;
	float AngleSpeed;
	float AngleValue;
	int16_t NumRev;
	float AngleRange;
} TLE5012B_Sensor;

typedef enum errorTypes
{
	NO_ERROR = 0x00,
	SYSTEM_ERROR = 0x01,
	INTERFACE_ACCESS_ERROR = 0x02,
	INVALID_ANGLE_ERROR = 0x03,
	SPI_TIMEOUT_ERROR = 0x04,
	CRC_ERROR = 0xFF
} errorTypes;

errorTypes readBlockCRC(TLE5012B_Sensor *sensor);

void triggerUpdate(TLE5012B_Sensor *sensor);

errorTypes getAngleSpeed(TLE5012B_Sensor *sensor);

errorTypes getAngleValue(TLE5012B_Sensor *sensor);

errorTypes getNumRevolutions(TLE5012B_Sensor *sensor);

errorTypes getUpdAngleSpeed(TLE5012B_Sensor *sensor, float *angleSpeed);

errorTypes getUpdAngleValue(TLE5012B_Sensor *sensor, float *angleValue);

errorTypes getTemperature(TLE5012B_Sensor *sensor);

float MechAngleToElecAngle(float mech_angle, float offset, int pole_pairs);

#endif
