/*
 * LEDx2_Interrupt_Test_OK.c
 * Test made on ATMega328p (Uno)
 *
 * Created: 1/2/2018 1:50:44 PM
 *  Author: Cristi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcdpcf8574/lcdpcf8574.h"

//Variables/flags used in main for displaying "On/Off" state of the LEDs
volatile int ledD4Flag = 0, ledB0Flag = 0;

ISR(PCINT1_vect)
{
	if(PINC & (1 << PORTC0))
	{
		//If the switch is pressed, turn on the LED
		PORTB |= (1 << PORTB0);
		ledB0Flag = 1;
	}
	else if(!(PINC & (PORTC0)))
	{
		//If the switch is not pressed, turn off the LED
		PORTB &= ~(1 << PORTB0);
		ledB0Flag = 0;
	}
}

ISR(PCINT2_vect)
{
	if(PIND & (1 << PORTD7))
	{
		//If the switch is pressed, turn on the LED
		PORTD |= (1 << PORTD4);
		ledD4Flag = 1;
	}
	//If the switch is not pressed, turn off the LED
	else if(!(PIND & (1 << PORTD7)))
	{
		PORTD &= ~(1 << PORTD4);
		ledD4Flag = 0;
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
	DDRD |= (1 << PORTD4);
	//Set Pin D7 as input
	DDRD &= ~(1 << PORTD7);
	//Enable Pin D7 pull-up resistor
	PORTD |= (1 << PORTD7);
	//Enable Pin Change Interrupt
	PCICR = (1 << PCIE2);
	//Enable PCINT23 (Pin D7)
	PCMSK2 = (1 << PCINT23);
	
	//Set Pin B0 as output
	DDRB |= (1 << PORTB0);
	//Set Pin C0 as input
	DDRD &= ~(1 << PORTC0);
	//Enable Pin C0 pull-up resistor
	PORTD |= (1 << PORTC0);
	//Enable Pin Change Interrupt
	PCICR |= (1 << PCIE1);
	//Enable PCINT8 (Pin A0)
	PCMSK1 |= (1 << PCINT8);
	
	//Enable global interrupts
	sei();
	//LCD clear screen
	lcd_clrscr();
	
    while(1)
    {
        if(ledD4Flag == 1)
		{
			//Set cursor on line 1
			lcd_home();
			lcd_gotoxy(0, 0);
			lcd_puts("     LED ");
			lcd_gotoxy(9, 0);
			lcd_puts("D4 On!    ");
		}
		else if(ledB0Flag == 1)
		{
			//Set cursor on line 1
			lcd_home();
			lcd_gotoxy(0, 0);
			lcd_puts("     LED ");
			lcd_gotoxy(9, 0);
			lcd_puts("B0 On!    ");
		}
		else if((ledD4Flag == 0) && (ledB0Flag == 0))
		{
			//Set cursor on line 1
			lcd_home();
			lcd_gotoxy(0, 0);
			lcd_puts("LEDs B0 and D4 Off!");
		}
    }
}