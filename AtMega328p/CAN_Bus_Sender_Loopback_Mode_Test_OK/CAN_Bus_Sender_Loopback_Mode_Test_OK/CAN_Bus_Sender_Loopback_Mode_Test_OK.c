/*
 * CAN_Bus_Sender_Loopback_Mode_Test_OK.c
 * The tests were made with ATMega328p (2560) and MCP2515
 * Created: 12/25/2017 10:53:31 AM
 *  Author: Cristi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "LCD/lcd.h"
#include "MCP2515/defaults.h"
#include "MCP2515/global.h"
#include "MCP2515/mcp2515.h"

void spi_init()
{
	//Set MOSI, SCK and SS as output
	DDRB |= (1 << PB2) | (1 << PB1) | (1 << PB0);
}

void print_message(tCAN *message)
{
	//LCD clear screen
	lcd_clrscr();
	lcd_home();
	lcd_puts("ID ");
	//Variables that holds the ID and data values received and converted with itoa function
	char varID[3], varData0[2], varData1[2];
	itoa(message->id, varID, 10);
	lcd_gotoxy(3, 0);
	lcd_puts(varID);
	_delay_ms(1000);
	lcd_gotoxy(0, 1);
	itoa(message->data[0], varData0, 10);
	lcd_puts(varData0);
	_delay_ms(1000);
	lcd_gotoxy(4, 1);
	itoa(message->data[1], varData1, 10);
	lcd_puts(varData1);
	_delay_ms(1000);
}

int main(void)
{
	//Enable interrupts
	sei();
	//LCD initialization with display ON, cursor OFF
	lcd_init(LCD_DISP_ON);
	//SPI initialization
	spi_init();
	//MCP2515 initialization
	if(!mcp2515_init())
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(0, 0);
		lcd_puts("Error CAN Bus");
		//Set cursor on line 2
		lcd_gotoxy(0, 1);
		lcd_puts("not initialized");
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		lcd_home();
		//Set cursor on line 1
		lcd_gotoxy(4, 0);
		lcd_puts("CAN Bus");
		//Set cursor on line 2
		lcd_gotoxy(2, 1);
		lcd_puts("initialized");
	}
	_delay_ms(2000);
	
	//Test message
	tCAN testMessage;
	testMessage.id = 123;
	testMessage.header.rtr = 0;
	testMessage.data[0] = 0xAB;
	testMessage.data[1] = 0xCD;
	
	//Switch to Loopback Mode
	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), (1 << REQOP1));
	
	//Send message
	if(mcp2515_send_message(&testMessage))
	{
		//LCD clear screen
		lcd_clrscr();
		lcd_home();
		//"Message writenn to buffer"
		lcd_puts("mess writ buff");
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		lcd_home();
		//"Message not written"
		lcd_puts("mess not writ");
	}
	_delay_ms(10);
	
	_delay_ms(1000);
	
	if(mcp2515_check_message())
	{
		//LCD clear screen
		lcd_clrscr();
		lcd_home();
		//"Message received"
		lcd_puts("mess received");
		_delay_ms(1000);
		if(mcp2515_get_message(&testMessage))
		{
			//Display message on the LCD
			print_message(&testMessage);
		}
		else
		{
			//"Message not read"
			lcd_puts("mess not read");
		}
	}
	else
	{
		//"Message not received"
		lcd_puts("messnotreceived");
	}
    while(1)
    {
        //TODO:: Please write your application code 
    }
}
