#ifndef GSM_H
#define GSM_H

// Renamed for clarity (since we removed GSM)
void uart_init(void);
void uart_tx(unsigned char c);
char uart_rx(void);            // NEW: Receive char from Terminal
void uart_send_str(unsigned char *str);

#endif