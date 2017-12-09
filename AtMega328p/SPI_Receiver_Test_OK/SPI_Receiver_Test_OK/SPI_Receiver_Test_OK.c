/*
 * SPI_Receiver_Test_OK.c
 * The test was performed on ATMega 328p (2560)
 * Created: 12/2/2017 2:31:17 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lcd.h"


void SPI_init()
{
	//set MISO as output
	DDRB |= (1 << PB3);
	//enable SPI
	SPCR = (1 << SPE);
}

char SPI_transmit(char data)
{
	//write data into the SPI data register
	SPDR = data;
	//wait till transmission is complete
	while(!(SPSR & (1 << SPIF)));
	//return received data
	return (SPDR);
}

char SPI_receive()
{
	//wait till reception is complete
	while(!(SPSR & (1 << SPIF)));
	//return received data
	return SPDR;
}

int main(void)
{
	//SPI initialization
	SPI_init();
	////counter sent through SPI
	uint8_t count;
	//array of chars used to convert the counter with itoa function
	char buffer[5];
	//LCD initialization
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("Slave Device");
	lcd_gotoxy(0, 1);
	lcd_puts("ReceivedData:");
	
	while(1)
	{
		lcd_gotoxy(13, 1);
		count = SPI_receive();
		itoa(count, buffer, 10);
		lcd_puts(buffer);
	}
}