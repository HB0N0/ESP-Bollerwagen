#include <Arduino.h>
#include "util.h"
#include "config.h"

float batVoltage = 0.0;

const float ADC_FACTOR = (12.0f / (12.0f * ((float) R_BOTTOM / ((float) R_BOTTOM + (float) R_TOP))));
extern uint32_t currentMillis;
uint32_t lastBatVoltageRead = 0;

float measureADC(){
    float adcValue = 0.0f,realValue = 0.0f;
    // Measure 10 times for more accurancy
    for(uint16_t i = 0; i < 10; i++){
        adcValue = adcValue + analogRead(PIN_BAT);
    }
    // Calc avg value
    adcValue = adcValue / 10.0f;
    // Calc real input pin voltage
    realValue = (adcValue / 1024.0f) * 3.3f;
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