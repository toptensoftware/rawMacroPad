#include "KM16.h"
#include "src/RawHID.h"




class Test : public KM16
{
public:
    Test()
    {
    }

    virtual void onKey(int key, bool pressed)
    {

        if (pressed)
        {
            if (key == 0)
            {
                int nlc = (getLayerColor() + 1) & 0x07;
                setLayerColor(nlc);
            }

            if (key == 3)
            {
                // Trigger MCU reset
                NVIC_SystemReset();
            }

            if (key == 4)
            {
                setEnableUnderglow(!getEnableUnderglow());
            }

            if (key == 5)
            {
                ulc++;
                unsigned long rgb = ((ulc & 0x04) ? 0xFF0000 : 0) | ((ulc & 0x02) ? 0x00FF00 : 0) | ((ulc & 0x01) ? 0x0000FF : 0);
                setUnderglow(rgb);
            }

            if (key == 8)
            {
                setEnableKeyLeds(!getEnableKeyLeds());
            }

            if (key == 9)
            {
                klc++;
                unsigned long rgb = ((klc & 0x04) ? 0xFF0000 : 0) | ((klc & 0x02) ? 0x00FF00 : 0) | ((klc & 0x01) ? 0x0000FF : 0);
                setKeyLeds(rgb);
            }

            if (key == 15)
            {
                setEnableLeds(!getEnableLeds());
            }

            // Send key event via Raw HID
            uint8_t report[64] = {0};
            report[0] = 0x01; // Report type: key event
            report[1] = key;
            report[2] = pressed ? 1 : 0;
            RawHID.send(report, 64);
        }
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

    km.setEnableLeds(true);
    km.setLayerColor(LAYER_COLOR_MAGENTA);
}

unsigned long lastTime = 0;
const unsigned long interval = 2000;

void loop()
{
    unsigned long now = millis();

    if (now - lastTime >= interval) {
        lastTime = now;  // reset timer
        km.setLayerColor(((now / interval) % 2) ? LAYER_COLOR_BLUE : LAYER_COLOR_MAGENTA);
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
