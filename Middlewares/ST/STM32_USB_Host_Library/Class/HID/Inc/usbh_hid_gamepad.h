#ifndef __USBH_HID_GAMEPAD_H
#define __USBH_HID_GAMEPAD_H

#include "usbh_hid.h"

#define HID_NO_SUB_CLASS						0x00U
#define HID_GMAEPAD_BOOT_CODE					0x00U
#define GAMEPAD_TIMEOUT_COUNT					100		//超时计数

// 十字键方向枚举
typedef enum
{
	DPAD_UP = 0,
	DPAD_UP_RIGHT = 1,
	DPAD_RIGHT = 2,
	DPAD_DOWN_RIGHT = 3,
	DPAD_DOWN = 4,
	DPAD_DOWN_LEFT = 5,
	DPAD_LEFT = 6,
	DPAD_UP_LEFT = 7,
	DPAD_CENTER = 8// 中位
} DpadDirection;

// 游戏手柄数据结构体
typedef struct
{
	uint8_t frame_header; // 帧头 (0x07)

	// 左摇杆 (范围: -128到127，0为中位)
	int8_t left_stick_x; // 左遥杆X轴
	int8_t left_stick_y; // 左遥杆Y轴

	// 右摇杆 (范围: -128到127，0为中位)
	int8_t right_stick_x;// 右遥杆X轴
	int8_t right_stick_y;// 右遥杆Y轴

	DpadDirection dpad;// 十字按键方向

	// 按键组1 (ABXY和LBRB)
	struct
	{
		uint8_t A		: 1; // A键 (1=按下, 0=释放)
		uint8_t B		: 1; // B键
		uint8_t			: 1; // 保留位
		uint8_t X		: 1; // X键
		uint8_t Y		: 1; // Y键
		uint8_t			: 1; // 保留位
		uint8_t LB		: 1; // LB键
		uint8_t RB		: 1; // RB键
	} buttons1;

	// 按键组2 (Select、Start和LTRT按键)
	struct
	{
		uint8_t LT_btn	: 1;// LT按键 (1=按下, 0=释放)
		uint8_t RT_btn	: 1;// RT按键
		uint8_t Select	: 1;// Select键
		uint8_t Start	: 1;// Start键
		uint8_t			: 4;// 保留位
	} buttons2;

	uint8_t RT_trigger;// 右扳机 (0x00=释放, 0xFF=按下)
	uint8_t LT_trigger;// 左扳机 (0x00=释放, 0xFF=按下)

} HID_GAMEPAD_Info_TypeDef;


extern uint8_t gamepad_rx_buf[];

// 游戏手柄信息结构体
extern HID_GAMEPAD_Info_TypeDef Gamepad_info;
extern uint32_t gamepad_data_counter; // 数据接收计数器

// 函数声明
USBH_StatusTypeDef USBH_HID_GamepadInit(USBH_HandleTypeDef *phost);
HID_GAMEPAD_Info_TypeDef *USBH_HID_GetGamepadInfo(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_HID_GamepadDecode(USBH_HandleTypeDef *phost);

void USBH_HID_IncrementGamepadCounter(void); // 计数器递增
void USBH_HID_CheckGamepadTimeout(void); // 超时检查

#endif
