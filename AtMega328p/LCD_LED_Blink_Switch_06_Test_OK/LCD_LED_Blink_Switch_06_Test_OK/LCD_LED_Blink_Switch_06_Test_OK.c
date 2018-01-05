/*
 * LCD_LED_Blink_Switch_06_Test_OK.c
 * Test made on ATMega328p (2560)
 *
 * Created: 1/5/2018 3:31:20 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>

#include "LCD/lcd.h"

#define LED (1 << PE4)
#define SWITCH (1 << PB6)

void timer1_init()
{
	/* Set up timer with Prescaler = 256 */
	TCCR1B |= (1 << CS12);
	/* Initialize counter */
	TCNT1 = 0;
}

int main(void)
{
	/* LCD initialization with display ON, cursor OFF */
	lcd_init(LCD_DISP_ON);
	/* Timer1 initialization */
	timer1_init();
	/* Set Pin E4 as output */
	DDRE |= LED;
	/* Set Pin B6 as input */
	DDRB &= ~SWITCH;
	/* Clear screen before entering the infinite loop */
	lcd_clrscr();
    while(1)
    {
		/* When the switch is active, the LED blinks with 0.6 seconds delay */
        if(PINB & SWITCH)
		{
			/* Value based on the formula Target Timer Count = (Input Frequency / (Prescaler * Target Frequency)) - 1 */
			if(TCNT1 >= 37649)
			{
				PORTE ^= LED;
				/* When the Pin E4 is High (LED On), display the message on the LCD */
				if(!(PINE & LED))
				{
					lcd_home();
					lcd_puts("LED On! ");
				}
				/* When the Pin E4 is Low (LED Off), display the message on the LCD */
				else
				{
					lcd_home();
					lcd_puts("LED Off!");	
				}
				/* Reset the counter */
				TCNT1 = 0;
			}
		}
		/* When the switch is not active, the LED is Off and the message is displayed on the LCD */
		else
		{
			PORTE &= ~LED;
			lcd_home();
			lcd_puts("Closed!");
		}
    }
}
