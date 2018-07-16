/*
 * USART_String_Test_OK.c
 *
 * Created: 7/16/2018 6:19:54 PM
 *  Author: cristi
 */ 


#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBBR ((F_CPU/(16UL*BAUD))-1)

void fnUSARTInit();
void fnUSARTTransmit(uint8_t data);
unsigned char fnUSARTReceive();
void fnUSARTStringTransmit(char *string);

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

unsigned char fnUSARTReceive()
{
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

void fnUSARTStringTransmit(char *string)
{
	int i = 0;
	while(string[i] != '\0')
	{
		fnUSARTTransmit(string[i]);
		i++;
	}
}

int main(void)
{
	char *str = "Arduino Mega 2560 USART Test.\n\r";
	fnUSARTInit();
	
    while(1)
    {
        //TODO:: Please write your application code 
		fnUSARTStringTransmit(str);
		_delay_ms(1000);
    }
}