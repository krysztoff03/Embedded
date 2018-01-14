/*
 * CAN_Bus_Master_FINAL.c
 *
 * Created: 12/27/2017 7:08:54 PM
 *  Author: Cristi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "LCD/lcd.h"
#include "MCP2515/defaults.h"
#include "MCP2515/global.h"
#include "MCP2515/mcp2515.h"

#define CAN_BUS (1 << PF0)
#define BRAKE_LIGHT (1 << PB4)
//#define HAZARD_SIGNAL (1 << PB5)
#define HAZARD_SIGNAL (1 << PH6)
#define LEFT_SIGNAL (1 << PB6)
#define RIGHT_SIGNAL (1 << PB7)
#define HEAD_LIGHT (1 << PH4)
#define LED (1 << PE4)


/* Prototypes */
void spi_init();
void mcp2515_init_and_test();
void mcp2515_normal_mode();
void timer0_init();
void timer1_init();
void adc_init();

/* */
volatile int leftSignalFlag = 0;
volatile int rightSignalFlag = 0;

/* Variable which counts the number of Timer0 overflows */
volatile uint8_t totalOverflows;

/* Variable that holds the 10-bit result of ADC */
static volatile uint16_t adcResult;

ISR(TIMER0_OVF_vect)
{
	/* Keep the number of overflows */
	totalOverflows++;
}

ISR(ADC_vect)
{
	/* Variable that holds the first 8 bits of the 10 bits ADC resolution */
	uint8_t adclReg = ADCL;
	/* ADCL starts with the first 8 bits of final 10 bits, then ADCH 2 remaining bits 
	   will be left shifted 8 positions to the left */
	adcResult = (ADCH << 8) | adclReg;
	/* Start single conversion */
	ADCSRA |= (1 << ADSC);
}

/*
ISR(PCINT0_vect)
{
	if(PINB & LEFT_SIGNAL)
	{
		leftSignalFlag = 1;
	}
	else if(PINB & RIGHT_SIGNAL)
	{
		rightSignalFlag = 1;
	}
	else
	{
		leftSignalFlag = 0;
		rightSignalFlag = 0;
	}
}*/

