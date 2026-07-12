/*****************************************************************************
程序名称：WS2812B驱动
程 序 员：李锦上 Email：lijinshang@126.com
使用说明：
WS2812B (TIM PWM + DMA)
定时器配置
预分频器：0（84MHz/(0+1)=42MHz，计数周期 0.02381us）
自动重载值：ARR_VALUE（周期 = (83+1)×0.02381us=2.0us）
通道配置：PWM Generation CH1，Mode 为 PWM Mode 1
DMA Setting：Add -> 选择 TIM_CH1，Direction 为 Memory To Peripheral，Mode 为 Normal
数据宽度：Memory 和 Peripheral 均为 HALFWORD 16bit
优先级：High
禁用 Memory Increment
GPIO 配置
将 TIM_CH1 对应的引脚配置为 Alternate Function Push Pull
速度设置：VeryHigh(必须设置)

*************************************************
//初始化WS2812B
WS2812B_Init(&ws2812b, &htim4, TIM_CHANNEL_1);
//设置颜色
WS2812B_SetRGB(dev, 0, 255,0,0);
//刷新LED显示
WS2812B_Show(dev);

*************************************************
//初始化WS2812B
WS2812B_Init(&ws2812b, &htim4, TIM_CHANNEL_1);
LED_ControllerInit(&led_controller, &ws2812b);

// 设置初始启动效果
LED_SetTargetColor(&led_controller, COLOR_BOOTING);
LED_SetEffect(&led_controller, EFFECT_CHASE);
*************************************************
// 更新LED显示 5ms
LED_Update(&led_controller);
//更新LED颜色 100ms
LED_UpdateByCarState(&led_controller);

修改历史：
2025-9-14：增加扩展功能 动态效果
2025-9-14：精细调整时序 调试成功
2025-9-13：基本编写完成
******************************************************************************/

#include "ws2812b.h"
#include "stdlib.h"
#include "tim.h"

WS2812B_DeviceTypeDef ws2812b;
LED_ControllerTypeDef led_controller;

void WS2812B_Init(WS2812B_DeviceTypeDef* dev, TIM_HandleTypeDef* htim, uint32_t channel)
{
	if (dev == NULL || htim == NULL) return;

	dev->htim = htim;
	dev->channel = channel;

	for (uint16_t i = 0; i < LED_COUNT; i++)
	{
		dev->leds[i].red = 0;
		dev->leds[i].green = 0;
		dev->leds[i].blue = 0;
	}

	for (uint16_t i = 0; i < RESET_COUNT; i++)
	{
		dev->pwm_buffer[LED_COUNT * 24 + i] = 0;
	}
}

void WS2812B_SetRGB(WS2812B_DeviceTypeDef* dev, uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
	if (dev == NULL || index >= LED_COUNT) return;

	dev->leds[index].red = r;
	dev->leds[index].green = g;
	dev->leds[index].blue = b;
}

static void WS2812B_Encode(WS2812B_DeviceTypeDef* dev)
{
	if (dev == NULL) return;

	uint32_t buf_idx = 0;

	for (uint16_t i = 0; i < LED_COUNT; i++)
	{
		for (uint8_t bit = 7; bit != 0xFF; bit--)
		{
			if (dev->leds[i].green & (1 << bit))
			{
				dev->pwm_buffer[buf_idx++] = PWM_1;
			}
			else
			{
				dev->pwm_buffer[buf_idx++] = PWM_0;
			}
		}

		for (uint8_t bit = 7; bit != 0xFF; bit--)
		{
			if (dev->leds[i].red & (1 << bit))
			{
				dev->pwm_buffer[buf_idx++] = PWM_1;
			}
			else
			{
				dev->pwm_buffer[buf_idx++] = PWM_0;
			}
		}

		for (uint8_t bit = 7; bit != 0xFF; bit--)
		{
			if (dev->leds[i].blue & (1 << bit))
			{
				dev->pwm_buffer[buf_idx++] = PWM_1;
			}
			else
			{
				dev->pwm_buffer[buf_idx++] = PWM_0;
			}
		}
	}
}

