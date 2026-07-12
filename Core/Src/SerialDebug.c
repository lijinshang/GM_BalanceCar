/*********************************************************************************************
//                     GM SerialDebug串口调试软件 by 李锦上&#xA;
// * 功能简介 &#xA;
// * =======================================&#xA;
// * 1.串口基本功能 &#xA;
// * 支持文本模式、HEX模式发送和接收； &#xA;
// * 支持自动搜索可用串口； &#xA;
// * 串口打开时可设置波特率； &#xA;
// * 支持自动发送； &#xA;
// * 支持保存接收数据、打开发送数据； &#xA;
// * 支持HEX模式发送时自动格式化输入数据：自动去除多余的空格、自动拆分少输入的空格、自动补0；&#xA;
// * 例如输入：A0 b  c D0E  &#xA;
// * 格式化后：A0 0B 0C D0 0E &#xA;
// * 选择：暂停接收后可选择接收区数据，否则不能选择 &#xA;
// * 自动发送：发送周期小于10ms按10ms处理 &#xA;
// * 智能滚动：鼠标在接受区内不自动滚动到底部，否则收到新数据自动滚动至底部方便观察 &#xA;
// * 分割帧：在HEX模式接收时自动尝试分割帧 &#xA;&#xA;
// * 自适应：所有窗体控件自适应大小
// * =======================================&#xA;
// * 2.数据示波功能：当连接的串口收到指定格式的数据时会将数据转成波形显示在数据示波窗口。&#xA;
// * 通信协议：支持CH0-CH9十个通道。&#xA;
// * 帧格式例：A5 5A 0B A0 A4 70 45 41 A4 70 45 C1 AA  &#xA;
// * Byte0-1 ：A5 5A 帧头 &#xA;
// * Byte2   ：0B 本帧包含的字节数，除了起始字节外 &#xA;
// * Byte3   ：A0 功能码 数据示波 &#xA;
// * Byte4-7 ：A4 70 45 41 (float) CH0通道的数据:12.34 &#xA;
// * Byte8-11：A4 70 45 C1 (float) CH1通道的数据:-12.34 &#xA;
// * Byte12  ：AA 帧尾 &#xA;
// * 点击右侧CH0-CH9通道按钮，可隐藏或显示曲线。隐藏曲线后只显示通道号，显示曲线时会显示当前通道的数据 &#xA;
// * 每屏点数：每屏固定显示点数，超过此值如在自动滚屏状态会自动滚屏；在鼠标拖拽查看时此项无效 &#xA;
// * 自动滚屏：自动跟踪最新数据并向左滚动屏幕 &#xA;
// * 暂停：暂停接收帧数据并停止更新屏幕 &#xA;
// * 保存图像：将屏幕的数据保存成PNG图片格式 &#xA;
// * 保存数据：将收到的波形数据保存成.CSV格式以便用Excel分析 &#xA;
// * 缩放：鼠标放在图表区滚动滚轮可缩放x轴；按住Ctrl+滚轮可同步缩放x轴和y轴；按住Ctrl+鼠标拖动可选区放大 &#xA;
// * 自测试：可以自动生成CH0-CH9十通道正弦数据供测试用 &#xA; &#xA;
// * =======================================&#xA;
// * 3.参数调试功能：当勾选启用参数调试时，串口发送部分由参数调试窗口接管，此时串口发送区内容无效。&#xA;
// * LED通信协议：&#xA;
// * 帧格式例：A5 5A 0B B0 01 00 FF 00 00 00 00 00 AA &#xA;
// * Byte0-1 ：A5 5A 帧头 &#xA;
// * Byte2   ：0B 本帧包含的字节数，除了起始字节外 &#xA;
// * Byte3   ：B0 功能码 LED状态 &#xA;
// * Byte4   ：01 LED0的状态:非0点亮 &#xA;
// * Byte5   ：00 LED1的状态:为0熄灭 &#xA;
// * Byte6   ：FF LED2的状态:非0点亮 &#xA;
// * Byte7   ：00 LED3的状态:为0熄灭 &#xA;
// * Byte8   ：00 LED4的状态:为0熄灭 &#xA;
// * Byte9   ：00 LED5的状态:为0熄灭 &#xA;
// * Byte10  ：00 LED6的状态:为0熄灭 &#xA;
// * Byte11  ：00 LED7的状态:为0熄灭 &#xA;
// * Byte12  ：AA 帧尾 &#xA; &#xA;
// * 滑块通信协议：&#xA;
// * 帧格式例：A5 5A 23 C0 A4 70 45 41 B8 1E 63 42     &#xA;
// *           00 00 00 00 00 00 00 00 00 00 00 00     &#xA;
// *           00 00 00 00 00 00 00 00 00 00 00 00 AA  &#xA;
// * Byte0-1 ：A5 5A 帧头 &#xA;
// * Byte2   ：23 本帧包含的字节数，除了起始字节外 &#xA;
// * Byte3   ：C0 功能码 参数调试-滑块 &#xA;
// * Byte4-7 ：A4 70 45 41 (float) 滑块0的数据:12.34 &#xA;
// * Byte8-11：B8 1E 63 42 (float) 滑块1的数据:56.78 &#xA;
// * Byte12-35：滑块2-滑块7的数据 &#xA;
// * Byte36  ：AA 帧尾 &#xA; &#xA;
// * 按钮通信协议：&#xA;
// * 帧格式例：A5 5A 05 D0 00 01 AA  &#xA;
// * Byte0-1 ：A5 5A 帧头 &#xA;
// * Byte2   ：05 本帧包含的字节数，除了起始字节外 &#xA;
// * Byte3   ：D0 功能码 参数调试-按钮 &#xA;
// * Byte4   ：00 按钮数据高字节:备用 &#xA;
// * Byte5   ：01 按钮数据低字节:按钮0按下 (0x80:按钮7按下) &#xA;
// * Byte6   ：AA 帧尾 &#xA; &#xA;
// * 保存配置：保存界面设置 &#xA; &#xA;
// * =======================================&#xA;
// * 软件更新地址：http://pan.baidu.com/s/1kVF8OdD &#xA;
// * E-mail:lijinshang@126.com &#xA;
**********************************************************************************************/

