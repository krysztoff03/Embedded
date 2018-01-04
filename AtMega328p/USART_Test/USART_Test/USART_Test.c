/*
 * USART_Test.c
 *
 * Created: 1/3/2018 1:04:45 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "USART_IRQ.h"

char String[]="Hello world!!\n\r";

int main(void)
{
	/* Calculate baudrate */
	USART0_Init(MYUBBR);
	/* Enable global interrupts */
	sei();
	
    while(1)
    {
		/* Echo the received character  */
		//USART0_Transmit(USART0_Receive());
		USART_putstring(String);
		_delay_ms(2000);
    }
}

