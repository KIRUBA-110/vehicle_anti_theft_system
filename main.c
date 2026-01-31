#include <reg51.h>
#include <string.h>
#include "lcd.h"
#include "keypad.h"
#include "gsm.h" 

// --- MOTOR / RELAY CONFIGURATION ---
// 1 = 5V (ON), 0 = 0V (OFF)
#define RELAY_ON  1  
#define RELAY_OFF 0 

// --- GLOBAL FLAGS ---
volatile bit flag_door = 0;
volatile bit flag_vibration = 0;

// --- PIN DEFINITIONS ---
sbit BUZZER = P0^0;
sbit RELAY = P0^1;      // MOTOR PIN
sbit LED_ARMED = P0^2;
sbit LED_ALERT = P0^3;
sbit LED_ENGINE = P0^5;

// Inputs
sbit IGNITION = P3^4;
sbit FINGERPRINT_BTN = P3^5; 

// --- STATES ---
#define DISARMED 0
#define ARMED 1
#define ALERT 2
#define THEFT_MODE 3

unsigned char system_state = DISARMED;
char password[5] = "1234"; 

// --- PROTOTYPES ---
void system_init(void);
void main_menu(void);
void arm_system(void);
void disarm_system(void);
void enter_theft_mode(void);
void alarm_beep(unsigned char times);
bit authenticate_rfid(void);
bit authenticate_password(void);
bit authenticate_fingerprint(void);

// --- INTERRUPTS ---
void door_interrupt(void) interrupt 0 {
    if(system_state == ARMED) flag_door = 1;
}

void vibration_interrupt(void) interrupt 2 {
    if(system_state == ARMED) flag_vibration = 1;
}

// --- MAIN LOOP ---
void main(void) {
    unsigned char choice;
    
    system_init();
    
    lcd_cmd(0x01); 
    lcd_string("SmartGuard Pro");
    uart_send_str("System Initialized.\r\n");
    delay_ms(1000);
    
    while(1) {
        // 1. Handle Sensors
        if(flag_door == 1) {
            flag_door = 0;
            EX0 = 0; 
            lcd_cmd(0x01); lcd_string("DOOR OPENED!");
            uart_send_str("SENSOR: Door Breach!\r\n");
            LED_ALERT = 1;
            alarm_beep(3);
            system_state = ALERT;
            EX0 = 1;
        }

        // 2. State Machine
        switch(system_state) {
            case DISARMED:
                main_menu();
                choice = get_key();
                
                if(choice == '1') arm_system();
                else if(choice == 'D') enter_theft_mode(); 
                break;
                
            case ARMED:
                lcd_cmd(0x01); lcd_string("System: ARMED");
                LED_ARMED = ~LED_ARMED; 
                delay_ms(200); 
                
                if(IGNITION == 0) { 
                    delay_ms(50);
                    if(IGNITION == 0) disarm_system();
                }
                break;
                
            case ALERT:
                disarm_system();
                break;
                
            case THEFT_MODE:
                lcd_cmd(0x01); lcd_string("THEFT DETECTED");
                LED_ALERT = 1; 
                BUZZER = ~BUZZER; 
                delay_ms(200);
                break;
        }
    }
}

// --- FUNCTIONS ---

void system_init(void) {
    P0 = 0xFF; 
    
    // *** CRITICAL: START MOTOR OFF ***
    RELAY = RELAY_OFF; 
    
    lcd_init();
    uart_init(); 
    
    IT0 = 1; EX0 = 1; 
    IT1 = 1; EX1 = 1; 
    EA = 1;           
}

void main_menu(void) {
    lcd_cmd(0x01); lcd_string("1:Arm System");
    lcd_goto(1, 0); lcd_string("Status: SAFE");
}

void arm_system(void) {
    unsigned char i;
    lcd_cmd(0x01); lcd_string("Arming...");
    uart_send_str("System Arming...\r\n");
    
    // Ensure Motor is OFF when arming
    RELAY = RELAY_OFF; 
    
    for(i=0; i<3; i++) {
        alarm_beep(1);
        delay_ms(1000);
    }
    system_state = ARMED;
    LED_ARMED = 1; 
    LED_ALERT = 0;
}

