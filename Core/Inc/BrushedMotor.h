#ifndef __BRUSHED_MOTOR_H
#define __BRUSHED_MOTOR_H

#include "main.h"
#include "tim.h"
#include "STM32_TLE5012B.h"
#include "STM32_TLE5012_Config.h"

#define PWM_PERIOD 1000

#define ENCODER_RESO 360.0f

typedef struct
{
	float coefficient1;
	float coefficient2;
	float last_result;
	float current_result;
} Filter_Structure_t;

typedef struct
{
	TIM_HandleTypeDef *htim;
	uint8_t in1_channel;
	uint8_t in2_channel;
	float speed;
	int8_t direction;
	uint8_t enabled;
} BrushedMotor_t;

typedef struct
{
	TLE5012B_Sensor sensor;
	float mechanical_speed;
	float last_mechanical_angle;
	Filter_Structure_t speed_filter;
} MotorSensor_t;

void BrushedMotor_Init(BrushedMotor_t *motor, TIM_HandleTypeDef *htim, uint8_t in1_channel, uint8_t in2_channel);
void BrushedMotor_SetSpeed(BrushedMotor_t *motor, float speed);
void BrushedMotor_Stop(BrushedMotor_t *motor);
void BrushedMotor_Start(BrushedMotor_t *motor);

extern BrushedMotor_t g_motor1;
extern BrushedMotor_t g_motor2;
extern MotorSensor_t g_sensor1;
extern MotorSensor_t g_sensor2;

void MotorSensor_InitFilter(MotorSensor_t *sensor, float cutoff_freq, float sample_freq);
void MotorSensor_Init(void);
void MotorSensor_UpdateAll(void);

#endif
