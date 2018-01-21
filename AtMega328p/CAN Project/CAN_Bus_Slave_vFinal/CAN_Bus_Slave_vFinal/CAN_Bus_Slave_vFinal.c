/*
 * CAN_Bus_Slave_vFinal.c
 *
 * Created: 1/11/2018 9:10:52 PM
 *  Author: Cristi
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcdpcf8574/lcdpcf8574.h"
#include "MCP2515/defaults.h"
#include "MCP2515/global.h"
#include "MCP2515/mcp2515.h"

#define HEAD_LIGHT (1 << PORTB0)
#define LEFT_SIGNAL (1 << PORTD4)
#define RIGHT_SIGNAL (1 << PORTD5)
#define BRAKE_LIGHT (1 << PORTD3)
#define LED (1 << PORTB0)

/* Prototypes */
void spi_init();
void mcp2515_init_and_test();
void mcp2515_normal_mode();
void timer1_init();
void print_message(tCAN *message, int rowLCDValue);

/* Variable of type pointer to char */
typedef char *string;

int main(void)
{
	/* Message received through CAN Bus */
	tCAN canRX;
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
	/* MCP2515 initialization and test */
	mcp2515_init_and_test();
	/* Set MCP2515 to Normal Mode of operation */
	mcp2515_normal_mode();
	/* Timer1 initialization */
	timer1_init();
	
	/* PORTB0 as output - headlights */
	DDRB |= HEAD_LIGHT;
	/* PORTD4 as output - left signal */
	DDRD |= LEFT_SIGNAL;
	/* PORTD5 as output - right signal */
	DDRD |= RIGHT_SIGNAL;
	/* PORTD3 as output - brake light */
	DDRD |= BRAKE_LIGHT;
	
	
	/* Servomotors 180 degrees direction */
	float ocrMin = 250;
	/* Servomotors 0 degrees direction */
	//float ocrMax = 650;
	float ocrMax = 2000;
	float ocrResult = (ocrMax - ocrMin) / 1023;
	
	uint8_t canRX_ADC;
	
	/* HEADLIGHT LED */
	DDRB |= LED;
	/* OC1A as output */
	DDRB |= (1 << PORTB1);
	
	char adcLCD[4];
	
	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("ID");
	lcd_gotoxy(5, 0);
	lcd_puts("DATA");
    while(1)
    {
        if(mcp2515_check_message())
		{
			if(mcp2515_get_message(&canRX))
			{
				
				if((canRX.id == 0x258) && (canRX.data[0] = 0xFF))
				{
					PORTB |= LED;
					canRX_ADC = canRX.data[1];
					OCR1A = (int)((ocrResult * canRX_ADC) + 250);
					itoa(canRX_ADC, adcLCD, 10);
					print_message(&canRX, 1);
				}
				
				//else if(canRX.id == 0x321) && (canRX.data[0] = 0x00) && ()
				
				/*
				else if((canRX.id == 0x1A1) && (canRX.data[0] == 0x04) && (canRX.data[1] == 0x01))
				{
					print_message(&canRX, 3);
					PORTD |= LEFT_SIGNAL;
				}
				else if((canRX.id == 0x1A1) && (canRX.data[0] == 0x04) && (canRX.data[1] == 0x00))
				{
					print_message(&canRX, 3);
					PORTD &= ~LEFT_SIGNAL;
				}
				*/
				
				if((canRX.id == 0x1A1) && (canRX.data[0] == 0x04) && (canRX.data[1] == 0x01))
				{
					print_message(&canRX, 3);
					PORTD |= LEFT_SIGNAL;
				}
				else if((canRX.id == 0x1A1) && (canRX.data[0] == 0x04) && (canRX.data[1] == 0x00))
				{
					print_message(&canRX, 3);
					PORTD &= ~LEFT_SIGNAL;
				}
				else if((canRX.id == 0x1A1) && (canRX.data[0] == 0x02) && (canRX.data[1] == 0x01))
				{
					print_message(&canRX, 3);
					PORTD |= RIGHT_SIGNAL;
				}
				else if((canRX.id == 0x1A1) && (canRX.data[0] == 0x02) && (canRX.data[1] == 0x00))
				{
					print_message(&canRX, 3);
					PORTD &= ~RIGHT_SIGNAL;
				}
				else if((canRX.id == 0x1A1) && (canRX.data[0] == 0x06) && (canRX.data[1] == 0x01))
				{
					print_message(&canRX, 3);
					PORTD |= LEFT_SIGNAL | RIGHT_SIGNAL;
				}
				else if((canRX.id == 0x1A1) && (canRX.data[0] == 0x06) && (canRX.data[1] == 0x00))
				{
					print_message(&canRX, 3);
					PORTD &= ~LEFT_SIGNAL & ~RIGHT_SIGNAL;
				}
				else if((canRX.id == 0xC9) && (canRX.data[0] == 0xFF))
				{
					print_message(&canRX, 2);
					PORTD |= BRAKE_LIGHT;
				}
				else if((canRX.id == 0xC9) && (canRX.data[0] == 0x00))
				{
					print_message(&canRX, 2);
					PORTD &= ~BRAKE_LIGHT;
				}
				
				
				
				
				
			}
		}
    }
}

