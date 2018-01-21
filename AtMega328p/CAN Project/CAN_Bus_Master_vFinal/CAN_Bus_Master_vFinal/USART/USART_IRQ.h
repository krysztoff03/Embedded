/*
 * USART_IRQ.h
 *
 * Created: 1/3/2018 1:05:20 PM
 *  Author: Cristi
 */ 


#ifndef USART_IRQ_H_
#define USART_IRQ_H_

/* USART Buffer Defines */
#define USART_RX_BUFFER_SIZE 128     /* 2,4,8,16,32,64,128 or 256 bytes */
#define USART_TX_BUFFER_SIZE 128     /* 2,4,8,16,32,64,128 or 256 bytes */
#define USART_RX_BUFFER_MASK (USART_RX_BUFFER_SIZE - 1)
#define USART_TX_BUFFER_MASK (USART_TX_BUFFER_SIZE - 1)

/* Prototypes */
void USART0_Init(unsigned int ubrr_val);
unsigned char USART0_Receive(void);
void USART0_Transmit(unsigned char data);
void USART_putstring(char* StringPtr);


#endif /* USART_IRQ_H_ */