int main(void)
{
	/* Messages sent through CAN bus (canTX_Signal = left, right and hazard signals,
	   canTX_Brake = brake signal, canTX_Headlight = headlights and servomotors signals */
	tCAN canTX_Signal;
	canTX_Signal.id = 0x1A1;
	canTX_Signal.header.rtr = 0;
	canTX_Signal.header.length = 2;
	canTX_Signal.data[0] = 0x00;
	canTX_Signal.data[1] = 0x00;
	
	tCAN canTX_Brake;
	canTX_Brake.id = 0xC9;
	canTX_Brake.header.rtr = 0;
	canTX_Brake.header.length = 2;
	canTX_Brake.data[0] = 0x00;
	canTX_Brake.data[1] = 0x00;
	
	tCAN canTX_Headlight;
	canTX_Headlight.id = 0x321;
	canTX_Headlight.header.rtr = 0;
	canTX_Headlight.header.length = 2;
	canTX_Headlight.data[0] = 0x00;
	canTX_Headlight.data[1] = 0x00;
	
	//Enable global interrupts
	sei();
	//LCD initialization with display ON, cursor OFF
	lcd_init(LCD_DISP_ON);
	//SPI initialization
	spi_init();
	/* MCP2515 initialization and test */
	mcp2515_init_and_test();
	/* Set MCP2515 to Normal Mode of operation */
	mcp2515_normal_mode();
	/* Timer0 initialization */
	timer0_init();
	/* Timer1 initialization */
	//timer1_init();
	/* ADC initialization*/
	adc_init();
	
	/* Dummy Pin used for toggling */
	DDRE |= LED;
	
	/* PB6 as input - left signal */
	DDRB &= ~LEFT_SIGNAL;
	///* Enable Pin B6 pull-up resistor */
	//PORTB |= (1 << PB6);
	/////* Enable Pin Change Interrupt */
	//PCICR = (1 << PCIE0);
	///* Enable PCINT6 (Pin B6) */
	//PCMSK0 = (1 << PCINT6);
	
	/* PB7 as input - right signal */
	DDRB &= ~RIGHT_SIGNAL;
	///* Enable Pin B7 pull-up resistor */
	//PORTB |= (1 << PB7);
	/////* Enable Pin Change Interrupt */
	//PCICR = (1 << PCIE0);
	///* Enable PCINT7 (Pin B7) */
	//PCMSK0 = (1 << PCINT7);
	
	/* PB5 as input - hazard signal */
	DDRH &= ~HAZARD_SIGNAL;
	
	/* PB4 as input - brake signal */
	DDRB &= ~BRAKE_LIGHT;
	
	/* PE4 as output - led brake light */
	//DDRE |= (1 << PE4);
	
	/* PH4 as input - headlights and servomotors signal */
	DDRH &= ~HEAD_LIGHT;
	
	DDRB |= (1 << PB5);
	
	/* Servomotors 180 degrees direction */
	float ocrMin = 250;
	/* Servomotors 0 degrees direction */
	float ocrMax = 650;
	float ocrResult = (ocrMax - ocrMin) * 1023;
	
	uint16_t ocr1Value;
	
	/* Clear the LCD before entering the infinite loop */
	lcd_clrscr();
    while(1)
    {
		if(PINH & HEAD_LIGHT)
		{
			OCR1A = (int)((ocrResult * adcResult) + 250);
			PORTE |= LED;
			lcd_home();
			lcd_puts("Headlight On!");
			
			canTX_Headlight.data[0] = 0xFF;
			canTX_Headlight.data[1] = adcResult / 4;
			mcp2515_send_message(&canTX_Headlight);
			ocr1Value = OCR1A;
			if(ocr1Value <= 450)
			{
				//lcd_home();
				lcd_puts("   Turn Left!   ");
			}
			else
			{
				//lcd_home();
				lcd_puts("   Turn Right!  ");
			}
		}
		
		/* Check if the left signal switch is on */
		else if(PINB & LEFT_SIGNAL)
		{
			/* Check if the number of overflows is equal with 36 */
			if(totalOverflows >= 36)
			{
				/* Check if the timer0 counts 195 ticks */
				if(TCNT0 >= 195)
				{
					PORTE ^= LED;
					/*  */
					if(!(PINE & LED))
					{
						canTX_Signal.data[0] = 0x04;
						canTX_Signal.data[1] = 0x01;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Left Signal On! ");
					}
					else
					{
						canTX_Signal.data[0] = 0x04;
						canTX_Signal.data[1] = 0x00;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Left Signal Off!");
					}
					/* Reset counter */
					TCNT0 = 0;
					/* Reset overflows counter */
					totalOverflows = 0;
				}
			}
		}
		
		else if(PINB & RIGHT_SIGNAL)
		{
			/* Check if the number of overflows is equal with 36 */
			if(totalOverflows >= 36)
			{
				/* Check if the timer0 counts 195 ticks */
				if(TCNT0 >= 195)
				{
					PORTE ^= LED;
					/*  */
					if(!(PINE & LED))
					{
						canTX_Signal.data[0] = 0x02;
						canTX_Signal.data[1] = 0x01;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Right Signal On!");
					}
					else
					{
						canTX_Signal.data[0] = 0x02;
						canTX_Signal.data[1] = 0x00;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Right Signal Off");
					}
					/* Reset counter */
					TCNT0 = 0;
					/* Reset overflows counter */
					totalOverflows = 0;
				}
			}
		}
		
		else if(PINH & HAZARD_SIGNAL)
		{
			/* Check if the number of overflows is equal with 36 */
			if(totalOverflows >= 36)
			{
				/* Check if the timer0 counts 195 ticks */
				if(TCNT0 >= 195)
				{
					PORTE ^= LED;
					/*  */
					if(!(PINE & LED))
					{
						canTX_Signal.data[0] = 0x06;
						canTX_Signal.data[1] = 0x01;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Hazard Signal On!");
					}
					else
					{
						canTX_Signal.data[0] = 0x06;
						canTX_Signal.data[1] = 0x00;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("HazardSignal Off");
					}
					/* Reset counter */
					TCNT0 = 0;
					/* Reset overflows counter */
					totalOverflows = 0;
				}
			}
		}
		
		else if(PINB & BRAKE_LIGHT)
		{
			PORTE |= LED;
			canTX_Brake.data[0] = 0xFF;
			canTX_Brake.data[1] = 0x00;
			mcp2515_send_message(&canTX_Brake);
			lcd_home();
			lcd_puts(" Brake Pressed! ");
			//_delay_ms(500);
			//lcd_clrscr();
		}
		
		else if(!(PINB & BRAKE_LIGHT))
		{
			mcp2515_send_message(&canTX_Brake);
			PORTE &= ~LED;
			canTX_Brake.data[0] = 0x00;
			canTX_Brake.data[1] = 0x00;
		}
		
		
		
		/*
		if(PINB & LEFT_SIGNAL)
		{
			if(TCNT1 >= 37649)
			{
				PORTE ^= LED;
				if(!(PINE & LED))
				{
					canTX_Signal.data[0] = 0x04;
					canTX_Signal.data[1] = 0x01;
					mcp2515_send_message(&canTX_Signal);
					lcd_home();
					lcd_puts("Left Signal On! ");
				}
				else
				{
					canTX_Signal.data[0] = 0x04;
					canTX_Signal.data[1] = 0x00;
					mcp2515_send_message(&canTX_Signal);
					lcd_home();
					lcd_puts("Left Signal Off!");
				}
				TCNT1 = 0;
			}
		}
		
		else if(PINB & RIGHT_SIGNAL)
		{
			if(TCNT1 >= 37649)
			{
				PORTE ^= LED;
				if(!(PINE & LED))
				{
					canTX_Signal.data[0] = 0x02;
					canTX_Signal.data[1] = 0x01;
					mcp2515_send_message(&canTX_Signal);
					lcd_home();
					lcd_puts("Right Signal On!");
				}
				else
				{
					canTX_Signal.data[0] = 0x02;
					canTX_Signal.data[1] = 0x00;
					mcp2515_send_message(&canTX_Signal);
					lcd_home();
					lcd_puts("Right Signal Off");
				}
				TCNT1 = 0;
			}
		}
		
		else if(PINH & HAZARD_SIGNAL)
		{
			if(TCNT1 >= 37649)
			{
				PORTE ^= LED;
				if(!(PINE & LED))
				{
					canTX_Signal.data[0] = 0x06;
					canTX_Signal.data[1] = 0x01;
					mcp2515_send_message(&canTX_Signal);
					lcd_home();
					lcd_puts("Hazard Signal On");
				}
				else
				{
					canTX_Signal.data[0] = 0x06;
					canTX_Signal.data[1] = 0x00;
					mcp2515_send_message(&canTX_Signal);
					lcd_home();
					lcd_puts("HazardSignal Off");
				}
				TCNT1 = 0;
			}
		}
		
		else if(PINB & BRAKE_LIGHT)
		{
			PORTE |= LED;
			canTX_Brake.data[0] = 0xFF;
			canTX_Brake.data[1] = 0x00;
			mcp2515_send_message(&canTX_Brake);
			lcd_home();
			lcd_puts(" Brake Pressed! ");
			//_delay_ms(500);
			//lcd_clrscr();
		}
		
		else if(!(PINB & BRAKE_LIGHT))
		{
			mcp2515_send_message(&canTX_Brake);
			PORTE &= ~LED;
			canTX_Brake.data[0] = 0x00;
			canTX_Brake.data[1] = 0x00;
		}
		*/
		
		/*
		if(leftSignalFlag == 1)
		{
			if(TCNT1 >= 37649)
			{
				
			}
		}
		*/
		
		
		/*
		if(leftSignalFlag == 1)
		{
			leftSignalMessage.data[1] = 0xFF;
			mcp2515_send_message(&leftSignalMessage);
			lcd_home();
			lcd_puts("  Left Signal!  ");
		}
		*/
		
		/*
		if(PINB & BRAKE_LIGHT)
		{
			//LCD clear screen
			lcd_clrscr();
			lcd_gotoxy(0, 0);
			lcd_puts("Brake light!");
		}
		*/
		
		
		
		/*
		if(PINF & (1 << PF1))
		{
			//LCD clear screen
			lcd_clrscr();
			lcd_gotoxy(0, 0);
			lcd_puts("Brake light!");
		}
		_delay_ms(1000);
		lcd_clrscr();
		*/
		
		/*
		//When the switch connected to PF0 is active, turn on the CAN bus
        if(PINF & CAN_BUS)
		{
			//LCD clear screen
			lcd_clrscr();
			lcd_home();
			//Set cursor on line 1
			lcd_gotoxy(2, 0);
			lcd_puts("Ignition on!");
			_delay_ms(1000);
			//MCP2515 initialization and test
			//jump:
			mcp2515_init_and_test();
			//goto jump;
			//MCP2515 normal mode functioning
			mcp2515_normal_mode();
			//LCD clear screen
			lcd_clrscr();
			if(PINB & BRAKE_LIGHT)
			{
				PORTE ^= (1 << PE4);
				//LCD clear screen
				lcd_clrscr();
				lcd_gotoxy(0, 0);
				lcd_puts("Brake light!");
			}
			
		}
		else
		{
			//LCD clear screen
			//lcd_clrscr();
			//Set cursor on line 1
			lcd_gotoxy(0, 0);
			lcd_puts(" Ignition off!  ");
		}
		*/
    }
}


