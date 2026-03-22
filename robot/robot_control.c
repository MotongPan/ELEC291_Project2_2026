#include "robot_control.h"
#include "adc.h"
#define HBRI_L1_HIGH()   (GPIOA->ODR |= BIT15)
#define HBRI_L1_LOW()    (GPIOA->ODR &= ~BIT15)

#define HBRI_L2_HIGH()   (GPIOA->ODR |= BIT14)
#define HBRI_L2_LOW()    (GPIOA->ODR &= ~BIT14)

#define HBRI_R1_HIGH()   (GPIOA->ODR |= BIT13)
#define HBRI_R1_LOW()    (GPIOA->ODR &= ~BIT13)

#define HBRI_R2_HIGH()   (GPIOA->ODR |= BIT8)
#define HBRI_R2_LOW()    (GPIOA->ODR &= ~BIT8)
#define INTERSECTION_THRESHOLD  2000
volatile unsigned char pwm_left = 0;
volatile unsigned char pwm_right = 0;
volatile unsigned char pwm_counter = 0;
// 3 path 8 intersection 
static const PathAction path_table[3][8] =
{ //path 1,2,3
    { Go_forward, Go_left,    Go_left,    Go_forward, Go_right,   Go_left,    Go_right,   End_stop },
    { Go_left,    Go_right,   Go_left,    Go_right,   Go_forward, Go_forward, End_stop,   End_stop },
    { Go_right,   Go_forward, Go_right,   Go_left,    Go_right,   Go_left,    Go_forward, End_stop }
};
PathAction get_path_action(unsigned char path_number, unsigned char intersection_count)
{ //path table start from 0 (not1)
    if (path_number < 1 || path_number > 3 || intersection_count < 1 || intersection_count > 8)
        return End_stop;
   return path_table[path_number - 1][intersection_count - 1];
}

void PWM_Hardware_Init(void)
{
	// Set up output pins
	RCC->IOPENR |= BIT0; // peripheral clock enable for port A
    GPIOA->MODER = (GPIOA->MODER & ~(BIT22|BIT23)) | BIT22;
	GPIOA->OTYPER &= ~BIT11; // pwm1
    GPIOA->MODER = (GPIOA->MODER & ~(BIT24|BIT25)) | BIT24;
    GPIOA->OTYPER &= ~BIT12;// pwm2
    // PA15 -> HBRI_L1
    GPIOA->MODER = (GPIOA->MODER & ~(BIT30 | BIT31)) | BIT30;
    GPIOA->OTYPER &= ~BIT15;
    // PA14 -> HBRI_L2
    GPIOA->MODER = (GPIOA->MODER & ~(BIT28 | BIT29)) | BIT28;
    GPIOA->OTYPER &= ~BIT14;
    // PA13 -> HBRI_R1
    GPIOA->MODER = (GPIOA->MODER & ~(BIT26 | BIT27)) | BIT26;
    GPIOA->OTYPER &= ~BIT13;
    // PA8 -> HBRI_R2
    GPIOA->MODER = (GPIOA->MODER & ~(BIT16 | BIT17)) | BIT16;
    GPIOA->OTYPER &= ~BIT8;
	// Set up timer
    GPIOA->ODR &= ~(BIT8 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15);//push down other pins
	RCC->APB1ENR |= BIT0;  // turn on clock for timer2 (UM: page 177)
	TIM2->ARR = F_CPU / 100000L - 1;
	NVIC->ISER[0] |= BIT15; // enable timer 2 interrupts in the NVIC
	TIM2->CR1 |= BIT4;      // Downcounting    
	TIM2->CR1 |= BIT7;      // ARPE enable    
	TIM2->DIER |= BIT0;     // enable update event (reload event) interrupt 
	TIM2->CR1 |= BIT0;      // enable counting    
	
	__enable_irq();
}
void TIM2_Handler(void) 
{
	TIM2->SR &= ~BIT0; // clear update interrupt flag
	pwm_counter++;
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
void forward_ini(void)
{
    HBRI_L1_HIGH();
    HBRI_L2_LOW();

    HBRI_R1_HIGH();
    HBRI_R2_LOW();
}

void backward_ini(void)
{
    HBRI_L1_LOW();
    HBRI_L2_HIGH();

    HBRI_R1_LOW();
    HBRI_R2_HIGH();
}

void rotate_left_ini(void)
{
    HBRI_L1_LOW();
    HBRI_L2_HIGH();

    HBRI_R1_HIGH();
    HBRI_R2_LOW();
}

void rotate_right_ini(void)
{
    HBRI_L1_HIGH();
    HBRI_L2_LOW();

    HBRI_R1_LOW();
    HBRI_R2_HIGH();
}
void motor_forward(unsigned char speed)
{
    forward_ini();
    pwm_left = speed;
    pwm_right = speed;
}
void motor_backward(unsigned char speed)
{
    backward_ini();
    pwm_left = speed;
    pwm_right = speed;
}
void motor_rotate_left(unsigned char speed)
{
    rotate_left_ini();
    pwm_left = speed;
    pwm_right = speed;
}
void motor_rotate_right(unsigned char speed)
{
    rotate_right_ini();
    pwm_left = speed;
    pwm_right = speed;
}
void motor_stop(void)
{
    pwm_left = 0;
    pwm_right = 0;

    HBRI_L1_LOW();
    HBRI_L2_LOW();
    HBRI_R1_LOW();
    HBRI_R2_LOW();
}
void motor_slight_change(unsigned char left_speed, unsigned char right_speed)
{
    forward_ini();
    pwm_left = left_speed;
    pwm_right = right_speed;
}
void execute_path_action(PathAction action)
{
    switch (action)
    {
        case Go_forward:
            motor_forward(10); //speed not sure
            delayms(300);
            break;

        case Go_left:
            motor_rotate_left(10);
            delayms(300);
            break;

        case Go_right:
            motor_rotate_right(10);
            delayms(300);
            break;

        case End_stop:
        default:
            motor_stop();
            break;
    }
}

#define span 2
void move_forward_with_field_detection(void)
{
    unsigned int left_val, right_val;
    int error;
    left_val = adc_read_left();
    right_val = adc_read_right();
    error = (int)left_val - (int)right_val;
    if (error >= -span && error <= span)
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
unsigned char intersection_detected(void)
{
    unsigned int inter_val;
    inter_val = adc_read_intersection();

    if (inter_val > INTERSECTION_THRESHOLD)
        return 1;
    else
        return 0;
}

void auto_mode_task(unsigned char path_number, unsigned char *intersection_count)
{
    static unsigned char in_intersection = 0;

    if (intersection_detected())
    {
        if (!in_intersection)
        {
            in_intersection = 1;
            (*intersection_count)++;
            execute_path_action(get_path_action(path_number, *intersection_count));
        }
    }
    else
    {
        in_intersection = 0;
        move_forward_with_field_detection();
    }
}
#define CMD_STOP      0
#define CMD_FORWARD   1
#define CMD_BACKWARD  2
#define CMD_LEFT      3
#define CMD_RIGHT     4

void manual_command_execute(unsigned char cmd)
{
    switch(cmd)
    {
        case CMD_FORWARD:
            motor_forward(10);
            break;

        case CMD_BACKWARD:
            motor_backward(10);
            break;

        case CMD_LEFT:
            motor_rotate_left(10);
            break;

        case CMD_RIGHT:
            motor_rotate_right(10);
            break;

        case CMD_STOP:
        default:
            motor_stop();
            break;
    }
}