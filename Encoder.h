#pragma once

class Encoder
{
public:
  Encoder()
  {

  }
  
  void setup(int pinA, int pinB, void (*callback)(void* ctx, int delta), void* ctx)
  {
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);

    _ctx = ctx;
    _callback = callback;
    _pinA = pinA;
    _pinB = pinB;
    _lastA = digitalRead(_pinA);
  }

  void poll()
  {
    // Read
    bool stateA = digitalRead(_pinA);
    bool stateB = digitalRead(_pinB);
    if (stateA != _lastA) 
    {
        _lastA  = stateA;
        if (stateA)
        {
          _callback(_ctx, stateA == stateB ? 1 : -1);
        }
    }    
  }

  void* _ctx = nullptr;
  void (*_callback)(void* ctx, int delta) = nullptr;
  int _pinA = -1;
  int _pinB = -1;
  bool _lastA = false;
};


