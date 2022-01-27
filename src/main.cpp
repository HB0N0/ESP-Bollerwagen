#include <Arduino.h>
#include <WS2812FX.h>
#include <Button2.h>
#include "config.h"


struct CarState{
  bool  isBraking;
  bool  lightsOn;
  bool  maxlightsOn;
  bool  blinkL;
  bool  blinkR;
} state;

#define FX_MODE_CAR_INDICATOR FX_MODE_CUSTOM
#define FX_MODE_DRIVELIGHT FX_MODE_CUSTOM_1
#define FX_MODE_MAX_DRIVELIGHT FX_MODE_CUSTOM_2
uint16_t car_indicator(void);
uint16_t drive_light(void);
uint16_t max_drive_light(void);

// Neopixel effect library
// First Led stripe
WS2812FX driveLight = WS2812FX(NUM_LEDS_FRONT + NUM_LEDS_REAR, PIN_LEDSTRIPE, NEO_GRB + NEO_KHZ800);
WS2812FX statusLed = WS2812FX(NUM_STATUS_LEDS, PIN_STATUS_LED, NEO_GRB + NEO_KHZ800);

Button2 btnMode, btnLight, btnBlinkL, btnBlinkR;

void btnMode_click(Button2& btn){
  Serial.println("Mode-Button clicked");
}
void btnLight_click(Button2& btn){
  Serial.println("Light-Button clicked");
  if(!state.lightsOn){
    state.lightsOn = true;
  }else if (!state.maxlightsOn){
    state.maxlightsOn = true;
  }else{
    state.lightsOn = false;
    state.maxlightsOn = false;
  }
  Serial.print("Lights: ");
  Serial.println(state.lightsOn);
  Serial.print("MaxLights: ");
  Serial.println(state.maxlightsOn);
}
void btnBlinkL_changed(Button2& btn){
  state.blinkL = btn.isPressed();
}
void btnBlinkR_changed(Button2& btn){
  state.blinkR = btn.isPressed();
}

void setupButtons(){
  btnMode.begin(PIN_BTN_MODE);
  btnMode.setClickHandler(btnMode_click);
  
  btnLight.begin(PIN_BTN_LIGHT);
  btnLight.setClickHandler(btnLight_click);
  
  btnBlinkL.begin(PIN_BLINK_L);
  btnBlinkL.setChangedHandler(btnBlinkL_changed);
  
  btnBlinkR.begin(PIN_BLINK_R);
  btnBlinkR.setChangedHandler(btnBlinkR_changed);
}
void loopButtons(){
  btnMode.loop();
  btnLight.loop();
  btnBlinkL.loop();
  btnBlinkR.loop();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Booting...");
  setupButtons();

  // Setup WS2812FX
  driveLight.init();
  driveLight.setCustomMode(0, F("Blinker"), car_indicator);
  driveLight.setCustomMode(1, F("Abblendlicht"), drive_light);
  driveLight.setCustomMode(2, F("Fernlicht"), max_drive_light);
  //                      index   first                               last                                      mode              color   speed   reverse
  driveLight.setSegment(  0,      0,                                  NUM_LEDS_FRONT/2 - 1,                     FX_MODE_STATIC,   BLACK,  1500,   true);
  driveLight.setSegment(  1,      NUM_LEDS_FRONT/2,                   NUM_LEDS_FRONT - 1,                       FX_MODE_STATIC,   BLACK,  1500,   false);
  driveLight.setSegment(  2,      NUM_LEDS_FRONT,                     NUM_LEDS_FRONT + (NUM_LEDS_REAR/2 - 1),   FX_MODE_STATIC,   BLACK,  1500,   true);
  driveLight.setSegment(  3,      NUM_LEDS_FRONT + NUM_LEDS_REAR / 2, NUM_LEDS_FRONT+NUM_LEDS_REAR - 1,         FX_MODE_STATIC,   BLACK,  1500,   false);
  driveLight.start();

  /*for(int i = 0; i < 4; i++){
    driveLight.setMode(i, FX_MODE_COLOR_WIPE);
    driveLight.setColor(i, ORANGE);
  }*/

  statusLed.init();
  statusLed.setBrightness(20);
  statusLed.setMode(FX_MODE_BREATH);
  statusLed.setColor(BLUE);
  statusLed.start();
}

bool isBlinkingL = false;
bool isBlinkingR = false;

