/*
 * CAN_Bus_Receiver_Loopback_Mode_Test_OK.c
 *
 * Created: 12/25/2017 11:12:27 AM
 *  Author: Cristi
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcdpcf8574/lcdpcf8574.h"
#include "MCP2515/defaults.h"
#include "MCP2515/global.h"
#include "MCP2515/mcp2515.h"

//Variable of type pointer to char
typedef char *string;

void spi_init()
{
	//Set MOSI, SCK and SS as output
	DDRB |= (1 << PORTB3) | (1 << PORTB5) | (1 << PORTB2);
}

/*
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
}*/

void print_message(tCAN *message)
{
	uint8_t lcdCellCounter;
	//Counter for the number of bytes in a message
	uint8_t counter;
	//Variable that holds the ID value received and converted with itoa function
	char varID[3];
	//Array of strings that holds the message bytes value received and converted with itoa function
	string varData[8];
	//LCD clear screen
	lcd_clrscr();
	lcd_home();
	lcd_puts("ID ");
	itoa(message->id, varID, 10);
	lcd_gotoxy(3, 0);
	lcd_puts(varID);
	for(lcdCellCounter = 0;lcdCellCounter < 1;lcdCellCounter++)
	{
		lcd_gotoxy(lcdCellCounter, 1);
		for(counter = 0;counter < message->header.length;counter++)
		{
			_delay_ms(1000);
			itoa(message->data[counter], varData[counter], 10);
			lcd_puts(varData[counter]);
		}
	}
}

int main(void)
{
	//Enable interrupts
	sei();
	//LCD initialization with display ON, cursor OFF, blink char
	lcd_init(LCD_DISP_ON);
	//Variable which turn on/off the LCD backlight (0 = ON, 1 = OFF)
	uint8_t led = 0;
	//Turn on the backlight
	lcd_led(led);
	//SPI initialization
	spi_init();
	//MCP2515 initialization
	if(!mcp2515_init())
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(3, 0);
		lcd_puts("Error CAN Bus");
		lcd_gotoxy(3, 1);
		lcd_puts("not initialized");
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(7, 0);
		lcd_puts("CAN Bus");
		_delay_ms(500);
		//Set cursor on line 2
		lcd_gotoxy(5, 1);
		lcd_puts("initialized");
	}
	
	_delay_ms(2000);
	
	//Test message
	tCAN testMessage;
	testMessage.id = 123;
	testMessage.header.length = 2;
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
		lcd_puts("Message writenn");
		lcd_gotoxy(0, 1);
		lcd_puts("to buffer");
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		lcd_home();
		lcd_puts("Message not written");
		lcd_gotoxy(0, 1);
		lcd_puts("to buffer");
	}
	
	_delay_ms(10);
	
	_delay_ms(1000);
	
	if(mcp2515_check_message())
	{
		//LCD clear screen
		lcd_clrscr();
		lcd_home();
		lcd_puts("Message received");
		_delay_ms(1000);
		if(mcp2515_get_message(&testMessage))
		{
			//LCD clear screen
			lcd_clrscr();
			//Display message on the LCD
			print_message(&testMessage);
		}
		else
		{
			//LCD clear screen
			lcd_clrscr();
			//"Message not read"
			lcd_puts("Message not read");
		}
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		//"Message not received"
		lcd_puts("Message not received");
	}
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}