/*****************************************************************************
项目名称：智能两轮平衡车
程 序 员：李锦上 Email：lijinshang@126.com
使用说明：

静态平衡时，车身在平衡点处如 “锚定” 般稳固，无丝毫左右晃悠的虚位，哪怕轻微外力触碰，也能稳稳停驻在平衡位置，不存在因重心偏移导致的晃动偏移；
动态行驶时，车身反馈格外 “硬朗”，转向、直行过程中无多余形变，操控角度精准且 “不软塌”，车身始终保持刚性联动，不会出现因角度支撑不足导致的 “发飘” 或 “偏摆”。
静若磐石，动若轻风。

控制环：速度环->角度环->转向环->力矩环->电机

直立环只管“别倒”，速度环只管“别跑”，转向环只管“别歪”。

平衡车系统特性：
直立电流小，重载下控制依然游刃有余，波形完美，参数调节直观简单，真正达到"稳如泰山、化险为夷、完美无瑕"的成熟产品状态

智能自适应控制：目标速度/方向双滤波、电流智能衰减，确保重载下依然平滑可控，防止动力异常摔倒


人机交互控制
零功耗软件开关机：待机功耗极低，电源键短按实现"紧急停止"，长按安全关机
USB_HID无线手柄控制：支持无线手柄精准操控，断线自动清零指令，防止失控；支持启动、停止；三档速度控制；方向支持限幅；支持慢速控制
智能启动逻辑：必须在启动角度范围内才能开机；拿起检测退出后需再次超启动角度才允许重启
参数在线调试：支持实时在线调整方向环、速度环、角度环等核心参数

安全防护机制
传感器故障保护：电机角度传感器故障时声音报警并阻止初始化
低电量双重保护：0档低电量禁止启动；平衡中低电量自动关机；行驶中低电量自动锁定速度为0，防止摔车
智能停车关机：行驶中收到关机指令，先锁定速度为0并维持平衡，再次确认后安全关机
过流自动衰减：电流过流时自动衰减输出而非直接停机，防止摔车
姿态异常保护：超过安全倾角自动停机；车轮抬起超时自动停机
二次启动保护：关机后必须先离开启动角度才能二次启动，防止重复启动
电机初始化保护：初始化过程中屏蔽短按电源键，防止误触终止

辅助功能
智能灯效系统：前后照明自动开关；自平衡状态全灯点亮；速度/转向大小映射颜色（慢蓝快红）
音乐蜂鸣提示：非阻塞队列播放，支持开机/关机/低电量/角度异常/启停等多种音效
状态音效反馈：系统状态切换、模式转换均有清晰声音提示
USB便捷充电：支持USB接口充电
遥控喇叭提醒：手柄可触发喇叭提醒


直立电流：150mA
最高速度电流：4A


******************************************************************************/

#include "math.h"
#include "BalanceCarControl.h"
#include "Power.h"
#include "BuzzerSound.h"
#include "ws2812b.h"
#include "usbh_hid_gamepad.h"
#include "BrushedMotor.h"

#define M_PI					3.14159265358979323846f
#define DEG_TO_RAD				(M_PI/180.0f)
#define RAD_TO_DEG				(180.0f/M_PI)
#define SIGN(x)					((x) > 0 ? 1 : -1)



SensorData sensor_data;
MotorOutput motor_output;

System_Type System;

IMU_Data_Type mpu6xxx;
EulerAngles_Type EulerAngles;
IMU_Frame_TypeDef imu_frame;

void UpdateParameters(float *Parameter)
{
	Positional_PID* angle_pid = PID_GetAngleController();
	angle_pid->Kp  = Parameter[0]*ANGLE_KP*2;
	angle_pid->Ki  = Parameter[1]*ANGLE_KI*2;
	angle_pid->Kd  = Parameter[2]*ANGLE_KD*2;

	speed_pid.Kp  = Parameter[3]*SPEED_KP*2;
	speed_pid.Ki  = Parameter[4]*SPEED_KI*2;
	speed_pid.Kd  = Parameter[5]*SPEED_KD*2;
}

void ReadSensors(SensorData* data)
{
	data->roll      = EulerAngles.Roll;
	data->pitch     = EulerAngles.Pitch;
	data->yaw_rate  = mpu6xxx.gyro.z * RAD_TO_DEG;

	data->left_speed  = M1_MECH_SPEED * MOTOR_LEFT_DIR * MOTOR_LEFT_SPEED_DIR;
	data->right_speed = M2_MECH_SPEED * MOTOR_RIGHT_DIR * MOTOR_RIGHT_SPEED_DIR;
	data->avg_speed   = (data->left_speed + data->right_speed) * 0.5f;

}

