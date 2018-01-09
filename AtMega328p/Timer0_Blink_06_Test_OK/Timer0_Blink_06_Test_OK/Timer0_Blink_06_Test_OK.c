/*
 * Timer0_Blink_06_Test_OK.c
 *
 * Created: 1/8/2018 9:45:39 PM
 *  Author: Cristi
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#define LEDS (1 << PORTD3)

/* Variable which counts the number of overflows */
volatile uint8_t totalOverflows;

ISR(TIMER0_OVF_vect)
{
	/* Keep the number of overflows */
	totalOverflows++;
}

/* Prototype */
void timer0_init();

void timer0_init()
{
	/* Set up Timer0 with Prescaler = 1024 */
	TCCR0B |= (1 << CS02) | (1 << CS00);
	/* Initialize counter */
	TCNT0 = 0;
	/* Enable overflow interrupt */
	TIMSK0 |= (1 << TOIE0);
	/* Enable global interrupts */
	sei();
	/* Initialize overflows counter */
	totalOverflows = 0;
}

int main(void)
{
	/* Initialize Timer0 */
	timer0_init();
	/* Set Pin D3 as output */
	DDRD |= LEDS;
    while(1)
    {
		/* Check if overflows number = 36*/
        if(totalOverflows >= 36)
		{
			/* Check if the timer count reaches 195 */
			if(TCNT0 >= 195)
			{
				/* Toggle the LEDs */
				PORTD ^= LEDS;
				/* Reset counter */
				TCNT0 = 0;
				/* Reset overflows counter */
				totalOverflows = 0;
			}
		}
    }
}