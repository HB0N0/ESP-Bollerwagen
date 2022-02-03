#ifndef HOVERSERIAL_H
#define HOVERSERIAL_H

#define LED1_SET                    (0x01)
#define LED2_SET                    (0x02)
#define LED3_SET                    (0x04)
#define LED4_SET                    (0x08)
#define LED5_SET                    (0x10)


typedef struct{
   uint16_t start;
   int16_t  steer;
   int16_t  speed;
   uint16_t checksum;
} SerialCommand;

typedef struct{
   uint16_t start;
   int16_t  cmd1;
   int16_t  cmd2;
   int16_t  speedR_meas;
   int16_t  speedL_meas;
   int16_t  batVoltage;
   int16_t  boardTemp;
   uint16_t cmdLed;
   uint16_t checksum;
} SerialFeedback;

typedef struct{
    /// Battery red
    bool led1;
    /// Battery yellow
    bool led2; 
    /// Battery green
    bool led3; 
    /// Bottom blue led (on IF motors enabled; ELSE blinking)
    bool led4; 
    /// Upper blue led (blinking IF backward drive; on IF braking; ELSE off)
    bool led5; 
} HoverBoardLeds;

void hoverserial_send(void);
bool hoverserial_receive(void);
void hoverserial_handleLeds(void);


#endif