void spi_init()
{
	/* Set MISO as output */
	DDRB |= (1 << PORTB4);
	/* Enable SPI */
	SPCR |= (1 << SPE);
}

void mcp2515_init_and_test()
{
	if(!mcp2515_init())
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_gotoxy(0, 0);
		lcd_puts("Error: CAN Bus");
		/* Set cursor on line 2 */
		lcd_gotoxy(0, 1);
		lcd_puts("not initialized!");
	}
	else
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_gotoxy(7, 0);
		lcd_puts("CAN Bus");
		/* Set cursor on line 2 */
		lcd_gotoxy(4, 1);
		lcd_puts("initialized!");
	}
	
	_delay_ms(2000);
	
	/* Test message used only to test the MCP2515 module */
	tCAN testTXRX;
	testTXRX.id = 0x123;
	testTXRX.header.rtr = 0;
	testTXRX.header.length = 2;
	testTXRX.data[0] = 0xFF;
	testTXRX.data[1] = 0xFF;
	
	
	/* Switch to Loopback Mode to test the MCP2515 module */
	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), (1 << REQOP1));
	
	/* Send test message */
	if(mcp2515_send_message(&testTXRX))
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_gotoxy(2, 0);
		lcd_puts("Message writenn");
		/* Set cursor on line 2 */
		lcd_gotoxy(5, 1);
		lcd_puts("to buffer!");
	}
	else
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_gotoxy(5, 0);
		lcd_puts("Message not");
		/* Set cursor on line 2 */
		lcd_gotoxy(2, 1);
		lcd_puts("written to buffer");
	}
	
	_delay_ms(1000);
	
	if(mcp2515_check_message())
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_gotoxy(2, 0);
		lcd_puts("Message received");
		_delay_ms(1000);
	}
	else
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_gotoxy(4, 0);
		lcd_puts("Message not");
		/* Set cursor on line 2 */
		lcd_gotoxy(6, 0);
		lcd_puts("received!");
	}
}

void mcp2515_normal_mode()
{
	/* Switch the MCP2515 module to normal mode */
	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
	lcd_clrscr();
	lcd_home();
	lcd_puts("CAN Bus Normal Mode ");
	_delay_ms(1000);
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

void timer1_init()
{
	/* Clear OC1A on Compare Match */
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	/* Fast PWM mode 14 with prescaler of 64 */
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
	/* TOP value calculated with Fpwm = Fcpu / (N * (1 + TOP)), N - Prescaler = 64; 4999 for 20ms */
	ICR1 = 4999;
	
	OCR1A = 250;
}