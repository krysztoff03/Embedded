/*
 * CAN_Bus_Master_vFinal.c
 *
 * Created: 1/11/2018 5:28:14 PM
 *  Author: Cristi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "LCD/lcd.h"
#include "MCP2515/defaults.h"
#include "MCP2515/global.h"
#include "MCP2515/mcp2515.h"
#include "USART/USART_IRQ.h"

#define HEAD_LIGHT (1 << PH4)
#define LEFT_SIGNAL (1 << PB6)
#define RIGHT_SIGNAL (1 << PB7)
#define HAZARD_SIGNAL (1 << PH6)
#define BRAKE_LIGHT (1 << PB4)
#define LED (1 << PE4)
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD-1

/* Prototypes */
void spi_init();
void mcp2515_init_and_test();
void mcp2515_normal_mode();
void timer0_init();
void timer1_init();
void adc_init();
//void print_message_on_USART(tCAN *message);
static inline void debounce();

/* Variable that holds the 10-bit result of ADC */
static volatile uint16_t adcResult;

/* Variable which counts the number of Timer0 overflows */
volatile uint8_t totalOverflows;

/* Variable that holds the button's value when is pressed and debounced */
volatile uint8_t button_down;

ISR(ADC_vect)
{
	/* Variable that holds the first 8 bits of the 10 bits ADC resolution */\
	uint8_t adclReg = ADCL;
	/* ADCL starts with the first 8 bits of final 10 bits, then ADCH 2 remaining bits
	will be left shifted 8 positions to the left */
	adcResult = (ADCH << 8) | adclReg;
	/* Start single conversion */
	ADCSRA |= (1 << ADSC);
}

/************************************************************************/
/* TIMER0 Calculation for 600ms delay
   1 tick for 16MHz = 62.5ns
   Timer Count = (Maximum Delay / Clock Time Period) - 1
   Maximum delay with a prescaler = 1024 is (1 / (16000000 / 1024)) * 255
   Maximum delay = 16.32ms
   1 tick for 15625Hz (1024 prescaler) = 64us = 0.064ms
   TIMER0 will overflow after 16.32ms are passed
   So, for 600ms delay, TIMER0 will overflow 600ms / 16.32ms = 36 times
   After 36 overflows, we have 36 * 16.32ms = 587.52ms    
   In the 37 iteration of the TIMER0, it is needed a delay of 
   600ms - 587.52ms = 12.48ms
   To achieve a delay of 12.48ms, we will have 12.48ms / 0.064ms = 195 ticks */
/************************************************************************/

