/*****************************************************************************
系统框架：李锦上 Email：lijinshang@126.com
硬件系统：通用
软件系统：通用
开发环境：通用
项目名称：软件开关机管理
项目适配：
功能简述：
长按电源键3秒开机
长按电源3秒关机
开机时长按电源键10秒以上清校准数据
应用变更：
修改历史：
2025-12-13：增加一键重置校准：长按开关机10秒清校准数据功能，下次再开机自动校准
2024-11-26：解决由关机调用emwin导致的复位而再次重启问题
2023-6-12：调试完成
2023-6-12：编写完成
立项时间：2023-6-12
控制芯片：通用
******************************************************************************/
#include "Power.h"
#include "BuzzerSound.h"
#include "BalanceCarControl.h"
#include "ws2812b.h"
#include "BrushedMotor.h"
#include "flash_storage.h"
#include "stm32f4xx_hal_cortex.h"

Power_Type Power =
{
	.PowerStatus = OFF,
	.PowerButtonPressCnt = 0,
	.PowerButtonActive = 0,
	.PowerOff = 0,
	.ShortPressActive = 0,
	.CalibClearActive = 0
};

uint8_t GetPowerButton()
{
	return POWER_KEY_READ;
}

void PowerOn(void)
{
	POWER_CON_WRITE(1);
}

void PowerOff(void)
{
	POWER_CON_WRITE(0);
}

void WaitPowerButtonRelease(void)
{
	while(GetPowerButton())
	{
		osDelay(100);
	}
}

void PowerButtonHandle(void)
{
	if(GetPowerButton())
	{
		if(Power.PowerButtonPressCnt < POWER_BUTTON_CALIB_CLEAR_CNT)
		{
			Power.PowerButtonPressCnt++;
		}
		else if(Power.PowerButtonPressCnt == POWER_BUTTON_CALIB_CLEAR_CNT)
		{
			Power.CalibClearActive = 1;
			Power.PowerButtonActive = 0;
			Power.PowerButtonPressCnt++;
		}
		else if(Power.PowerButtonPressCnt == POWER_BUTTON_CALIB_CLEAR_CNT + 1)
		{
			Power.PowerButtonPressCnt = POWER_BUTTON_CALIB_CLEAR_CNT + 2;
		}

		if(Power.PowerButtonPressCnt == POWER_BUTTON_PRESS_CNT &&
		        !Power.CalibClearActive)
		{
			Power.PowerButtonActive = 1;
		}
	}
	else
	{
		if (Power.PowerButtonPressCnt > 0 && Power.PowerButtonPressCnt < POWER_BUTTON_PRESS_CNT)
		{
			if (Power.PowerButtonPressCnt >= POWER_BUTTON_SHORT_PRESS_CNT)
			{
				Power.ShortPressActive = 1;
			}
		}

		Power.PowerButtonPressCnt = 0;
		Power.PowerButtonActive = 0;
		Power.CalibClearActive = 0;
	}
}

void PowerHandle(void)
{
	PowerButtonHandle();

	switch(Power.PowerStatus)
	{
		case OFF:
			if(Power.PowerButtonActive)
			{
				PowerOn();
				Power.PowerStatus = ON;
				Power.PowerButtonActive = 0;
				Power.PowerOff = 0;
				buzzer_play_sound(&buzzer_ctrl, SND_HAPPY_SHORT);
				LED_RUN_WRITE(1);
			}
			break;

		case ON:
			if(Power.PowerButtonActive)
			{
				Power.PowerStatus = OFF;
				Power.PowerButtonActive = 0;
				Power.PowerOff = 1;
				System.last_stop_reason = STOP_MANUAL_BUTTON;

				ManualStopSystem();

				buzzer_play_sound(&buzzer_ctrl, SND_SLEEPING);
				LED_Shutdown(&ws2812b);
				LAMP_F_WRITE(0);
				LAMP_B_WRITE(0);
				LED_RUN_WRITE(0);

				PowerOff();
			}

			if (Power.ShortPressActive)
			{
				Power.ShortPressActive = 0;
				System.last_stop_reason = STOP_MANUAL_BUTTON;
				ManualStopSystem();
				buzzer_play_sound(&buzzer_ctrl, SND_BUTTON_PUSHED);
				System.manual_speed_target = 0.0f;
				System.filtered_turn_target = 0.0f;
				System.filtered_speed_target = 0.0f;
			}
			break;
	}
}

void ForcePowerOff(void)
{
	if (Power.PowerStatus == OFF)
		return;

	Power.PowerStatus = OFF;
	Power.PowerButtonActive = 0;
	Power.PowerOff = 1;

	ManualStopSystem();

	buzzer_play_sound(&buzzer_ctrl, SND_SLEEPING);

	LED_RUN_WRITE(0);

	PowerOff();
}


void LowBatteryShutdownDetection(void)
{
	if (Power.PowerStatus == ON)
	{
		if (System.battery_level == 0)
		{
			if (GetSystemActiveState() && System.shutdown_ramp <= 0.0f)
			{
				if (!System.is_speed_lock)
				{
					buzzer_play_sound(&buzzer_ctrl, SND_SAD);
				}
				System.is_speed_lock = true;
				System.state = SYS_SELF_BALANCE;
				System.last_stop_reason = STOP_LOW_BATTERY;
			}
			else
			{
				System.battery_zero_count++;

				if (System.battery_zero_count >= BATTERY_ZERO_CNT)
				{
					buzzer_play_sound(&buzzer_ctrl, SND_SLEEPING);

					osDelay(1000);

					ForcePowerOff();
				}
			}
		}
		else
		{
			System.battery_zero_count = 0;
		}
	}
}



