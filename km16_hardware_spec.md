## Matrix

```
        Key Pad                    Knobs

[R0C0][R1C0][R2C0][R3C0]     [R2C4]      [R1C4]
[R0C1][R1C1][R2C1][R3C1]
[R0C3][R1C3][R2C3][R3C3]           [R3C4] 
[R0C2][R1C2][R2C2][R3C2]
```

Row Pins:    PB0, PB10, PB2, PB1
Column Pins: PA0, PA1, PA2, PA3, PA4

``` C
// Setup
for (uint8_t i = 0; i < numRows; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
}
for (uint8_t i = 0; i < numCols; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
}
```

``` C  
// Read 
for (uint8_t row = 0; row < numRows; row++) 
{
    digitalWrite(rowPins[row], LOW);
    delayMicroseconds(10);
    for (uint8_t col = 0; col < numCols; col++)
    {
        bool pressed = !digitalRead(colPins[col]);
    }
    digitalWrite(rowPins[row], HIGH);
}
```


## Encoders

Main: PA7 (A), PC15 (B)
Left: PC14 (A), PA6 (B)
Right: PA5 (A), PC13 (B)

``` C
// Setup
pinMode(enc_A, INPUT_PULLUP);
pinMode(enc_B, INPUT_PULLUP);
bool enc1_last_A = digitalRead(enc1_A);
```

``` C
// Read
bool enc_A_state = digitalRead(enc_A);
bool enc_B_state = digitalRead(enc_B);
if (enc_A_state != enc_last_A) 
{
    enc_last_A = enc_A_state;
    if (enc_A_state == enc_B_state) 
    {
        // CW
    }
    else
    {
        // CCW
    }
}

```


## Layer Indicator LEDs

PB6 = Green (active LOW)
PB7 = Blue (active LOW)
PB8 = Red (active LOW)

```C
// Setup
pinMode(PB6, OUTPUT);
pinMode(PB7, OUTPUT);
pinMode(PB8, OUTPUT);
digitalWrite(PB6, HIGH);  // Off
digitalWrite(PB7, HIGH);  // Off
digitalWrite(PB8, HIGH);  // Off
```

```C
// Write
digitalWrite(PB8, r ? LOW : HIGH);
digitalWrite(PB6, g ? LOW : HIGH);
digitalWrite(PB7, b ? LOW : HIGH);
```

## Underglow LEDs

PB12 = Power enable (active LOW)
PB9 = WS2812 data (6 LEDs)

LED Positions: (when views from above device):

```
[2] [1] [0]
[5] [4] [3]
```

```C
// Power on
pinMode(PB12, OUTPUT);
digitalWrite(PB12, LOW);

// Setup data pin
pinMode(PB9, OUTPUT);
GPIOB->CRH &= ~(0xF << 4);   
GPIOB->CRH |= (0x3 << 4);
GPIOB->BRR = (1 << 9);
```

```C

noInterrupts();
for (int i = 0; i < 6; i++) {
    setUnderglowPixel(...);
}
interrupts();

void sendUnderglowPixel(uint8_t r, uint8_t g, uint8_t b) {
  sendUnderglowByte(g);
  sendUnderglowByte(r);
  sendUnderglowByte(b);
}

void sendUnderglowByte(uint8_t byte) {
  volatile uint32_t *bsrr = &GPIOB->BSRR;
  for (uint8_t i = 0; i < 8; i++) {
    if (byte & 0x80) {
      *bsrr = (1 << 9);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
      *bsrr = (1 << 25);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    } else {
      *bsrr = (1 << 9);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
      *bsrr = (1 << 25);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    }
    byte <<= 1;
  }
}
```


## Per-Key LEDs

PB11 = WS2812 data (17 LEDs, last not used)
PB14 = power enable (active HIGH)
PB13 = blank (requires transition from high to low at boot)

LED positions:

```
[ 0] [ 1] [ 2] [ 3]
[ 7] [ 6] [ 5] [ 4]
[ 8] [ 9] [10] [11]
[15] [14] [13] [12]
```

```C
// Setup
// Configure PB11 as 50MHz push-pull output
GPIOB->CRH &= ~(0xF << 12);
GPIOB->CRH |= (0x3 << 12);
GPIOB->BRR = (1 << 11);

// Per-key LED power enable
pinMode(PB13, OUTPUT);
pinMode(PB14, OUTPUT);
digitalWrite(PB14, HIGH);   // Power enable
digitalWrite(PB13, HIGH);   // Start HIGH
delayMicroseconds(100);     // need to test
digitalWrite(PB13, LOW);    // high->low edge
```

```C
// Write

noInterrupts();
for (int i = 0; i < 17; i++) {
    setKeyLedPixel(...);
}
interrupts();


void sendKeyLedPixel(uint8_t r, uint8_t g, uint8_t b) {
  sendKeyLedByte(g);
  sendKeyLedByte(r);
  sendKeyLedByte(b);
}

void sendKeyLedByte(uint8_t byte) {
  volatile uint32_t *bsrr = &GPIOB->BSRR;
  for (uint8_t i = 0; i < 8; i++) {
    if (byte & 0x80) {
      *bsrr = (1 << 11);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
      *bsrr = (1 << 27);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    } else {
      *bsrr = (1 << 11);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
      *bsrr = (1 << 27);
      __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    }
    byte <<= 1;
  }
}
```