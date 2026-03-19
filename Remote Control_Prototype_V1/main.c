#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"
#include "lcd.h"
#include "SoftwareUart.h"
#include "pushbutton.h"
#include <util/delay.h>

#define BUTT0 (1<<4)
#define BUTT1 (1<<3)
#define BUTT2 (1<<2)
#define BUTT3 (1<<1)
#define BUTT4 (1<<0)
#define BUTT5 (1<<2)
#define BUTT6 (1<<1)

#define AUTO 1
#define MANU 0

unsigned int mode = 0;
unsigned int page = 0;
unsigned int LCD_update_request = 0;
// Auto command signals (paths)
// Send 8 bits for convenience. Example function takes unsigned char input
unsigned char autoP1=0b10000000;
unsigned char autoP2=0b10000001;
unsigned char autoP3=0b10000010;
// Manual Command signals
unsigned char stop=0b00000000;
unsigned char forward=0b00000001;
unsigned char backward=0b00000010;
unsigned char left=0b00000011;
unsigned char right=0b00000100;
unsigned char one80=0b00000101;

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

// These are the connections between the LCD and the ATMega328P:
//
// LCD          ATMega328P
//----------------------------
// D7           PB0
// D6           PD7
// D5           PD6
// D4           PD5
// LCD_E        PD4
// LCD_W        GND
// LCD_RS       PD3
// V0           2k+GND
// VCC          5V
// GND          GND
//
// There is also a picture that shows how the LCD is attached to the ATMega328P.

void update_lcd(void)
{
	if(LCD_update_request)
	{
		LCD_update_request = 0;
		if(page)
		{
			LCDprint("STP FOR BCK L R", 2, 1);
		}
		else
		{
			LCDprint("STP 180 P1 P2 P3", 2, 1);
		}
	}
}

void check_press(void)
{
	if(((~PINC)&0b00001111) || ((~PINB)&0b00000110))
	{
		stop_car();
		toggle_mode();
		swap_page();
		forward_and_pi_rad();
		back_and_p1();
		left_and_p2();
		right_and_p3();
	}
}

void stop_car(void)
{
	if(!(PINC&BUTT0))
	{
		_delay_ms(20);		//Debounce Press
		if(!(PINC&BUTT0))
		{
			SendByte(stop);
			while(!(PINC&BUTT0));
			_delay_ms(20); 	// Debounce Release
		}
	}
}

void toggle_mode(void)
{
	if(!(PINC&BUTT1))
	{
		_delay_ms(20);		//Debounce Press
		if(!(PINC&BUTT1))
		{
			mode^=1;
			while(!(PINC&BUTT1));
			_delay_ms(20); 	// Debounce Release
		}
	}
}

void swap_page(void)
{
	if(!(PINC&BUTT2))
	{
		_delay_ms(20);		//Debounce Press
		if(!(PINC&BUTT2))
		{
			page^=1;
			LCD_update_request = 1; // Request to update LCD with new button functions
			while(!(PINC&BUTT2));
			_delay_ms(20); 	// Debounce Release
		}
	}
}

void forward_and_pi_rad(void) //pi rad sounds cooler than one_eighty
{
	if(!(PINC&BUTT3))
	{
		_delay_ms(20);		//Debounce Press
		if(!(PINC&BUTT3))
		{
			if(page)
			{
				LCDprint("Button Test 3.1", 1, 1);
				SendByte(forward);
			}
			else
			{
				SendByte(one80);
				LCDprint("Button Test 3.0", 1, 1);
			}
			while(!(PINC&BUTT3));
			_delay_ms(20); 	// Debounce Release
		}
	}
}

void back_and_p1(void)
{
	if(!(PINC&BUTT4))
	{
		_delay_ms(20);		//Debounce Press
		if(!(PINC&BUTT4))
		{
			if(page)
			{
				SendByte(backward);
				LCDprint("Button Test 4.1", 1, 1);
			}
			else
			{
				SendByte(autoP1);
				LCDprint("Button Test 4.0", 1, 1);
			}
			while(!(PINC&BUTT4));
			_delay_ms(20); 	// Debounce Release
		}
	}
}

void left_and_p2(void)
{
	if(!(PINB&BUTT5))
	{
		_delay_ms(20);		//Debounce Press
		if(!(PINB&BUTT5))
		{
			if(page)
			{
				SendByte(left);
				LCDprint("Button Test 5.1", 1, 1);
			}
			else
			{
				SendByte(autoP2);
				LCDprint("Button Test 5.0", 1, 1);
			}
			while(!(PINB&BUTT5));
			_delay_ms(20); 	// Debounce Release
		}
	}
}

void right_and_p3(void)
{
	if(!(PINB&BUTT6))
	{
		_delay_ms(20);		//Debounce Press
		if(!(PINB&BUTT6))
		{
			if(page)
			{
				SendByte(right);
				LCDprint("Button Test 6.1", 1, 1);
			}
			else
			{
				SendByte(autoP3);
				LCDprint("Button Test 6.0", 1, 1);
			}
			while(!(PINB&BUTT6));
			_delay_ms(20); 	// Debounce Release
		}
	}
}

void Configure_Pins(void)
{
	DDRB|=0b00000001; // PB0 is output.
	DDRD|=0b11111100; // PD2, PD4, PD5, PD6, and PD7 are outputs.
	
	DDRC  &= 0b11100000; // Buttons 0-4
	PORTC |= 0b00011111; // Activate pull-up in PC0-4
	DDRB  &= 0b11111001; // Buttons 5-6
	PORTB |= 0b00000110; // Activate pull-up in PB1-2
}

int main( void )
{
	//char buff[17];
	

	usart_init(); // configure the usart and baudrate
	
	Configure_Pins();
	LCD_4BIT();
	IR_Init();
	IR_On();
	//ConfigureSoftwareUART();
	
	_delay_ms(500); // Give putty some time to start.
	printf("ATMega328P 4-bit LCD test.\n");

   	// Display something in the LCD
	LCDprint("Intro", 1, 1);
	LCDprint("STP 180 P1 P2 P3", 2, 1);
	while(1)
	{
		check_press();
		update_lcd();
	}
}
