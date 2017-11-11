/*
 * Servo_Test_OK.c
 *
 * Test made on ATMega328p (Uno)
 *
 * Created: 11/9/2017 6:25:10 PM
 *  Author: cristi
 */ 


#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	
	TCCR1A |= (1 << COM1A1) | (1 << WGM11); // Clear OC1A on Compare Match
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); // Fast PWM mode 14 with prescaler of 64
	ICR1 = 4999; // TOP value calculated with Fpwm = Fcpu / (N * (1 + TOP)), N - Prescaler = 64;
	DDRB |= (1 << PINB1); // Set Pin B1 (9) as output (OC1A)
	
	
	
    while(1)
    {
		/*
		OCR1A = 250; // 180 deg
		_delay_ms(1000);
		OCR1A = 650; // 0 deg
		_delay_ms(1000);
		OCR1A = 250; // 180 deg
		_delay_ms(1000);*/
		
		/*
		OCR1A = 250; // 180 deg
		_delay_ms(1000);
		OCR1A = 450; // 90 deg
		_delay_ms(1000);
		OCR1A = 650; // 0 deg
		_delay_ms(1000);
		OCR1A = 450; // 90 deg
		_delay_ms(1000);
		OCR1A = 250; // 180 deg
		_delay_ms(1000);*/
		
		/*
		// 180 deg -> 135 deg -> 90 deg -> 45 deg -> 0 deg -> 45 deg -> 90 deg -> 135 deg -> 180 deg
		OCR1A = 250; // 180 deg
		_delay_ms(1000);
		OCR1A = 350; // 135 deg
		_delay_ms(1000);
		OCR1A = 450; // 90 deg
		_delay_ms(1000);
		OCR1A = 550; // 45 deg;
		_delay_ms(1000);
		OCR1A = 650; // 0 deg
		_delay_ms(1000);
		OCR1A = 550; // 45 deg
		_delay_ms(1000); 
		OCR1A = 450; // 90 deg
		_delay_ms(1000);
		OCR1A = 350; // 135 deg
		_delay_ms(1000);
		OCR1A = 250; // 180 deg
		_delay_ms(1000);*/
		
		
		/*
		OCR1A = 250;
		_delay_ms(1000);
		while(OCR1A != 650)
		{
			OCR1A += 10;
			_delay_ms(100);
		}
		_delay_ms(1000);*/
    }
}
