#include <Arduino.h>
#include <WS2812FX.h>
#include <Button2.h>
#include "config.h"
#include "hoverserial.h"


struct CarState{
  bool  isBraking;
  bool  lightsOn;
  bool  maxlightsOn;
  bool  blinkL;
  bool  blinkR;
} state;

extern HoverBoardLeds hoverLeds;

#define LIGHT_FRONT_LEFT 0
#define LIGHT_FRONT_RIGHT 1
#define LIGHT_REAR_LEFT 2
#define LIGHT_REAR_RIGHT 3

#define FX_MODE_CAR_INDICATOR FX_MODE_CUSTOM
#define FX_MODE_DRIVELIGHT FX_MODE_CUSTOM_1
uint16_t car_indicator(void);
uint16_t drive_light(void);


// Neopixel effect library
// First Led stripe
WS2812FX driveLight = WS2812FX(NUM_LEDS_FRONT + NUM_LEDS_REAR, PIN_LEDSTRIPE, NEO_GRB + NEO_KHZ800);
WS2812FX statusLed = WS2812FX(NUM_STATUS_LEDS, PIN_STATUS_LED, NEO_RGB + NEO_KHZ800);

Button2 btnMode, btnLight, btnBlinkL, btnBlinkR;

void btnMode_click(Button2& btn){
  #ifdef SERIAL_DEBUG
    Serial.println("Mode-Button clicked");
  #endif
}
void btnMode_changed(Button2& btn){
  state.isBraking = btn.isPressed();
}

void btnLight_click(Button2& btn){
  #ifdef SERIAL_DEBUG
    Serial.println("Light-Button clicked");
  #endif

  if(!state.lightsOn){
    state.lightsOn = true;
  }else if (!state.maxlightsOn){
    state.maxlightsOn = true;
  }else{
    state.lightsOn = false;
    state.maxlightsOn = false;
  }

  #ifdef SERIAL_DEBUG
    Serial.print("Lights: ");
    Serial.println(state.lightsOn);
    Serial.print("MaxLights: ");
    Serial.println(state.maxlightsOn);
  #endif
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
  btnMode.setChangedHandler(btnMode_changed);
  
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
  #ifdef SERIAL_DEBUG
    Serial.println("Booting...");
  #endif
  setupButtons();

  // Setup WS2812FX - this is the led stripe for drive illumination
  driveLight.init();
  driveLight.setCustomMode(0, F("Blinker"), car_indicator);
  driveLight.setCustomMode(1, F("Abblendlicht"), drive_light);

  //                      index   first                               last                                      mode              color   speed   reverse
  driveLight.setSegment(  0,      0,                                  NUM_LEDS_FRONT/2 - 1,                     FX_MODE_STATIC,   BLACK,  1500,   true);
  driveLight.setSegment(  1,      NUM_LEDS_FRONT/2,                   NUM_LEDS_FRONT - 1,                       FX_MODE_STATIC,   BLACK,  1500,   false);
  driveLight.setSegment(  2,      NUM_LEDS_FRONT,                     NUM_LEDS_FRONT + (NUM_LEDS_REAR/2 - 1),   FX_MODE_STATIC,   BLACK,  1500,   true);
  driveLight.setSegment(  3,      NUM_LEDS_FRONT + NUM_LEDS_REAR / 2, NUM_LEDS_FRONT+NUM_LEDS_REAR - 1,         FX_MODE_STATIC,   BLACK,  1500,   false);
  driveLight.start();

  
  // Statusled is digital but on other pin then rest of stripe
  statusLed.init();
  statusLed.setBrightness(20);
  statusLed.setMode(FX_MODE_STATIC);
  statusLed.setSpeed(2000);
  statusLed.setColor(BLACK);
  statusLed.start();
}

bool isBlinkingL = false;
bool isBlinkingR = false;

uint16_t globalTimerInterval = 1000;
uint32_t lastTimer = 0;
bool indicatorON = false;
bool lightsON = false;
bool brakeLightON = false;

