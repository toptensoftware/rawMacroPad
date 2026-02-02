#pragma once

#include "Encoder.h"

#define _countof(array) (sizeof(array) / sizeof(array[0]))

const int rowPins[] = { PB0, PB10, PB2, PB1 };
const int colPins[] = { PA0, PA1, PA3, PA2, PA4 };

const int numRows = _countof(rowPins);
const int numCols = _countof(colPins);
const int numKeys = numRows * numCols;

const int keymap[numRows][numCols] = {
  { 0, 4, 8, 12, 19},
  { 1, 5, 9, 13, 18 },
  { 2, 6, 10, 14, 17 },
  { 3, 7, 11, 15, 16 },
};

#define LAYER_COLOR_BLACK   0b000, // 0
#define LAYER_COLOR_BLUE    0b001, // 1
#define LAYER_COLOR_GREEN   0b010, // 2
#define LAYER_COLOR_CYAN    0b011, // 3
#define LAYER_COLOR_RED     0b100, // 4
#define LAYER_COLOR_MAGENTA 0b101, // 5
#define LAYER_COLOR_YELLOW  0b110, // 6
#define LAYER_COLOR_WHITE   0b111  // 7

#define UNDERGLOW_BACK_RIGHT 0
#define UNDERGLOW_BACK_CENTER 1
#define UNDERGLOW_BACK_LEFT 2
#define UNDERGLOW_FRONT_RIGHT 3
#define UNDERGLOW_FRONT_CENTER 4
#define UNDERGLOW_FRONT_LEFT 5

class KM16
{
public:
  KM16()
  {
  }

  void setup()
  {
    // Basic GPIO init
    pinMode(PB3, OUTPUT);
    digitalWrite(PB3, HIGH);

    // Keypad Matrix
    for (uint8_t i = 0; i < numRows; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH);
    }
    for (uint8_t i = 0; i < numCols; i++) {
        pinMode(colPins[i], INPUT_PULLUP);
    }

    // Encoders
    _enc0.setup(PA7, PC15, enc0_callback, this);
    _enc1.setup(PC14, PA6, enc1_callback, this);
    _enc2.setup(PA5, PC13, enc2_callback, this);

    // Level LEDs
    pinMode(PB6, OUTPUT);
    pinMode(PB7, OUTPUT);
    pinMode(PB8, OUTPUT);
    digitalWrite(PB6, HIGH);
    digitalWrite(PB7, HIGH);
    digitalWrite(PB8, HIGH);
}

  void update()
  {
    // Poll encoders
    _enc0.poll();
    _enc1.poll();
    _enc2.poll();

    // Poll keyboard matrix
    pollMatrix();

  }

  virtual void onKey(int key, bool pressed)
  {

  }

  virtual void onEncoder(int index, int delta)
  {

  }

  void setLayerColor(int color)
  {
    if (_layerColor == color)
      return;
    _layerColor = color;
    digitalWrite(PB8, (color & 0x4) ? LOW : HIGH);
    digitalWrite(PB6, (color & 0x2) ? LOW : HIGH);
    digitalWrite(PB7, (color & 0x1) ? LOW : HIGH);    
  }

  int getLayerColor()
  {
    return _layerColor;
  }


private:
  struct KeyState
  {
    bool pressed;
    unsigned long time;
  };

  KeyState _keyState[numKeys] = { 0 };
  Encoder _enc0;
  Encoder _enc1;
  Encoder _enc2;

  int _layerColor = 0;



  static void enc0_callback(void* ctx, int delta)
  {
    ((KM16*)ctx)->onEncoder(0,  delta);
  }

  static void enc1_callback(void* ctx, int delta)
  {
    ((KM16*)ctx)->onEncoder(1,  delta);
  }

  static void enc2_callback(void* ctx, int delta)
  {
    ((KM16*)ctx)->onEncoder(2,  delta);
  }

  void pollMatrix()
  {
    // Read 
    unsigned long now = millis();

    for (uint8_t row = 0; row < numRows; row++) 
    {
        digitalWrite(rowPins[row], LOW);
        delayMicroseconds(10);
        for (uint8_t col = 0; col < numCols; col++)
        {
            int key = keymap[row][col];
            KeyState* pks = &_keyState[key];
            bool pressed = !digitalRead(colPins[col]);

            if (pressed != pks->pressed)
            {
              if (now - pks->time >= 5)
              {
                pks->pressed = pressed;
                pks->time = now;
                onKey(key, pressed);
              }
            }
        }
        digitalWrite(rowPins[row], HIGH);
    }    
  }

};

