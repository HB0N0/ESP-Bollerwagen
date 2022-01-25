#include <Arduino.h>
#include <WS2812FX.h>
#include "config.h"

// Neopixel effect library
// First Led stripe
WS2812FX driveLight = WS2812FX(NUM_LEDS_FRONT + NUM_LEDS_REAR, PIN_LEDSTRIPE, NEO_GRB + NEO_KHZ800);
WS2812FX statusLed = WS2812FX(NUM_STATUS_LEDS, PIN_STATUS_LED, NEO_GRB + NEO_KHZ800);


void setup() {
  // put your setup code here, to run once:

  // Setup buttons
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_LIGHT, INPUT_PULLUP);
  pinMode(PIN_BLINK_L, INPUT_PULLUP);
  pinMode(PIN_BLINK_R, INPUT_PULLUP);

  // Setup WS2812FX
  driveLight.init();
  //                      index   first                               last                                      mode              color   speed   reverse
  driveLight.setSegment(  0,      0,                                  NUM_LEDS_FRONT/2 - 1,                     FX_MODE_STATIC,   BLACK,  1500,   false);
  driveLight.setSegment(  0,      NUM_LEDS_FRONT/2,                   NUM_LEDS_FRONT - 1,                       FX_MODE_STATIC,   BLACK,  1500,   true);
  driveLight.setSegment(  0,      NUM_LEDS_FRONT,                     NUM_LEDS_FRONT + (NUM_LEDS_REAR/2 - 1),   FX_MODE_STATIC,   BLACK,  1500,   false);
  driveLight.setSegment(  0,      NUM_LEDS_FRONT + NUM_LEDS_REAR / 2, NUM_LEDS_FRONT+NUM_LEDS_REAR - 1,         FX_MODE_STATIC,   BLACK,  1500,   true);
  driveLight.start();

  statusLed.init();
  statusLed.setBrightness(20);
  statusLed.setMode(FX_MODE_BREATH);
  statusLed.setColor(ORANGE);
  statusLed.start();


}

void loop() {

  driveLight.service();
  statusLed.service();
  // put your main code here, to run repeatedly:
}