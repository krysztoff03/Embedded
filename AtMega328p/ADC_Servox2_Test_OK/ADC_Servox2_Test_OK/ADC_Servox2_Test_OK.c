/*
 * ADC_Servox2_Test_OK.c
 *
 * Created: 1/7/2018 12:24:43 PM
 *  Author: Cristi
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "LCD/lcd.h"

/* Variable that holds the 10-bit result of ADC */
static volatile uint16_t adcResult;

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

void adc_init()
{
	/* Clear OC1A on Compare Match */
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	/* Fast PWM mode 14 with prescaler of 64 */
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
	/* Top value calculated with Fpwm = Fcpu / (N * (1 + Top)), N - Prescaler = 64 
	   ICR1 = 4999 for 50Hz PWM */
	ICR1 = 4999;
	/* Pin B5 set as output (OC1A) */
	DDRB |= (1 << PB5);
	/*  */
	OCR1A = 250;
	/* AREF = Avcc, Potentiometer connected on channel 0 (Pin A0) */
	ADMUX = (1 << REFS0);
	/* Enable ADC with a prescaler = 128 (16000000 / 128) = 125 KHz */
	ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	/* Enable global interrupts */
	sei();
	/* Start single conversion */
	ADCSRA |= (1 << ADSC);
}

int main(void)
{
	/* LCD initialization with display ON, cursor OFF */
	lcd_init(LCD_DISP_ON);
	/* ADC initialization */	
	adc_init();
	/* 180 degrees */
	float ocrMin = 250;
	/* 0 degrees */
	float ocrMax = 650;
	float ocrResult = (ocrMax - ocrMin) / 1023;
	
	uint16_t ocr1aValue;
	uint16_t finalADCResult;
	char value[3];
	char adcValue[5];
	
	lcd_gotoxy(0, 0);
	lcd_puts("ADC: ");
	
    while(1)
    {
        OCR1A = (int)((ocrResult * adcResult) + 250);
		ocr1aValue = OCR1A;
		itoa(ocr1aValue, value, 10);
		lcd_gotoxy(6, 0);
		lcd_puts(value);
		
		finalADCResult = adcResult / 4;
		itoa(finalADCResult, adcValue, 16);
		lcd_gotoxy(6, 1);
		lcd_puts(adcValue);
    }
}