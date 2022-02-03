#include <Arduino.h>
#include "hoverserial.h"
#include "config.h"

SerialCommand Command;
SerialFeedback Feedback;
SerialFeedback NewFeedback;
HoverBoardLeds HoverLeds;

uint8_t idx = 0;                        // Index for new data pointer
uint16_t bufStartFrame;                 // Buffer Start Frame
byte *p;                                // Pointer declaration for the new received data
byte incomingByte;
byte incomingBytePrev;


// ########################## SEND ##########################
void hoverserial_send(){
  // Create command
  Command.start    = (uint16_t)START_FRAME;
  //Command.steer    = (int16_t)uSteer;
  //Command.speed    = (int16_t)uSpeed;
  Command.checksum = (uint16_t)(Command.start ^ Command.steer ^ Command.speed);

  // Write to Serial
  HOVER_SERIAL.write((uint8_t *) &Command, sizeof(Command)); 
}

// ########################## RECEIVE ##########################
bool hoverserial_receive()
{
    bool newData = false;
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

void hoverserial_handleLeds(){
    HoverLeds.led1 = (Feedback.cmdLed & LED1_SET);
    HoverLeds.led2 = (Feedback.cmdLed & LED2_SET);
    HoverLeds.led3 = (Feedback.cmdLed & LED3_SET);
    HoverLeds.led4 = (Feedback.cmdLed & LED4_SET);
    HoverLeds.led5 = (Feedback.cmdLed & LED5_SET);
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