#ifndef WS2812B_H
#define WS2812B_H

#include "main.h"

#define LED_COUNT			4
#define TIMER_PRESCALER		(0)
#define ARR_VALUE			83

#define TIMER_FREQ_MHZ		(84.0f / (TIMER_PRESCALER + 1))
#define COUNT_PERIOD_US		(1.0f / TIMER_FREQ_MHZ)
#define PWM_PERIOD_US		(COUNT_PERIOD_US * (ARR_VALUE + 1))

#define T0H					0.3f
#define T0L					0.7f
#define T1H					0.7f
#define T1L					0.3f
#define RESET_US			80

#define TARGET_BIT_PERIOD	1.0f
#define PERIOD_ERROR		(abs(PWM_PERIOD_US - TARGET_BIT_PERIOD) > 0.3f)

#define PWM_0				((uint16_t)(T0H / COUNT_PERIOD_US + 0.5f))
#define PWM_1				((uint16_t)(T1H / COUNT_PERIOD_US + 0.5f))
#define RESET_COUNT			((uint16_t)(RESET_US / PWM_PERIOD_US + 0.5f))

#define PWM_BUFFER_SIZE		(LED_COUNT * 24 + RESET_COUNT)


typedef struct
{
	uint8_t green;
	uint8_t red;
	uint8_t blue;
} WS2812B_ColorTypeDef;

typedef struct
{
	TIM_HandleTypeDef* htim;
	uint32_t channel;
	WS2812B_ColorTypeDef leds[LED_COUNT];
	uint16_t pwm_buffer[PWM_BUFFER_SIZE];
} WS2812B_DeviceTypeDef;

extern WS2812B_DeviceTypeDef ws2812b;

void WS2812B_Init(WS2812B_DeviceTypeDef* dev, TIM_HandleTypeDef* htim, uint32_t channel);
void WS2812B_SetRGB(WS2812B_DeviceTypeDef* dev, uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void WS2812B_Show(WS2812B_DeviceTypeDef* dev);
void LED_Shutdown(WS2812B_DeviceTypeDef* dev);


typedef enum
{
	EFFECT_SOLID,
	EFFECT_BREATH,
	EFFECT_CHASE,
	EFFECT_GLITCH,
	EFFECT_GRADIENT
} LED_EffectType;

typedef struct
{
	WS2812B_DeviceTypeDef* dev;
	uint8_t current_r;
	uint8_t current_g;
	uint8_t current_b;
	uint8_t target_r;
	uint8_t target_g;
	uint8_t target_b;
	LED_EffectType effect;
	uint16_t effect_step;
	uint8_t brightness;
	uint8_t chase_pos;
	uint16_t effect_divider;
	uint16_t chase_interval;
	uint16_t glitch_interval;
	uint16_t breath_interval;
	uint16_t breath_counter;
} LED_ControllerTypeDef;

extern LED_ControllerTypeDef led_controller;

void LED_ControllerInit(LED_ControllerTypeDef* led_ctrl, WS2812B_DeviceTypeDef* dev);

void LED_SetTargetColor(LED_ControllerTypeDef* led_ctrl, uint8_t r, uint8_t g, uint8_t b);

void LED_SetEffect(LED_ControllerTypeDef* led_ctrl, LED_EffectType effect);

void LED_Update(LED_ControllerTypeDef* led_ctrl);

#endif