void loop() {
  loopButtons();

  if(hoverserial_receive()){
    // New data avaliable
    
    // Brake Light lights up when braking, blinks while backward drive
    state.isBraking = hoverLeds.led4;

    // Status Led
    if(hoverLeds.led1){
      // Red Led
      statusLed.setColor(0xff0000);
    }else if(hoverLeds.led2){
      // Yellow Led
      statusLed.setColor(0xff6a00);
    }else if(hoverLeds.led3){
      // Green Led
      statusLed.setColor(0x00ff00);
    }else{
      // Black Led
      statusLed.setColor(BLACK);
    }
  }

  /* Timer to keep all indicator lights in sync */
  if(isBlinkingL || isBlinkingR){
    if(millis() - lastTimer > globalTimerInterval / 2){
      indicatorON = !indicatorON;
      lastTimer = millis();
    }
  }

  uint8_t currentMode = FX_MODE_STATIC;

  if(state.lightsOn && !lightsON){
    lightsON = true;
    // Turn Lights on - only if indicator lights are not turned on at the moment
    if(!isBlinkingL){
      driveLight.setMode(LIGHT_FRONT_LEFT, FX_MODE_DRIVELIGHT);
      driveLight.setMode(LIGHT_REAR_LEFT, FX_MODE_DRIVELIGHT);
    }
    if(!isBlinkingR){
      driveLight.setMode(LIGHT_FRONT_RIGHT, FX_MODE_DRIVELIGHT);
      driveLight.setMode(LIGHT_REAR_RIGHT, FX_MODE_DRIVELIGHT);
    }
  }else if (lightsON && ! state.lightsOn){
    lightsON = false;
    // Reset Lights - only if indicator lights are not turned on at the moment
    if(!isBlinkingL){
      driveLight.setMode(LIGHT_FRONT_LEFT, FX_MODE_STATIC);
      driveLight.setMode(LIGHT_REAR_LEFT, FX_MODE_STATIC);
    }
    if(!isBlinkingR){
      driveLight.setMode(LIGHT_FRONT_RIGHT, FX_MODE_STATIC);
      driveLight.setMode(LIGHT_REAR_RIGHT, FX_MODE_STATIC);
    }
  }

  if(state.lightsOn) currentMode = FX_MODE_DRIVELIGHT;  // Max-Light gets handled in effect


  if(state.blinkL && !isBlinkingL){
    driveLight.setMode(0, FX_MODE_CAR_INDICATOR);
    driveLight.setMode(2, FX_MODE_CAR_INDICATOR);
    isBlinkingL = true;
  }else if (isBlinkingL && !state.blinkL && driveLight.isCycle(LIGHT_FRONT_LEFT)){
    driveLight.setMode(0, currentMode);
    driveLight.setMode(2, currentMode);
    isBlinkingL = false;
  }

  if(state.blinkR && !isBlinkingR){
    driveLight.setMode(1, FX_MODE_CAR_INDICATOR);
    driveLight.setMode(3, FX_MODE_CAR_INDICATOR);
    isBlinkingR = true;
  }else if (isBlinkingR && !state.blinkR && driveLight.isCycle(LIGHT_FRONT_RIGHT)){
    driveLight.setMode(1, currentMode);
    driveLight.setMode(3, currentMode);
    isBlinkingR = false;
  }

  if(state.isBraking && !brakeLightON){
    driveLight.removeActiveSegment(LIGHT_REAR_LEFT);
    driveLight.removeActiveSegment(LIGHT_REAR_RIGHT);

    for(int i = NUM_LEDS_FRONT; i < (NUM_LEDS_FRONT + NUM_LEDS_REAR); i++){
      driveLight.setPixelColor(i, COLOR_BRAKE_LIGHT);
    }
    brakeLightON = true;
  }else if(brakeLightON && !state.isBraking){
    driveLight.addActiveSegment(LIGHT_REAR_LEFT);
    driveLight.addActiveSegment(LIGHT_REAR_RIGHT);
    brakeLightON = false;
  }

  driveLight.service();
  statusLed.service();
}

uint8_t getSegmentLocation(uint16_t start){
  if(start == 0) return 0;
  if(start == NUM_LEDS_FRONT / 2) return 1;
  if(start == NUM_LEDS_FRONT) return 2;
  if(start == NUM_LEDS_FRONT + (NUM_LEDS_REAR / 2)) return 3;
  return 0;
}

bool isSegmentFront(uint16_t start){
  uint8_t seg_idx = getSegmentLocation(start);
  return seg_idx == LIGHT_FRONT_LEFT || seg_idx == LIGHT_FRONT_RIGHT;
}

bool isSegmentLeft(uint16_t start){
  uint8_t seg_idx = getSegmentLocation(start);
  return seg_idx == LIGHT_FRONT_LEFT || seg_idx == LIGHT_REAR_LEFT;
}

uint16_t drive_light(void){
  uint32_t driveLightColorFront = (state.maxlightsOn)? COLOR_MAX_LIGHT : COLOR_FRONT_LIGHT;
  uint32_t driveLightColorRear = COLOR_REAR_LIGHT;
  WS2812FX::Segment* seg = driveLight.getSegment();
  WS2812FX::Segment_runtime* segrt = driveLight.getSegmentRuntime();
  //uint16_t seglen = seg->stop - seg->start + 1;

  uint32_t segment_color = isSegmentFront(seg->start)? driveLightColorFront : driveLightColorRear;
  uint16_t updateInterval = 200;

  // Fade actual color to light
  if(segrt->counter_mode_step < 255){
    segment_color = driveLight.color_blend(driveLight.getPixelColor(seg->start), segment_color, segrt->counter_mode_step);
    segrt->counter_mode_step += 1;
    updateInterval = 1000 / 255;
  }
  for(uint16_t i=seg->start; i <= seg->stop; i++) {
    driveLight.setPixelColor(i, segment_color);
  }
  return updateInterval;
}

uint16_t car_indicator(void) {
  uint32_t indicator_color = COLOR_INDICATOR;

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