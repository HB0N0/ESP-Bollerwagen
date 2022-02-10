#include <Arduino.h>
#include "hoverserial.h"
#include "config.h"

SerialSideboard Sideboard;
SerialFeedback Feedback;
SerialFeedback NewFeedback;
HoverBoardLeds hoverLeds;

uint8_t idx = 0;                        // Index for new data pointer
uint16_t bufStartFrame;                 // Buffer Start Frame
byte *p;                                // Pointer declaration for the new received data
byte incomingByte;
byte incomingBytePrev;

uint32_t timeoutLastSerial;
uint8_t timeoutFlagSerial = true;


// ########################## SEND ##########################
uint32_t lastSerialCommand;
void hoverserial_handleEmergencyStop(bool emergencyStop){
    if (emergencyStop){
        if(millis() - lastSerialCommand > SEND_INTERVAL){

            // Set Switches
            uint8_t switch1 = 1;    // 2 Pos Switch || Input          0: ADC,     1: UART
            uint8_t switch2 = 0;    // 3 Pos Switch || Control Type   0: FOC,     1: SIN,     2: COM
            uint8_t switch3 = 1;    // 3 Pos Switch || Control Mode   0: VOLT,    1: SPD,     2: TORQ
            uint8_t switch4 = 0;    // 2 Pos Switch || Field W.       0: Off,     1: ON

            uint16_t cmdSwitch = (uint16_t)(switch1 | switch2 << 1 | switch3 << 3 | switch4 << 5);      
        
            // Sensor 1 and Sensor 2
            uint8_t sensor1 = 0;
            uint8_t sensor2 = 0;    

            // MPU Status - normally used to indicate if gyro sensor is working
            uint8_t mpuStatus = 0;

            // Create command
            Sideboard.start    = (uint16_t) START_FRAME;
            Sideboard.pitch     = (int16_t) 0;
            Sideboard.dPitch    = (int16_t) 0;
            Sideboard.cmd1      = (int16_t) 0;
            Sideboard.cmd2      = (int16_t) 0; 
            Sideboard.sensors   = (uint16_t)( (cmdSwitch << 8)  | (sensor1 | (sensor2 << 1) | (mpuStatus << 2)) );
            Sideboard.checksum  = (uint16_t)(Sideboard.start ^ Sideboard.pitch ^ Sideboard.dPitch ^ Sideboard.cmd1 ^ Sideboard.cmd2 ^ Sideboard.sensors);
                

            // Write to Serial
            HOVER_SERIAL.write((uint8_t *) &Sideboard, sizeof(Sideboard));
            
            lastSerialCommand = millis();
        }
    }
}

// ########################## RECEIVE ##########################
bool hoverserial_receive()
{
    bool newData = false;
    uint32_t timeNow = millis();
    
    // Handle Serial Timeout
    if(!timeoutFlagSerial   &&    timeNow - timeoutLastSerial > SERIAL_TIMEOUT){
        timeoutFlagSerial = true;
    }

    // Check for new data availability in the Serial buffer
    if (HOVER_SERIAL.available()) {
        incomingByte 	  = HOVER_SERIAL.read();                                   // Read the incoming byte
        bufStartFrame	= ((uint16_t)(incomingByte) << 8) | incomingBytePrev;       // Construct the start frame
    }
    else {
        return false;
    }

    // If DEBUG_RX is defined print all incoming bytes
    #ifdef DEBUG_RX
        Serial.print(incomingByte);
        return false;
    #endif

    // Copy received data
    if (bufStartFrame == START_FRAME) {	                    // Initialize if new data is detected
        p       = (byte *)&NewFeedback;
        *p++    = incomingBytePrev;
        *p++    = incomingByte;
        idx     = 2;	
    } else if (idx >= 2 && idx < sizeof(SerialFeedback)) {  // Save the new received data
        *p++    = incomingByte; 
        idx++;
    }	
    
    // Check if we reached the end of the package
    if (idx == sizeof(SerialFeedback)) {
        uint16_t checksum;
        checksum = (uint16_t)(NewFeedback.start ^ NewFeedback.cmd1 ^ NewFeedback.cmd2 ^ NewFeedback.speedR_meas ^ NewFeedback.speedL_meas
                            ^ NewFeedback.batVoltage ^ NewFeedback.boardTemp ^ NewFeedback.cmdLed);

        // Check validity of the new data
        if (NewFeedback.start == START_FRAME && checksum == NewFeedback.checksum) {
            // Copy the new data
            memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback));
            hoverserial_handleLeds();

            timeoutFlagSerial = false;
            timeoutLastSerial = timeNow;

            newData = true; // function returns true
            #ifdef SERIAL_DEBUG
                // Print data to built-in Serial
                Serial.print("cmd1: ");   Serial.print(Feedback.cmd1);
                Serial.print(" cmd2: ");  Serial.print(Feedback.cmd2);
                Serial.print(" speedR: ");  Serial.print(Feedback.speedR_meas);
                Serial.print(" speedL: ");  Serial.print(Feedback.speedL_meas);
                Serial.print(" bat: ");  Serial.print(Feedback.batVoltage);
                Serial.print(" temp: ");  Serial.print(Feedback.boardTemp);
                Serial.print(" cmdLed: ");  Serial.println(Feedback.cmdLed);
            #endif
        } else {
            #ifdef SERIAL_DEBUG
                Serial.println("Non-valid data skipped");
            #endif
        }
        idx = 0;    // Reset the index (it prevents to enter in this if condition in the next cycle)
    }

    // Update previous states
    incomingBytePrev = incomingByte;
    return newData;
}

/**
* Handles cmdLed byte on hoverboard feedback and reads led values to hoverLeds struct
*/
void hoverserial_handleLeds(){
    hoverLeds.led1 = (Feedback.cmdLed & LED1_SET);
    hoverLeds.led2 = (Feedback.cmdLed & LED2_SET);
    hoverLeds.led3 = (Feedback.cmdLed & LED3_SET);
    hoverLeds.led4 = (Feedback.cmdLed & LED4_SET);
    hoverLeds.led5 = (Feedback.cmdLed & LED5_SET);
}

// ########################## LOOP ##########################
/*
unsigned long iTimeSend = 0;
int iTest = 0;
int iStep = SPEED_STEP;

void loop(void)
{ 
  unsigned long timeNow = millis();

  // Check for new received data
  Receive();

  // Send commands
  if (iTimeSend > timeNow) return;
  iTimeSend = timeNow + TIME_SEND;
  Send(0, iTest);

  // Calculate test command signal
  iTest += iStep;

  // invert step if reaching limit
  if (iTest >= SPEED_MAX_TEST || iTest <= -SPEED_MAX_TEST)
    iStep = -iStep;

  // Blink the LED
  digitalWrite(LED_BUILTIN, (timeNow%2000)<1000);
}
*/
// ########################## END ##########################