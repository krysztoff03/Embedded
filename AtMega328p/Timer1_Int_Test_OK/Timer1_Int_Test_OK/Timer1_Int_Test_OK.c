/*
 * Timer1_Int_Test_OK.c
 * Test made on ATMega 328p (2560)
 * 
 * Created: 12/2/2017 11:15:31 AM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t timer1Overflow; //global variable which holds the number of Timer1 overflows

void timer1_init()
{
	//set Timer1 with Prescaler = 8
	TCCR1B |= (1 << CS11);
	//initialize Timer1 counter
	TCNT1 = 0;
	//enable overflow interrupt
	TIMSK1 |= (1 << TOIE1);
	//enable global interrupts
	sei();
	//initialize overflow's counter
	timer1Overflow = 0;
}

ISR(TIMER1_OVF_vect)
{
	//retain the number of overflows
	timer1Overflow++;
	//31 overflows = approx. 0.8 second delay
	if(timer1Overflow >= 24)
	{
		//toggle the LED on PortE
		PORTE ^= (1 << PE5);
		//reset overflow counter
		timer1Overflow = 0;
	}
}

int main(void)
{
	//set Pin5 on PortE as output
	DDRE |= (1 << PE5);
	//initialize Timer1
	timer1_init();
    while(1)
    {
        //TODO:: Please write your application code 
    }
}