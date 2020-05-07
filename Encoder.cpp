
#include "Encoder.h"


Encoder::Encoder(uint8_t pin1, uint8_t pin2, uint8_t button) {
  this->pin1 = pin1;
  this->pin2 = pin2;
  this->button = button;
}

void Encoder::init() {
  pinMode(pin1, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);
  pinMode(button, INPUT_PULLUP);
}


void Encoder::_up() {
  if (up)
    up();
}

void Encoder::_down() {
  if (down)
    down();
}

void Encoder::_click() {
  if (click)
    click();
}

void Encoder::_doubleClick() {
  if (doubleClick)
    doubleClick();
}

void Encoder::_hold() {
  if (hold)
    hold();
}

void Encoder::readButton(unsigned long ms) {
  uint8_t b0 = digitalRead( button );
  if ( b0 != buttonStatus ) {
    buttonStatus = b0;
    if ( (buttonDownStart > 0) && (HIGH == b0) ) {
      if ( ms - buttonDownStart >= clickThreshold ) {
        if ( doubleClick ) {
          if ( (lastClick > 0) && (ms - lastClick <= doubleClickThreshold) ) {
            _doubleClick();
            lastClick = 0;
          } else {
            lastClick = ms;
          }
        } else {
          _click();
        }
      }
      buttonDownStart = 0;
    } else if ( LOW == b0 ) {
      buttonDownStart = ms;
    }
  } else if ( (b0 == LOW) && (buttonDownStart > 0) ) {
    if ( ms - buttonDownStart >= holdThreshold ) {
      _hold();
      buttonDownStart = 0;
    }
  } else if ( doubleClick
              && (b0 == HIGH)
              && (lastClick > 0)
              && (ms - lastClick > doubleClickThreshold) ) {
    _click();
    lastClick = 0;
  }
}

void Encoder::readEncoder(unsigned long ms) {

  readings[0] = digitalRead( pin1 );

  if ( readings[0] != readings[1] ) {
    readings[1] = readings[0];
    if ( HIGH == readings[0] ) {
      readings[2] = digitalRead( pin2 );
      if ( LOW == readings[2] ) {
        _up();
      } else {
        _down();
      }
    }
  }
}

void Encoder::read() {
  unsigned long r = millis();
  readButton(r);
  readEncoder(r);
}

