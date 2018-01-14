/*
 * CAN_Bus_Slave_FINAL.c
 *
 * Created: 12/29/2017 3:38:32 PM
 *  Author: Cristi
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "lcdpcf8574/lcdpcf8574.h"
#include "MCP2515/defaults.h"
#include "MCP2515/global.h"
#include "MCP2515/mcp2515.h"

#define BRAKE_LIGHT (1 << PORTD3)
#define LEFT_SIGNAL (1 << PORTD4)
#define RIGHT_SIGNAL (1 << PORTD5)
#define HEAD_LIGHT (1 << PORTD6)

/* Prototypes */
void spi_init();
void mcp2515_init_and_test();
void mcp2515_normal_mode();
void timer1_init();
void print_message(tCAN *message, int rowLCDValue);

//Variable of type pointer to char
typedef char *string;

int main(void)
{
	tCAN receivedMessage;
	/* Enable global interrupts */
	sei();
	/* LCD initialization with display ON, cursor OFF */
	lcd_init(LCD_DISP_ON);
	/* Variable which turn on/off the LCD backlight (0 = ON, 1 = OFF) */
	uint8_t led = 0;
	/* Turn on the backlight */
	lcd_led(led);
	/* SPI initialization */
	spi_init();
	
	mcp2515_init_and_test();
	
	mcp2515_normal_mode();
	
	timer1_init();
	
	/* Brake Light */
	DDRD |= BRAKE_LIGHT;
	/* Left Signal */
	DDRD |= LEFT_SIGNAL;
	/* Right Signal */
	DDRD |= RIGHT_SIGNAL;
	/* Head Light */
	DDRD |= HEAD_LIGHT;
	
	//tCAN receivedSignalMessage;
	//receivedSignalMessage.id = 0x1A1;
	
	/* Servomotors 180 degrees direction */
	float ocrMin = 250;
	/* Servomotors 0 degrees direction */
	float ocrMax = 650;
	float ocrResult = (ocrMax - ocrMin) * 1023;
	
	uint16_t ocr1Value;
	
	lcd_clrscr();
	
	lcd_gotoxy(0, 0);
	lcd_puts("ID ");
	lcd_gotoxy(5, 0);
	lcd_puts("Data");
	
    while(1)
    {
        //TODO:: Please write your application code 
		
		if(mcp2515_check_message())
		{
			if(mcp2515_get_message(&receivedMessage))
			{
				if((receivedMessage.id == 0x1A1) && (receivedMessage.data[0] == 0x04) && (receivedMessage.data[1] == 0x01))
				{
					print_message(&receivedMessage, 3);
					PORTD |= LEFT_SIGNAL;
				}
				else if((receivedMessage.id == 0x1A1) && (receivedMessage.data[0] == 0x04) && (receivedMessage.data[1] == 0x00))
				{
					print_message(&receivedMessage, 3);
					PORTD &= ~LEFT_SIGNAL;
				}
				else if((receivedMessage.id == 0x1A1) && (receivedMessage.data[0] == 0x02) && (receivedMessage.data[1] == 0x01))
				{
					print_message(&receivedMessage, 3);
					PORTD |= RIGHT_SIGNAL;
				}
				else if((receivedMessage.id == 0x1A1) && (receivedMessage.data[0] == 0x02) && (receivedMessage.data[1] == 0x00))
				{
					print_message(&receivedMessage, 3);
					PORTD &= ~RIGHT_SIGNAL;
				}
				else if((receivedMessage.id == 0x1A1) && (receivedMessage.data[0] == 0x06) && (receivedMessage.data[1] == 0x01))
				{
					print_message(&receivedMessage, 3);
					PORTD |= LEFT_SIGNAL | RIGHT_SIGNAL;
				}
				else if((receivedMessage.id == 0x1A1) && (receivedMessage.data[0] == 0x06) && (receivedMessage.data[1] == 0x00))
				{
					print_message(&receivedMessage, 3);
					PORTD &= ~LEFT_SIGNAL & ~RIGHT_SIGNAL;
				}
				else if((receivedMessage.id == 0xC9) && (receivedMessage.data[0] == 0xFF))
				{
					print_message(&receivedMessage, 2);
					PORTD |= BRAKE_LIGHT;
				}
				else if((receivedMessage.id == 0xC9) && (receivedMessage.data[0] == 0x00))
				{
					print_message(&receivedMessage, 2);
					PORTD &= ~BRAKE_LIGHT;
				}
				else if((receivedMessage.id == 0x321) && (receivedMessage.data[0] = 0xFF))
				{
					OCR1A = (int)((ocrResult * receivedMessage.data[1]) + 250);
					print_message(&receivedMessage, 1);
					PORTD |= HEAD_LIGHT;
				}
				/*
				else
				{
					PORTD &= ~LEFT_SIGNAL;
					PORTD &= ~RIGHT_SIGNAL;
				}*/
			}
		}
		
		//if(mcp2515_check_message())
		//{
		//	if(mcp2515_get_message(&receivedMessage))
		//	{
		//		if(receivedMessage.id == leftSignalMessage.id)
		//		{
		//			if(TCNT1 >= 37649)
		//			{
		//				/* Toggle the left signal LED */
		//				PORTD ^= (1 << PORTD4);
		//				/* Reset counter */
		//				TCNT1 = 0;
		//			}	
		//		}
		//	}
		//}
    }
}