void spi_init()
{
	//Set MOSI, SCK and SS as output
	DDRB |= (1 << PB2) | (1 << PB1) | (1 << PB0);
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

void timer0_init()
{
	/* Set up Timer0 with Prescaler = 1024 */
	TCCR0B |= (1 << CS02) | (1 << CS00);
	/* Initialize counter */
	TCNT0 = 0;
	/* Enable overflow interrupt */
	TIMSK0 |= (1 << TOIE0);
	/* Initialize overflows counter */
	totalOverflows = 0;
}

//void timer1_init()
//{
//	///* Set up timer with Prescaler = 256 */
//	//TCCR1B |= (1 << CS12);
//	///* Initialize counter */
//	//TCNT1 = 0;
//}

void adc_init()
{
	/* Clear OC1A on Compare Match */
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	/* Fast PWM mode 14 with prescaler of 64 */
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
	/* TOP value calculated with Fpwm = Fcpu / (N * (1 + TOP)), N - Prescaler = 64; */
	ICR1 = 4999;
	///* Set Pin B1 (9) as output (OC1A) */
	//DDRB |= (1 << PINB1); 
	OCR1A = 250;
	
	/* AREF = AVcc, Potentiometer connected on channel 0 (Pin A0) */
	ADMUX = (1 << REFS0);
	
	/* Enable ADC with a prescaler = 128 (16000000 / 128 = 125000 KHz) */
	ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	/* Start single conversion */
	ADCSRA |= (1 << ADSC);
}