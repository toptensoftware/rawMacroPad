#include "KM16.h"


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

void setup() {
  Serial.begin(115200);
  delay(5000);
  
  Serial.println("Firmware starting...");
  km.setup();
}

void loop() {
  km.update();
}
