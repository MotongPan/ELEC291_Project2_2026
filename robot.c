
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
#define L_PWM P2_0
#define R_PWM P2_1

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
//===========================
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
            delay_ms(300);
            break;

        case Go_left:
            motor_rotate_left(10);
            delay_ms(300);
            break;

        case Go_right:
            motor_rotate_right(10);
            delay_ms(300);
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
    motor_stop();

    while (RobotMode=0)
    {      
        execute_path_action(get_path_action(path_num, inter_count));
    }
}