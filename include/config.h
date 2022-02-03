// Config File
// Created by Hannes Bosch 25.01.2022

// Pin configuration
#define PIN_BTN_MODE    D3 //D1
#define PIN_BTN_LIGHT   D5
#define PIN_BLINK_L     D6
#define PIN_BLINK_R     D7
#define PIN_STATUS_LED  D2
#define PIN_LEDSTRIPE   D1 //D3

// Serial configuration
// Serial protcol is used to capture live data from hoverboard mainboard
#define SERIAL_BAUD     115200
#define HOVER_SERIAL   Serial  // If changed, Serial.begin has to be called somewhere
#define START_FRAME     0xABCD // Do not change!
#define SERIAL_DEBUG    // if defined debug output is printed to serial
//#define DEBUG_RX // if defined all incoming bytes will be printed to console

// Neopixel configuration
#define NUM_LEDS_FRONT  2 * 5       // Sum of all LEDs in the front (left and right are equally distributed)
#define NUM_LEDS_REAR   2 * 10      // Sum of all Rear LEDs

// COLORS
#define COLOR_INDICATOR 0xff6a00
#define COLOR_REAR_LIGHT 0x220000
#define COLOR_BRAKE_LIGHT 0xff0000
#define COLOR_FRONT_LIGHT 0x222222
#define COLOR_MAX_LIGHT 0xffffff


#define NUM_STATUS_LEDS 1