void SystemReset(void)
{
	System.over_angle_counter = 0;
	System.pickup_counter = 0;
	System.start_counter = 0;
	System.speed_boot_counter = 0;
	System.control_cycle_counter = 0;

	System.start_ramp = 0.0f;
	System.torque_ramp = 0.0f;
	System.shutdown_ramp = 0.0f;

	System.filtered_speed = 0.0f;

	speed_pid.target = 0.0f;
	turn_pid.target = 0.0f;
	PID_Init(PID_GetAngleController());
	PID_SetAngleTarget(BALANCE_ANGLE_ZERO);
	
	speed_pid.integral = 0.0f;
	turn_pid.integral = 0.0f;

	PID_ResetAngleController();
	motor_output.left = 0.0f;
	motor_output.right = 0.0f;
	
	System.imu_rx_timeout = 0;
	System.imu_comm_ok = false;
	System.imu_comm_error_count = 0;

}

void FirstOrderFilter(float *filtered_value, float current_value, float alpha)
{
    *filtered_value = alpha * current_value + (1.0f - alpha) * *filtered_value;
}

void SpeedControlLoop(void)
{
	float output = PID_Calculate(&speed_pid);

	if (System.start_ramp < 1.0f)
		System.start_ramp += 0.05f;

	speed_pid.output = output * System.start_ramp;

	float target = BALANCE_ANGLE_ZERO - speed_pid.output;
	target = CONSTRAIN(target, -SPEED_MAX_OUTPUT, SPEED_MAX_OUTPUT);
	PID_SetAngleTarget(target);

	
}

void AngleControlLoop(void)
{
	PID_UpdateAngle(sensor_data.pitch);
}

void TurnControlLoop(void)
{
	turn_pid.output = PID_Calculate(&turn_pid);
}

void TurnControlOpenLoop(void)
{
	turn_pid.output = turn_pid.target * TURN_OPEN_LOOP_SCALE;
	
	turn_pid.output = CONSTRAIN(turn_pid.output, -TURN_MAX_OUTPUT, TURN_MAX_OUTPUT);
}

void NormalizeOutput(MotorOutput* out)
{
	float max_val = fmaxf(fabsf(out->left), fabsf(out->right));

	if(max_val > MAX_MOTOR_OUTPUT)
	{
		float scale = MAX_MOTOR_OUTPUT / max_val;
		out->left  *= scale;
		out->right *= scale;
	}
}

void ApplyMotorOutput(MotorOutput* output)
{
	float ramp = 1.0f;

	if (System.shutdown_ramp > 0.0f)
	{
		ramp = System.shutdown_ramp;
	}
	else if (GetSystemActiveState())
	{
		if (System.torque_ramp < 1.0f)
			System.torque_ramp += 0.01f;
		ramp = System.torque_ramp;
	}

	float target_left = output->left  * MOTOR_LEFT_DIR  * ramp;
	float target_right = output->right * MOTOR_RIGHT_DIR * ramp;

	float m1_current_abs = fabsf(System.M1_Current);
	if (m1_current_abs > M1_CURRENT_LIMIT)
	{
		float excess = m1_current_abs - M1_CURRENT_LIMIT;
		float excess_ratio = excess / M1_MAX_EXCESS_CURRENT;
		if (excess_ratio > 1.0f) excess_ratio = 1.0f;

		float current_ramp = 1.0f - (1.0f - MIN_CURRENT_RAMP) * excess_ratio;
		target_left *= current_ramp;
	}

	float m2_current_abs = fabsf(System.M2_Current);
	if (m2_current_abs > M2_CURRENT_LIMIT)
	{
		float excess = m2_current_abs - M2_CURRENT_LIMIT;
		float excess_ratio = excess / M2_MAX_EXCESS_CURRENT;
		if (excess_ratio > 1.0f) excess_ratio = 1.0f;

		float current_ramp = 1.0f - (1.0f - MIN_CURRENT_RAMP) * excess_ratio;
		target_right *= current_ramp;
	}

	float max_output = MAX_MOTOR_OUTPUT * ramp;

	target_left = CONSTRAIN(target_left, -max_output, max_output);
	target_right = CONSTRAIN(target_right, -max_output, max_output);

	BrushedMotor_SetSpeed(&g_motor1, target_left);
	BrushedMotor_SetSpeed(&g_motor2, target_right);
}

