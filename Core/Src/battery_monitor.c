#include "battery_monitor.h"
#include <stdio.h>

/******************************************************************************
函数：BatteryMonitor_Init
参数：
  monitor - 电池监控器实例指针
  cell_count - 电池串数（1~MAX_CELL_COUNT）
  level_count - 电量挡位数（2~MAX_LEVEL_COUNT）
  hysteresis - 滞回电压（V）
返回：0-成功，-1-失败
******************************************************************************/
int8_t BatteryMonitor_Init(BatteryMonitor *monitor, uint8_t cell_count, uint8_t level_count, float hysteresis)
{
	if (monitor == NULL || cell_count < 1 || cell_count > MAX_CELL_COUNT ||
	        level_count < 2 || level_count > MAX_LEVEL_COUNT || hysteresis < 0.0f)
	{
		return -1;
	}

	float pack_full = CELL_FULL * cell_count;
	float pack_empty = CELL_EMPTY * cell_count;
	float range = pack_full - pack_empty;

	monitor->cell_count = cell_count;
	monitor->level_count = level_count;
	monitor->hysteresis = hysteresis;
	monitor->last_level = 0;

	for (uint8_t i = 0; i < level_count; i++)
	{
		float ratio = (float)i / (level_count - 1);

		if (ratio < 0.3f)
		{
			ratio = ratio * 1.2f;
		}

		monitor->thresholds[i] = pack_empty + (ratio * range);
		if (monitor->thresholds[i] > pack_full)
		{
			monitor->thresholds[i] = pack_full;
		}
	}

	return 0;
}

int8_t BatteryMonitor_GetLevel(BatteryMonitor *monitor, float current_volt)
{
	if (monitor == NULL)
	{
		return -1;
	}

	uint8_t current_level = 0;
	for (uint8_t i = monitor->level_count - 1; i > 0; i--)
	{
		if (current_volt >= monitor->thresholds[i])
		{
			current_level = i;
			break;
		}
	}

	if (current_level > monitor->last_level)
	{
		if (current_volt <= monitor->thresholds[current_level] + monitor->hysteresis)
		{
			current_level = monitor->last_level;
		}
	}
	else if (current_level < monitor->last_level)
	{
		if (current_volt >= monitor->thresholds[current_level] - monitor->hysteresis)
		{
			current_level = monitor->last_level;
		}
	}

	monitor->last_level = current_level;
	return current_level;
}

void BatteryMonitor_PrintConfig(BatteryMonitor *monitor)
{
	if (monitor == NULL)
	{
		printf("监控器实例为空\n");
		return;
	}

	printf("=== 电池配置信息 ===\n");
	printf("串数：%dS（满电: %.1fV，截止: %.1fV）\n",
	       monitor->cell_count,
	       4.2f * monitor->cell_count,
	       3.0f * monitor->cell_count);
	printf("挡位数：%d挡，滞回：%.2fV\n",
	       monitor->level_count,
	       monitor->hysteresis);
	printf("阈值（从低到高）：\n");
	for (uint8_t i = 0; i < monitor->level_count; i++)
	{
		printf("  挡位%d（%.0f%%）：%.2fV\n",
		       i,
		       (float)i/(monitor->level_count-1) * 100,
		       monitor->thresholds[i]);
	}
	printf("===================\n");
}

