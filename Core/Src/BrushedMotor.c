#include "BrushedMotor.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

static TLE5012B_Sensor tle5012b_1 = {0};
static TLE5012B_Sensor tle5012b_2 = {0};

BrushedMotor_t g_motor1 = {0};
BrushedMotor_t g_motor2 = {0};
MotorSensor_t g_sensor1 = {0};
MotorSensor_t g_sensor2 = {0};

static void SetPWM(BrushedMotor_t *motor, uint32_t in1_compare, uint32_t in2_compare)
{
	switch (motor->in1_channel)
	{
		case 1:
			__HAL_TIM_SET_COMPARE(motor->htim, TIM_CHANNEL_1, in1_compare);
			break;
		case 2:
			__HAL_TIM_SET_COMPARE(motor->htim, TIM_CHANNEL_2, in1_compare);
			break;
		case 3:
			__HAL_TIM_SET_COMPARE(motor->htim, TIM_CHANNEL_3, in1_compare);
			break;
	}
	switch (motor->in2_channel)
	{
		case 1:
			__HAL_TIM_SET_COMPARE(motor->htim, TIM_CHANNEL_1, in2_compare);
			break;
		case 2:
			__HAL_TIM_SET_COMPARE(motor->htim, TIM_CHANNEL_2, in2_compare);
			break;
		case 3:
			__HAL_TIM_SET_COMPARE(motor->htim, TIM_CHANNEL_3, in2_compare);
			break;
	}
}

void BrushedMotor_Init(BrushedMotor_t *motor, TIM_HandleTypeDef *htim, uint8_t in1_channel, uint8_t in2_channel)
{
	motor->htim = htim;
	motor->in1_channel = in1_channel;
	motor->in2_channel = in2_channel;
	motor->speed = 0.0f;
	motor->direction = 1;
	motor->enabled = 0;

	switch (in1_channel)
	{
		case 1:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
			break;
		case 2:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
			break;
		case 3:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
			break;
	}
	switch (in2_channel)
	{
		case 1:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
			break;
		case 2:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2);
			break;
		case 3:
			HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
			break;
	}

	SetPWM(motor, PWM_PERIOD / 2, PWM_PERIOD / 2);
}

void BrushedMotor_SetSpeed(BrushedMotor_t *motor, float speed)
{
	if (!motor->enabled)
		return;

	motor->speed = speed;
	int8_t dir = (speed >= 0) ? 1 : -1;
	float abs_speed = (speed >= 0) ? speed : -speed;
	uint32_t pwm_value = (uint32_t)(abs_speed * PWM_PERIOD);

	if (pwm_value > PWM_PERIOD) pwm_value = PWM_PERIOD;

	if (dir > 0)
	{
		SetPWM(motor, pwm_value, 0);
	}
	else
	{
		SetPWM(motor, 0, pwm_value);
	}
}

void BrushedMotor_Stop(BrushedMotor_t *motor)
{
	motor->enabled = 0;
	SetPWM(motor, 0, 0);
	motor->speed = 0.0f;
}

void BrushedMotor_Start(BrushedMotor_t *motor)
{
	motor->enabled = 1;
}

/******************************************************************************
函数：配置一阶滤波器参数
参数：*param：滤波器结构体, cutoff_freq：截止频率, sample_freq：采样频率
返回：无
******************************************************************************/
void filter_coefficient_config(Filter_Structure_t *param, float cutoff_freq, float sample_freq)
{
	memset(param, 0, sizeof(Filter_Structure_t));

	const float tau = 1.0f / (2.0f * 3.14159265358979323846f * cutoff_freq);
	const float alpha = tau * sample_freq;
	const float denom = 1.0f + alpha;

	param->coefficient1 = 1.0f / denom;
	param->coefficient2 = alpha / denom;
}

float filter_update_value(Filter_Structure_t *param, float value)
{
	param->current_result = param->coefficient1 * value + param->coefficient2 * param->last_result;
	param->last_result = param->current_result;
	return param->current_result;
}

