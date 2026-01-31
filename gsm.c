#include <reg51.h>
#include "gsm.h"

// Initialize Serial Port (9600 Baud)
void uart_init(void) {
    TMOD |= 0x20;   // Timer 1, Mode 2
    TH1 = 0xFD;     // 9600 baud
    SCON = 0x50;    // Mode 1, Receive Enabled (REN=1)
    TR1 = 1;        // Start Timer
}

// Send Character to Terminal
void uart_tx(unsigned char c) {
    SBUF = c;
    while(TI == 0);
    TI = 0;
}

// Receive Character from Terminal (Waits for input)
char uart_rx(void) {
    while(RI == 0); // Wait until character arrives
    RI = 0;         // Clear flag
    return SBUF;    // Return the character
}

// Send String to Terminal
void uart_send_str(unsigned char *str) {
    while(*str) {
        uart_tx(*str++);
    }
}