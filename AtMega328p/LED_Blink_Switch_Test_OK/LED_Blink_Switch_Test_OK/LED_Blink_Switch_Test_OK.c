/*
 * LED_Blink_Switch_Test_OK.c
 * Test made with ATMega 328p (Uno)
 *
 * Created: 12/24/2017 5:18:21 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>

void timer1_init()
{
	//Set up timer with Prescaler = 256
	TCCR1B |= (1 << CS12);
	//Initialize counter
	TCNT1 = 0;
}
int main(void)
{
	//Pin D4 set as output
	DDRD |= (1 << PORTD4);
	//Initialize Timer1
	timer1_init();
	
    while(1)
    {
		//When the switch is active, turn on the LED which blinks with a 1 second delay
        if(!(PINC & (1 << PORTC0)))
		{
			//Value based on the formula Target Timer Count = (Input Frequency / (Prescaler * Target Frequency)) - 1
			if(TCNT1 >= 62499)
			{
				//Toggle the LED
				PORTD ^= (1 << PORTD4);
				//Reset timer value
				TCNT1 = 0;
			}
		}
		else
		{
			//Turn off the LED
			PORTD &= ~(1 << PORTD4);
		}
    }
}
