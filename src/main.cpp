#include <Arduino.h>
#include <WS2812FX.h>
#include <Button2.h>
#include "config.h"
#include "defines.h"
#include "util.h"
#include "light.h"
#include "hoverserial.h"

/* Holds current state of the Car -
   these values get updated by user inputs (like button press)
  and the serial feedback of the hoverboard mainboard */
CarState state;

/* Serial variables (used here for updating the status led) */
extern HoverBoardLeds hoverLeds;
extern uint16_t timeoutFlagSerial;

/* Real battery voltage calculated from ADC reading (in our case the Voltage of a 12V car battery)*/
extern float batVoltage;

/* for performance reasons current millis gets updated only once per loop by calling millis() */
uint32_t currentMillis;


// Neopixel effect library is also used for the status led
// Notice: the effects and logic of the "drive lights" is located in "lights.cpp"
WS2812FX statusLed = WS2812FX(NUM_STATUS_LEDS, PIN_STATUS_LED, NEO_RGB + NEO_KHZ800);

// Init Buttons
Button2 btnMode, btnLight, btnBlinkL, btnBlinkR, btnEmergencyStop;

void btnMode_click(Button2& btn){
  #ifdef SERIAL_DEBUG
    Serial.println("Mode-Button clicked");
  #endif
}

void btnMode_changed(Button2& btn){
  //state.isBraking = btn.isPressed();
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

void handleIndicatorSwitch(Button2& btn){
  if(btn.isPressed() && btnMode.isPressed()){
    // Hazard Lights
    state.blinkR = true;
    state.blinkL = true;
    state.hazardLights = true;
  }else{
    if(state.hazardLights){
      state.hazardLights = false;
    }
    state.blinkL = btnBlinkL.isPressed();
    state.blinkR = btnBlinkR.isPressed();
  }
}
void btnBlinkL_changed(Button2& btn){
  handleIndicatorSwitch(btn);
}
void btnBlinkR_changed(Button2& btn){
  handleIndicatorSwitch(btn);
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


uint16_t statusLedCnt = 0; // counter increases by one every 10 ms
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

      if(btnMode.isPressed()){
        // Display battery status of the 12V battery while mode button is presed
        if(batVoltage < BAT_DEAD){
          // Dead
          if(statusLedCnt % 1 == 0)   toggleColor(statusLed, RED); // Blink fast red
        }else if (batVoltage < BAT_LVL1){
          // 0%
          if(statusLedCnt % 30 == 0)  toggleColor(statusLed, RED); // Blink normal red
        }else if (batVoltage < BAT_LVL2){
          // 20%
          statusLed.setColor(RED); // red
        }else if (batVoltage < BAT_LVL3){
          // 40%
          if(statusLedCnt % 30 == 0)  toggleColor(statusLed, 0xff6a00); // Blink normal orange
        }else if (batVoltage < BAT_LVL4){
          // 60%
          statusLed.setColor(0xff6a00); // orange
        }else if (batVoltage < BAT_LVL5){
          // 80 %
          if(statusLedCnt % 30 == 0)  toggleColor(statusLed, GREEN); // Blink normal green
        }else { 
          // 100%
          statusLed.setColor(GREEN); // green
        }
      }
      else if(timeoutFlagSerial){
        // In case of serial timeout (the hoverboard sends no data) blink led blue twice every 2s
        if(statusLedCnt % 200 == 0 || statusLedCnt % 200 == 10)  statusLed.setColor(BLUE);
        if(statusLedCnt % 200 == 5 || statusLedCnt % 200 == 15)  statusLed.setColor(BLACK);
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
    state.isBraking = hoverLeds.led5;
  }
  if(timeoutFlagSerial && state.isBraking){
    // Reset brake lights on serial timeout
    state.isBraking = false;
  }
  readBatteryVoltage();
  handleStatusLed();
  light_loop();    
  hoverserial_handleEmergencyStop(state.emergencyStop);

}

