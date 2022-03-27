#ifndef LIGHT_H
#define LIGHT_H

#define LIGHT_FRONT_LEFT 0
#define LIGHT_FRONT_RIGHT 1
#define LIGHT_REAR_LEFT 3
#define LIGHT_REAR_RIGHT 2


void light_setup(void);
void light_loop(void);

uint8_t getSegmentLocation(uint16_t);
bool isSegmentFront(uint16_t);
bool isSegmentLeft(uint16_t);

uint16_t car_indicator(void);
uint16_t drive_light(void);


#endif