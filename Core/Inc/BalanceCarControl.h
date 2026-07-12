#ifndef BALANCE_CAR_CONTROL_H_
#define BALANCE_CAR_CONTROL_H_

#include "stdbool.h"
#include "BrushedMotor.h"
#include "STM32_TLE5012B.h"
#include "battery_monitor.h"
#include "ws2812b.h"
#include "pid_controller.h"

#define MOTOR_DEBUG					0
#define ENABLE_ANGLE_LOOP			1
#define ENABLE_SPEED_LOOP			1
#define ENABLE_TURN_LOOP			1
#define ENABLE_TURN_OPEN_LOOP		0
#define MOTOR_OPEN_LOOP				1
#define MOTOR_SPEED_LOOP			0

#define MOTOR_LEFT_DIR				1
#define MOTOR_RIGHT_DIR				-1
#define MOTOR_LEFT_SPEED_DIR		1
#define MOTOR_RIGHT_SPEED_DIR		1
#define SPEED_LOOP_TARGET_DIR		1
#define TURN_LOOP_TARGET_DIR		-1

#define MAX_BALANCE_ANGLE			30.0f
#define MAX_ANGLE_COUNT_THRESH		500
#define MAX_TARGET_ANGLE			20.0f
#define OVER_ANGLE_SOUND_INTERVAL	5000

#define PICKUP_SPEED_THRESH			450.0f
#define PICKUP_ANGLE_THRESH			30.0f
#define PICKUP_COUNT_THRESH			500

#define START_ANGLE_THRESH			10.0f
#define START_ANGLE_COUNT_THRESH	300
#define SPEED_BOOT_COUNT_THRESH		100

#define BALANCE_ANGLE_ZERO			-0.0f
#define MAX_MOTOR_OUTPUT			1.0f
#define MAX_MOTOR_RPM				500.0f
#define CONTROL_PERIOD_SEC			0.002f
#define SPEED_LOOP_PERIOD			1
#define TURN_LOOP_PERIOD			1
#define IMU_RX_TIMEOUT_THRESH		50

#define TURN_OPEN_LOOP_SCALE		0.0005f

#define M1_CURRENT_LIMIT			5.0f
#define M2_CURRENT_LIMIT			5.0f
#define M1_MAX_EXCESS_CURRENT		2.0f
#define M2_MAX_EXCESS_CURRENT		2.0f
#define MIN_CURRENT_RAMP			0.5f

#define MAX_SPEED_LOW				100.0f
#define MAX_SPEED_NORMAL			200.0f
#define MAX_SPEED_FULL				300.0f
#define MAX_TURN_NORMAL				200.0f
#define MAX_TURN_FULL				300.0f
#define STICK_DEADZONE				5
#define DPAD_SPEED					50.0f
#define DPAD_TURN					50.0f
#define SPEED_TARGET_FILTER_ALPHA	0.02f
#define TURN_TARGET_FILTER_ALPHA	0.1f


#define BASE_GREEN					0x00, 0xFF, 0x00

#ifndef CONSTRAIN
#define CONSTRAIN(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif

#define BASE_RED					0xFF, 0x00, 0x00
#define BASE_BLUE					0x00, 0xB4, 0xFF
#define COLOR_INACTIVE				0xFF, 0x33, 0x33
#define COLOR_STANDBY				0x00, 0xFF, 0x00
#define COLOR_BALANCING				0x00, 0xFF, 0x66
#define COLOR_MOVING				0x00, 0x99, 0xFF
#define COLOR_TURNING				0xCC, 0x00, 0xFF
#define COLOR_BOOTING				0x99, 0x00, 0xFF
#define COLOR_OFF					0x00, 0x00, 0x00

#define M1_SPEED    (g_motor1.speed)
#define M2_SPEED    (g_motor2.speed)
#define M1_MECH_SPEED   (g_sensor1.mechanical_speed)
#define M2_MECH_SPEED   (g_sensor2.mechanical_speed)
#define M1_ANGLE       (g_sensor1.sensor.AngleValue)
#define M2_ANGLE       (g_sensor2.sensor.AngleValue)

typedef struct
{
	float x;
	float y;
	float z;
} mpu_3axes;

typedef struct
{
	mpu_3axes accel;
	mpu_3axes gyro;
	mpu_3axes mag;
} IMU_Data_Type;

typedef struct
{
	float Roll;
	float Pitch;
	float Yaw;
} EulerAngles_Type;

typedef struct {
    uint8_t sof;
    uint8_t len;
    float accel_g[3];
    float gyro_dps[3];
    float mag_uT[3];
    float euler[3];
    uint8_t crc;
    uint8_t eof;
} IMU_Frame_TypeDef;

extern IMU_Data_Type mpu6xxx;
extern EulerAngles_Type EulerAngles;

void IMU_ParseFrame(uint8_t *buf, uint16_t len);

typedef struct
{
	float roll;
	float pitch;
	float yaw_rate;
	float left_speed;
	float right_speed;
	float avg_speed;
} SensorData;

typedef struct
{
	float left;
	float right;
} MotorOutput;

extern MotorOutput motor_output;

typedef enum {
	SYS_INIT = 0,
	SYS_SELF_BALANCE = 1,
	SYS_DYNAMIC = 2
} SystemState;

typedef enum {
	STOP_NONE = 0,
	STOP_OVER_ANGLE,
	STOP_PICKUP,
	STOP_LOW_BATTERY,
	STOP_MANUAL_BUTTON,
	STOP_GAMEPAD_SELECT,
	STOP_IMU_TIMEOUT
} StopReason;

typedef struct
{
	SystemState state;
	StopReason last_stop_reason;
	
	bool is_angle_start;
	bool is_speed_lock;
	bool need_over_angle_start;
	int over_angle_counter;
	int over_angle_sound_counter;
	int pickup_counter;
	int start_counter;
	int speed_boot_counter;
	int control_cycle_counter;
	float filtered_speed;
	float start_ramp;
	float torque_ramp;
	float shutdown_ramp;
	
	float battery_voltage;
	BatteryMonitor bat_monitor;
	uint8_t battery_level;
	
	uint16_t battery_zero_count;
	
	bool M1_Current_Limit_Active;
	bool M2_Current_Limit_Active;
	float M1_Current;
	float M2_Current;
	
	float manual_speed_target;
	float filtered_turn_target;
	float filtered_speed_target;
	
	int imu_rx_timeout;
	bool imu_comm_ok;
	int imu_comm_error_count;

} System_Type;

extern System_Type System;

void UpdateParameters(float *Parameter);
void ReadSensors(SensorData* data);
void SystemReset(void);
void FirstOrderFilter(float *filtered_value, float current_value, float alpha);

void SpeedControlLoop(void);
void AngleControlLoop(void);
void TurnControlLoop(void);

void NormalizeOutput(MotorOutput* out);
void ApplyMotorOutput(MotorOutput* output);

void BalanceCarControl(void);
void CheckSafetyConditions(void);

bool GetSystemActiveState(void);
void ManualStartSystem(void);
void ManualStopSystem(void);
void SetTargetSpeed(float speed);
void SetTargetTurn(float turn_rate);
void HandleGamepadInput(void);
void LED_UpdateByCarState(WS2812B_DeviceTypeDef* dev, LED_ControllerTypeDef* led_ctrl);

#endif
