//           VDD -|1       32|- VSS
//          PC14 -|2       31|- BOOT0
//          PC15 -|3       30|- PB7 (I2C1 SDA)
//          NRST -|4       29|- PB6 (I2C1_SCL)
//          VDDA -|5       28|- PB5
// (ADC_IN0) PA0 -|6       27|- PB4
// (ADC_IN1) PA1 -|7       26|- PB3 
//           PA2 -|8       25|- PA15 Hbri_L1
//           PA3 -|9       24|- PA14 Hbri_L2
// (ADC_IN4) PA4 -|10      23|- PA13 Hbri_R1
//           PA5 -|11      22|- PA12 (pwm2)
//           PA6 -|12      21|- PA11 (pwm1)
//           PA7 -|13      20|- PA10 (Reserved for RXD)
//           PB0 -|14      19|- PA9  (Reserved for TXD)
//           PB1 -|15      18|- PA8  Hbri_R2
//           VSS -|16      17|- VDD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../Common/Include/stm32l051xx.h"
#include "../Common/Include/serial.h"
#include "adc.h"
#include "robot_control.h"

#define F_CPU 32000000L
#define PWM_PERIOD_COUNT 100

RobotMode current_mode = Auto_mode;
unsigned char path_number = 1;
unsigned char intersection_count = 0;

void wait_1ms(void)
{
    SysTick->LOAD = (F_CPU / 1000L) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while ((SysTick->CTRL & BIT16) == 0);

    SysTick->CTRL = 0x00;
}

void delayms(int len)
{
    while (len--) wait_1ms();
}

int main(void)
{
    ADC_Pin_Init();
    initADC();
    PWM_Hardware_Init();
    motor_stop();

    while (1)
    {
        if (current_mode == Auto_mode)
        {
            auto_mode_task(path_number, &intersection_count);
        }
        else
        {
            // manual mode
            // later replace 0 with bluetooth command
            manual_command_execute(0);
        }
    }
}