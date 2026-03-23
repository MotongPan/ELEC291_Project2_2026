#ifndef __ROBOT_CONTROL_H__
#define __ROBOT_CONTROL_H__
#include "../Common/Include/stm32l051xx.h"
#define F_CPU 32000000L
#define PWM_PERIOD_COUNT 100


typedef enum
{
    Auto_mode = 0,
    Manual_mode
} RobotMode;

typedef enum
{
    Go_forward = 0,
    Go_left,
    Go_right,
    End_stop
} PathAction;

extern volatile unsigned char pwm_left;
extern volatile unsigned char pwm_right;
extern volatile unsigned char pwm_counter;

void PWM_Hardware_Init(void);
void TIM2_Handler(void);

void motor_stop(void);
void motor_forward(unsigned char speed);
void motor_backward(unsigned char speed);
void motor_slight_change(unsigned char left_speed, unsigned char right_speed);
void motor_rotate_left(unsigned char speed);
void motor_rotate_right(unsigned char speed);
void auto_mode_task(unsigned char path_number,unsigned char *intersection_count);
void manual_command_execute(unsigned char cmd);

PathAction get_path_action(unsigned char path_number, unsigned char intersection_count);
void execute_path_action(PathAction action);
void move_forward_with_field_detection(void);
void delayms(int len);
#endif