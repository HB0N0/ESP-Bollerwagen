#include <Arduino.h>
#include "util.h"
#include "config.h"

float batVoltage = 0.0;

extern uint32_t currentMillis;
uint32_t lastBatVoltageRead = 0;

float measureADC(){
    float adcValue = 0.0,realValue = 0.0;
    // Measure 10 times for more accurancy
    for(uint16_t i = 0; i < 10; i++){
        adcValue += analogRead(PIN_BAT);
    }
    // Calc avg value
    adcValue = adcValue / 10.0;
    // Calc real input pin voltage
    realValue = (adcValue / 1024.0) * 3.3;
    // return battery voltage (factor depends on voltage divider)
    return realValue * ADC_FACTOR;
}

void readBatteryVoltage(){
    // Read battery voltage in configured intervall
    if(currentMillis - lastBatVoltageRead > BAT_READ_INTERVAL){
        batVoltage = measureADC();
        lastBatVoltageRead = currentMillis;
    }
}