#include "math.h"
#include "usart.h"

#include "SerialDebug.h"
#include "BalanceCarControl.h"
#include "BrushedMotor.h"


//模式选择
//#define MODEL		SIN_WAVE	//正弦波
//#define MODEL		SELF_DEBUG	//自我调试
#define MODEL		NORMAL		//正常使用

float Parameter[8] = {0};		//滑块0-7 滑块值会更新到这里
unsigned int Button = 0;		//按钮0-7 按钮状态会更新到这里
unsigned char LED_State[8] = {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};		//LED0-7 LED状态会更新到上位机

unsigned char SerialDebugBuffer[45] = {0};		//SerialDebug数据缓冲


unsigned char count_ch0 = 100 / 10*0;
unsigned char count_ch1 = 100 / 10*1;
unsigned char count_ch2 = 100 / 10*2;
unsigned char count_ch3 = 100 / 10*3;
unsigned char count_ch4 = 100 / 10*4;
unsigned char count_ch5 = 100 / 10*5;
unsigned char count_ch6 = 100 / 10*6;
unsigned char count_ch7 = 100 / 10*7;
unsigned char count_ch8 = 100 / 10*8;
unsigned char count_ch9 = 100 / 10*9;

float SinTab[100] = {//正弦表
			0.00000, 0.06276, 0.12527, 0.18729, 0.24857, 0.30887, 0.36795, 0.42558, 0.48153, 0.53558,
			0.58753, 0.63715, 0.68427, 0.72869, 0.77023, 0.80874, 0.84405, 0.87605, 0.90458, 0.92955,
			0.95086, 0.96842, 0.98216, 0.99202, 0.99798, 1.00000, 0.99808, 0.99222, 0.98245, 0.96881,
			0.95135, 0.93014, 0.90526, 0.87681, 0.84491, 0.80967, 0.77124, 0.72977, 0.68543, 0.63838,
			0.58882, 0.53693, 0.48293, 0.42702, 0.36943, 0.31038, 0.25011, 0.18885, 0.12685, 0.06435,
			0.00159, -0.06117, -0.12369, -0.18572, -0.24702, -0.30735, -0.36647, -0.42414, -0.48013, -0.53424,
			-0.58624, -0.63593, -0.68311, -0.72759, -0.76921, -0.80780, -0.84320, -0.87528, -0.90390, -0.92897,
			-0.95037, -0.96802, -0.98185, -0.99182, -0.99788, -1.00000, -0.99818, -0.99242, -0.98275, -0.96921,
			-0.95184, -0.93072, -0.90594, -0.87758, -0.84576, -0.81061, -0.77226, -0.73086, -0.68659, -0.63961,
			-0.59010, -0.53827, -0.48432, -0.42846, -0.37091, -0.31189, -0.25165, -0.19042, -0.12843, -0.06594
		};


