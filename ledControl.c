#include "../Common/Include/stm32l051xx.h"
#include "led.h"

volatile int x;

static void delay(int dly)
{
    while(dly--) x++;
}

void LED_Init(void)
{
    RCC->IOPENR |= BIT0; 
    GPIOA->MODER = (GPIOA->MODER & ~(BIT17 | BIT16)) | BIT16;
    GPIOA->MODER = (GPIOA->MODER & ~(BIT7 | BIT6)) | BIT6;
    GPIOA->ODR &= ~(BIT8 | BIT3);
}

void Turn_Left_Flash(void)
{
    GPIOA->ODR &= ~BIT3;
    GPIOA->ODR |= BIT8;
    delay(400000);
    GPIOA->ODR &= ~BIT8;
    delay(400000);
}

void Turn_Right_Flash(void)
{
    GPIOA->ODR &= ~BIT8;
    GPIOA->ODR |= BIT3;
    delay(400000);
    GPIOA->ODR &= ~BIT3;
    delay(400000);
}

void Hazard_Lights(void)
{
    GPIOA->ODR |= (BIT8 | BIT3);
    delay(400000);
    GPIOA->ODR &= ~(BIT8 | BIT3);
    delay(400000);
}