void BalanceCarControl(void)
{
	static SystemState last_state = SYS_INIT;
	
	ReadSensors(&sensor_data);

	HandleGamepadInput();

	FirstOrderFilter(&System.filtered_speed, sensor_data.avg_speed, SPEED_FILTER_ALPHA);

	speed_pid.actual = System.filtered_speed;
	
	FirstOrderFilter(&turn_pid.actual, sensor_data.yaw_rate, YAW_RATE_FILTER_ALPHA);

	CheckSafetyConditions();
	
	if(GetSystemActiveState() || System.shutdown_ramp > 0.0f)
	{
		System.control_cycle_counter++;
		
#if ENABLE_SPEED_LOOP
		if(System.speed_boot_counter < SPEED_BOOT_COUNT_THRESH)
			System.speed_boot_counter++;
		else
		{
			if(System.control_cycle_counter % SPEED_LOOP_PERIOD == 0)
			{
				SpeedControlLoop();
			}
		}
#else
		PID_SetAngleTarget(BALANCE_ANGLE_ZERO);
#endif

#if ENABLE_ANGLE_LOOP
		AngleControlLoop();
#endif

#if ENABLE_TURN_LOOP
		if(System.control_cycle_counter % TURN_LOOP_PERIOD == 0)
		{
			TurnControlLoop();
		}
#endif
#if ENABLE_TURN_OPEN_LOOP
		if(System.control_cycle_counter % TURN_LOOP_PERIOD == 0)
		{
			TurnControlOpenLoop();
		}
#endif

		float base_output = PID_GetAngleOutput();
		float turn_adjust = turn_pid.output;

		motor_output.left  = base_output - turn_adjust;
		motor_output.right = base_output + turn_adjust;

		NormalizeOutput(&motor_output);
	}
	else
	{
		motor_output.left  = 0;
		motor_output.right = 0;
	}
#if MOTOR_DEBUG == 0
	ApplyMotorOutput(&motor_output);
#endif
	if (last_state != SYS_SELF_BALANCE && System.state == SYS_SELF_BALANCE)
	{
		osMessagePut(BuzzerQueueHandle, (uint32_t)SND_SIREN, 0);
	}
	last_state = System.state;
}

void CheckSafetyConditions(void)
{
	if (System.need_over_angle_start)
	{
		if (fabsf(sensor_data.pitch) > MAX_BALANCE_ANGLE)
		{
			System.need_over_angle_start = false;
			System.start_counter = 0;
		}
	}
	
	if (!System.imu_comm_ok && System.state == SYS_INIT && System.shutdown_ramp <= 0.0f)
	{
		System.imu_comm_error_count++;
		if (System.imu_comm_error_count % 1500 == 0)
		{
			osMessagePut(BuzzerQueueHandle, (uint32_t)SND_SIREN, 0);
		}
	}
	else
	{
		System.imu_comm_error_count = 0;
	}
	
	if(!GetSystemActiveState() && !System.need_over_angle_start)
	{
		if(fabsf(sensor_data.pitch) < START_ANGLE_THRESH && fabsf(sensor_data.roll) < START_ANGLE_THRESH && System.imu_comm_ok)
		{
			System.is_angle_start = true;
			if(System.start_counter < START_ANGLE_COUNT_THRESH)
				System.start_counter++;
			else
			{
				if (System.battery_level == 0)
				{
					System.start_counter  = 0;
				}
				else
				{
					System.state = SYS_SELF_BALANCE;
					SystemReset();
					BrushedMotor_Start(&g_motor1);
					BrushedMotor_Start(&g_motor2);
				}
			}
		}
		else
		{
			System.is_angle_start = false;
			System.start_counter  = 0;
		}
	}

	if(fabsf(speed_pid.actual) > PICKUP_SPEED_THRESH && fabsf(sensor_data.pitch) < PICKUP_ANGLE_THRESH)
	{
		if(++System.pickup_counter > PICKUP_COUNT_THRESH)
		{
			if (System.shutdown_ramp <= 0.0f)
			{
				System.state = SYS_INIT;
				System.shutdown_ramp = 1.0f;
				System.last_stop_reason = STOP_PICKUP;
				System.need_over_angle_start = true;
				osMessagePut(BuzzerQueueHandle, (uint32_t)SND_CONFUSED, 0);
			}
		}
	}
	else
	{
		System.pickup_counter = 0;
	}

	if(fabsf(sensor_data.pitch) > MAX_BALANCE_ANGLE)
	{
		if(++System.over_angle_counter > MAX_ANGLE_COUNT_THRESH)
		{
			if (System.shutdown_ramp <= 0.0f)
			{
				System.state = SYS_INIT;
				System.shutdown_ramp = 1.0f;
				System.last_stop_reason = STOP_OVER_ANGLE;
				System.need_over_angle_start = true;
			}
		}
		
		System.over_angle_sound_counter++;
		if (System.over_angle_sound_counter % OVER_ANGLE_SOUND_INTERVAL == 0)
		{
			osMessagePut(BuzzerQueueHandle, (uint32_t)SND_CONFUSED, 0);
			System.over_angle_sound_counter = 0;
		}
	}
	else
	{
		System.over_angle_counter = 0;
	}

	if (System.shutdown_ramp > 0.0f)
	{
		System.shutdown_ramp -= 0.01f;

		if (System.shutdown_ramp <= 0.0f)
		{
			System.state = SYS_INIT;

			BrushedMotor_Stop(&g_motor1);
			BrushedMotor_Stop(&g_motor2);

			SystemReset();
		}
	}
	
	if(System.imu_rx_timeout > IMU_RX_TIMEOUT_THRESH)
	{
		System.imu_comm_ok = false;
		if (System.shutdown_ramp <= 0.0f)
		{
			System.state = SYS_INIT;
			System.shutdown_ramp = 1.0f;
			System.last_stop_reason = STOP_IMU_TIMEOUT;
			osMessagePut(BuzzerQueueHandle, (uint32_t)SND_SIREN, 0);
		}
	}
}