void WS2812B_Show(WS2812B_DeviceTypeDef* dev)
{
	if (dev == NULL || dev->htim == NULL) return;

	WS2812B_Encode(dev);

	HAL_TIM_PWM_Stop_DMA(dev->htim, dev->channel);

	HAL_TIM_PWM_Start_DMA(dev->htim, dev->channel,
	                      (uint32_t*)dev->pwm_buffer,
	                      PWM_BUFFER_SIZE);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim4)
	{
		HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_3);
	}
}

void LED_Shutdown(WS2812B_DeviceTypeDef* dev)
{
	if (dev == NULL)
	{
		return;
	}

	for (uint16_t i = 0; i < LED_COUNT; i++)
	{
		WS2812B_SetRGB(dev, i, 0, 0, 0);
	}

	WS2812B_Show(dev);
}

void LED_ControllerInit(LED_ControllerTypeDef* led_ctrl, WS2812B_DeviceTypeDef* dev)
{
	if (led_ctrl == NULL) return;

	led_ctrl->dev = dev;
	led_ctrl->current_r = 0;
	led_ctrl->current_g = 0;
	led_ctrl->current_b = 0;
	led_ctrl->target_r = 0;
	led_ctrl->target_g = 0;
	led_ctrl->target_b = 0;
	led_ctrl->effect = EFFECT_SOLID;
	led_ctrl->effect_step = 0;
	led_ctrl->brightness = 128;
	led_ctrl->chase_pos = 0;
	led_ctrl->effect_divider = 0;
	led_ctrl->chase_interval = 5;
	led_ctrl->glitch_interval = 5;
	led_ctrl->breath_interval = 5;
	led_ctrl->breath_counter = 0;
}

static uint8_t color_blend(uint8_t current, uint8_t target, uint8_t speed)
{
	if (current < target)
	{
		return (current + speed > target) ? target : current + speed;
	}
	else if (current > target)
	{
		return (current - speed < target) ? target : current - speed;
	}
	return current;
}

void LED_SetTargetColor(LED_ControllerTypeDef* led_ctrl, uint8_t r, uint8_t g, uint8_t b)
{
	if (led_ctrl == NULL) return;

	float brightness_factor = (float)led_ctrl->brightness / 255.0f;
	led_ctrl->target_r = (uint8_t)(r * brightness_factor);
	led_ctrl->target_g = (uint8_t)(g * brightness_factor);
	led_ctrl->target_b = (uint8_t)(b * brightness_factor);
}

void LED_SetEffect(LED_ControllerTypeDef* led_ctrl, LED_EffectType effect)
{
	if (led_ctrl == NULL) return;
	led_ctrl->effect = effect;
	led_ctrl->effect_step = 0;
}

static void effect_breath(LED_ControllerTypeDef* led_ctrl)
{
	if (led_ctrl->effect_divider % led_ctrl->breath_interval == 0)
	{
		const uint8_t min_brightness = 5;
		const uint8_t max_brightness = 250;
		const uint16_t breath_steps = 100;

		led_ctrl->breath_counter = (led_ctrl->breath_counter + 1) % (breath_steps * 2);

		uint8_t brightness;
		if (led_ctrl->breath_counter < breath_steps)
		{
			brightness = min_brightness +
			             (uint8_t)(((max_brightness - min_brightness) * led_ctrl->breath_counter) / breath_steps);
		}
		else
		{
			uint16_t step = led_ctrl->breath_counter - breath_steps;
			brightness = max_brightness -
			             (uint8_t)(((max_brightness - min_brightness) * step) / breath_steps);
		}

		float bright_ratio = (float)brightness / 255.0f;

		for (uint16_t i = 0; i < LED_COUNT; i++)
		{
			WS2812B_SetRGB(led_ctrl->dev, i,
			               (uint8_t)(led_ctrl->current_r * bright_ratio),
			               (uint8_t)(led_ctrl->current_g * bright_ratio),
			               (uint8_t)(led_ctrl->current_b * bright_ratio));
		}
	}
}

