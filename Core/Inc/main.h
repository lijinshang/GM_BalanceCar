/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LAMP_F_Pin GPIO_PIN_2
#define LAMP_F_GPIO_Port GPIOC
#define LAMP_B_Pin GPIO_PIN_3
#define LAMP_B_GPIO_Port GPIOC
#define POWER_KEY_Pin GPIO_PIN_4
#define POWER_KEY_GPIO_Port GPIOC
#define POWER_CON_Pin GPIO_PIN_5
#define POWER_CON_GPIO_Port GPIOC
#define OLED_CS_Pin GPIO_PIN_12
#define OLED_CS_GPIO_Port GPIOB
#define OLED_DC_Pin GPIO_PIN_14
#define OLED_DC_GPIO_Port GPIOB
#define LED_RUN_Pin GPIO_PIN_15
#define LED_RUN_GPIO_Port GPIOA
#define M1_CS_Pin GPIO_PIN_10
#define M1_CS_GPIO_Port GPIOC
#define M2_CS_Pin GPIO_PIN_11
#define M2_CS_GPIO_Port GPIOC
#define M3_CS_Pin GPIO_PIN_12
#define M3_CS_GPIO_Port GPIOC
#define WS2812B_Pin GPIO_PIN_8
#define WS2812B_GPIO_Port GPIOB
#define BUZZ_Pin GPIO_PIN_9
#define BUZZ_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define LED_ON				GPIO_PIN_SET
#define LED_OFF				GPIO_PIN_RESET

#define LED_RUN_FLASH			HAL_GPIO_TogglePin(LED_RUN_GPIO_Port, LED_RUN_Pin)
#define LED_RUN_WRITE(a)		HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, (GPIO_PinState)(a))
#define LED_RUN_READ			HAL_GPIO_ReadPin(LED_RUN_GPIO_Port, LED_RUN_Pin)

#define POWER_KEY_READ			HAL_GPIO_ReadPin(POWER_KEY_GPIO_Port, POWER_KEY_Pin)
#define POWER_CON_WRITE(a)		HAL_GPIO_WritePin(POWER_CON_GPIO_Port, POWER_CON_Pin, (GPIO_PinState)(a))

#define SPI2_CS_WRITE(a)		HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, (GPIO_PinState)(a))
#define M1_CS_WRITE(a)			HAL_GPIO_WritePin(M1_CS_GPIO_Port, M1_CS_Pin, (GPIO_PinState)(a))
#define M2_CS_WRITE(a)			HAL_GPIO_WritePin(M2_CS_GPIO_Port, M2_CS_Pin, (GPIO_PinState)(a))
#define M3_CS_WRITE(a)			HAL_GPIO_WritePin(M3_CS_GPIO_Port, M3_CS_Pin, (GPIO_PinState)(a))

#define BUZZ_WRITE(a)			HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, (GPIO_PinState)(a))

#define OLED_DC_WRITE(a)		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, (GPIO_PinState)(a))
#define OLED_CS_WRITE(a)		HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, (GPIO_PinState)(a))


#define LAMP_F_WRITE(a)			HAL_GPIO_WritePin(LAMP_F_GPIO_Port, LAMP_F_Pin, (GPIO_PinState)(a))
#define LAMP_B_WRITE(a)			HAL_GPIO_WritePin(LAMP_B_GPIO_Port, LAMP_B_Pin, (GPIO_PinState)(a))

#define WS2812B_WRITE(a)		HAL_GPIO_WritePin(WS2812B_GPIO_Port, WS2812B_Pin, (GPIO_PinState)(a))

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
