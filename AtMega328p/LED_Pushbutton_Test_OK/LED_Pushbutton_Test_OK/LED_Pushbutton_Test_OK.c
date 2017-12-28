/*
 * LED_Pushbutton_Test_OK.c
 *
 * Created: 12/28/2017 2:04:36 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>

int main(void)
{
	//Set Pin B0 as input
	DDRB &= ~(1 << PORTB0);
	//Set Pin B1 as output
	DDRB |= (1 << PORTB1);
    while(1)
    {
        if(PINB & (1 << PORTB0))
		{
			//When the pushbutton is pressed, the LED is on
			PORTB |= (1 << PORTB1);
		}
		else
		{
			//When the pushbutton is not pressed, the LED is off
			PORTB &= ~(1 << PORTB1);
		}
    }
}