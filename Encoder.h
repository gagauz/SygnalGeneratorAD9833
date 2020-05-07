#ifndef ENCODER_H
#define ENCODER_H

#include "arduino.h"

#define NONE {}; 

typedef void (*VoidFunction) ();

class Encoder {
  private:
    uint8_t pin1;
    uint8_t pin2;
    uint8_t button;

    uint8_t readings[3] = {HIGH, HIGH, HIGH};
    uint8_t buttonStatus = HIGH;
    unsigned long buttonDownStart = 0;
    unsigned long lastClick = 0;
   
    void readButton(unsigned long ms);
    void readEncoder(unsigned long ms);

    void _up();
    void _down();
    void _click();
    void _doubleClick();
    void _hold();
    
  public:
    uint16_t holdThreshold = 600;
    uint16_t doubleClickThreshold = 400;
    uint16_t clickThreshold = 10;
    
    (*up)();
    (*down)();
    (*click)();
    (*doubleClick)();
    (*hold)();
    
    Encoder(uint8_t pin1, uint8_t pin2, uint8_t pin3);
    void init();
    void setButtonPin(uint8_t pin);
    void setEncoderPin1(uint8_t pin);
    void setEncoderPin2(uint8_t pin);
    void read();
};

#endif
