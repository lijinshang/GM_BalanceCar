#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <stdint.h>

#define CELL_FULL				4.2f
#define CELL_EMPTY				3.0f
#define MAX_CELL_COUNT			6
#define MAX_LEVEL_COUNT			10
#define DEFAULT_HYSTERESIS		0.1f

typedef struct
{
	uint8_t cell_count;
	uint8_t level_count;
	float thresholds[MAX_LEVEL_COUNT];
	float hysteresis;
	uint8_t last_level;
} BatteryMonitor;

/******************************************************************************
函数：BatteryMonitor_Init
参数：
    monitor: 电池监控器实例指针
    cell_count: 电池串数(1~MAX_CELL_COUNT)
    level_count: 电量挡位数(2~MAX_LEVEL_COUNT)
    hysteresis: 滞回电压(V)
返回：
    0: 初始化成功
    -1: 参数错误
******************************************************************************/
int8_t BatteryMonitor_Init(BatteryMonitor *monitor, uint8_t cell_count,
                           uint8_t level_count, float hysteresis);

/******************************************************************************
函数：BatteryMonitor_GetLevel
参数：
    monitor: 电池监控器实例指针
    current_pack_volt: 当前电池组电压(V)
返回：
    电量挡位(0为最低挡，level_count-1为最高挡)
    -1: 参数错误
******************************************************************************/
int8_t BatteryMonitor_GetLevel(BatteryMonitor *monitor, float current_pack_volt);

/******************************************************************************
函数：BatteryMonitor_PrintConfig
参数：
    monitor: 电池监控器实例指针
返回：无
******************************************************************************/
void BatteryMonitor_PrintConfig(BatteryMonitor *monitor);

#endif