ISR(TIMER0_OVF_vect)
{
	/* Keep the number of overflows */
	totalOverflows++;
}

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
	canTX_Headlight.id = 0x258;
	canTX_Headlight.header.rtr = 0;
	canTX_Headlight.header.length = 2;
	canTX_Headlight.data[0] = 0x00;
	canTX_Headlight.data[1] = 0x00;
	
	/* Calculate baudrate */
	USART0_Init(MYUBBR);
	/* Enable global interrupts */
	sei();
	/* LCD initialization with display ON, cursor OFF */
	lcd_init(LCD_DISP_ON);
	/* SPI initialization */
	spi_init();
	/* MCP2515 initialization and test */
	mcp2515_init_and_test();
	/* Set MCP2515 to Normal Mode of operation */
	mcp2515_normal_mode();
	/* Timer0 initialization */
	timer0_init();
	/* Timer1 initialization */
	timer1_init();
	/* ADC initialization */
	adc_init();
	
	/* Dummy Pin used for toggling, as output */
	DDRE |= LED;
	
	/* PH4 as input - headlights and servomotors signal */
	DDRH &= ~HEAD_LIGHT;
	/* PB6 as input - left signal */
	DDRB &= ~LEFT_SIGNAL;
	/* PB7 as input - right signal */
	DDRB &= ~RIGHT_SIGNAL;
	/* PH6 as input - hazard signal */
	DDRH &= ~HAZARD_SIGNAL;
	/* PB4 as input - brake light */
	DDRB &= ~BRAKE_LIGHT;
	/* Enable pull-up resistor for Pin B4 */
	PORTB |= BRAKE_LIGHT;
	
	//char terminalHeader[] = " ID\tRTR\tDLC\tDATA\n\n\r";
	//USART_putstring(terminalHeader);
	//USART_putstring("TEST");
	
	/* Servomotors 180 degrees direction */
	float ocrMin = 250;
	/* Servomotors 0 degrees direction */
	float ocrMax = 650;
	float ocrResult = (ocrMax - ocrMin) / 1023;
	
	DDRB |= (1 << PB5);
	uint8_t canTX_ADC;
	
	uint16_t ocr1aResult;
	char ocr1aLCDResult[3];
	char adcLCDResult[4];
	
	/* Clear the LCD before entering the infinite loop */
	lcd_clrscr();
    while(1)
    {
		
		//startingPoint:
        if(PINH & HEAD_LIGHT)
		{
			canTX_ADC = adcResult / 4;
			canTX_Headlight.data[0] = 0xFF;
			canTX_Headlight.data[1] = canTX_ADC;
			mcp2515_send_message(&canTX_Headlight);
			OCR1A = (int)((ocrResult * adcResult) + 250);
			
			/*
			ocr1aResult = OCR1A;
			itoa(OCR1A, ocr1aLCDResult, 10);
			lcd_puts(ocr1aLCDResult);
			*/
			/*
			itoa(adcResult / 4, adcLCDResult, 10);
			lcd_gotoxy(0, 1);
			lcd_puts(adcLCDResult);
			lcd_puts("  ");*/
			
			PORTE |= LED;
			lcd_gotoxy(0, 0);
			lcd_puts(" Headlight On!  ");
			
			if(adcResult / 4 <= 127)
			{
				lcd_gotoxy(3, 1);
				lcd_puts("Right Turn!");
			}
			else if(adcResult / 4 > 128)
			{
				lcd_gotoxy(3, 1);
				lcd_puts("Left Turn!");
			}
			
			
		}
		/*
		else if(!(PINH & HEAD_LIGHT))
		{
			canTX_Headlight.data[0] = 0x00;
			canTX_Headlight.data[1] = 0x00;
			PORTE &= ~LED;
		}*/
		
		/* Check if the left signal switch is on */
		if(PINB & LEFT_SIGNAL)
		{
			/* If Hazard switch is on when left switch is on, the Hazard signal is active */
			if(PINH & HAZARD_SIGNAL)
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
							lcd_gotoxy(0, 1);
							lcd_puts("                ");
						}
						else
						{
							canTX_Signal.data[0] = 0x06;
							canTX_Signal.data[1] = 0x00;
							mcp2515_send_message(&canTX_Signal);
							/* Set cursor to home position */
							lcd_home();
							lcd_puts("HazardSignal Off");
							lcd_gotoxy(0, 1);
							lcd_puts("                ");
						}
						/* Reset counter */
						TCNT0 = 0;
						/* Reset overflows counter */
						totalOverflows = 0;
					}
				}
			}
			/* Check if the number of overflows is equal with 36 */
			else if(totalOverflows >= 36)
			{
				/* Check if the timer0 counts 195 ticks */
				if(TCNT0 >= 195)
				{
					PORTE ^= LED;
					/*  */
					if(!(PINE & LED))
					{
						//USART_putstring("FF");
						canTX_Signal.data[0] = 0x04;
						canTX_Signal.data[1] = 0x01;
						mcp2515_send_message(&canTX_Signal);
						//print_message_on_USART(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Left Signal On! ");
						lcd_gotoxy(0, 1);
						lcd_puts("                ");
					}
					else
					{
						//USART_putstring("00");
						canTX_Signal.data[0] = 0x04;
						canTX_Signal.data[1] = 0x00;
						mcp2515_send_message(&canTX_Signal);
						//print_message_on_USART(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Left Signal Off!");
						lcd_gotoxy(0, 1);
						lcd_puts("                ");
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
			if(PINH & HAZARD_SIGNAL)
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
							lcd_gotoxy(0, 1);
							lcd_puts("                ");
						}
						else
						{
							canTX_Signal.data[0] = 0x06;
							canTX_Signal.data[1] = 0x00;
							mcp2515_send_message(&canTX_Signal);
							/* Set cursor to home position */
							lcd_home();
							lcd_puts("HazardSignal Off");
							lcd_gotoxy(0, 1);
							lcd_puts("                ");
						}
						/* Reset counter */
						TCNT0 = 0;
						/* Reset overflows counter */
						totalOverflows = 0;
					}
				}
			}
			/* Check if the number of overflows is equal with 36 */
			else if(totalOverflows >= 36)
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
						lcd_gotoxy(0, 1);
						lcd_puts("                ");
					}
					else
					{
						canTX_Signal.data[0] = 0x02;
						canTX_Signal.data[1] = 0x00;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("Right Signal Off");
						lcd_gotoxy(0, 1);
						lcd_puts("                ");
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
						lcd_gotoxy(0, 1);
						lcd_puts("                ");
					}
					else
					{
						canTX_Signal.data[0] = 0x06;
						canTX_Signal.data[1] = 0x00;
						mcp2515_send_message(&canTX_Signal);
						/* Set cursor to home position */
						lcd_home();
						lcd_puts("HazardSignal Off");
						lcd_gotoxy(0, 1);
						lcd_puts("                ");
					}
					/* Reset counter */
					TCNT0 = 0;
					/* Reset overflows counter */
					totalOverflows = 0;
				}
			}
		}
		
		/* Update button state */
		debounce();
		/* Check if the button is pressed */
		if(button_down)
		{
			button_down = 0;
			PORTE ^= LED;
			canTX_Brake.data[0] = 0x00;
			canTX_Brake.data[1] = 0x00;
			mcp2515_send_message(&canTX_Brake);
			lcd_home();
			lcd_puts("                ");
			lcd_gotoxy(0, 1);
			lcd_puts("                ");
			if(~PINE & LED)
			{
				canTX_Brake.data[0] = 0xFF;
				canTX_Brake.data[1] = 0x00;
				mcp2515_send_message(&canTX_Brake);
				lcd_home();
				lcd_puts(" Brake Pressed! ");
				lcd_gotoxy(0, 1);
				lcd_puts("                ");
			}
		}
		_delay_ms(10);
		
		/*
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
		while(!(PINB & BRAKE_LIGHT))
		{
			canTX_Brake.data[0] = 0x00;
			canTX_Brake.data[1] = 0x00;
			mcp2515_send_message(&canTX_Brake);
			break;
		}
		*/
		
		/*
		else if(!(PINB & BRAKE_LIGHT))
		{
			mcp2515_send_message(&canTX_Brake);
			PORTE &= ~LED;
			canTX_Brake.data[0] = 0x00;
			canTX_Brake.data[1] = 0x00;
			//goto startingPoint;
		}*/
		
		
		
		
		//lcd_puts("                ");
		
    }
}

