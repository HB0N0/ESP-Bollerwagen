#include <Arduino.h>
#include <WS2812FX.h>
#include "config.h"
#include "defines.h"
#include "light.h"

#define FX_MODE_CAR_INDICATOR FX_MODE_CUSTOM
#define FX_MODE_DRIVELIGHT FX_MODE_CUSTOM_1

extern CarState state;

WS2812FX driveLight = WS2812FX(NUM_LEDS_FRONT + NUM_LEDS_REAR, PIN_LEDSTRIPE, NEO_GRB + NEO_KHZ800);


bool isBlinkingL = false;
bool isBlinkingR = false;

extern uint32_t currentMillis;
uint16_t globalTimerInterval = 1000;
uint32_t lastTimer = 0;
bool indicatorON = false;
bool lightsON = false;
bool brakeLightON = false;

void light_setup(){
    // Setup WS2812FX - this is the led stripe for drive illumination
    driveLight.init();
    driveLight.setCustomMode(0, F("Blinker"), car_indicator);
    driveLight.setCustomMode(1, F("Abblendlicht"), drive_light);

    // Drive light segments index                 first                               last                                      mode              color   speed   reverse
    driveLight.setSegment(  LIGHT_FRONT_LEFT,      0,                                  NUM_LEDS_FRONT/2 - 1,                     FX_MODE_STATIC,   BLACK,  1500,   true);
    driveLight.setSegment(  LIGHT_FRONT_RIGHT,     NUM_LEDS_FRONT/2,                   NUM_LEDS_FRONT - 1,                       FX_MODE_STATIC,   BLACK,  1500,   false);
    driveLight.setSegment(  LIGHT_REAR_RIGHT,      NUM_LEDS_FRONT,                     NUM_LEDS_FRONT + (NUM_LEDS_REAR/2 - 1),   FX_MODE_STATIC,   BLACK,  1500,   true);
    driveLight.setSegment(  LIGHT_REAR_LEFT,       NUM_LEDS_FRONT + NUM_LEDS_REAR / 2, NUM_LEDS_FRONT+NUM_LEDS_REAR - 1,         FX_MODE_STATIC,   BLACK,  1500,   false);
    driveLight.start();
}
void light_loop(){
    /* Timer to keep all indicator lights in sync */
    if(isBlinkingL || isBlinkingR){
        if(currentMillis - lastTimer > globalTimerInterval / 2){
            indicatorON = !indicatorON;
            lastTimer = currentMillis;
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
        driveLight.setMode(LIGHT_FRONT_LEFT, FX_MODE_CAR_INDICATOR);
        driveLight.setMode(LIGHT_REAR_LEFT, FX_MODE_CAR_INDICATOR);
        isBlinkingL = true;
    }else if (isBlinkingL && !state.blinkL && driveLight.isCycle(LIGHT_FRONT_LEFT)){
        driveLight.setMode(LIGHT_FRONT_LEFT, currentMode);
        driveLight.setMode(LIGHT_REAR_LEFT, currentMode);
        isBlinkingL = false;
    }

    if(state.blinkR && !isBlinkingR){
        driveLight.setMode(LIGHT_FRONT_RIGHT, FX_MODE_CAR_INDICATOR);
        driveLight.setMode(LIGHT_REAR_RIGHT, FX_MODE_CAR_INDICATOR);
        isBlinkingR = true;
    }else if (isBlinkingR && !state.blinkR && driveLight.isCycle(LIGHT_FRONT_RIGHT)){
        driveLight.setMode(LIGHT_FRONT_RIGHT, currentMode);
        driveLight.setMode(LIGHT_REAR_RIGHT, currentMode);
        isBlinkingR = false;
    }

    if(state.isBraking && !brakeLightON){
        // prevents WS8266FX from updating this segment and overwriting brake lights
        driveLight.removeActiveSegment(LIGHT_REAR_LEFT);
        driveLight.removeActiveSegment(LIGHT_REAR_RIGHT);

        for(int i = NUM_LEDS_FRONT; i < (NUM_LEDS_FRONT + NUM_LEDS_REAR); i++){
            driveLight.setPixelColor(i, COLOR_BRAKE_LIGHT);
        }
        // force new frame update to make sure the brake gets immediately visible
        driveLight.trigger();
        brakeLightON = true;

    }else if(brakeLightON && !state.isBraking){
        driveLight.addActiveSegment(LIGHT_REAR_LEFT);
        driveLight.addActiveSegment(LIGHT_REAR_RIGHT);
        brakeLightON = false;
    }
    driveLight.service();
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