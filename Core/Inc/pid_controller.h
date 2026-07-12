#ifndef PID_CONTROLLER_H_
#define PID_CONTROLLER_H_

#include <stdint.h>

#define ANGLE_KP					0.50f
#define ANGLE_KI					0.00f
#define ANGLE_KD					2.00f
#define ANGLE_MAX_INTEGRAL			100
#define ANGLE_MAX_OUTPUT			1.0f

#define SPEED_FILTER_ALPHA			0.01f
#define SPEED_KP					0.120f
#define SPEED_KI					0.0001f
#define SPEED_KD					0.000f
#define SPEED_MAX_INTEGRAL			5000
#define SPEED_MAX_OUTPUT			20.0f

#define YAW_RATE_FILTER_ALPHA		0.3f
#define TURN_KP						0.010f
#define TURN_KI						0.000f
#define TURN_KD						0.010f
#define TURN_MAX_INTEGRAL			10
#define TURN_MAX_OUTPUT				0.2f

typedef struct
{
	float Kp;
	float Ki;
	float Kd;
	float target;
	float actual;
	float error;
	float last_error;
	float integral;
	float output;
	float max_output;
	float max_integral;
} Positional_PID;

void PID_Init(Positional_PID* pid);
float PID_Calculate(Positional_PID* pid);

Positional_PID* PID_GetAngleController(void);
void PID_SetAngleTarget(float target);
float PID_GetAngleOutput(void);
float PID_UpdateAngle(float actual);
void PID_ResetAngleController(void);

extern Positional_PID speed_pid;
extern Positional_PID turn_pid;

#endif

