/*
 * Timer0_Button_Interrupt_Test_OK.c
 *
 * Created: 7/13/2018 10:19:45 PM
 *  Author: cristi
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#define SWITCH (1 << PB4)
#define LED (1 << PE5)

// Flag variable which signals when the button is pressed
volatile uint8_t buttonPressed = 0;

void timer0_init();

/* On every 2ms it generates an interrupt that checks if the button is pressed */
void timer0_init()
{
	/* Clear OC0A on Compare Match and CTC Mode */
	TCCR0A |= (1 << COM0A1) | (1 << WGM01);
	/* Timer0 Output Compare Match Interrupt Enable */
	TIMSK0 |= (1 << OCIE0A);
	/* Timer0 Interrupt Flag Register */
	TIFR0 |= (1 << OCF0A);
	/* Maximum value for 2 ms interrupt */
	OCR0A = 124;
	/* Enable global interrupts */
	sei();
	/* Set up Timer0 with Prescaler = 256 */
	TCCR0B |= (1 << CS02);
}

ISR(TIMER0_COMPA_vect)
{
	// Check if the button state is High	
	if(PINB & SWITCH)
	{
		buttonPressed = 1;
	}
	else
	{
		buttonPressed = 0;
	}
}

int main(void)
{
	/* PE5 as output - LED */
	DDRE |= LED;
	//PORTE |= LED;
	/* PB4 as input - Pushbutton */
	DDRB &= ~SWITCH;
	//PORTB |= SWITCH;
	/* Timer0 initialization */
	timer0_init();
    while(1)
    {
		if(buttonPressed == 1)
		{
			PORTE |= LED;	
		}
		else
		{
			PORTE &= ~LED;
		}
        //TODO:: Please write your application code 
    }
}