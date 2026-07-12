/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
#define ADC_SAMP_CH_MAX				(4)
#define ADC_SAMP_NUM				10

#define ADC_CH_BATTERY_VOLTAGE		0
#define ADC_CH_M1_CURRENT			1
#define ADC_CH_M2_CURRENT			2
#define ADC_CH_M3_CURRENT			3

#define BATTERY_R1					20000
#define BATTERY_R2					2000

#define ADC_REF_VOLTAGE				3.3f
#define ADC_RESOLUTION				4096.0f
#define CURRENT_ZERO_ADC			2048
#define INA181A1_GAIN				20.0f
#define CURRENT_SENSE_RESISTOR		0.01f

extern uint16_t uhADCxConvertedValue[ADC_SAMP_NUM][ADC_SAMP_CH_MAX];
extern uint16_t ADC_Result[ADC_SAMP_CH_MAX];

extern const float* BatteryCapacity;
extern const float BatteryCapacityTab[];
extern const float BatteryChrgCapacityTab[];
extern float battery_voltage;
/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */
void ADC_ResultFilter(void);
void Power_Check(void);
float Get_Battery_Voltage(void);
float Get_Current(uint16_t filtered_adc_value);
float Get_M1_Current(void);
float Get_M2_Current(void);
float Get_M3_Current(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