void disarm_system(void) {
    lcd_cmd(0x01); lcd_string("Auth Required");
    LED_ALERT = 1;
    delay_ms(1000); 

    // --- STAGE 1: RFID (Type 'A' in Terminal) ---
    if(authenticate_rfid() == 0) {
        lcd_cmd(0x01); lcd_string("RFID Failed!");
        uart_send_str("AUTH FAIL: RFID Invalid\r\n");
        delay_ms(1000);
        return; 
    }

    // --- STAGE 2: PASSWORD (Type 1234 on Keypad) ---
    if(authenticate_password() == 0) {
        lcd_cmd(0x01); lcd_string("Wrong PIN!");
        uart_send_str("AUTH FAIL: Wrong PIN\r\n");
        delay_ms(1000);
        enter_theft_mode(); 
        return;
    }

    // --- STAGE 3: FINGERPRINT (Click Button) ---
    if(authenticate_fingerprint() == 0) {
        lcd_cmd(0x01); lcd_string("Bio Failed!");
        uart_send_str("AUTH FAIL: Bio Invalid\r\n");
        delay_ms(1000);
        return;
    }

    // --- SUCCESS ---
    system_state = DISARMED;
    LED_ARMED = 0;
    LED_ALERT = 0;
    
    // *** MOTOR START LOGIC ***
    RELAY = RELAY_ON; // Turn P0.1 HIGH immediately
    
    lcd_cmd(0x01); lcd_string("Welcome Owner!");
    uart_send_str("\r\n--- ACCESS GRANTED ---\r\n");
    uart_send_str("Engine Started.\r\n"); // Motor is now running
    delay_ms(2000);
}

// --- AUTH FUNCTIONS ---

bit authenticate_rfid(void) {
    unsigned char rx_char;
    
    lcd_cmd(0x01); lcd_string("Stage 1: RFID");
    lcd_goto(1,0); lcd_string("Scan Tag...");
    
    uart_send_str("\r\n[RFID] Type 'A' in Terminal: ");
    
    // Wait for Terminal Input
    rx_char = uart_rx(); 
    uart_tx(rx_char); // Echo
    uart_send_str("\r\n");
    
    if(rx_char == 'A' || rx_char == 'a') {
        lcd_cmd(0x01); lcd_string("RFID Accepted");
        uart_send_str("RFID Valid.\r\n");
        delay_ms(1000);
        return 1;
    }
    
    return 0;
}

bit authenticate_password(void) {
    char entered[5];
    unsigned char i = 0;
    unsigned char key = 0;
    
    lcd_cmd(0x01); lcd_string("Enter PIN:");
    lcd_goto(1,0); 
    
    while(i < 4) {
        key = get_key();
        if(key >= '0' && key <= '9') {
            entered[i] = key;
            lcd_data('*');
            i++;
            delay_ms(300); 
        }
    }
    entered[4] = '\0';
    
    if(strcmp(entered, password) == 0) return 1;
    return 0;
}

bit authenticate_fingerprint(void) {
    lcd_cmd(0x01); lcd_string("Stage 3: Bio");
    lcd_goto(1,0); lcd_string("Press Sensor");
    
    // Wait for P3.5 (Button)
    while(FINGERPRINT_BTN == 1); 
    
    lcd_cmd(0x01); lcd_string("Finger Matched!");
    delay_ms(1000);
    return 1;
}

void enter_theft_mode(void) {
    system_state = THEFT_MODE;
    RELAY = RELAY_OFF; // Stop Motor
    
    lcd_cmd(0x01); lcd_string("THEFT DETECTED");
    LED_ALERT = 1;
    
    uart_send_str("\r\n!!! THEFT DETECTED !!!\r\n");
    uart_send_str("Engine Locked. alert Sent.\r\n");
}

void alarm_beep(unsigned char times) {
    unsigned char i;
    for(i=0; i<times; i++) {
        BUZZER = 0; delay_ms(100);
        BUZZER = 1; delay_ms(100);
    }
}