void MotorSensor_InitFilter(MotorSensor_t *sensor, float cutoff_freq, float sample_freq)
{
	filter_coefficient_config(&sensor->speed_filter, cutoff_freq, sample_freq);
}

void MotorSensor_Init(void)
{
	tle5012b_1.spi = &hspi1;
	tle5012b_1.cs_gpio_port = M1_CS_GPIO_Port;
	tle5012b_1.cs_pin = M1_CS_Pin;
	tle5012b_1.mosi_gpio_port = TLE5012_MOSI_GPIO_Port;
	tle5012b_1.mosi_pin = TLE5012_MOSI_Pin;
	tle5012b_1.sck_gpio_port = TLE5012_SCK_GPIO_Port;
	tle5012b_1.sck_pin = TLE5012_SCK_Pin;

	tle5012b_2.spi = &hspi1;
	tle5012b_2.cs_gpio_port = M2_CS_GPIO_Port;
	tle5012b_2.cs_pin = M2_CS_Pin;
	tle5012b_2.mosi_gpio_port = TLE5012_MOSI_GPIO_Port;
	tle5012b_2.mosi_pin = TLE5012_MOSI_Pin;
	tle5012b_2.sck_gpio_port = TLE5012_SCK_GPIO_Port;
	tle5012b_2.sck_pin = TLE5012_SCK_Pin;

	readBlockCRC(&tle5012b_1);
	getAngleValue(&tle5012b_1);

	readBlockCRC(&tle5012b_2);
	getAngleValue(&tle5012b_2);

	g_sensor1.sensor = tle5012b_1;
	g_sensor1.last_mechanical_angle = tle5012b_1.AngleValue;
	g_sensor1.mechanical_speed = 0.0f;
	MotorSensor_InitFilter(&g_sensor1, 10.0f, 1000.0f);

	g_sensor2.sensor = tle5012b_2;
	g_sensor2.last_mechanical_angle = tle5012b_2.AngleValue;
	g_sensor2.mechanical_speed = 0.0f;
	MotorSensor_InitFilter(&g_sensor2, 10.0f, 1000.0f);
}

void MotorSensor_UpdateAll(void)
{
	float delta_angle;
	float rpm;

	getAngleValue(&tle5012b_1);
	g_sensor1.sensor = tle5012b_1;

	float current_angle = tle5012b_1.AngleValue;
	float last_angle = g_sensor1.last_mechanical_angle;
	delta_angle = current_angle - last_angle;

	if(delta_angle > ENCODER_RESO / 2.0f)
	{
		delta_angle -= ENCODER_RESO;
	}
	else if(delta_angle < -ENCODER_RESO / 2.0f)
	{
		delta_angle += ENCODER_RESO;
	}

	float delta_revs = delta_angle / ENCODER_RESO;
	rpm = (delta_revs / 0.001f) * 60.0f;

	float filtered_rpm = filter_update_value(&g_sensor1.speed_filter, rpm);

	g_sensor1.mechanical_speed = filtered_rpm;
	g_sensor1.last_mechanical_angle = current_angle;

	getAngleValue(&tle5012b_2);
	g_sensor2.sensor = tle5012b_2;

	current_angle = tle5012b_2.AngleValue;
	last_angle = g_sensor2.last_mechanical_angle;
	delta_angle = current_angle - last_angle;

	if(delta_angle > ENCODER_RESO / 2.0f)
	{
		delta_angle -= ENCODER_RESO;
	}
	else if(delta_angle < -ENCODER_RESO / 2.0f)
	{
		delta_angle += ENCODER_RESO;
	}

	delta_revs = delta_angle / ENCODER_RESO;
	rpm = (delta_revs / 0.001f) * 60.0f;

	filtered_rpm = filter_update_value(&g_sensor2.speed_filter, rpm);

	g_sensor2.mechanical_speed = filtered_rpm;
	g_sensor2.last_mechanical_angle = current_angle;
}



