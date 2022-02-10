#include <Arduino.h>
#include <WS2812FX.h>
#include <Button2.h>
#include "config.h"
#include "light.h"
#include "hoverserial.h"


struct CarState{
  bool  isBraking;
  bool  lightsOn;
  bool  maxlightsOn;
  bool  blinkL;
  bool  blinkR;
  bool  emergencyStop;
} state;

extern HoverBoardLeds hoverLeds;
extern uint16_t timeoutFlagSerial;

uint32_t currentMillis;


// Neopixel effect library is also used for the status led
WS2812FX statusLed = WS2812FX(NUM_STATUS_LEDS, PIN_STATUS_LED, NEO_RGB + NEO_KHZ800);

Button2 btnMode, btnLight, btnBlinkL, btnBlinkR, btnEmergencyStop;

void btnMode_click(Button2& btn){
  #ifdef SERIAL_DEBUG
    Serial.println("Mode-Button clicked");
  #endif
}

void btnMode_changed(Button2& btn){
  state.isBraking = btn.isPressed();
}

void btnEmergencyStop_changed(Button2& btn){
  #ifdef SERIAL_DEBUG
    Serial.print("Emergencystop: ");
    Serial.println(btn.isPressed() == false);
  #endif
  state.emergencyStop = btn.isPressed() == false;
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

  btnEmergencyStop.begin(PIN_NC_EMERGENCY);
  btnEmergencyStop.setChangedHandler(btnEmergencyStop_changed);
  
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
  btnEmergencyStop.loop();
}

uint16_t statusLedCnt = 0;
uint32_t statusLedLastUpdate = 0;

void toggleColor(WS2812FX& stripe, uint32_t color1){
  if(stripe.getColor() != BLACK){
    stripe.setColor(BLACK);
  }else{
    stripe.setColor(color1);
  }
}

void handleStatusLed(){
    // Status led gets updated every 10 ms
    if(currentMillis - statusLedLastUpdate > 10){
      statusLedCnt ++;
      statusLedLastUpdate = currentMillis;

      if(timeoutFlagSerial){
        // In case of serial timeout (the hoverboard sends no data) blink led blue every 120ms
        if(statusLedCnt % 12 == 0)  toggleColor(statusLed, BLUE);
      }
      else if(state.emergencyStop){
        // In case of emergency stop blink led red every 500ms
        if(statusLedCnt % 50 == 0)  toggleColor(statusLed, RED);
      }
      else if (!timeoutFlagSerial){
        // Normal case, hoverboard is connected
        if(hoverLeds.led1){
          // Red Led - Low battery value or error
          statusLed.setColor(0xff0000);
        }else if(hoverLeds.led2){
          // Yellow Led - Medium battery value
          statusLed.setColor(0xff6a00);
        }else if(hoverLeds.led3){
          // Green Led - Battery full
          statusLed.setColor(0x00ff00);
        }else{
          // Black Led - turn led off (the hoverboard toggles the led values to blink)
          statusLed.setColor(BLACK);
        }
      }
    }
    statusLed.service();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  #ifdef SERIAL_DEBUG
    Serial.println("Booting...");
  #endif
  setupButtons();
  
  light_setup();
  // Statusled is digital but on other pin then rest of stripe
  statusLed.init();
  statusLed.setBrightness(20);
  statusLed.setMode(FX_MODE_STATIC);
  statusLed.setSpeed(10); //IMPORTANT
  statusLed.setColor(PINK);
  statusLed.start();
}


void loop() {
  currentMillis = millis();
  loopButtons();

  // is new feedback data avaliable ?
  bool newData = hoverserial_receive();
  if(newData){
    // Brake Light lights up when braking, blinks while backward drive
    state.isBraking = hoverLeds.led4;
  }
  if(timeoutFlagSerial && state.isBraking){
    // Reset brake lights on serial timeout
    state.isBraking = false;
  }

  handleStatusLed();
  light_loop();    
  hoverserial_handleEmergencyStop(state.emergencyStop);

}

