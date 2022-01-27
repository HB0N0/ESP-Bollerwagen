// Config File
// Created by Hannes Bosch 25.01.2022

// Pin configuration
#define PIN_BTN_MODE    D1
#define PIN_BTN_LIGHT   D5
#define PIN_BLINK_L     D6
#define PIN_BLINK_R     D7
#define PIN_STATUS_LED  D3
#define PIN_LEDSTRIPE   D2

// Serial configuration
// Serial protcol is used to capture live data from hoverboard mainboard
#define SERIAL_BAUD     115200
#define START_FRAME     0xABCD  

// Neopixel configuration
#define NUM_LEDS_FRONT  2 * 5       // Sum of all LEDs in the front (left and right are equally distributed)
#define NUM_LEDS_REAR   2 * 20      // Sum of all Rear LEDs

#define NUM_STATUS_LEDS 1
