#include "KM16.h"
#include "src/RawHID.h"




class Test : public KM16
{
public:
    Test()
    {
    }

    bool _reset1Pressed = false;
    bool _reset2Pressed = false;

    virtual void onKey(int key, bool pressed)
    {
        // Press knob 2 and 3 to trigger reset
        if (key == 17)
            _reset1Pressed = pressed;
        if (key == 18)
            _reset2Pressed = pressed;
        if (_reset1Pressed && _reset2Pressed)
        {
            // Trigger MCU reset
            NVIC_SystemReset();
        }

        // Send key event via Raw HID
        uint8_t report[64] = {0};
        report[0] = 0x01; // Report type: key event
        report[1] = key;
        report[2] = pressed ? 1 : 0;
        RawHID.send(report, 64);
    }

    virtual void onEncoder(int index, int delta)
    {
        // Send encoder event via Raw HID
        uint8_t report[64] = {0};
        report[0] = 0x02; // Report type: encoder event
        report[1] = index;
        report[2] = (int8_t)delta;
        RawHID.send(report, 64);
    }

    int ulc = 0;
    int klc = 0;
};

Test km;

void setup()
{
    delay(1000);

    // Initialize Raw HID
    RawHID.begin();

    km.setup();

    // Set all LEDs red!
    km.setEnableLeds(true);
    km.setIndicatorColor(INDICATOR_COLOR_RED);
    
}

bool watchDogIndicator = true;
unsigned long lastTime = 0;
const unsigned long interval = 300;
unsigned short watchDogTimeOut = 0;      
bool watchDogTriggered = true;
unsigned long lastWatchDogPing = 0;


void loop()
{
    unsigned long now = millis();

    if (watchDogTriggered)
    {
        if (now - lastTime >= interval) {
            lastTime = now;  // reset timer
            watchDogIndicator = !watchDogIndicator;
            km.setIndicatorColor(watchDogIndicator ? INDICATOR_COLOR_RED : INDICATOR_COLOR_BLACK);
        }
    }
    else
    {
        if (watchDogTimeOut != 0 && now - lastWatchDogPing > watchDogTimeOut)
        {
            // Mark triggered
            watchDogTriggered = true;

            // Reset LEDs
            km.setKeyLeds(0);
            km.setUnderglow(0);
            km.setEnableUnderglow(false);
            km.setEnableKeyLeds(false);
            km.setEnableLeds(true);

            // Start watch dog indicator
            lastTime = now;  // reset timer
            km.setIndicatorColor(INDICATOR_COLOR_RED);
            watchDogIndicator = true;
        }
    }



    km.update();

    // Check for commands from host
    if (RawHID.available())
    {
        uint8_t buf[64];
        RawHID.recv(buf, sizeof(buf));

        // Process command based on first byte
        switch (buf[0])
        {
            case 0x01:
                // Set indicator
                km.setIndicatorColor(buf[1]);
                break;

            case 0x02:
                // Set/reset watchdog timer
                watchDogTimeOut = (buf[1] << 8) | buf[2];
                watchDogTriggered = false;
                lastWatchDogPing = now;
                break;

            case 0x10:
                km.setEnableUnderglow(!!buf[1]);
                break;

            case 0x11:
                km.setUnderglow((buf[1] << 16) | (buf[2] << 8) | (buf[3]));
                break;

            case 0x12:
                for (int i=0; i<6; i++)
                {
                    km.setUnderglow(i, (buf[i*3+1] << 16) | (buf[i*3+2] << 8) | (buf[i*3+3]));
                }
                break;


            case 0x20:
                km.setEnableKeyLeds(!!buf[1]);
                break;

            case 0x21:
                km.setKeyLeds((buf[1] << 16) | (buf[2] << 8) | (buf[3]));
                break;

            case 0x22:
                for (int i=0; i<16; i++)
                {
                    km.setKeyLed(i, (buf[i*3+1] << 16) | (buf[i*3+2] << 8) | (buf[i*3+3]));
                }
                break;

        }
    }
}
