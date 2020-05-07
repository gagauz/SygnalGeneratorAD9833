#include "AD9833.h"
#include "Encoder.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

typedef enum { MOVE, BLINK} Mode;
typedef int (*GeneralFunction) (const int8_t increment);


// Defines

#define DISPLAY_TYPE 1602

//#define DEBUG_MODE 1
#ifdef DEBUG_MODE
#define DEBUG(info) Serial.println(info);
#else
#define DEBUG(info)
#endif

#define FNC_PIN 4

#define MIN_FREQ        0.001f
#define MAX_FREQ 12500000.00f

#define MIN_PHASE   0.00f
#define MAX_PHASE 360.00f

#define DISPLAY_ADDR 0x27

// Variables

LiquidCrystal_I2C lcd(DISPLAY_ADDR, 16, 2);
AD9833 gen(FNC_PIN);
Encoder encoder(A1, A0, A2);

Mode mode = MOVE;

WaveformType waveforms[] = {SINE_WAVE, TRIANGLE_WAVE, SQUARE_WAVE, HALF_SQUARE_WAVE};

// [0123456789ABCDEF]
// [0 10000000.00 Hz]
// [WaveForm 180.00']

uint8_t* movements[2][16] = {
  {1, 0, 1, 9, 9, 9, 9, 9, 9, 9, 0, 9, 9, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 9, 9, 0, 9, 9, 0}
};

int8_t multiplier[2][16] = {
  {0, 0, 7, 6, 5, 4, 3, 2, 1, 0, 0, -1, -2, 0, 0, 0},
  {3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0, 0, -1, -2, 0}
};



char* prefixes[] = {
  "       ",    //         1 Hz and 0.10 Hz
  "      ",     //        10 Hz
  "     ",      //       100 Hz
  "    ",       //     1 000 Hz
  "   ",        //    10 000 Hz
  "  ",         //   100 000 Hz
  " ",          // 1 000 000 Hz
  ""            //12 500 000 Hz
};

char* prefixes1[] = {
  "  ",  //  1.01
  " ",   // 60.00
  ""     // 360.00
};

Registers curReg = REG0;
int8_t curX = 0;
int8_t curY = 0;
float frequency = 5000.0;
float phase = 0.0;
int8_t waveForm = 0;
boolean redraw = true;

void setNone(int8_t increment) {
  DEBUG("None function");
}

void setRegister(int8_t increment) {
  DEBUG("Change register");
  if (curReg == REG0)
    curReg = REG1;
  else
    curReg = REG0;
  gen.SetOutputSource(curReg);
  readValues();

  redraw = true;
}

void setFreq(int8_t increment) {
  float mult = increment * pow(10, multiplier[curY][curX]);
  uint8_t c = frequency / mult;
  //  DEBUG(frequency);
  //  DEBUG(mult);
  //  DEBUG("c");
  //  DEBUG(c);
  //  if (increment > 0 || c > 1)
  frequency += mult;
  if (frequency > MAX_FREQ)
    frequency = MAX_FREQ;
  else if (frequency < MIN_FREQ)
    frequency = MIN_FREQ;
  redraw = true;
}

void setWaveform(int8_t increment) {
  waveForm += increment;
  if (waveForm > 3) waveForm = 0;
  if (waveForm < 0) waveForm = 3;
  redraw = true;
}

void setPhase(int8_t increment) {
  DEBUG("Change phase");
  float mult = pow(10, multiplier[curY][curX]);
  phase += increment * mult;
  if (phase > MAX_PHASE)
    phase = MAX_PHASE;
  else if (phase < MIN_PHASE)
    phase = MIN_PHASE;

  redraw = true;
}

const GeneralFunction handlers[][16] = {
  {setRegister, setNone, setFreq, setFreq, setFreq, setFreq, setFreq, setFreq, setFreq, setFreq, setNone, setFreq, setFreq, setNone, setNone, setNone},
  {setWaveform, setNone, setNone, setNone, setNone, setNone, setNone, setNone, setNone, setPhase, setPhase, setPhase, setNone, setPhase, setPhase, setNone},
};