uint16_t globalTimerInterval = 1000;
uint32_t lastTimer = 0;
bool indicatorON = false;

void loop() {
  loopButtons();

  /* Timer to keep all indicator lights in sync */
  if(isBlinkingL || isBlinkingR){
    if(millis() - lastTimer > globalTimerInterval / 2){
      indicatorON = !indicatorON;
      lastTimer = millis();
    }
  }

  /*if(state.maxlightsOn){
    for(int i = 0; i < 4; i++){
      driveLight.setMode(i, FX_MODE_MAX_DRIVELIGHT);
    }
  }else if (state.lightsOn) {
    for(int i = 0; i < 4; i++){
      driveLight.setMode(i, FX_MODE_DRIVELIGHT);
    }
  }else{
    for(int i = 0; i < 4; i++){
      driveLight.setMode(i, FX_MODE_STATIC);
    }
  }*/


  if(state.blinkL && !isBlinkingL){
    driveLight.setMode(0, FX_MODE_CAR_INDICATOR);
    driveLight.setMode(2, FX_MODE_CAR_INDICATOR);
    isBlinkingL = true;
  }else if (isBlinkingL && !state.blinkL && driveLight.isCycle(2)){
    driveLight.setMode(0, FX_MODE_STATIC);
    driveLight.setMode(2, FX_MODE_STATIC);
    isBlinkingL = false;
  }

  if(state.blinkR && !isBlinkingR){
    driveLight.setMode(1, FX_MODE_CAR_INDICATOR);
    driveLight.setMode(3, FX_MODE_CAR_INDICATOR);
    isBlinkingR = true;
  }else if (isBlinkingR && !state.blinkR && driveLight.isCycle(3)){
    driveLight.setMode(1, FX_MODE_STATIC);
    driveLight.setMode(3, FX_MODE_STATIC);
    isBlinkingR = false;
  }

  driveLight.service();
  statusLed.service();


  // put your main code here, to run repeatedly:
}

uint16_t drive_light(void){
  uint32_t driveLightColorFront = 0x999999;
  WS2812FX::Segment* seg = driveLight.getSegment();
  WS2812FX::Segment_runtime* segrt = driveLight.getSegmentRuntime();
  uint16_t seglen = seg->stop - seg->start + 1;

  for(uint16_t i=seg->start; i <= seg->stop; i++) {
    driveLight.setPixelColor(i, driveLightColorFront);
  }
  return 200;
}

uint16_t max_drive_light(void){
  uint32_t driveLightColorFront = 0xffffff;
  WS2812FX::Segment* seg = driveLight.getSegment();
  WS2812FX::Segment_runtime* segrt = driveLight.getSegmentRuntime();
  uint16_t seglen = seg->stop - seg->start + 1;

  for(uint16_t i=seg->start; i <= seg->stop; i++) {
    driveLight.setPixelColor(i, driveLightColorFront);
  }
  return 200;
}


uint16_t car_indicator(void) {
  uint32_t indicator_color = 0xff6a00;

  // get the current segment
  WS2812FX::Segment* seg = driveLight.getSegment();
  WS2812FX::Segment_runtime* segrt = driveLight.getSegmentRuntime();
  uint16_t seglen = seg->stop - seg->start + 1;
  bool isReverse = (seg->options & REVERSE) == REVERSE;

  if(indicatorON){
    if(segrt->counter_mode_step < seglen) {
      uint32_t led_offset = segrt->counter_mode_step;
      if(isReverse) {
        driveLight.setPixelColor(seg->stop - led_offset, indicator_color);
      } else {
        driveLight.setPixelColor(seg->start + led_offset, indicator_color);
      }

      segrt->counter_mode_step ++;

      return (100 / seglen);

    } else if(segrt->counter_mode_step == seglen) {
      // Filled up
      for(uint16_t i=seg->start; i <= seg->stop; i++) {
        driveLight.setPixelColor(i, indicator_color);
      }
      segrt->counter_mode_step++;
    }
    return 20;
  } else {
    // Turn off
    for(uint16_t i=seg->start; i <= seg->stop; i++) {
      driveLight.setPixelColor(i, BLACK);
    }
    //driveLight.setCycle();
    if(segrt->counter_mode_step > 0){
      segrt->aux_param3++;
      if(segrt->aux_param3 > 2){
        driveLight.setCycle();
      }
      segrt->counter_mode_step = 0;
    }
    return 20;
  }
}