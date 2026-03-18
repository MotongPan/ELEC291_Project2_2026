#include "../Common/Include/stm32l051xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "../Common/Include/serial.h"

#define PWM_PERIOD_COUNT 100 
#define MOTOR_STOP      0
#define MOTOR_FORWARD   1
#define MOTOR_BACKWARD  2

//Hardware Pins
//the H-bridge forward and backward
#define Hbri_L1  P1_1
#define Hbri_L2  P1_2
#define Hbri_R1  P1_3
#define Hbri_R2  P1_4
#define L_PWM P2_0 //speed
#define R_PWM P2_1
/ LQFP32 pinout with the pins that can be analog inputs.  This code uses ADC_IN9.
//                 ----------
//the pin out is from example from canvas so we can use their initial module
//           VDD -|1       32|- VSS
//          PC14 -|2       31|- BOOT0
//          PC15 -|3       30|- PB7 (I2C1 SDA)
//          NRST -|4       29|- PB6 (I2C1_SCL)
//          VDDA -|5       28|- PB5
// (ADC_IN0) PA0 -|6       27|- PB4
// (ADC_IN1) PA1 -|7       26|- PB3 
// (ADC_IN2) PA2 -|8       25|- PA15 Hbri_L1
// (ADC_IN3) PA3 -|9       24|- PA14 Hbri_L2
// (ADC_IN4) PA4 -|10      23|- PA13 Hbri_R1
// (ADC_IN5) PA5 -|11      22|- PA12 (pwm2)
// (ADC_IN6) PA6 -|12      21|- PA11 (pwm1)
// (ADC_IN7) PA7 -|13      20|- PA10 (Reserved for RXD)
// (ADC_IN8) PB0 -|14      19|- PA9  (Reserved for TXD)
// (ADC_IN9) PB1 -|15      18|- PA8  Hbri_R2
//           VSS -|16      17|- VDD
//         
//Shared Variables. duty for speed (8bits)
volatile unsigned char pwm_left = 0;
volatile unsigned char pwm_right = 0; 
volatile pwm_counter    = 0;   //timer
//mode
typedef enum
{
    Auto_mode = 0,
    Manual_mode
} RobotMode;
//Pre-configure paths
typedef enum
{
    Go_forward=0,
    Go_left,
    Go_right,
    End_stop //path end stop not collision stop
} PathAction;
// 3 path 8 intersection 
const PathAction path_table[3][8] =
{ //path 1,2,3
    { Go_forward, Go_left,    Go_left,    Go_forward, Go_right,   Go_left,    Go_right,   End_stop },
    { Go_left,    Go_right,   Go_left,    Go_right,   Go_forward, Go_forward, End_stop,   End_stop },
    { Go_right,   Go_forward, Go_right,   Go_left,    Go_right,   Go_left,    Go_forward, End_stop }
};
PathAction get_path_action(unsigned char path_number, unsigned char intersection_count)
{ //path table start from 0 (not1)
   return path_table[path_number - 1][intersection_count - 1];
}
void wait_1ms(void)
{
	// For SysTick info check the STM32l0xxx Cortex-M0 programming manual.
	SysTick->LOAD = (F_CPU/1000L) - 1;  // set reload register, counter rolls over from zero, hence -1
	SysTick->VAL = 0; // load the SysTick counter
	//SysTick->CTRL = 0x05; // Bit 0: ENABLE, BIT 1: TICKINT, BIT 2:CLKSOURCE
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // Enable SysTick IRQ and SysTick Timer */
	while((SysTick->CTRL & BIT16)==0); // Bit 16 is the COUNTFLAG.  True when counter rolls over from zero.
	SysTick->CTRL = 0x00; // Disable Systick counter
}