bool GetSystemActiveState(void)
{
	return System.state != SYS_INIT;
}

void ManualStartSystem(void)
{
	if (System.battery_level == 0)
	{
		buzzer_play_sound(&buzzer_ctrl, SND_CONFUSED);
		return;
	}
	
	if(!System.imu_comm_ok)
	{
		buzzer_play_sound(&buzzer_ctrl, SND_SIREN);
		return;
	}
	
	if(System.state == SYS_INIT)
	{
		System.state = SYS_SELF_BALANCE;
		PID_SetAngleTarget(BALANCE_ANGLE_ZERO);
		SystemReset();
		BrushedMotor_Start(&g_motor1);
		BrushedMotor_Start(&g_motor2);
	}
}

void ManualStopSystem(void)
{
	System.start_counter = 0;
	System.speed_boot_counter = 0;
	System.control_cycle_counter = 0;

	System.need_over_angle_start = true;

	System.state = SYS_INIT;
	System.shutdown_ramp = 1.0f;

	motor_output.left  = 0;
	motor_output.right = 0;
	ApplyMotorOutput(&motor_output);
	BrushedMotor_Stop(&g_motor1);
	BrushedMotor_Stop(&g_motor2);
}

void SetTargetSpeed(float speed)
{
	speed_pid.target = speed * SPEED_LOOP_TARGET_DIR;
}

void SetTargetTurn(float turn_rate)
{
	turn_pid.target = turn_rate * TURN_LOOP_TARGET_DIR;
}