void checkXYLimits() {
  if (curX > 15) {
    curX = 0;
    curY++;
  } else if (curX < 0) {
    curX = 15;
    curY--;
  }

  if (curY > 1) {
    curY = 0;
  } else if (curY < 0) {
    curY = 1;
  }
}

void readValues() {
  frequency = gen.GetFrequency(curReg);
  phase = gen.GetPhase(curReg);
  WaveformType w = gen.GetWaveform(curReg);
  for (waveForm = 0; waveforms[waveForm] != w; waveForm++);
}

void applyValues() {
  gen.SetPhase(curReg, phase);
  gen.SetFrequency(curReg, frequency);
  gen.SetWaveform(curReg, waveforms[waveForm]);
}

void skipUnusedChars(int8_t increment) {
  checkXYLimits();
  while (movements[curY][curX] == 0) {
    curX += increment;
    checkXYLimits();
  }
}

void doEncoder(int8_t value) {
  if (value != 0) {
    if (mode == BLINK) {
      handlers[curY][curX](value);
    } else {
      curX += value;
      skipUnusedChars(value);
      lcd.setCursor(curX, curY);
      lcd.cursor();
      DEBUG(curX);
      DEBUG(", ");
      DEBUG(curY);
    }
  }
  redraw = true;
}

void encUp() {
  DEBUG("Up ");
  doEncoder(1);
}

void encDown() {
  DEBUG("down ");
  doEncoder(-1);
}

void encClick() {
  DEBUG("clicked ");
  if (mode == MOVE) {
    mode = BLINK;
    lcd.blink();
  } else {
    applyValues();
    mode = MOVE;
    lcd.noBlink();
  }
  redraw = true;
}

void encHold() {
  DEBUG(F("Encoder hold"));
  setRegister(1);
}

int8_t log10_2(double value) {
  int8_t prefixIdx = log10(value);
  if (prefixIdx < 0) prefixIdx = 0;
  return prefixIdx;
}

void draw() {
  if (redraw) {

    //    ------------------------
    //     Print Current Register
    //    ------------------------
    lcd.setCursor(0, 0);
    lcd.print(curReg);


    //    ------------------------
    //     Print Current Frequency
    //    ------------------------
    int8_t prefixIdx = log10_2(frequency);

    lcd.setCursor(2, 0);
    lcd.print(prefixes[prefixIdx]);

    lcd.print(frequency);
    lcd.print(F(" Hz"));

    //    ------------------------
    //     Print Current waveform
    //    ------------------------
    lcd.setCursor(0, 1);

    switch (waveforms[waveForm]) {
      case SINE_WAVE:
        lcd.print(F("Sine       "));
        break;
      case TRIANGLE_WAVE:
        lcd.print(F("Triangle   "));
        break;
      case SQUARE_WAVE:
        lcd.print(F("Square     "));
        break;
      case HALF_SQUARE_WAVE:
        lcd.print(F("Square/2   "));
        break;
    }

    //    ------------------------
    //     Print Current Phase
    //    ------------------------
    lcd.setCursor(9, 1);
    prefixIdx = log10_2(phase);
    lcd.print(prefixes1[prefixIdx]);
    lcd.print(phase);
    lcd.print(F("'"));
    //

    redraw = false;
    lcd.setCursor(curX, curY);
    lcd.cursor();
    if (mode == BLINK) {
      lcd.blink();
    }
  }
}

void setup() {

  gen.Begin();
  gen.SetFrequency(curReg, frequency);
  gen.EnableOutput(true);
  gen.SetOutputSource(curReg);

  lcd.begin();
  lcd.backlight();
  lcd.clear();

#ifdef DEBUG_MODE
  Serial.begin(9600);
#endif

  encoder.up = encUp;
  encoder.down = encDown;
  encoder.hold = encHold;
  encoder.click = encClick;

  encoder.init();
}

void loop() {
  encoder.read();
  draw();
}
