/*
 * SPI_Sender_Test_OK.c
 * The test was performed on ATMega 328p (Uno)
 * Created: 12/2/2017 1:17:52 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcdpcf8574/lcdpcf8574.h"

void SPI_init()
{
	//set MOSI, SCK and SS as output
	DDRB |= (1 << PORTB3) | (1 << PORTB5) | (1 << PORTB2);
	//enable SPI, set as Master, Prescaler = F_CPU / 16
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
	
}

void SPI_write(char data)
{
	//write data into the SPI data register
	SPDR = data;
	//wait till transmission is complete
	while(!(SPSR & (1 << SPIF)));
}

char SPI_read()
{
	SPDR = 0xFF;
	//wait till reception is complete
	while(!(SPSR & (1 << SPIF)));
	//return received data
	return (SPDR);
}

int main(void)
{
	//variable used to illuminate LCD (0 = on, 1 = off)
	uint8_t led = 0;
	//counter sent through SPI
	char count = 0x00;
	//SPI initialization
	SPI_init();
	//LCD initialization
	lcd_init(LCD_DISP_ON);
	//set the LCD to be illuminated
	lcd_led(led);
	//array of chars used to convert the counter with itoa function
	char buffer[10];
	
	while(1)
	{	
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Master Device");
		lcd_gotoxy(0, 1);
		lcd_puts("Data Sent:");
		SPI_write(count);
		lcd_gotoxy(11, 1);
		itoa(count, buffer, 10);
		lcd_puts(buffer);
		count++;
		_delay_ms(1000);	
	}
}