void HandleGamepadInput(void)
{
	static uint8_t a_key_prev_state = 0;
	static uint8_t start_prev_state = 0;
	static uint8_t select_prev_state = 0;

	if (Gamepad_info.buttons2.Start && !start_prev_state)
	{
		buzzer_play_sound(&buzzer_ctrl, SND_HAPPY);
		ManualStartSystem();
		System.is_speed_lock = false;
		System.filtered_speed_target = System.manual_speed_target;
	}
	if (Gamepad_info.buttons2.Select && !select_prev_state)
	{
		if (fabsf(System.manual_speed_target) > 0.01f || fabsf(System.filtered_speed_target) > 0.01f)
		{
			System.is_speed_lock = true;
			buzzer_play_sound(&buzzer_ctrl, SND_BUTTON_PUSHED);
		}
		else
		{
			System.last_stop_reason = STOP_GAMEPAD_SELECT;
			ManualStopSystem();
			buzzer_play_sound(&buzzer_ctrl, SND_SLEEPING);
			System.manual_speed_target = 0.0f;
			System.filtered_turn_target = 0.0f;
			System.filtered_speed_target = 0.0f;
		}
	}

	if (Gamepad_info.buttons1.A && !a_key_prev_state)
	{
		osMessagePut(BuzzerQueueHandle, (uint32_t)SND_BEEP, 0);
	}
	a_key_prev_state = Gamepad_info.buttons1.A;
	start_prev_state = Gamepad_info.buttons2.Start;
	select_prev_state = Gamepad_info.buttons2.Select;

	if (!GetSystemActiveState())
		return;

	float speed_factor = 0.0f;
	if (abs(Gamepad_info.right_stick_y) > STICK_DEADZONE)
	{
		speed_factor = -(float)Gamepad_info.right_stick_y / 127.0f;
	}

	float max_speed = MAX_SPEED_LOW;
	if (Gamepad_info.RT_trigger > 0x80)
	{
		max_speed = MAX_SPEED_FULL;
	}
	else if (Gamepad_info.buttons1.RB)
	{
		max_speed = MAX_SPEED_NORMAL;
	}
	float raw_speed_target = speed_factor * max_speed;

	float raw_turn_target = 0.0f;

	switch (Gamepad_info.dpad)
	{
		case DPAD_UP:
			raw_speed_target = DPAD_SPEED;
			break;
		case DPAD_DOWN:
			raw_speed_target = -DPAD_SPEED;
			break;
		case DPAD_LEFT:
			raw_turn_target = -DPAD_TURN;
			break;
		case DPAD_RIGHT:
			raw_turn_target = DPAD_TURN;
			break;
		default:
			break;
	}

	if (System.is_speed_lock) raw_speed_target = 0.0f;
	
	raw_speed_target = CONSTRAIN(raw_speed_target, -max_speed, max_speed);

	FirstOrderFilter(&System.filtered_speed_target, raw_speed_target, SPEED_TARGET_FILTER_ALPHA);

	float turn_factor = 0.0f;
	if (abs(Gamepad_info.left_stick_x) > STICK_DEADZONE)
	{
		turn_factor = (float)Gamepad_info.left_stick_x / 127.0f;
	}

	float max_turn = Gamepad_info.buttons1.LB ? MAX_TURN_NORMAL : MAX_TURN_FULL;
	if (raw_turn_target == 0.0f) raw_turn_target = turn_factor * max_turn;
	
	if (System.is_speed_lock) raw_turn_target = 0.0f;
	
	FirstOrderFilter(&System.filtered_turn_target, raw_turn_target, TURN_TARGET_FILTER_ALPHA);
	
	System.filtered_turn_target = CONSTRAIN(System.filtered_turn_target, -max_turn, max_turn);
	
	System.manual_speed_target = System.filtered_speed_target;
	SetTargetSpeed(System.manual_speed_target);
	SetTargetTurn(System.filtered_turn_target);
	
	if (GetSystemActiveState())
	{
		float sp = System.manual_speed_target;
		float tr = System.filtered_turn_target;
		if (fabsf(sp) > 1e-3f || fabsf(tr) > 1e-3f)
			System.state = SYS_DYNAMIC;
		else
			System.state = SYS_SELF_BALANCE;
	}
}

void LED_UpdateByCarState(WS2812B_DeviceTypeDef* dev, LED_ControllerTypeDef* led_ctrl)
{
	if (led_ctrl == NULL) return;


	if (System.state == SYS_INIT && !System.is_angle_start)
	{
		LED_SetTargetColor(led_ctrl, COLOR_INACTIVE);
		LED_SetEffect(led_ctrl, EFFECT_BREATH);
	}
	else if (System.state == SYS_INIT && System.is_angle_start)
	{
		LED_SetTargetColor(led_ctrl, COLOR_STANDBY);
		LED_SetEffect(led_ctrl, EFFECT_SOLID);
		LAMP_F_WRITE(1);
		LAMP_B_WRITE(1);
	}
	else if (System.state == SYS_SELF_BALANCE)
	{
		LED_SetTargetColor(led_ctrl, COLOR_BALANCING);
		LED_SetEffect(led_ctrl, EFFECT_BREATH);
		LAMP_F_WRITE(1);
		LAMP_B_WRITE(1);
	}
	else if (System.state == SYS_DYNAMIC)
	{
		if(speed_pid.target >= 0)
			LAMP_F_WRITE(1);
		else
			LAMP_F_WRITE(0);

		if(speed_pid.target < 0)
			LAMP_B_WRITE(1);
		else
			LAMP_B_WRITE(0);

		
		float speed_abs = fabsf(speed_pid.target);
		float turn_abs  = fabsf(turn_pid.target);
		float speed_ratio = CONSTRAIN(speed_abs / MAX_SPEED_FULL, 0.0f, 1.0f);
		float turn_ratio  = CONSTRAIN(turn_abs  / MAX_TURN_FULL, 0.0f, 1.0f);
		float ratio = fmaxf(speed_ratio, turn_ratio);
		uint8_t r = (uint8_t)(ratio * 255.0f);
		uint8_t b = (uint8_t)((1.0f - ratio) * 255.0f);
		LED_SetTargetColor(led_ctrl, r, 0, b);
		LED_SetEffect(led_ctrl, EFFECT_SOLID);
	}

}

