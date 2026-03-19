#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

#define sBAUD 9600

/*
#define sOUTPORT PORTB
#define sINPORT  PINB
#define sTXD (1<<PC5) // PC5 is used as sTXd, Pin28 of ATmega328p (DIP28)
#define sRXD (1<<PB1) // PB1 is used as sRXD, Pin15 of ATmega328p (DIP28)
*/

/* Pinout for DIP28 ATMega328P:

                           -------
     (PCINT14/RESET) PC6 -|1    28|- PC5 (ADC5/SCL/PCINT13)
       (PCINT16/RXD) PD0 -|2    27|- PC4 (ADC4/SDA/PCINT12)
       (PCINT17/TXD) PD1 -|3    26|- PC3 (ADC3/PCINT11)
      (PCINT18/INT0) PD2 -|4    25|- PC2 (ADC2/PCINT10)
 (PCINT19/OC2B/INT1) PD3 -|5    24|- PC1 (ADC1/PCINT9)
    (PCINT20/XCK/T0) PD4 -|6    23|- PC0 (ADC0/PCINT8)
                     VCC -|7    22|- GND
                     GND -|8    21|- AREF
(PCINT6/XTAL1/TOSC1) PB6 -|9    20|- AVCC
(PCINT7/XTAL2/TOSC2) PB7 -|10   19|- PB5 (SCK/PCINT5)
   (PCINT21/OC0B/T1) PD5 -|11   18|- PB4 (MISO/PCINT4)
 (PCINT22/OC0A/AIN0) PD6 -|12   17|- PB3 (MOSI/OC2A/PCINT3)
      (PCINT23/AIN1) PD7 -|13   16|- PB2 (SS/OC1B/PCINT2)
  (PCINT0/CLKO/ICP1) PB0 -|14   15|- PB1 (OC1A/PCINT1)
                           -------
*/



unsigned char echo=0;

void IR_Init(void)
{
	PORTD&=~(1<<3);
	
	TCCR2A=0;
	TCCR2B=0;
	TCNT2=0;
	
	TCCR2A|=(1<<WGM20)|(1<<WGM21);
	TCCR2B |= (1 << WGM22);
	
	TCCR2B |= (1 << CS21);
	
	OCR2A = 51;
	OCR2B = 17;
}

void IR_On(void)
{
	TCCR2A &= ~(1 << COM2B0);
	TCCR2A |=  (1 << COM2B1);
}

void IR_Off(void)
{
	TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));
	PORTD &= ~(1 << PORTD3);
}

void SendByte (unsigned char c)
{
	unsigned char i;
	
	// Send start bit
	IR_Off();
  	_delay_us(1E6/sBAUD);
  	// Send 8 data bits
	for (i=0; i<8; i++)
  	{
    	if( c & 1 )
    	{
      		IR_Off();
      	}
    	else
      	{
      		IR_On();
      	}
    	c >>= 1;
		_delay_us(1E6/sBAUD);
 	}
 	// Send the stop bit
	IR_On();
	_delay_us(1E6/sBAUD);
}

void SendString(char * s)
{
	while(*s != 0) SendByte(*s++);
}