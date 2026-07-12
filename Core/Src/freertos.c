/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Power.h"
#include "adc.h"
#include "spi.h"
#include "tim.h"

#include "BrushedMotor.h"
#include "BalanceCarControl.h"
#include "BuzzerSound.h"
#include "battery_monitor.h"
#include "SerialDebug.h"
#include "ws2812b.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osMessageQDef(BuzzerQueue, 1, SoundType);
osMessageQId BuzzerQueueHandle;

/* USER CODE END Variables */
osThreadId TimeBase_TaskHandle;
osThreadId BUZZ_TaskHandle;
osThreadId Menu_TaskHandle;
osThreadId SmartCar_TaskHandle;
osThreadId Navigation_TaskHandle;
osMutexId SPI_MutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Start_TimeBase_Task(void const * argument);
void Start_BUZZ_Task(void const * argument);
void Start_Menu_Task(void const * argument);
void Start_SmartCar_Task(void const * argument);
void Start_Navigation_Task(void const * argument);

extern void MX_USB_HOST_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of SPI_Mutex */
  osMutexDef(SPI_Mutex);
  SPI_MutexHandle = osMutexCreate(osMutex(SPI_Mutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  BuzzerQueueHandle = osMessageCreate(osMessageQ(BuzzerQueue), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of TimeBase_Task */
  osThreadDef(TimeBase_Task, Start_TimeBase_Task, osPriorityAboveNormal, 0, 256);
  TimeBase_TaskHandle = osThreadCreate(osThread(TimeBase_Task), NULL);

  /* definition and creation of BUZZ_Task */
  osThreadDef(BUZZ_Task, Start_BUZZ_Task, osPriorityAboveNormal, 0, 128);
  BUZZ_TaskHandle = osThreadCreate(osThread(BUZZ_Task), NULL);

  /* definition and creation of Menu_Task */
  osThreadDef(Menu_Task, Start_Menu_Task, osPriorityLow, 0, 512);
  Menu_TaskHandle = osThreadCreate(osThread(Menu_Task), NULL);

  /* definition and creation of SmartCar_Task */
  osThreadDef(SmartCar_Task, Start_SmartCar_Task, osPriorityAboveNormal, 0, 256);
  SmartCar_TaskHandle = osThreadCreate(osThread(SmartCar_Task), NULL);

  /* definition and creation of Navigation_Task */
  osThreadDef(Navigation_Task, Start_Navigation_Task, osPriorityNormal, 0, 256);
  Navigation_TaskHandle = osThreadCreate(osThread(Navigation_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Start_TimeBase_Task */
/**
  * @brief  Function implementing the TimeBase_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_TimeBase_Task */
void Start_TimeBase_Task(void const * argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN Start_TimeBase_Task */
	uint32_t n_1ms = 0;
	
	osDelay(500);

	__HAL_TIM_CLEAR_IT(&htim10,TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT(&htim10);
	
	WS2812B_Init(&ws2812b, &htim4, TIM_CHANNEL_3);
	LED_ControllerInit(&led_controller, &ws2812b);
	
	LED_SetTargetColor(&led_controller, COLOR_BOOTING);
	LED_SetEffect(&led_controller, EFFECT_CHASE);
	
	/* Infinite loop */
	for(;;)
	{
		n_1ms++;
				
		
		if(n_1ms%10==0)
		{
			ADC_ResultFilter();
			
			PowerHandle();
			
		}
		
		if(n_1ms%50==0)
		{
			SerialDebug_DataWaveform();
		}
		
		if(Power.PowerStatus == ON)
		{
			if(n_1ms % 5 == 0)
			{
				LED_Update(&led_controller);
			}
			if(n_1ms%10==0)
			{
				System.battery_voltage =  Get_Battery_Voltage();
				
				System.battery_level = BatteryMonitor_GetLevel(&System.bat_monitor, System.battery_voltage);
							
				LowBatteryShutdownDetection();
				
			}
			if(n_1ms%100==0)
			{
				LED_UpdateByCarState(&ws2812b, &led_controller);
			}
			if(n_1ms%500==0)
			{
				LED_RUN_FLASH;
			}
		}
		else
		{
			LED_RUN_WRITE(LED_OFF);
		}
		osDelay(1);
	}
  /* USER CODE END Start_TimeBase_Task */
}

/* USER CODE BEGIN Header_Start_BUZZ_Task */
/**
* @brief Function implementing the BUZZ_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_BUZZ_Task */
void Start_BUZZ_Task(void const * argument)
{
  /* USER CODE BEGIN Start_BUZZ_Task */
	buzzer_init(&buzzer_ctrl, &htim11, TIM_CHANNEL_1, 1000000);
	
	buzzer_play_sound_handle_in_task(&buzzer_ctrl, SND_CONNECTION);
  /* Infinite loop */
	for(;;)
	{
		if(!GetSystemActiveState() && System.is_angle_start && !System.need_over_angle_start)
		{
			osMessagePut(BuzzerQueueHandle, (uint32_t)SND_BEEP, 0);
		}
	  
		osEvent event = osMessageGet(BuzzerQueueHandle, 0);
		
		if (event.status == osEventMessage)
		{
			SoundType snd = (SoundType)event.value.v;
			buzzer_play_sound_handle_in_task(&buzzer_ctrl, snd);
		}
		
		osDelay(100);
	}
  /* USER CODE END Start_BUZZ_Task */
}

/* USER CODE BEGIN Header_Start_Menu_Task */
/**
* @brief Function implementing the Menu_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Menu_Task */
void Start_Menu_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Menu_Task */
	
  /* Infinite loop */
	for(;;)
	{
		osDelay(10);
	}
  /* USER CODE END Start_Menu_Task */
}

/* USER CODE BEGIN Header_Start_SmartCar_Task */
/**
* @brief Function implementing the SmartCar_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_SmartCar_Task */
void Start_SmartCar_Task(void const * argument)
{
  /* USER CODE BEGIN Start_SmartCar_Task */
	do{osDelay(1);}
	while(Power.PowerStatus == OFF);
	
	buzzer_play_sound(&buzzer_ctrl, SND_SURPRISE);


	osDelay(100);

	MotorSensor_Init();
    BrushedMotor_Init(&g_motor1, &htim1, 1, 2);
    BrushedMotor_Init(&g_motor2, &htim2, 1, 2);

	buzzer_play_sound(&buzzer_ctrl, SND_HAPPY);

  /* Infinite loop */
	for(;;)
	{

		if(Power.PowerStatus == ON)
		{
			BalanceCarControl();
		}
		
		osDelay(2);
	}
  /* USER CODE END Start_SmartCar_Task */
}

/* USER CODE BEGIN Header_Start_Navigation_Task */
/**
* @brief Function implementing the Navigation_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Navigation_Task */
void Start_Navigation_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Navigation_Task */
	
  /* Infinite loop */
	for(;;)
	{
		osDelay(2);
	}
  /* USER CODE END Start_Navigation_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
