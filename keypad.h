#ifndef KEYPAD_H
#define KEYPAD_H

#include <reg51.h>

#define KEYPAD_PORT P1

unsigned char get_key(void);

#endif