static void effect_chase(LED_ControllerTypeDef* led_ctrl)
{
	for (uint16_t i = 0; i < LED_COUNT; i++)
	{
		WS2812B_SetRGB(led_ctrl->dev, i, 0,0,0);
	}

	WS2812B_SetRGB(led_ctrl->dev, led_ctrl->chase_pos,
	               led_ctrl->current_r, led_ctrl->current_g, led_ctrl->current_b);

	for (int16_t i = 1; i <= 3; i++)
	{
		int16_t pos = (led_ctrl->chase_pos - i + LED_COUNT) % LED_COUNT;
		uint8_t fade = 255 - (i * 64);
		WS2812B_SetRGB(led_ctrl->dev, pos,
		               (led_ctrl->current_r * fade) / 255,
		               (led_ctrl->current_g * fade) / 255,
		               (led_ctrl->current_b * fade) / 255);
	}

	if (led_ctrl->effect_divider % led_ctrl->chase_interval == 0)
	{
		led_ctrl->chase_pos = (led_ctrl->chase_pos + 1) % LED_COUNT;
	}
}

static void effect_glitch(LED_ControllerTypeDef* led_ctrl)
{
	if (led_ctrl->effect_divider % led_ctrl->glitch_interval == 0)
	{
		for (uint16_t i = 0; i < LED_COUNT; i++)
		{
			if ((rand() % 100) < 30)
			{
				WS2812B_SetRGB(led_ctrl->dev, i,
				               led_ctrl->current_r,
				               led_ctrl->current_g,
				               led_ctrl->current_b);
			}
			else if ((rand() % 100) < 5)
			{
				WS2812B_SetRGB(led_ctrl->dev, i,
				               rand() % 128,
				               rand() % 128,
				               rand() % 255);
			}
			else
			{
				WS2812B_SetRGB(led_ctrl->dev, i, 0,0,0);
			}
		}
	}
}

static void effect_gradient(LED_ControllerTypeDef* led_ctrl)
{
	for (uint16_t i = 0; i < LED_COUNT; i++)
	{
		float ratio = (float)i / LED_COUNT;

		uint8_t r = (uint8_t)(led_ctrl->current_r * ratio + (255 - led_ctrl->current_r) * (1 - ratio));
		uint8_t g = (uint8_t)(led_ctrl->current_g * ratio + (255 - led_ctrl->current_g) * (1 - ratio));
		uint8_t b = (uint8_t)(led_ctrl->current_b * ratio + (255 - led_ctrl->current_b) * (1 - ratio));

		WS2812B_SetRGB(led_ctrl->dev, i, r, g, b);
	}
}

void LED_Update(LED_ControllerTypeDef* led_ctrl)
{
	if (led_ctrl == NULL || led_ctrl->dev == NULL) return;

	led_ctrl->current_r = color_blend(led_ctrl->current_r, led_ctrl->target_r, 5);
	led_ctrl->current_g = color_blend(led_ctrl->current_g, led_ctrl->target_g, 5);
	led_ctrl->current_b = color_blend(led_ctrl->current_b, led_ctrl->target_b, 5);

	led_ctrl->effect_divider++;

	switch (led_ctrl->effect)
	{
		case EFFECT_SOLID:
			for (uint16_t i = 0; i < LED_COUNT; i++)
			{
				WS2812B_SetRGB(led_ctrl->dev, i,
							   led_ctrl->current_r,
							   led_ctrl->current_g,
							   led_ctrl->current_b);
			}
			break;

		case EFFECT_BREATH:
			effect_breath(led_ctrl);
			break;

		case EFFECT_CHASE:
			effect_chase(led_ctrl);
			break;

		case EFFECT_GLITCH:
			effect_glitch(led_ctrl);
			break;

		case EFFECT_GRADIENT:
			effect_gradient(led_ctrl);
			break;

		default:
			break;
	}

	WS2812B_Show(led_ctrl->dev);

	led_ctrl->effect_step++;
}
