/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart6;

/* USER CODE BEGIN Private defines */
#define	COM1_TX_BUFFER_LENTH				100
#define	COM1_RX_BUFFER_LENTH				100
#define	COM6_TX_BUFFER_LENTH				100
#define	COM6_RX_BUFFER_LENTH				100

extern uint8_t COM1_TX_Buffer[COM1_TX_BUFFER_LENTH];
extern uint8_t COM1_RX_Buffer[COM1_RX_BUFFER_LENTH];
extern uint8_t COM6_TX_Buffer[COM6_TX_BUFFER_LENTH];
extern uint8_t COM6_RX_Buffer[COM6_RX_BUFFER_LENTH];

typedef struct
{
	FlagStatus DMA_Send_flag;
	uint16_t TX_Cnt;
	uint16_t TX_Len;
	
	uint16_t RX_Cnt;
	uint16_t RX_Len;
	uint16_t RX_TimeOut;
} COMx_Define; 

extern COMx_Define	COM1;
extern COMx_Define	COM6;



/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART6_UART_Init(void);

/* USER CODE BEGIN Prototypes */
void USART_SendData_DMA(UART_HandleTypeDef *huart, uint8_t *pdata, uint16_t Length);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void UsartReceive_IDLE(UART_HandleTypeDef *huart);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

