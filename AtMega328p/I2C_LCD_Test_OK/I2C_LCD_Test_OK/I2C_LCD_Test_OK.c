/*
 * I2C_LCD_Test_OK.c
 * The test was performed on ATMega 328p (Uno)
 * LCD consists of an 2004A LCD Module with an LCD1602 I2C Adapter
 * Created: 12/1/2017 4:10:19 PM
 *  Author: Cristi
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcdpcf8574/lcdpcf8574.h"


int main(void)
{
	sei();
	lcd_init(LCD_DISP_ON);
	lcd_home();
	uint8_t led = 0;
	uint8_t i;
	char buffer[10];
    while(1)
    {
        //TODO:: Please write your application code
		lcd_led(led); 
		_delay_ms(1000);
		lcd_gotoxy(2, 1);
		lcd_puts("20x4 LCD Test OK!");
		_delay_ms(1000);
		lcd_gotoxy(4, 2);
		lcd_puts("ATMega 328p");
		for(i = 0;i <= 255;i++)
		{
			_delay_ms(1000);
			itoa(i, buffer, 10);
			lcd_gotoxy(9, 3);
			lcd_puts(buffer);
		}
    }
}