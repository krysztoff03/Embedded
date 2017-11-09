/*
 * Blink_Radar_v2.c
 * 
 * A mini radar system based on a HC SR04 ultrasonic sensor, a HD44780 LCD and an ATMega328p (Mega 2560).
 * When the distance measured is below 10cm, a red LED is on and off, a yellow LED is on and off for the
 * distance between 10 and 20cm, and a green LED for distances above 20cm.
 *
 * A video can be found at https://www.youtube.com/watch?v=Pdrkqjr50Lo
 *
 * Created: 11/9/2017 11:28:25 AM
 *  Author: crist
 */ 

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lcd.h"
#include "usonic.h"

int main(void)
{
	uint16_t sensorDistance; // Variable that holds the return value of the pulse_width function in usonic.h
	DDRE |= (1 << PE3) | (1 << PE4) | (1 << PE5); // Configure pins 5, 2 and 3 as outputs
	US_init(); // Initialize the ultrasonic sensor
	lcd_init(LCD_DISP_ON); // Initialize LCD
	lcd_puts(" HC SR04 Sensor"); 
	_delay_ms(1000);
	uint16_t distance; // Variable that holds the distance measured in centimetres
	char value[4]; // Variable converted to string by itoa function and displayed on the LCD
    while(1)
    {
        US_trig(); // Send a 10us pulse on the trigger line
		sensorDistance = pulse_width();
		distance = (sensorDistance / 58);
		// If the distance is below 10cm, turn the red LED on and off
		if(distance <= 10)
		{
			PORTE |= (1 << PE3); // Turn the red LED on
			_delay_ms(100);
			PORTE &= ~(1 << PE3); // Turn the red LED off
			_delay_ms(100);
		}
		// If the distance is between 10 and 20cm, turn the yellow LED on and off
		else if(distance > 10 && distance <= 20)
		{
			PORTE |= (1 << PE4); // Turn the yellow LED on
			_delay_ms(100);
			PORTE &= ~(1 << PE4); // Turn the yellow LED off
			_delay_ms(100);
		}
		else
		// If the distance is above 20cm, turn the green LED on and off
		{
			PORTE |= (1 << PE5); // Turn the green LED on
			_delay_ms(100);
			PORTE &= ~(1 << PE5); // Turn the green LED off
			_delay_ms(100);
		}
		//distance2 = (r / 148);
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Distance:");
		lcd_gotoxy(10, 0);
		if(distance == 1129)
		{
			lcd_puts("max");
		}
		else
		{
			itoa(distance, value, 10);
			lcd_puts(value);
		}
		lcd_gotoxy(14, 0);
		lcd_puts("cm");
		_delay_ms(500);
    }
}
