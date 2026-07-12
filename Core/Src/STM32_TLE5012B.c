/*****************************************************************************
程序名称：TLE5012B驱动
程 序 员：李锦上 Email：lijinshang@126.com
使用说明：

hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;

tle5012b_1.spi = &hspi1;
tle5012b_1.cs_gpio_port = GPIOC;
tle5012b_1.cs_pin = GPIO_PIN_10;
tle5012b_1.mosi_gpio_port = TLE5012_MOSI_GPIO_Port;
tle5012b_1.mosi_pin = TLE5012_MOSI_Pin;
tle5012b_1.sck_gpio_port = TLE5012_SCK_GPIO_Port;
tle5012b_1.sck_pin = TLE5012_SCK_Pin;

readBlockCRC(&tle5012b_1);//初始化
getAngleValue(&tle5012b_1);//读角度 -180 - 180

修改历史：
2025-9-26：角度由-180-180改为0-360
2025-7-18：修复有时读不出角度的问题
2025-6-1：测试成功
2025-6-1：增加多传感器支持
2025-6-1：移植于STM32_TLE5012库 有改动
******************************************************************************/
#include "STM32_TLE5012B.h"


#ifndef TLE5012_NOT_MODIFY_MOSI_MANUALLY

void TLE5012_SET_MOSI_MODE_AF_PP(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin			= TLE5012_MOSI_Pin;
	GPIO_InitStruct.Mode		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull		= GPIO_NOPULL;
	GPIO_InitStruct.Speed		= GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate	= TLE5012_MOSI_GPIO_ALTERNATE;
	HAL_GPIO_Init(TLE5012_MOSI_GPIO_Port, &GPIO_InitStruct);
}

void TLE5012_SET_MOSI_MODE_INPUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin			= TLE5012_MOSI_Pin;
	GPIO_InitStruct.Mode		= GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull		= GPIO_NOPULL;
	GPIO_InitStruct.Speed		= GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(TLE5012_MOSI_GPIO_Port, &GPIO_InitStruct);
}
#endif

uint8_t _getFirstByte(uint16_t twoByteWord)
{
	return (uint8_t)(twoByteWord >> 8U);
}

uint8_t _getSecondByte(uint16_t twoByteWord)
{
	return (uint8_t)twoByteWord;
}

