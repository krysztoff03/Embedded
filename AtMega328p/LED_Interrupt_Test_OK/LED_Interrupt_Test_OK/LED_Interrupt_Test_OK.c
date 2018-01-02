/*
 * LED_Interrupt_Test_OK.c
 * Test made on ATMega328p (Uno)
 *
 * Created: 12/31/2017 12:04:21 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcdpcf8574/lcdpcf8574.h"

//Variable/flag used in main for displaying "On/Off" state of the LCD
volatile int ledFlag = 0;

ISR(PCINT2_vect)
{
	if(PIND & (1 << PD7))
	{
		//If the switch is pressed, turn on the LED
		PORTD |= (1 << PD4);
		ledFlag = 1;
	}
	//If the switch is not pressed, turn off the LED
	else if(!(PIND & (1 << PD7)))
	{
		PORTD &= ~(1 << PD4);
		ledFlag = 0;
	}
}

int main(void)
{
	//LCD initialization with display ON, cursor OFF
	lcd_init(LCD_DISP_ON);
	//Variable used to illuminate LCD (0 = on, 1 = off)
	uint8_t led = 0;
	//Illuminate the LCD
	lcd_led(led);
	//Set Pin D4 as output
	DDRD |= (1 << PD4);
	//Set Pin D7 as input
	DDRD &= ~(1 << PD7);
	//Enbale Pin D7 pull-up resistor
	PORTD |= (1 << PD7);
	//Enable Pin Change Interrupt
	PCICR = (1 << PCIE2);
	//Enable PCINT23 (Pin D7)
	PCMSK2 = (1 << PCINT23);
	//Enable global interrupts
	sei();
	//LCD clear screen
	lcd_clrscr();
	while(1)
	{
		if(ledFlag == 1)
		{
			//Set cursor on line 1
			lcd_home();
			lcd_gotoxy(6, 0);
			lcd_puts("LED");
			lcd_gotoxy(10, 0);
			lcd_puts("On! ");
		}
		else
		{
			//Set cursor on line 1
			lcd_home();
			lcd_gotoxy(6, 0);
			lcd_puts("LED");
			lcd_gotoxy(10, 0);
			lcd_puts("Off!");
		}
	}
}