/*****************************************************************************
程序名称：PID控制器
程 序 员：李锦上 Email：lijinshang@126.com
使用说明：
  1. 本文件实现了角度环PID控制器的实例化和访问接口
  2. 控制器参数在pid_controller.h中定义（ANGLE_KP, ANGLE_KI, ANGLE_KD等）
  3. 通过PID_SetAngleTarget()设置目标角度
  4. 通过PID_UpdateAngle()更新实际角度并计算输出
  5. 通过PID_GetAngleOutput()获取控制输出
  6. 通过PID_ResetAngleController()复位控制器状态
算法说明：
  -位置式PID：output = Kp*error + Ki*integral + Kd*(error-last_error)
  -积分限幅：防止积分饱和
  -输出限幅：保护电机和执行机构
******************************************************************************/
#include "pid_controller.h"
#include "BalanceCarControl.h"
#include <math.h>
#include <stdio.h>


static Positional_PID angle_pid =
{
	.Kp = 0,
	.Ki = 0,
	.Kd = 0,
	.target = 0,
	.actual = 0,
	.error = 0,
	.last_error = 0,
	.integral = 0,
	.output = 0,
	.max_output = 0,
	.max_integral = 0
};

Positional_PID speed_pid =
{
	.Kp = SPEED_KP,
	.Ki = SPEED_KI,
	.Kd = SPEED_KD,
	.target = 0,
	.actual = 0,
	.error = 0,
	.last_error = 0,
	.integral = 0,
	.output = 0,
	.max_output = SPEED_MAX_OUTPUT,
	.max_integral = SPEED_MAX_INTEGRAL
};

Positional_PID turn_pid =
{
	.Kp = TURN_KP,
	.Ki = TURN_KI,
	.Kd = TURN_KD,
	.target = 0,
	.actual = 0,
	.error = 0,
	.last_error = 0,
	.integral = 0,
	.output = 0,
	.max_output = TURN_MAX_OUTPUT,
	.max_integral = TURN_MAX_INTEGRAL
};


Positional_PID* PID_GetAngleController(void)
{
	return &angle_pid;
}

void PID_SetAngleTarget(float target)
{
	angle_pid.target = target;
}

float PID_GetAngleOutput(void)
{
	return angle_pid.output;
}

float PID_UpdateAngle(float actual)
{
	angle_pid.actual = actual;
	return PID_Calculate(&angle_pid);
}

void PID_ResetAngleController(void)
{
	angle_pid.integral = 0.0f;
	angle_pid.output = 0.0f;
	angle_pid.error = 0.0f;
	angle_pid.last_error = 0.0f;
}

void PID_Init(Positional_PID* pid)
{
	pid->Kp = ANGLE_KP;
	pid->Ki = ANGLE_KI;
	pid->Kd = ANGLE_KD;
	pid->target = 0.0f;
	pid->actual = 0.0f;
	pid->error = 0.0f;
	pid->last_error = 0.0f;
	pid->integral = 0.0f;
	pid->output = 0.0f;
	pid->max_output = ANGLE_MAX_OUTPUT;
	pid->max_integral = ANGLE_MAX_INTEGRAL;
}

float PID_Calculate(Positional_PID* pid)
{
	pid->error = pid->target - pid->actual;

	pid->integral += pid->error;
	if(pid->integral > pid->max_integral)
		pid->integral = pid->max_integral;
	else if(pid->integral < -pid->max_integral)
		pid->integral = -pid->max_integral;

	float derivative = pid->error - pid->last_error;

	pid->output = pid->Kp * pid->error
				+ pid->Ki * pid->integral
				+ pid->Kd * derivative;

	pid->output = CONSTRAIN(pid->output, -pid->max_output, pid->max_output);

	pid->last_error = pid->error;

	return pid->output;
}

