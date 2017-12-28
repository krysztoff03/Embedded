/*
 * LCD_LED_Pushbutton_Debounce_test_OK.c
 * Test made on ATMega328p (2560)
 *
 * Created: 12/28/2017 6:53:02 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#include "LCD/lcd.h"

//Variable that holds the button's value when is pressed and debounced
volatile uint8_t button_down;

//Function that checks the button state and set button_down variable
//if a debounced button is detected
static inline void debounce()
{
	//Counter for equal states
	static uint8_t count = 0;
	//Holds the number of debounced states
	static uint8_t button_state = 0;
	//Check if the button is pressed or not
	uint8_t current_state = (~PINF & (1 << PF1)) != 0;
	if(current_state != button_state)
	{
		//Button state will change and counter is increased
		count++;
		if(count >= 4)
		{
			//Button have not bounced for four checks, change state
			button_state = current_state;
			//If the button was pressed, communicate to main
			if(current_state != 0)
			{
				button_down = 1;
			}
			count = 0;
		}
	}
	else
	{
		//Reset counter
		count = 0;
	}
}

int main(void)
{
	//Enable pull-up resistor on the Pin F1
	PORTF |= (1 << PF1);
	//Set Pin E5 as output
	DDRE |= (1 << PE5);
	////LCD initialization with display ON, cursor OFF
	lcd_init(LCD_DISP_ON);
    while(1)
    {
        //Update button state
        debounce();
        //Check if the button is pressed
        if(button_down)
        {
	        button_down = 0;
	        PORTE ^= (1 << PE5);
			//LCD clear screen
			lcd_clrscr();
			lcd_home();
			//Set cursor on line 1
			lcd_gotoxy(5, 0);
			lcd_puts("LED On!");
			//Check if the LED is off and clear screen
			if(~PINE & (1 << PE5))
			{
				lcd_clrscr();
				lcd_home();
				//Set cursor on line 1
				lcd_gotoxy(5, 0);
				lcd_puts("LED Off!");
			}
        }
        _delay_ms(10);
    }
}