#pragma once

#include "Encoder.h"

#define _countof(array) (sizeof(array) / sizeof(array[0]))

const int rowPins[] = {PB0, PB10, PB2, PB1};
const int colPins[] = {PA0, PA1, PA3, PA2, PA4};

const int numRows = _countof(rowPins);
const int numCols = _countof(colPins);
const int numKeys = numRows * numCols;

const int keymap[numRows][numCols] = {
    {0, 4, 8, 12, 19},
    {1, 5, 9, 13, 18},
    {2, 6, 10, 14, 17},
    {3, 7, 11, 15, 16},
};

#define INDICATOR_COLOR_BLACK 0b000   // 0
#define INDICATOR_COLOR_BLUE 0b001    // 1
#define INDICATOR_COLOR_GREEN 0b010   // 2
#define INDICATOR_COLOR_CYAN 0b011    // 3
#define INDICATOR_COLOR_RED 0b100     // 4
#define INDICATOR_COLOR_MAGENTA 0b101 // 5
#define INDICATOR_COLOR_YELLOW 0b110  // 6
#define INDICATOR_COLOR_WHITE 0b111   // 7

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
        // Keypad Matrix
        for (uint8_t i = 0; i < numRows; i++)
        {
            pinMode(rowPins[i], OUTPUT);
            digitalWrite(rowPins[i], HIGH);
        }
        for (uint8_t i = 0; i < numCols; i++)
        {
            pinMode(colPins[i], INPUT_PULLUP);
        }

        // Encoders
        _enc0.setup(PA7, PC15, enc0_callback, this);
        _enc1.setup(PC14, PA6, enc1_callback, this);
        _enc2.setup(PA5, PC13, enc2_callback, this);

        // LED Power
        pinMode(PB14, OUTPUT);
        digitalWrite(PB14, LOW); // Power disable to start with

        // Level LEDs
        pinMode(PB6, OUTPUT);
        pinMode(PB7, OUTPUT);
        pinMode(PB8, OUTPUT);
        digitalWrite(PB6, HIGH);
        digitalWrite(PB7, HIGH);
        digitalWrite(PB8, HIGH);

        // Configure underglow
        GPIOB->CRH &= ~(0xF << 4); // Configure PB9 as 50MHz push-pull output
        GPIOB->CRH |= (0x3 << 4);
        GPIOB->BRR = (1 << 9);
        pinMode(PB12, OUTPUT);    // Power enable pin
        digitalWrite(PB12, HIGH); // Power enable (active LOW)

        // Per-key LEDs
        GPIOB->CRH &= ~(0xF << 12); // Configure PB11 as 50MHz push-pull output
        GPIOB->CRH |= (0x3 << 12);
        GPIOB->BRR = (1 << 11);
        pinMode(PB13, OUTPUT);
        digitalWrite(PB13, HIGH); // Output blank (active HIGH, ie: set LOW to unblank)
    }

    void update()
    {
        // Poll encoders
        _enc0.poll();
        _enc1.poll();
        _enc2.poll();

        // Poll keyboard matrix
        pollMatrix();

        // Update underglow leds
        if (_underglowDirty)
        {
            sendUnderglowPixels();
            _underglowDirty = false;
        }

        // Update key leds
        if (_keyledsDirty)
        {
            sendKeyLedPixels();
            _keyledsDirty = false;
        }
    }

    virtual void onKey(int key, bool pressed)
    {
    }

    virtual void onEncoder(int index, int delta)
    {
    }

    void setEnableLeds(bool enable)
    {
        if (_enableLeds == enable)
            return;
        _enableLeds = enable;
        digitalWrite(PB14, _enableLeds ? HIGH : LOW); // Power enable

        if (_enableLeds)
        {
            delay(30);
            if (_underglowEnabled)
            {
                sendUnderglowPixels();
                _underglowDirty = false;
            }

            if (_keyledsEnabled)
            {
                sendKeyLedPixels();
                _keyledsDirty = false;
            }
        }
    }

    bool getEnableLeds()
    {
        return _enableLeds;
    }

    void setIndicatorColor(int color)
    {
        if (_indicatorColor == color)
            return;
        _indicatorColor = color;
        digitalWrite(PB8, (color & 0x4) ? LOW : HIGH);
        digitalWrite(PB6, (color & 0x2) ? LOW : HIGH);
        digitalWrite(PB7, (color & 0x1) ? LOW : HIGH);
    }

    int getIndicatorColor()
    {
        return _indicatorColor;
    }

    void setEnableUnderglow(bool enable)
    {
        if (_underglowEnabled == enable)
            return;

        digitalWrite(PB12, enable ? LOW : HIGH);

        _underglowEnabled = enable;
        _underglowDirty = false;
        if (enable)
            sendUnderglowPixels();
    }

    bool getEnableUnderglow()
    {
        return _underglowEnabled;
    }

    void setUnderglow(unsigned long rgb)
    {
        for (int i = 0; i < 6; i++)
        {
            setUnderglow(i, rgb);
        }
    }

    void setUnderglow(int index, unsigned long rgb)
    {
        if (_underglow[index] == rgb)
            return;
        _underglow[index] = rgb;
        if (_underglowEnabled)
            _underglowDirty = true;
    }

    unsigned long getUnderglow(int index)
    {
        return _underglow[index];
    }

    void setEnableKeyLeds(bool enable)
    {
        if (_keyledsEnabled == enable)
            return;

        // digitalWrite(PB14, enable ? HIGH : LOW);
        digitalWrite(PB13, enable ? LOW : HIGH);

        _keyledsEnabled = enable;
        _keyledsDirty = false;
        if (enable)
            sendKeyLedPixels();
    }

    bool getEnableKeyLeds()
    {
        return _keyledsEnabled;
    }

    void setKeyLeds(unsigned long rgb)
    {
        for (int i = 0; i < 16; i++)
        {
            setKeyLed(i, rgb);
        }
    }

    void setKeyLed(int index, unsigned long rgb)
    {
        if (_keyleds[index] == rgb)
            return;
        _keyleds[index] = rgb;
        if (_keyledsEnabled)
            _keyledsDirty = true;
    }

    unsigned long getKeyLed(int index)
    {
        return _keyleds[index];
    }

