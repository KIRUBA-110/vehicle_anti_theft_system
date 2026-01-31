#include "lcd.h"

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 123; j++); // Calibrated for 11.0592MHz
}

void lcd_cmd(unsigned char cmd) {
    // Send higher nibble
    P2 = (P2 & 0x0F) | (cmd & 0xF0);
    RS = 0;  // Command mode
    RW = 0;  // Write mode
    EN = 1;  // Enable pulse
    delay_ms(1);
    EN = 0;
    
    // Send lower nibble
    P2 = (P2 & 0x0F) | (cmd << 4);
    EN = 1;
    delay_ms(1);
    EN = 0;
    delay_ms(2);
}

void lcd_data(unsigned char dat) {
    // Send higher nibble
    P2 = (P2 & 0x0F) | (dat & 0xF0);
    RS = 1;  // Data mode
    RW = 0;  // Write mode
    EN = 1;
    delay_ms(1);
    EN = 0;
    
    // Send lower nibble
    P2 = (P2 & 0x0F) | (dat << 4);
    EN = 1;
    delay_ms(1);
    EN = 0;
    delay_ms(2);
}

void lcd_init(void) {
    delay_ms(20);        // Wait for LCD to power up
    lcd_cmd(0x02);       // Return home
    lcd_cmd(0x28);       // 4-bit mode, 2 lines, 5x7 font
    lcd_cmd(0x0C);       // Display ON, cursor OFF
    lcd_cmd(0x06);       // Entry mode: increment cursor
    lcd_cmd(0x01);       // Clear display
    delay_ms(2);
}

void lcd_string(char *str) {
    while(*str) {
        lcd_data(*str++);
    }
}

void lcd_clear(void) {
    lcd_cmd(0x01);
    delay_ms(2);
}

void lcd_goto(unsigned char row, unsigned char col) {
    unsigned char position;
    if(row == 0) {
        position = 0x80 + col;  // First line
    } else {
        position = 0xC0 + col;  // Second line
    }
    lcd_cmd(position);
}