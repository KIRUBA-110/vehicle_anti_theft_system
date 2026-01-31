#ifndef LCD_H
#define LCD_H

#include <reg51.h>

// LCD Pin Definitions
sbit RS = P2^0;
sbit RW = P2^1;
sbit EN = P2^2;

// Function Prototypes
void delay_ms(unsigned int ms);
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char dat);
void lcd_init(void);
void lcd_string(char *str);
void lcd_clear(void);
void lcd_goto(unsigned char row, unsigned char col);

#endif