private:
    struct KeyState
    {
        bool pressed;
        unsigned long time;
    };

    KeyState _keyState[numKeys] = {0};
    Encoder _enc0;
    Encoder _enc1;
    Encoder _enc2;

    bool _enableLeds = false;

    int _indicatorColor = 0;

    bool _underglowDirty = false;
    bool _underglowEnabled = false;
    unsigned long _underglow[6] = {0};

    bool _keyledsDirty = false;
    bool _keyledsEnabled = false;
    unsigned long _keyleds[16] = {0};

    static void enc0_callback(void *ctx, int delta)
    {
        ((KM16 *)ctx)->onEncoder(0, delta);
    }

    static void enc1_callback(void *ctx, int delta)
    {
        ((KM16 *)ctx)->onEncoder(1, delta);
    }

    static void enc2_callback(void *ctx, int delta)
    {
        ((KM16 *)ctx)->onEncoder(2, delta);
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
                KeyState *pks = &_keyState[key];
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

    void sendUnderglowPixels()
    {
        noInterrupts();
        for (int i = 0; i < 6; i++)
        {
            sendPixel(_underglow[i], 9);
        }
        interrupts();
    }

    void sendKeyLedPixels()
    {
        noInterrupts();
        for (int i = 0; i < 16; i++)
        {
            sendPixel(_keyleds[i], 11);
        }
        sendPixel(0, 11);
        interrupts();
    }

    static void sendPixel(unsigned long rgb, int bit)
    {
        sendPixelByte((rgb >> 8) & 0xFF, bit);
        sendPixelByte((rgb >> 16) & 0xFF, bit);
        sendPixelByte((rgb >> 0) & 0xFF, bit);
    }

    static void sendPixelByte(uint8_t byte, int bit)
    {
        volatile uint32_t *bsrr = &GPIOB->BSRR;
        for (uint8_t i = 0; i < 8; i++)
        {
            if (byte & 0x80)
            {
                *bsrr = (1 << bit);
                __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
                *bsrr = (1 << (bit + 16));
                __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
            }
            else
            {
                *bsrr = (1 << bit);
                __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
                *bsrr = (1 << (bit + 16));
                __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
            }
            byte <<= 1;
        }
    }
};
