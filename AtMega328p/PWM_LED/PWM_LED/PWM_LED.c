/*
 * PWM_LED.c
 *
 * Created: 9/5/2017 7:33:49 PM
 *  Author: Cristi
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

void port_init()
{
	//Set Pin6(PWM pin) of PORTD as output
	DDRD |= (1 << PD6);
}

void pwm_init()
{
	//Set TIMER0 in Non-Inverting Fast PWM Mode
	TCCR0A = (1<< COM0A1) | (1 << WGM01) | (1 << WGM00);
	//No prescaler for TIMER0
	TCCR0B = (1 << CS00);
}

int main(void)
{
	/************************************************************************/
	/* TIMER0 Fast PWM Mode - LED increase and decrease it's brightness     */
	/************************************************************************/
	//Initialize PORTD
	port_init();
	//Initialize PWM
	pwm_init();
	uint8_t brightness;
    while(1)
    {
		//Increase LED brightness
        for(brightness = 0;brightness < 255;brightness++)
		{	//Set brightness variable as duty cycle
			OCR0A = brightness;
			//10ms delay
			_delay_ms(10);
		}
		//Decrease LED brightness
		for(brightness = 255;brightness > 0;brightness--)
		{
			//Set brightness variable as duty cycle
			OCR0A = brightness;
			//10ms delay
			_delay_ms(10);
		}
		
    }
}