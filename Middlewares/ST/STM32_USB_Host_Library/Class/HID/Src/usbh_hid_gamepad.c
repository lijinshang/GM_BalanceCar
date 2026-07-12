/*****************************************************************************
程序名称：无线游戏手柄驱动
程 序 员：李锦上 Email：lijinshang@126.com 
使用说明：
shanwan公司Android GamePad手柄
按住LB+Home键开机：LED1+LED2模式

10ms调用一次
// 超时检查函数
USBH_HID_CheckGamepadTimeout();

修改历史：
2025-9-17：增加超时清空
2025-8-22：增加游戏手柄解析
2025-8-22：通信3秒左右断开（TGZ Controller手柄接收器问题，使用shanwan公司Android GamePad手柄正常）
2025-8-21：HID通信成功
******************************************************************************/
#include "usbh_hid_gamepad.h"
#include "usbh_hid_parser.h"

uint8_t gamepad_rx_buf[16];
uint8_t Gamepad_report_data[16];

HID_GAMEPAD_Info_TypeDef Gamepad_info;

uint32_t gamepad_data_counter = 0;       // 数据接收计数器，初始为0


/******************************************************************************
函数：游戏手柄初始化
参数：phost: USB主机句柄
返回：USBH状态
******************************************************************************/
USBH_StatusTypeDef USBH_HID_GamepadInit(USBH_HandleTypeDef *phost)
{

	HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;
	HID_Handle->length = (uint16_t)sizeof(gamepad_rx_buf);
	HID_Handle->pData = (uint8_t *)gamepad_rx_buf;

	USBH_HID_FifoInit(&HID_Handle->fifo, phost->device.Data, (uint16_t)(HID_QUEUE_SIZE * 10));
	return USBH_OK;

}

/******************************************************************************
函数：获取游戏手柄信息
参数：phost: USB主机句柄
返回：手柄信息
******************************************************************************/
HID_GAMEPAD_Info_TypeDef *USBH_HID_GetGamepadInfo(USBH_HandleTypeDef *phost)
{
	if (USBH_HID_GamepadDecode(phost) == USBH_OK)
	{
		gamepad_data_counter = 0;  // 收到数据，重置计数器
		return &Gamepad_info;
	}
	else
	{
		return NULL;
	}
}

/******************************************************************************
函数：解析游戏手柄数据
参数：phost: USB主机句柄
返回：USBH状态
******************************************************************************/
static USBH_StatusTypeDef USBH_HID_GamepadDecode(USBH_HandleTypeDef *phost)
{
	HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

	if (HID_Handle->length == 0U)
	{
		return USBH_FAIL;
	}

	// 读取FIFO中的数据
	if (USBH_HID_FifoRead(&HID_Handle->fifo, &Gamepad_report_data, HID_Handle->length) == HID_Handle->length)
	{
		// 验证帧头 (0x07)
		if (Gamepad_report_data[0] != 0x07)
		{
			return USBH_FAIL; // 帧头错误
		}

		// 解析数据到结构体
		Gamepad_info.frame_header = Gamepad_report_data[0];

		// 解析左摇杆 (转换为-128到127范围，0为中位)
		Gamepad_info.left_stick_x = (int8_t)(Gamepad_report_data[1] - 0x80);
		Gamepad_info.left_stick_y = (int8_t)(Gamepad_report_data[2] - 0x80);

		// 解析右摇杆
		Gamepad_info.right_stick_x = (int8_t)(Gamepad_report_data[3] - 0x80);
		Gamepad_info.right_stick_y = (int8_t)(Gamepad_report_data[4] - 0x80);

		// 解析十字键
		Gamepad_info.dpad = (DpadDirection)Gamepad_report_data[5];

		// 解析ABXY和LBRB键
		Gamepad_info.buttons1.A = (Gamepad_report_data[6] >> 0) & 0x01;
		Gamepad_info.buttons1.B = (Gamepad_report_data[6] >> 1) & 0x01;
		Gamepad_info.buttons1.X = (Gamepad_report_data[6] >> 3) & 0x01;
		Gamepad_info.buttons1.Y = (Gamepad_report_data[6] >> 4) & 0x01;
		Gamepad_info.buttons1.LB = (Gamepad_report_data[6] >> 6) & 0x01;
		Gamepad_info.buttons1.RB = (Gamepad_report_data[6] >> 7) & 0x01;

		// 解析Select、Start和LTRT按键
		Gamepad_info.buttons2.LT_btn = (Gamepad_report_data[7] >> 0) & 0x01;
		Gamepad_info.buttons2.RT_btn = (Gamepad_report_data[7] >> 1) & 0x01;
		Gamepad_info.buttons2.Select = (Gamepad_report_data[7] >> 2) & 0x01;
		Gamepad_info.buttons2.Start = (Gamepad_report_data[7] >> 3) & 0x01;

		// 解析扳机
		Gamepad_info.RT_trigger = Gamepad_report_data[8];
		Gamepad_info.LT_trigger = Gamepad_report_data[9];

		return USBH_OK;
	}

	return USBH_FAIL;
}

/******************************************************************************
函数：超时检查函数
参数：无
返回：无
******************************************************************************/
void USBH_HID_CheckGamepadTimeout(void)
{
	// 防止溢出，达到最大值时不再递增
	if (gamepad_data_counter < GAMEPAD_TIMEOUT_COUNT)
	{
		gamepad_data_counter++;
	}
	
	// 检查是否超过设定的超时计数值
	if (gamepad_data_counter >= GAMEPAD_TIMEOUT_COUNT)
	{
		// 清除游戏手柄信息（重置为初始状态）
		memset(&Gamepad_info, 0, sizeof(HID_GAMEPAD_Info_TypeDef));
		Gamepad_info.dpad = DPAD_CENTER;  // 十字键复位到中心
	}
}