uint8_t _crc8(uint8_t *data, uint8_t length)
{
	uint32_t crc;
	int16_t i, bit;

	crc = CRC_SEED;

	for (i = 0; i < length; i++)
	{
		crc ^= data[i];

		for (bit = 0; bit < 8; bit++)
		{
			if ((crc & 0x80) != 0)
			{
				crc <<= 1;
				crc ^= CRC_POLYNOMIAL;
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	return (uint8_t)(~crc);
}

uint8_t _crcCalc(uint8_t *crcData, uint8_t length)
{
	return _crc8(crcData, length);
}

void triggerUpdate(TLE5012B_Sensor *sensor)
{
	HAL_GPIO_WritePin(sensor->sck_gpio_port, sensor->sck_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(sensor->mosi_gpio_port, sensor->mosi_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(sensor->cs_gpio_port, sensor->cs_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(sensor->cs_gpio_port, sensor->cs_pin, GPIO_PIN_SET);
}

void resetSafety(TLE5012B_Sensor *sensor)
{
	uint16_t u16RegValue = 0;

	triggerUpdate(sensor);

	HAL_GPIO_WritePin(sensor->cs_gpio_port, sensor->cs_pin, GPIO_PIN_RESET);

	u16RegValue = READ_STA_CMD;
	HAL_SPI_Transmit(sensor->spi, (uint8_t *)(&u16RegValue), sizeof(u16RegValue) / sizeof(uint16_t), 0xFF);
	u16RegValue = DUMMY;
	HAL_SPI_Transmit(sensor->spi, (uint8_t *)(&u16RegValue), sizeof(u16RegValue) / sizeof(uint16_t), 0xFF);
	HAL_SPI_Transmit(sensor->spi, (uint8_t *)(&u16RegValue), sizeof(u16RegValue) / sizeof(uint16_t), 0xFF);

	HAL_GPIO_WritePin(sensor->cs_gpio_port, sensor->cs_pin, GPIO_PIN_SET);
}

errorTypes checkSafety(TLE5012B_Sensor *sensor, uint16_t safety, uint16_t command, uint16_t *readreg, uint16_t length)
{
	errorTypes errorCheck;

	if (!((safety) & SYSTEM_ERROR_MASK))
	{
		errorCheck = SYSTEM_ERROR;
	}
	else if (!((safety) & INTERFACE_ERROR_MASK))
	{
		errorCheck = INTERFACE_ACCESS_ERROR;
	}
	else if (!((safety) & INV_ANGLE_ERROR_MASK))
	{
		errorCheck = INVALID_ANGLE_ERROR;
	}
	else
	{
		uint16_t lengthOfTemp = length * 2 + 2;
		uint8_t temp[CRC_NUM_REGISTERS*2+2];

		temp[0] = _getFirstByte(command);
		temp[1] = _getSecondByte(command);

		for (uint16_t i = 0; i < length; i++)
		{
			temp[2 + 2 * i] = _getFirstByte(readreg[i]);
			temp[2 + 2 * i + 1] = _getSecondByte(readreg[i]);
		}

		uint8_t crcReceivedFinal = _getSecondByte(safety);

		uint8_t crc = _crcCalc(temp, lengthOfTemp);

		if (crc == crcReceivedFinal)
		{
			errorCheck = NO_ERROR;
		}
		else
		{
			errorCheck = CRC_ERROR;
			resetSafety(sensor);
		}
	}

	return errorCheck;
}

errorTypes readFromSensor(TLE5012B_Sensor *sensor, uint16_t command, uint16_t *data)
{
	uint16_t safety = 0;
	uint16_t readreg;
	uint16_t u16RegValue = 0;

	HAL_GPIO_WritePin(sensor->cs_gpio_port, sensor->cs_pin, GPIO_PIN_RESET);

	u16RegValue = command;

#ifndef TLE5012_NOT_MODIFY_MOSI_MANUALLY
	TLE5012_SET_MOSI_MODE_AF_PP();
#endif

	HAL_SPI_Transmit(sensor->spi, (uint8_t *)(&u16RegValue), sizeof(u16RegValue) / sizeof(uint16_t), 0xFF);

#ifndef TLE5012_NOT_MODIFY_MOSI_MANUALLY
	TLE5012_SET_MOSI_MODE_INPUT();
#endif

	HAL_SPI_Receive(sensor->spi, (uint8_t *)(&readreg), 1, 0xFF);
	HAL_SPI_Receive(sensor->spi, (uint8_t *)(&safety), 1, 0xFF);

	HAL_GPIO_WritePin(sensor->cs_gpio_port, sensor->cs_pin, GPIO_PIN_SET);

	errorTypes checkError = checkSafety(sensor, safety, command, &readreg, 1);

	if (checkError != NO_ERROR)
	{
		*data = 0;
		return checkError;
	}
	else
	{
		*data = readreg;
		return NO_ERROR;
	}
}

errorTypes readBlockCRC(TLE5012B_Sensor *sensor)
{
	uint16_t u16RegValue = 0;
	uint16_t safety = 0;

	HAL_GPIO_WritePin(sensor->cs_gpio_port, sensor->cs_pin, GPIO_PIN_RESET);

#ifndef TLE5012_NOT_MODIFY_MOSI_MANUALLY
	TLE5012_SET_MOSI_MODE_AF_PP();
#endif

	u16RegValue = READ_BLOCK_CRC;
	HAL_SPI_Transmit(sensor->spi, (uint8_t *)(&u16RegValue), sizeof(u16RegValue) / sizeof(uint16_t), 0xFFFF);

#ifndef TLE5012_NOT_MODIFY_MOSI_MANUALLY
	TLE5012_SET_MOSI_MODE_INPUT();
#endif

	HAL_SPI_Receive(sensor->spi, (uint8_t *)(sensor->registers), CRC_NUM_REGISTERS, 0xFF);
	HAL_SPI_Receive(sensor->spi, (uint8_t *)(&safety), 1, 0xFF);

	errorTypes checkError = checkSafety(sensor, safety, READ_BLOCK_CRC, sensor->registers, CRC_NUM_REGISTERS);

	return checkError;
}


errorTypes readAngleValue(TLE5012B_Sensor *sensor, uint16_t *data)
{
	uint16_t rawData = 0;
	errorTypes status = readFromSensor(sensor, READ_ANGLE_VAL_CMD, &rawData);

	if (status != NO_ERROR)
	{
		return status;
	}

	rawData = (rawData & (DELETE_BIT_15));
	*data = rawData;

	return NO_ERROR;
}

errorTypes readAngleSpeed(TLE5012B_Sensor *sensor, int16_t *data)
{
	uint16_t rawData = 0;
	errorTypes status = readFromSensor(sensor, READ_ANGLE_SPD_CMD, &rawData);

	if (status != NO_ERROR)
	{
		*data = 0;
		return status;
	}

	rawData = (rawData & (DELETE_BIT_15));

	if (rawData & CHECK_BIT_14)
	{
		rawData = rawData - CHANGE_UINT_TO_INT_15;
	}

	*data = rawData;

	return NO_ERROR;
}

errorTypes readUpdAngleValue(TLE5012B_Sensor *sensor, uint16_t *data)
{
	uint16_t rawData = 0;
	errorTypes status = readFromSensor(sensor, READ_UPD_ANGLE_VAL_CMD, &rawData);

	if (status != NO_ERROR)
	{
		*data = 0;
		return status;
	}

	rawData = (rawData & (DELETE_BIT_15));
	*data = (int16_t)rawData;

	return NO_ERROR;
}

errorTypes readUpdAngleSpeed(TLE5012B_Sensor *sensor, int16_t *data)
{
	uint16_t rawData = 0;
	errorTypes status = readFromSensor(sensor, READ_UPD_ANGLE_SPD_CMD, &rawData);

	if (status != NO_ERROR)
	{
		*data = 0;
		return status;
	}

	rawData = (rawData & (DELETE_BIT_15));

	if (rawData & CHECK_BIT_14)
	{
		rawData = rawData - CHANGE_UINT_TO_INT_15;
	}

	*data = rawData;

	return NO_ERROR;
}

errorTypes readUpdAngleRevolution(TLE5012B_Sensor *sensor, int16_t *data)
{
	uint16_t rawData = 0;

	errorTypes status = readFromSensor(sensor, READ_UPD_ANGLE_REV_CMD, &rawData);

	if (status != NO_ERROR)
	{
		*data = 0;
		return status;
	}

	rawData = (rawData & (DELETE_7BITS));

	if (rawData & CHECK_BIT_9)
	{
		rawData = rawData - CHANGE_UNIT_TO_INT_9;
	}

	*data = rawData;

	return NO_ERROR;
}

errorTypes getNumRevolutions(TLE5012B_Sensor *sensor)
{
	uint16_t rawData = 0;
	errorTypes status = readFromSensor(sensor, READ_ANGLE_REV_CMD, &rawData);

	if (status != NO_ERROR)
	{
		return status;
	}

	rawData = (rawData & (DELETE_7BITS));

	if (rawData & CHECK_BIT_9)
	{
		rawData = rawData - CHANGE_UNIT_TO_INT_9;
	}

	sensor->NumRev = rawData;

	return NO_ERROR;
}

errorTypes readTemp(TLE5012B_Sensor *sensor, int16_t *data)
{
	uint16_t rawData = 0;
	errorTypes status = readFromSensor(sensor, READ_TEMP_CMD, &rawData);

	if (status != NO_ERROR)
	{
		*data = 0;
		return status;
	}

	rawData = (rawData & (DELETE_7BITS));

	if (rawData & CHECK_BIT_9)
	{
		rawData = rawData - CHANGE_UNIT_TO_INT_9;
	}

	*data = rawData;

	return NO_ERROR;
}

float _calculateAngleSpeed(float angRange, int16_t rawAngleSpeed, uint16_t firMD, uint16_t predictionVal)
{
	float finalAngleSpeed;
	float microsecToSec = 0.000001;
	float firMDVal;

	if (firMD == 1)
	{
		firMDVal = 42.7;
	}
	else if (firMD == 0)
	{
		firMDVal = 21.3;
	}
	else if (firMD == 2)
	{
		firMDVal = 85.3;
	}
	else if (firMD == 3)
	{
		firMDVal = 170.6;
	}
	else
	{
		firMDVal = 0;
	}

	finalAngleSpeed = ((angRange / POW_2_15) * ((float)rawAngleSpeed)) / (((float)predictionVal) * firMDVal * microsecToSec);

	return finalAngleSpeed;
}

errorTypes getAngleSpeed(TLE5012B_Sensor *sensor)
{
	int16_t rawAngleSpeed = 0;
	uint16_t firMDVal = 0;
	uint16_t intMode2Prediction = 0;
	uint16_t angleRangeRaw = 0;

	errorTypes checkError = readAngleSpeed(sensor, &rawAngleSpeed);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	checkError = readAngleValue(sensor, &angleRangeRaw);
	sensor->AngleRange = angleRangeRaw;

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	checkError = readFromSensor(sensor, READ_INTMODE_1, &firMDVal);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	firMDVal >>= 14;

	checkError = readFromSensor(sensor, READ_INTMODE_2, &intMode2Prediction);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	if (intMode2Prediction & 0x0004)
	{
		intMode2Prediction = 3;
	}
	else
	{
		intMode2Prediction = 2;
	}

	sensor->AngleSpeed = _calculateAngleSpeed(sensor->AngleRange, rawAngleSpeed, firMDVal, intMode2Prediction);

	return NO_ERROR;
}

errorTypes getAngleValue(TLE5012B_Sensor *sensor)
{
	uint16_t rawAnglevalue = 0;
	errorTypes checkError = readAngleValue(sensor, &rawAnglevalue);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	sensor->AngleValue = ((float)rawAnglevalue) / POW_2_15 * ANGLE_360_VAL;

	return NO_ERROR;
}

errorTypes getUpdAngleSpeed(TLE5012B_Sensor *sensor, float *angleSpeed)
{
	int16_t rawAngleSpeed = 0;
	uint16_t firMDVal = 0;
	uint16_t intMode2Prediction = 0;
	uint16_t angleRangeRaw = 0;

	errorTypes checkError = readUpdAngleSpeed(sensor, &rawAngleSpeed);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	checkError = readAngleValue(sensor, &angleRangeRaw);
	sensor->AngleRange = (float)angleRangeRaw / POW_2_15 * ANGLE_360_VAL;

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	checkError = readFromSensor(sensor, READ_INTMODE_1, &firMDVal);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	checkError = readFromSensor(sensor, READ_INTMODE_2, &intMode2Prediction);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	if (intMode2Prediction & 0x0004)
	{
		intMode2Prediction = 3;
	}
	else
	{
		intMode2Prediction = 2;
	}

	*angleSpeed = _calculateAngleSpeed(sensor->AngleRange, rawAngleSpeed, firMDVal, intMode2Prediction);

	return NO_ERROR;
}

errorTypes getUpdAngleValue(TLE5012B_Sensor *sensor, float *angleValue)
{
	uint16_t rawAnglevalue = 0;
	errorTypes checkError = readUpdAngleValue(sensor, &rawAnglevalue);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	*angleValue = ((float)rawAnglevalue) / POW_2_15 * ANGLE_360_VAL;

	return NO_ERROR;
}

errorTypes getTemperature(TLE5012B_Sensor *sensor)
{
	int16_t rawTemp = 0;
	errorTypes checkError = readTemp(sensor, &rawTemp);

	if (checkError != NO_ERROR)
	{
		return checkError;
	}

	sensor->Temperature = (rawTemp + TEMP_OFFSET) / (TEMP_DIV);

	return NO_ERROR;
}

float MechAngleToElecAngle(float mech_angle, float offset, int pole_pairs)
{
	if (pole_pairs <= 0)
	{
		return 0.0f;
	}

	float total_mech = mech_angle + offset;

	total_mech = fmodf(total_mech, 360.0f);
	if (total_mech < 0.0f)
	{
		total_mech += 360.0f;
	}

	float elec_angle = fmodf(total_mech * pole_pairs, 360.0f);

	if (elec_angle < 0.0f)
	{
		elec_angle += 360.0f;
	}

	return elec_angle;
}
