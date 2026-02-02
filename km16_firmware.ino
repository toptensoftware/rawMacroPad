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