union
{
	unsigned char floatByte[4];
	float floatValue;
}FloatUnion;

/*********************************************************************************************
函数名：void Float2Byte(float *FloatValue, unsigned char *Byte, unsigned char Subscript)
参  数：FloatValue:float值
		Byte:数组
		Subscript:指定从数组第几个元素开始写入
返回值：无
结  果：将float数据转成4字节数据并存入指定地址
**********************************************************************************************/
void Float2Byte(float *FloatValue, unsigned char *Byte, unsigned char Subscript)
{
	FloatUnion.floatValue = (float)2;
	
	if(FloatUnion.floatByte[0] == 0)
	{//小端模式
		FloatUnion.floatValue = *FloatValue;

		Byte[Subscript]     = FloatUnion.floatByte[0];
		Byte[Subscript + 1] = FloatUnion.floatByte[1];
		Byte[Subscript + 2] = FloatUnion.floatByte[2];
		Byte[Subscript + 3] = FloatUnion.floatByte[3];
	}
	else
	{//大端模式
		FloatUnion.floatValue = *FloatValue;

		Byte[Subscript]     = FloatUnion.floatByte[3];
		Byte[Subscript + 1] = FloatUnion.floatByte[2];
		Byte[Subscript + 2] = FloatUnion.floatByte[1];
		Byte[Subscript + 3] = FloatUnion.floatByte[0];
	}
}

/*********************************************************************************************
函数名：void Byte2Float(unsigned char *Byte, unsigned char Subscript, float *FloatValue)
参  数：Byte:数组
		Subscript:指定从数组第几个元素开始写入
		FloatValue:float值
返回值：无
结  果：从指定地址将4字节数据转成float数据
**********************************************************************************************/
void Byte2Float(unsigned char *Byte, unsigned char Subscript, float *FloatValue)
{
	FloatUnion.floatByte[0] = Byte[Subscript];
	FloatUnion.floatByte[1] = Byte[Subscript + 1];
	FloatUnion.floatByte[2] = Byte[Subscript + 2];
	FloatUnion.floatByte[3] = Byte[Subscript + 3];

	*FloatValue = FloatUnion.floatValue;
}

/*********************************************************************************************
函数名：void SerialDebug_SetChannelData(unsigned char Channel, float Data)
参  数：Channel:选择通道(1-10)
		Data:通道数据
返回值：无
结  果：将待发送通道的浮点数据写入发送缓冲区
**********************************************************************************************/
void SerialDebug_SetChannelData(unsigned char Channel, float Data)
{
	if (Channel > 9) return ;

	Float2Byte(&Data, SerialDebugBuffer, 4 + Channel * 4);
}

/*********************************************************************************************
函数名：unsigned char SerialDebug_DataWaveform_Generate(unsigned char Channel_Number)
参  数：Channel_Number:需要发送的通道个数
返回值：返回发送缓冲区数据个数
		返回0表示帧格式生成失败
结  果：SerialDebug数据示波:生成帧数据
**********************************************************************************************/
unsigned char SerialDebug_DataWaveform_Generate(unsigned char Channel_Number)
{
	if (Channel_Number > 10) return 0;		//通道个数大于9，直接跳出

	SerialDebugBuffer[0] = 0xA5;		//帧头
	SerialDebugBuffer[1] = 0x5A;		//帧头
	SerialDebugBuffer[2] = 3 + Channel_Number * 4;		//本帧包含的字节数，除了起始字节外
	SerialDebugBuffer[3] = 0xA0;		//功能码 数据示波
	SerialDebugBuffer[4 + Channel_Number * 4] = 0xAA;		//帧尾

	return 4 + Channel_Number * 4 + 1;
}

