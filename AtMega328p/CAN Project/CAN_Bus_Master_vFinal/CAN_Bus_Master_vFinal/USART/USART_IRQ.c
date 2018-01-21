/*
 * USART_IRQ.c
 *
 * Created: 1/3/2018 1:05:08 PM
 *  Author: Cristi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "USART_IRQ.h"

/* Static Variables */
static unsigned char USART_RxBuf[USART_RX_BUFFER_SIZE];
static volatile unsigned char USART_RxHead;
static volatile unsigned char USART_RxTail;
static unsigned char USART_TxBuf[USART_TX_BUFFER_SIZE];
static volatile unsigned char USART_TxHead;
static volatile unsigned char USART_TxTail;


/* Initialize USART */
void USART0_Init(unsigned int ubrr_val)
{
	unsigned char x;

	/* Set the baud rate */
	UBRR0H = (unsigned char) (ubrr_val>>8);                  
	UBRR0L = (unsigned char) ubrr_val;
	
	/* Enable USART receiver and transmitter */
	UCSR0B = ((1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0)); 

	/* For devices in which UBRRH/UCSRC shares the same location
	* eg; ATmega16, URSEL should be written to 1 when writing UCSRC
	* 
	*/
	/* Set frame format: 8 data 2stop */
	UCSR0C = (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);
	
	/* It appears that ATMega328p on Arduino Uno U2X0 bit is On by default
	* Clearing the bit on register UCSR0A solves the baud rate problem */
	UCSR0A = (0 << U2X0);
	/* Flush receive buffer */
	x = 0; 			    

	USART_RxTail = x;
	USART_RxHead = x;
	USART_TxTail = x;
	USART_TxHead = x;
}


//#if __GNUC__
ISR(USART0_RX_vect)
//#else
//#pragma vector=USART_RX_vect
//__interrupt void USART_RX_interrupt(void)
//#endif
{
	unsigned char data;
	unsigned char tmphead;

	/* Read the received data */
	data = UDR0;                 
	/* Calculate buffer index */
	tmphead = (USART_RxHead + 1) & USART_RX_BUFFER_MASK;
	/* Store new index */
	USART_RxHead = tmphead;

	if (tmphead == USART_RxTail) {
		/* ERROR! Receive buffer overflow */
	}
	/* Store received data in buffer */
	USART_RxBuf[tmphead] = data; 
}


//#if __GNUC__
ISR(USART0_UDRE_vect)
//#else
//#pragma vector=USART_UDRE_vect
//__interrupt void USART1_DRE_interrupt(void)
//#endif
{
	unsigned char tmptail;

	/* Check if all data is transmitted */
	if (USART_TxHead != USART_TxTail) {
		/* Calculate buffer index */
		tmptail = (USART_TxTail + 1) & USART_TX_BUFFER_MASK;
		/* Store new index */
		USART_TxTail = tmptail;      
		/* Start transmission */
		UDR0 = USART_TxBuf[tmptail];  
	} else {       
		/* Disable UDRE interrupt */
		UCSR0B &= ~(1<<UDRIE0);         
	}
}

unsigned char USART0_Receive(void)
{
	unsigned char tmptail;
	
	/* Wait for incoming data */
	while (USART_RxHead == USART_RxTail);
	/* Calculate buffer index */
	tmptail = (USART_RxTail + 1) & USART_RX_BUFFER_MASK;
	/* Store new index */
	USART_RxTail = tmptail;                
	/* Return data */
	return USART_RxBuf[tmptail];          
}

void USART0_Transmit(unsigned char data)
{
	unsigned char tmphead;
	
	/* Calculate buffer index */
	tmphead = (USART_TxHead + 1) & USART_TX_BUFFER_MASK;
	/* Wait for free space in buffer */
	while (tmphead == USART_TxTail);
	/* Store data in buffer */
	USART_TxBuf[tmphead] = data;
	/* Store new index */
	USART_TxHead = tmphead;               
	/* Enable UDRE interrupt */
	UCSR0B |= (1<<UDRIE0);                    
}

void USART_putstring(char* StringPtr)
{
	/* Here we check if there is still more chars to send, this is done checking the actual char and see if it is different from the null char */
	while(*StringPtr != 0x00)
	{
		/* Using the simple send function we send one char at a time */
		USART0_Transmit(*StringPtr);
		/* We increment the pointer so we can read the next char */
		StringPtr++;
	}
	
}