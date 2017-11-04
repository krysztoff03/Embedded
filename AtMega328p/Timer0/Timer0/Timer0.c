/*
 * TIMER0 Modes of Operation
 *
 * Created: 8/28/2017 7:09:53 PM
 *  Author: Cristi
 */ 


#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void timer0_normal_operation_init()
{
	//set TIMER0 with 1:64 prescaler value
	TCCR0B = (1 << CS01) | (1 << CS00);
}

void timer0_ctc_init()
{
	//set TIMER0 with 1:64 prescaler value
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	//initialize counter
	TCNT0 = 0;
	//initialize compare value
	OCR0A = 249;
}

void timer0_fast_pwm_init()
{
	
}

int main(void)
{
	/************************************************************************/
	/* TIMER0 Normal Operation Mode - LED blinks with 1kHz frequency = 1ms  */
	/************************************************************************/
	/*
	//initialize Pin 7 of PORTD
	DDRD |= (1 << PD7);
	//initialize TIMER0 in Normal Operation Mode
	timer0_normal_operation_init();
    while(1)
    {
        if(TCNT0 >= 249)
		{
			//toggle LED on Pin 7 when the counter reaches 249
			PORTD ^= (1 << PD7); 
			//reset counter
			TCNT0 = 0;
		}
    }*/
	
	/************************************************************************/
	/* TIMER0 Clear Timer on Compare Mode - LED blinks with 1kHz frequency = 1ms */
	/************************************************************************/
	
	//set Pin 7 of PORTD
	DDRD |= (1 << PD7);
	//initialize TIMER0 in Clear Timer on Compare Mode
	timer0_ctc_init();
	while(1)
	{
		//check if the flag bit is set
		if(TIFR0 & (1 << OCF0A))
		{
			//toggle LED on Pin 7
			PORTD ^= (1 << PD7);
		}
		//clear the bit manually since there is no interrupt
		TIFR0 |= (1 << OCF0A);
	}
}