#include "KM16.h"

#include <Adafruit_TinyUSB.h>

// 64-byte raw HID report
const uint8_t hid_report_descriptor[] = {
  0x06, 0x00, 0xFF,  // Vendor-defined
  0x09, 0x01,
  0xA1, 0x01,        // Collection
  0x15, 0x00,
  0x26, 0xFF, 0x00,
  0x75, 0x08,
  0x95, 0x40,
  0x09, 0x00,
  0x81, 0x02,        // Input
  0x09, 0x00,
  0x91, 0x02,        // Output
  0xC0
};

Adafruit_USBD_HID hid;

class Test : public KM16
{
public:
  Test()
  {
  }

  virtual void onKey(int key, bool pressed)
  {
    Serial.print("KEY ");
    Serial.print(key);
    Serial.print(": ");
    Serial.println(pressed ? "press" : "release");

    if (pressed)
    {
      if (key == 0)
      {
        int nlc = (getLayerColor() + 1) & 0x07;
        Serial.print("Setting layer color: ");
        Serial.println(nlc);
        setLayerColor(nlc);
      }

      if (key == 4)
      {
        bool n = !getEnableUnderglow();
        Serial.print("Setting underglow: ");
        Serial.println(n ? "enabled" : "disabled");
        setEnableUnderglow(n);
      }

      if (key == 5)
      {
        ulc++;
        unsigned long rgb = ((ulc & 0x04) ? 0xFF0000 : 0) | ((ulc & 0x02) ? 0x00FF00 : 0) | ((ulc & 0x01) ? 0x0000FF : 0);
        Serial.print("Setting underglow color: ");
        Serial.println(rgb, HEX);
        setUnderglow(rgb);
      }


      if (key == 8)
      {
        bool n = !getEnableKeyLeds();
        Serial.print("Setting keyleds: ");
        Serial.println(n ? "enabled" : "disabled");
        setEnableKeyLeds(n);
      }

      if (key == 9)
      {
        klc++;
        unsigned long rgb = ((klc & 0x04) ? 0xFF0000 : 0) | ((klc & 0x02) ? 0x00FF00 : 0) | ((klc & 0x01) ? 0x0000FF : 0);
        Serial.print("Setting key led color: ");
        Serial.println(rgb, HEX);
        setKeyLeds(rgb);
      }

      if (key == 15)
      {
        bool n = !getEnableLeds();
        Serial.print("Enabling LEDs: ");
        Serial.println(n ? "enabled" : "disabled");
        setEnableLeds(n);
      }
    }
  }

  virtual void onEncoder(int index, int delta)
  {
    Serial.print("ENC ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(delta);
  }

  int ulc = 0;
  int klc = 0;

};

Test km;

void setup() 
{
  hid.setReportDescriptor(hid_report_descriptor, sizeof(hid_report_descriptor));
  hid.begin();

  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Firmware starting...");
  km.setup();
}

void loop() 
{
  km.update();
}


void tud_hid_report_received_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
  // handle incoming report
}