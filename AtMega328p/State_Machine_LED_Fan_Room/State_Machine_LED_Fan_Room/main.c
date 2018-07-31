/*
 * State_Machine_LED_Fan_Room.c
 *
 * Created: 7/17/2018 8:20:21 PM
 * Author : cristi
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#define SWITCH (1 << PB6)
#define LED_LIGHT (1 << PE4)
#define LED_FAN (1 << PE5)

#define TIMER0_COUNTER_2MSEC 124
#define TIMER1_COUNTER_1SEC 15624

#define F_CPU 16000000UL
#define BAUD 57600
#define MYUBBR ((F_CPU/(16UL*BAUD))-1)

typedef struct _stTime_t
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;	
}stTime_t;

typedef struct _stFan_t
{
	stTime_t waitTimeBeforeStart;
	stTime_t activePeriod;	
}stFan_t;


/* Flag variables which signals when the buttons for rooms are pressed */
volatile uint8_t room1ButtonPressed = 0;

/* On every 2ms it generates an interrupt that checks if the buttons are pressed */
void fnTimer0Init()
{
	/* Clear OC0A on Compare Match and CTC Mode */
	TCCR0A |= (1 << COM0A1) | (1 << WGM01);
	/* Timer0 Output Compare Match Interrupt Enable */
	TIMSK0 |= (1 << OCIE0A);
	/* Timer0 Interrupt Flag Register */
	TIFR0 |= (1 << OCF0A);
	/* Maximum value for 2 ms interrupt */
	OCR0A = TIMER0_COUNTER_2MSEC;
	/* Enable global interrupts */
	sei();
	/* Set up Timer0 with Prescaler = 256 */
	TCCR0B |= (1 << CS02);
}

void fnTimer1Init()
{
	/* Set up Timer1 with Prescaler = 1024 */
	TCCR1B |= (1 << CS12) | (1 << CS10);
}

void fnUSARTInit()
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(MYUBBR >> 8);
	UBRR0L = (unsigned char)MYUBBR;
	/* Enable receiver and transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1 << UCSZ01) | (3 << UCSZ00);
}

void fnUSARTTransmit(uint8_t data)
{
	while(!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

uint8_t fnUSARTReceive()
{
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void fnUSARTStringTransmit(char *string)
{
	int i = 0;
	/* Parse the string until reaches the NULL character and send each character through USART */
	while(string[i] != '\0')
	{
		fnUSARTTransmit(string[i]);
		i++;
	}
}

/* Calculate the duration of light on period in seconds */
uint32_t fnCalculateLightOn(stFan_t *lightOn)
{
	return ((lightOn->waitTimeBeforeStart.hour * 3600) + (lightOn->waitTimeBeforeStart.minute * 60) + (lightOn->waitTimeBeforeStart.second));
}

/* Calculate the duration of fan on period in seconds */
uint32_t fnCalculateFanOn(stFan_t *fanOn)
{
	return ((fanOn->activePeriod.hour * 3600) + (fanOn->activePeriod.minute * 60) + (fanOn->activePeriod.second));
}

ISR(TIMER0_COMPA_vect)
{
	/* Check if the button state is High */
	if(PINB & SWITCH)
	{
		room1ButtonPressed = 1;
	}
	else
	{
		room1ButtonPressed = 0;
	}
}

int main(void)
{
	/* List of states */
	typedef enum {IDLE_STATE, LED_LIGHT_ON_STATE, LED_LIGHT_ON_FAN_ON_STATE, LED_LIGHT_OFF_FAN_ON_STATE} states;
	
	states state = IDLE_STATE;
	
	/* Holds the value of Timer1 overflows, 1 overflow = 1 second
	   uint32_t because one day has 86400 seconds */
	uint32_t seconds = 0;
	
	stFan_t room1Fan;
	
	room1Fan.waitTimeBeforeStart.hour = 0;
	room1Fan.waitTimeBeforeStart.minute = 0;
	room1Fan.waitTimeBeforeStart.second = 30;
	room1Fan.activePeriod.hour = 0;
	room1Fan.activePeriod.minute = 2;
	room1Fan.activePeriod.second = 0;
	
	
	
	/* PB6 as input - Switch button */
	DDRB &= ~SWITCH;
	/* PE4 as output - LED used to signal the light on in the room */
	DDRE |= LED_LIGHT;
	/* PE5 as output - LED used to signal the fan running in the room */
	DDRE |= LED_FAN;
	
	fnTimer0Init();
	/* Timer1 initialization */
	fnTimer1Init();
	/* USART initialization */
	void fnUSARTInit();
	
	fnUSARTStringTransmit("IDLE STATE\n\r");
	
	while(1)
	{
		switch(state)
		{
			case IDLE_STATE:
			/* Turn of the LED Light */
			PORTE &= ~LED_LIGHT;
			/* Check if the switch button is on */
			if(room1ButtonPressed == 1)
			{
				state = LED_LIGHT_ON_STATE;
				fnUSARTStringTransmit("LIGHT ON STATE\n\r");
			}
			break;
			
			case LED_LIGHT_ON_STATE:
			// Check if the switch button is on
			if(room1ButtonPressed == 1)
			{
				/* Delay for 1 second, true when counter reaches 1 second
				   Calculated with the formula Target Timer Count = (Input Frequency / Prescaler) / Target Frequency - 1 = (16MHz / 1024) / 1Hz - 1 = 15624 */
				if(TCNT1 >= TIMER1_COUNTER_1SEC )
				{
					/* Turn on the LED Light */
					PORTE |= LED_LIGHT;
					/* Reset Timer1 value */
					TCNT1 = 0;
					/* Increment the number of seconds */
					seconds++;
					if(seconds == (fnCalculateLightOn(&room1Fan)))
					{
						seconds = 0;
						state = LED_LIGHT_ON_FAN_ON_STATE;
						fnUSARTStringTransmit("LIGHT ON FAN ON STATE\n\r");
					}
				}
			}
			else
			{
				state = IDLE_STATE;
				fnUSARTStringTransmit("IDLE STATE\n\r");
			}
			break;
			
			case LED_LIGHT_ON_FAN_ON_STATE:
			PORTE |= LED_FAN;
			/* If the switch button is closed */
			if(room1ButtonPressed != 1)
			{
				/* Turn off the LED Light */
				PORTE &= ~LED_LIGHT;
				state = LED_LIGHT_OFF_FAN_ON_STATE;
				fnUSARTStringTransmit("LIGHT OFF FAN ON STATE\n\r");
			}
			break;
			
			case LED_LIGHT_OFF_FAN_ON_STATE:
			if(TCNT1 >= TIMER1_COUNTER_1SEC )
			{
				/* Reset Timer1 value */
				TCNT1 = 0;
				/* Increment the number of seconds */
				seconds++;
				if(seconds == (fnCalculateFanOn(&room1Fan)))
				{
					seconds = 0;
					/* Turn off the LED Fan */
					PORTE &= ~LED_FAN;
					state = IDLE_STATE;
					fnUSARTStringTransmit("IDLE STATE\n\r");
				}
			}
			break;
		}
	}
}