void timer1_init()
{
//	/* Set up timer with Prescaler = 256 */
//	TCCR1B |= (1 << CS12);
//	/* Initialize counter */
//	TCNT1 = 0;
	/* Clear OC1A on Compare Match */
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	/* Fast PWM Mode with prescaler of 64 */
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
	/* Top value calculated with Fpwm = Fcpu / (N * (1 + Top)), N - Prescaler = 64
	   ICR1 = 4999 for 50Hz PWM */
	ICR1 = 4999;
}

void spi_init()
{
	//Set MISO as output
	DDRB |= (1 << PORTB4);
	//Enable SPI
	SPCR = (1 << SPE);
}

void mcp2515_init_and_test()
{
	if(!mcp2515_init())
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(0, 0);
		lcd_puts("Error: CAN Bus");
		//Set cursor on line 2
		lcd_gotoxy(0, 1);
		lcd_puts("not initialized!");
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		lcd_home();
		//Set cursor on line 1
		lcd_gotoxy(5, 0);
		lcd_puts("CAN Bus");
		//Set cursor on line 2
		lcd_gotoxy(2, 1);
		lcd_puts("initialized!");
	}
	
	_delay_ms(2000);
	
	//Test message used only to test the MCP2515 module
	tCAN testMessage;
	testMessage.id = 123;
	testMessage.header.rtr = 0;
	testMessage.header.length = 2;
	testMessage.data[0] = 0xFF;
	testMessage.data[1] = 0xFF;
	
	//Switch to Loopback Mode to test the MCP2515 module
	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), (1 << REQOP1));
	
	//Send test message
	if(mcp2515_send_message(&testMessage))
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(0, 0);
		lcd_puts("Message writenn");
		//Set cursor on line 2
		lcd_gotoxy(4, 1);
		lcd_puts("to buffer!");
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(0, 0);
		lcd_puts("Message not");
		//Set cursor on line 2
		lcd_gotoxy(0, 1);
		lcd_puts("writenn to buffer");
	}
	_delay_ms(1000);
	if(mcp2515_check_message())
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(0, 0);
		lcd_puts("Message received");
		_delay_ms(1000);
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(0, 0);
		lcd_puts("CAN Bus tested");
		//Set cursor on line 2
		lcd_gotoxy(0, 1);
		lcd_puts("successfully!");
	}
	else
	{
		//LCD clear screen
		lcd_clrscr();
		//Set cursor on line 1
		lcd_gotoxy(0, 0);
		lcd_puts("Message not");
		//Set cursor on line 2
		lcd_gotoxy(4, 0);
		lcd_puts("received!");
	}
}

void mcp2515_normal_mode()
{
	//Switch to Normal Mode
	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
}

void print_message(tCAN *message, int rowLCDValue)
{
	uint8_t lcdCellCounter;
	//Counter for the number of bytes in a message
	uint8_t counter;
	//Variable that holds the ID value received and converted with itoa function
	char varID[3];
	//Array of strings that holds the message bytes value received and converted with itoa function
	string varData[8];
	//LCD clear screen
	//lcd_clrscr();
	//lcd_home();
	//lcd_puts("ID ");
	itoa(message->id, varID, 16);
	lcd_gotoxy(0, rowLCDValue);
	lcd_puts(varID);
	for(lcdCellCounter = 5;lcdCellCounter < 6;lcdCellCounter++)
	{
		lcd_gotoxy(lcdCellCounter, rowLCDValue);
		for(counter = 0;counter < message->header.length;counter++)
		{
			//_delay_ms(1000);
			itoa(message->data[counter], varData[counter], 16);
			lcd_puts(varData[counter]);
			lcd_puts(" ");
		}
	}
}

/*
void print_message(tCAN *message, int rowLCDValue)
{
	uint8_t lcdCellCounter;
	//Counter for the number of bytes in a message
	uint8_t counter;
	//Variable that holds the ID value received and converted with itoa function
	char varID[3];
	//Array of strings that holds the message bytes value received and converted with itoa function
	string varData[8];
	//LCD clear screen
	//lcd_clrscr();
	//lcd_home();
	//lcd_puts("ID ");
	itoa(message->id, varID, 16);
	lcd_gotoxy(3, rowLCDValue);
	lcd_puts(varID);
	for(lcdCellCounter = 7;lcdCellCounter < 8;lcdCellCounter++)
	{
		lcd_gotoxy(lcdCellCounter, rowLCDValue);
		for(counter = 0;counter < message->header.length;counter++)
		{
			//_delay_ms(1000);
			itoa(message->data[counter], varData[counter], 16);
			lcd_puts(varData[counter]);
			lcd_puts(" ");
		}
	}
}
*/