void spi_init()
{
	/* Set MOSI, SCK and SS as outputs */
	DDRB |= (1 << PB2) | (1 << PB1) | (1 << PB0);
}

void mcp2515_init_and_test()
{
	if(!mcp2515_init())
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_home();
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
		lcd_home();
		lcd_gotoxy(5, 0);
		lcd_puts("CAN Bus");
		/* Set cursor on line 2 */
		lcd_gotoxy(2, 1);
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
		lcd_home();
		lcd_puts("Message writenn");
		/* Set cursor on line 2 */
		lcd_gotoxy(4, 1);
		lcd_puts("to buffer!");
	}
	else
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_puts("Message not");
		/* Set cursor on line 2 */
		lcd_gotoxy(0, 1);
		lcd_puts("writenn to buffer");
	}
	
	_delay_ms(1000);
	
	if(mcp2515_check_message())
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_home();
		lcd_puts("Message received");
		_delay_ms(1000);
	}
	else
	{
		/* LCD clear screen */
		lcd_clrscr();
		/* Set cursor on line 1 */
		lcd_home();
		lcd_puts("Message not");
		/* Set cursor on line 2 */
		lcd_gotoxy(4, 0);
		lcd_puts("received!");
	}
}

void mcp2515_normal_mode()
{
	/* Switch the MCP2515 module to normal mode */
	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
	lcd_clrscr();
	lcd_home();
	lcd_puts("CANBusNormalMode");
	_delay_ms(1000);
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

void adc_init()
{
	/* AREF = AVCC, Potentiometer connected on channel 0 (Pin A0) */
	ADMUX |= (1 << REFS0);
	/* Enable ADC with a prescaler = 128, 16000000 / 128 = 125 KHz */
	ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	/* Start single conversion */
	ADCSRA |= (1 << ADSC);
}

//void print_message_on_USART(tCAN *message)
//{
//	/* Counter for the number of bytes in a message */
//	uint8_t counter;
//	char strID[3], strRTR[1], strLength[1], strData[8];
//	itoa(message->id, strID, 16);
//	itoa(message->header.rtr, strRTR, 10);
//	itoa(message->header.length, strLength, 10);
//	USART_putstring(strID);
//	USART_putstring("\t");
//	USART_putstring(strRTR);
//	USART_putstring("\t");
//	USART_putstring(strLength);
//	USART_putstring("\t");
//	for(counter = 0;counter < message->header.length;counter++)
//	{
//		itoa(message->data[counter], strData[counter], 16);
//		USART_putstring(strData[counter]);
//		USART_putstring(" ");
//	}
//	USART_putstring("\n\r");
//}

/* Function that checks the button state and set button_down variable
   if a debounced button is detected */
static inline void debounce()
{
	/* Counter for equal states */
	static uint8_t count = 0;
	/* Holds the number of debounced states */
	static uint8_t button_state = 0;
	/* Check if the button is pressed or not */ 
	uint8_t current_state = (~PINB & BRAKE_LIGHT) != 0;
	if(current_state != button_state)
	{
		/* Button state will change and counter is increased */
		count++;
		if(count >= 4)
		{
			/* Button have not bounced for four checks, change state */
			button_state = current_state;
			/* If the button was pressed, communicate to main */
			if(current_state != 0)
			{
				button_down = 1;
			}
			count = 0;
		}
	}
	else
	{
		/* Reset counter */
		count = 0;
	}
}