void delayms(int len)
{
	while(len--) wait_1ms();
}
//===========================
//Read ADC from 3 detectors need the adc pin number stm32
//===========================
void Configure_Pins (void)
{
	RCC->IOPENR  |= BIT0;
    GPIOA->MODER  = (GPIOA->MODER & ~(BIT17|BIT16) ) | BIT16; // Make pin PA8 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0))
	
	RCC->IOPENR  |= BIT1;         // peripheral clock enable for port B
	GPIOB->MODER |= (BIT2|BIT3);  // Select analog mode for PB1 (pin 15 of LQFP32 package)
}
unsigned int adc_read_left(void)
{
    return readADC(0x00);
}
unsigned int adc_read_right(void)
{
   return readADC(0x00);
}
unsigned int adc_read_intersection(void)
{
    return readADC(0x00);
}
//===========================================
//PWM part
//=========================================
//timer2 handler (from servo_PWM)
void TIM2_Handler(void) 
{
	TIM2->SR &= ~BIT0; // clear update interrupt flag
	PWM_Counter++;
    if (pwm_counter >= 100)
    {
        pwm_counter = 0;
    }
    // Left motor PWM output on PA11
    if (pwm_counter < pwm_left)
        GPIOA->ODR |= BIT11;
    else
        GPIOA->ODR &= ~BIT11;
    // Right motor PWM output on PA12
    if (pwm_counter < pwm_right)
        GPIOA->ODR |= BIT12;
    else
        GPIOA->ODR &= ~BIT12;
}
//PWM harware initial（now only the PWM and may add other hardware initial(H-bridge and adc)
void Hardware_Init(void)
{
	// Set up output pins
	RCC->IOPENR |= BIT0; // peripheral clock enable for port A
    GPIOA->MODER = (GPIOA->MODER & ~(BIT22|BIT23)) | BIT22; // Make pin PA11 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
	GPIOA->OTYPER &= ~BIT11; // Push-pull
    GPIOA->MODER = (GPIOA->MODER & ~(BIT24|BIT25)) | BIT24; // Make pin PA12 output (page 200 of RM0451, two bits used to configure: bit0=1, bit1=0)
	GPIOA->OTYPER &= ~BIT12; // Push-pull

	// Set up timer
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = F_CPU / 100000L - 1;
	NVIC->ISER[0] |= BIT15; // enable timer 2 interrupts in the NVIC
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	__enable_irq();
}
//PMW control+ Timmer ISR
//===========================
//Forward and backward by the voltage on two side of H-bridge
void motor_stop(void)
{
    pwm_left  = 0;
    pwm_right = 0;
    Hbri_L1 = 0;
    Hbri_L2 = 0;
    Hbri_R1 = 0;
    Hbri_R2 = 0;
}
void motor_forward(speed)
{
    Hbri_L1 = 1;
    Hbri_L2 = 0;
    Hbri_R1 = 1;
    Hbri_R2 = 0;
    pwm_left  = speed;
    pwm_right = speed;
}
void motor_backward(speed)
{
    Hbri_L1 = 0;
    Hbri_L2 = 1;
    Hbri_R1 = 0;
    Hbri_R2 = 1;
    pwm_left  = speed;
    pwm_right = speed;
}

void motor_slight_change(left_speed,right_speed)
{
    Hbri_L1 = 1;
    Hbri_L2 = 0;
    Hbri_R1 = 1;
    Hbri_R2 = 0;
    pwm_left  = left_speed;
    pwm_right = right_speed;
}
void motor_rotate_left(speed)
{
    //left forward right backward
    Hbri_L1 = 1;
    Hbri_L2 = 0;
    Hbri_R1 = 0;
    Hbri_R2 = 1;
    pwm_left  = speed;
    pwm_right = speed;
}
void motor_rotate_right(speed)
{
    //left backward right forward
    Hbri_L1 = 0;
    Hbri_L2 = 1;
    Hbri_R1 = 1;
    Hbri_R2 = 0;
    pwm_left  = speed;
    pwm_right = speed;
}
//Timer ISR PWM. PWM frequency = ISR_frequency / PWM_PERIOD_COUNT
//from lecture Microcomputer Interfacing using transistors page45 8051
void PwmGeneration (void) interrupt 5 using 1
{
    static unsigned char dutcnt;

    dutcnt++;
    if (dutcnt == 100) dutcnt = 0;
    P2_0 = dutcnt < pwm_left ? 0 : 1;
    P2_1 = dutcnt < pwm_right ? 0 : 1;
}
//====================================
//execute path command to motor action
//=====================================
void execute_path_action(PathAction action)
{
    switch (action)
    {
        case Go_forward:
            motor_forward(10); //speed not sure
            waitms(300);
            break;

        case Go_left:
            motor_rotate_left(10);
            waitms(300);
            break;

        case Go_right:
            motor_rotate_right(10);
            waitms(300);
            break;

        case End_stop:
        default:
            motor_stop();
            break;
    }
}
//============================================
//move forward correct
//ADC read from 2 field detectors and slightly left or right
//============================================
#define span 2
void move_forward_with_field_detection(void)
{
    unsigned int left_val, right_val;
    int error;
    left_val = adc_read_left();
    right_val = adc_read_right();
    error = (int)left_val - (int)right_val;
    if (error <= span || -error <= span )
    {
        motor_forward(10);
    }
    else if (error > span) //left bigger
    {
        motor_slight_change(10,20);
    }
    else
    {
        motor_slight_change(20, 10);
    }
}
void main(void)
{
    Configure_Pins();
	initADC();
    motor_stop();

    while (RobotMode=0)
    {      
        execute_path_action(get_path_action(path_num, inter_count));
    }
}
