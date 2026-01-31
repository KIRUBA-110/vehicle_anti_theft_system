#include "keypad.h"
#include "lcd.h"  // For delay function

unsigned char keypad_map[4][4] = {
    {'7', '8', '9', 'A'},
    {'4', '5', '6', 'B'},
    {'1', '2', '3', 'C'},
    {'*', '0', '#', 'D'}
};

unsigned char get_key(void) {
    unsigned char row, col;
    
    while(1) {
        for(row = 0; row < 4; row++) {
            // Make all rows HIGH
            KEYPAD_PORT = 0xFF;
            
            // Make current row LOW
            KEYPAD_PORT &= ~(1 << row);
            
            // Check each column
            for(col = 0; col < 4; col++) {
                // If column is LOW, key is pressed
                if(!(KEYPAD_PORT & (1 << (col + 4)))) {
                    // Wait for key release (debounce)
                    delay_ms(20);
                    while(!(KEYPAD_PORT & (1 << (col + 4))));
                    delay_ms(20);
                    
                    return keypad_map[row][col];
                }
            }
        }
    }
}