/*
 * Test_LED_OK.c
 * This test was performed on ATMega328p (Mega 2560)
 * Pins used are on PORTA
 * Created: 11/8/2017 5:52:51 PM
 *  Author: Cristi
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include "lcd.h"

int main(void)
{
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	char s1[] = "Testing LCD";
	char s2[] = " OK";
    while(1)
    {
        _delay_ms(1000);
		lcd_home();
		lcd_puts(s1);
		_delay_ms(1000);
		lcd_puts(s2);
		_delay_ms(1000);
		lcd_clrscr(); 
    }
}