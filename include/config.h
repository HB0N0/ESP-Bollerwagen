// Config File
// Created by Hannes Bosch 25.01.2022

// Pin configuration
#define PIN_BTN_LIGHT    D2
#define PIN_BLINK_L      D6
#define PIN_BLINK_R      D7
#define PIN_NC_EMERGENCY D1
#define PIN_BTN_MODE     D5
#define PIN_STATUS_LED   D4
#define PIN_LEDSTRIPE    D3

// Serial configuration
// Serial protcol is used to capture live data from hoverboard mainboard
#define SERIAL_BAUD     115200
#define HOVER_SERIAL   Serial  // If changed, Serial.begin has to be called somewhere
#define START_FRAME     0xABCD // Do not change!
#define SERIAL_TIMEOUT  50  // ms
#define SEND_INTERVAL   100
//#define SERIAL_DEBUG    // if defined debug output is printed to serial
//#define DEBUG_RX // if defined all incoming bytes will be printed to console

// Neopixel configuration
#define NUM_LEDS_FRONT  2 * 5       // Sum of all LEDs in the front (left and right are equally distributed)
#define NUM_LEDS_REAR   2 * 10      // Sum of all Rear LEDs

#define NUM_STATUS_LEDS 1

// COLORS
#define COLOR_INDICATOR 0xff6a00
#define COLOR_REAR_LIGHT 0x220000
#define COLOR_BRAKE_LIGHT 0xff0000
#define COLOR_FRONT_LIGHT 0x222222
#define COLOR_MAX_LIGHT 0xffffff

// ADC config - the internal ADC of the WEMOS D1 is used to measure the Voltage of our 2nd battery (12V) over a voltage divider.
// Configuration of the voltage divider:  +12V --|47k|-- Vin --|10k|-- GND
#define PIN_BAT     A0
#define BAT_READ_INTERVAL 500 // ms
// Resistor values
#define R_BOTTOM    10000
#define R_TOP       47000
// Status led feedback
#define BAT_FULL 12.6
#define BAT_DEAD 10.5
// Calculate battery levels
#define BAT_LVL0    0.0 * (BAT_FULL - BAT_DEAD) + BAT_DEAD
#define BAT_LVL1    0.2 * (BAT_FULL - BAT_DEAD) + BAT_DEAD
#define BAT_LVL2    0.4 * (BAT_FULL - BAT_DEAD) + BAT_DEAD
#define BAT_LVL3    0.6 * (BAT_FULL - BAT_DEAD) + BAT_DEAD
#define BAT_LVL4    0.8 * (BAT_FULL - BAT_DEAD) + BAT_DEAD
#define BAT_LVL5    1.0 * (BAT_FULL - BAT_DEAD) + BAT_DEAD