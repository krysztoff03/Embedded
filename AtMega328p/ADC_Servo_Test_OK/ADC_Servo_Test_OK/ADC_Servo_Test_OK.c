/*
 * ADC_Servo_Test_OK.c
 *
 * Tests were made with ATMega328p (Uno), a SG90 servo motor and the potentiometer;
 *
 * Created: 11/11/2017 9:59:25 AM
 *  Author: cristi
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

static volatile uint16_t adcResult; // Variable that holds the 10-bit result of ADC

ISR (ADC_vect)
{
	uint8_t adclReg = ADCL; // Variable that holds the first 8 bits of the 10 bits ADC resolution
	adcResult = (ADCH << 8) | adclReg; // ADCL starts with the first 8 bits of final 10 bits, then ADCH 2 remaining bits will be left shifted 8 positions to the left
	ADCSRA |= 1 << ADSC; // Start single conversion
}

int main(void)
{
	
	TCCR1A |= (1 << COM1A1) | (1 << WGM11); // Clear OC1A on Compare Match
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); // Fast PWM mode 14 with prescaler of 64
	ICR1 = 4999; // TOP value calculated with Fpwm = Fcpu / (N * (1 + TOP)), N - Prescaler = 64;
	DDRB |= (1 << PINB1); // Set Pin B1 (9) as output (OC1A)
	OCR1A = 250;
	
	// AREF = AVcc, Potentiometer connected on channel 0 (Pin A0)
	ADMUX = (1 << REFS0);
	
	// Enable ADC with a prescaler = 128 (16000000 / 128 = 125000 KHz)
	ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	sei(); // Enable global interrupts
	
	ADCSRA |= (1 << ADSC); // Start single conversion
	
	float ocrMin = 250; // 180 degrees
	float ocrMax = 650; // 0 degrees
	float ocrResult = (ocrMax - ocrMin) / 1023;
	
    while(1)
    {
		OCR1A = (int)((ocrResult * adcResult) + 250);
    }
}
