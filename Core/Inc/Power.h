#ifndef __POWER_H__
#define __POWER_H__

#include "main.h"
#include "cmsis_os.h"


#define POWER_BUTTON_PRESS_CNT			150
#define POWER_BUTTON_SHORT_PRESS_CNT	10
#define POWER_BUTTON_CALIB_CLEAR_CNT	1000
#define BATTERY_ZERO_CNT				500

typedef enum{
	OFF = 0,
	ON = 1,
}PowerStatus_Type;

typedef struct{
	PowerStatus_Type PowerStatus;
	uint8_t PowerOff;
	uint16_t PowerButtonPressCnt;
	uint8_t PowerButtonActive;
	uint8_t ShortPressActive;
	uint8_t CalibClearActive;
}Power_Type;

extern Power_Type Power;
extern void PowerOff(void);
extern void ClearDisplay(void);



void PowerHandle(void);
void ForcePowerOff(void);
void LowBatteryShutdownDetection(void);

#endif