/*********************************************************************************************
函数名：unsigned char SerialDebug_ParameterDebug_LED_Generate(void)
参  数：无
返回值：返回发送缓冲区数据个数
结  果：SerialDebug参数调试-LED:生成帧数据
**********************************************************************************************/
unsigned char SerialDebug_ParameterDebug_LED_Generate(void)
{
	SerialDebugBuffer[0] = 0xA5;		//帧头
	SerialDebugBuffer[1] = 0x5A;		//帧头
	SerialDebugBuffer[2] = 0x0B;		//本帧包含的字节数，除了起始字节外
	SerialDebugBuffer[3] = 0xB0;		//功能码 参数调试:LED

	SerialDebugBuffer[4] = LED_State[0];
	SerialDebugBuffer[5] = LED_State[1];
	SerialDebugBuffer[6] = LED_State[2];
	SerialDebugBuffer[7] = LED_State[3];
	SerialDebugBuffer[8] = LED_State[4];
	SerialDebugBuffer[9] = LED_State[5];
	SerialDebugBuffer[10] = LED_State[6];
	SerialDebugBuffer[11] = LED_State[7];

	SerialDebugBuffer[12] = 0xAA;		//帧尾

	return 13;
}


/*以下是SerialDebug接口函数*/
/*********************************************************************************************
函数名：void SerialDebug_DataWaveform(void)
参  数：无
返回值：无
结  果：SerialDebug数据示波:向数据示波窗体添加一组数据
备  注：推荐100ms调用一次
**********************************************************************************************/
void SerialDebug_DataWaveform(void)
{
	unsigned char i;          
	unsigned char Send_Count; 
	
	//制造数据
#if MODEL == SIN_WAVE		//SIN波形
	SerialDebug_SetChannelData(0, SinTab[count_ch0]);
	SerialDebug_SetChannelData(1, SinTab[count_ch1]);
	SerialDebug_SetChannelData(2, SinTab[count_ch2]);
	SerialDebug_SetChannelData(3, SinTab[count_ch3]);
	SerialDebug_SetChannelData(4, SinTab[count_ch4]);
	SerialDebug_SetChannelData(5, SinTab[count_ch5]);
	SerialDebug_SetChannelData(6, SinTab[count_ch6]);
	SerialDebug_SetChannelData(7, SinTab[count_ch7]);
	SerialDebug_SetChannelData(8, SinTab[count_ch8]);
	SerialDebug_SetChannelData(9, SinTab[count_ch9]);
	
	count_ch0++;
	count_ch1++;
	count_ch2++;
	count_ch3++;
	count_ch4++;
	count_ch5++;
	count_ch6++;
	count_ch7++;
	count_ch8++;
	count_ch9++;

	if (count_ch0 > 99) count_ch0 = 0;
	if (count_ch1 > 99) count_ch1 = 0;
	if (count_ch2 > 99) count_ch2 = 0;
	if (count_ch3 > 99) count_ch3 = 0;
	if (count_ch4 > 99) count_ch4 = 0;
	if (count_ch5 > 99) count_ch5 = 0;
	if (count_ch6 > 99) count_ch6 = 0;
	if (count_ch7 > 99) count_ch7 = 0;
	if (count_ch8 > 99) count_ch8 = 0;
	if (count_ch9 > 99) count_ch9 = 0;

#endif
#if MODEL == SELF_DEBUG		//自我调试
	SerialDebug_SetChannelData(0, Parameter[0]);
	SerialDebug_SetChannelData(1, Parameter[1]);
	SerialDebug_SetChannelData(2, Parameter[2]);
	SerialDebug_SetChannelData(3, Parameter[3]);
	SerialDebug_SetChannelData(4, Parameter[4]);
	SerialDebug_SetChannelData(5, Parameter[5]);
	SerialDebug_SetChannelData(6, Parameter[6]);
	SerialDebug_SetChannelData(7, Parameter[7]);
#endif

#if MODEL == NORMAL		//正常使用
	SerialDebug_SetChannelData(0, mpu6xxx.accel.x);		//通道0
	SerialDebug_SetChannelData(1, mpu6xxx.accel.y);		//通道1
	SerialDebug_SetChannelData(2, mpu6xxx.accel.z);		//通道2
	SerialDebug_SetChannelData(3, mpu6xxx.gyro.x);		//通道3
	SerialDebug_SetChannelData(4, mpu6xxx.gyro.y);		//通道4
	SerialDebug_SetChannelData(5, mpu6xxx.gyro.z);		//通道5
	SerialDebug_SetChannelData(6, EulerAngles.Roll);	//通道6
	SerialDebug_SetChannelData(7, EulerAngles.Pitch);	//通道7
	SerialDebug_SetChannelData(8, EulerAngles.Yaw);		//通道8
	SerialDebug_SetChannelData(9, mpu6xxx.accel.y*100);	//通道9
	
//	SerialDebug_SetChannelData(0, TinyFOC1.last_mechanical_angle);		//通道0
//	SerialDebug_SetChannelData(1, TinyFOC2.last_mechanical_angle);		//通道1
//	SerialDebug_SetChannelData(2, TinyFOC1.elect_angle);		//通道2
//	SerialDebug_SetChannelData(3, TinyFOC2.elect_angle);		//通道3
//	SerialDebug_SetChannelData(4, TinyFOC1.total_machine_angle);		//通道4
//	SerialDebug_SetChannelData(5, TinyFOC2.total_machine_angle);		//通道5
//	SerialDebug_SetChannelData(6, 0);		//通道6
//	SerialDebug_SetChannelData(7, 0);		//通道7
//	SerialDebug_SetChannelData(8, 0);		//通道8
//	SerialDebug_SetChannelData(9, 0);		//通道9

#endif

	//制造数据结束

	Send_Count = SerialDebug_DataWaveform_Generate(10);		//获取10通道数据帧

	COM6.TX_Len = 0;
	for ( i = 0 ; i < Send_Count; i++)		//添加到串口发送缓冲区
	{//这里需要改成自己的串口发送函数
//		USART_SendData(USART1, SerialDebugBuffer[i]);
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
		COM6_TX_Buffer[COM6.TX_Len++] = SerialDebugBuffer[i];
	}
	
//	USART_SendData(USART1, TX1_Buffer[COM6.TX_Cnt++]);
//	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
//	if(HAL_UART_Transmit_IT(&huart3,(uint8_t*)COM6_TX_Buffer,COM6.TX_Len)!=HAL_OK)
//		Error_Handler();
	USART_SendData_DMA(&huart6,COM6_TX_Buffer,COM6.TX_Len);

}

