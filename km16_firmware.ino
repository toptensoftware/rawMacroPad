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
        bool n = !isUnderglowEnabled();
        Serial.print("Setting underglow: ");
        Serial.println(n ? "enabled" : "disabled");
        enableUnderglow(n);
      }

      if (key == 5)
      {
        lc++;
        unsigned long rgb = ((lc & 0x04) ? 0xFF0000 : 0) | ((lc & 0x02) ? 0x00FF00 : 0) | ((lc & 0x01) ? 0x0000FF : 0);
        Serial.print("Setting underglow color: ");
        Serial.println(rgb, HEX);
        setUnderglow(rgb);
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

  int lc = 0;

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
