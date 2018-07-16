/*
 * Timer0_CTC_Mode_Test_OK.c
 *
 * Created: 7/14/2018 2:16:07 PM
 *  Author: crist
 */ 


#include <avr/io.h>

#define LED (1 << PE5)
#define SWITCH (1 << PB4)

int main(void)
{
	DDRE |= LED;
	DDRB &= ~SWITCH;
    while(1)
    {
        //TODO:: Please write your application code 
		if(!(PINB & SWITCH))
		{
			PORTE |= LED;
		}
		else
		{
			PORTE &= ~LED;
		}
    }
}