/*********************************************************************************************
函数名：void SerialDebug_ParameterDebug_LED(void)
参  数：无
返回值：无
结  果：SerialDebug参数调试-LED
备  注：更新LED状态时调用，与SerialDebug_DataWaveform()函数调用间隔至少50ms以上
**********************************************************************************************/
void SerialDebug_ParameterDebug_LED(void)
{
	unsigned char i;         
	unsigned char Send_Count;
	
	//更新LED状态
	LED_State[0] = ~LED_State[0];
	LED_State[1] = ~LED_State[1];
	LED_State[2] = ~LED_State[2];
	LED_State[3] = ~LED_State[3];
	LED_State[4] = ~LED_State[4];
	LED_State[5] = ~LED_State[5];
	LED_State[6] = ~LED_State[6];
	LED_State[7] = ~LED_State[7];

	Send_Count = SerialDebug_ParameterDebug_LED_Generate();		//获取LED数据帧

	COM6.TX_Len = 0;
	for ( i = 0 ; i < Send_Count; i++)		//添加到串口发送缓冲区
	{//这里需要改成自己的串口发送函数
//		USART_SendData(USART1, SerialDebugBuffer[i]);
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
		COM6_TX_Buffer[COM6.TX_Len++] = SerialDebugBuffer[i];
	}

//	USART_SendData(USART1, TX1_Buffer[COM6.TX_Cnt++]);
//	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
//	if(HAL_UART_Transmit_IT(&huart3,(uint8_t*)COM6_TX_Buffer,COM6.TX_Len)!=HAL_OK)
//		Error_Handler();
	
	USART_SendData_DMA(&huart6,COM6_TX_Buffer,COM6.TX_Len);
}

/*********************************************************************************************
函数名：void SerialDebug_ParameterDebug(void)
参  数：无
返回值：无
结  果：SerialDebug参数调试-滑块
备  注：帧结束时调用，即连续5ms以上没收到数据时调用
**********************************************************************************************/
void SerialDebug_ParameterDebug(void)
{
	unsigned char i;

	if (COM6_RX_Buffer[0] == 0xA5 && COM6_RX_Buffer[1] == 0x5A && COM6_RX_Buffer[COM6_RX_Buffer[2] + 1] == 0xAA)
	{
		switch (COM6_RX_Buffer[3])
		{
		case 0xC0:		//滑块
			for (i = 0;i < (COM6_RX_Buffer[2] - 2) / 4;i++)
				Byte2Float(COM6_RX_Buffer, i*4 + 4, &Parameter[i]);
			break;
		case 0xD0:		//按钮
			Button = (unsigned int)COM6_RX_Buffer[4] << 8 | COM6_RX_Buffer[5];
			break;
		default: break;
		}
	}
}
