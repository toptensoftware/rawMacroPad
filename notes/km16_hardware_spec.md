# MMD KM16 Macropad Keyboard

## MCU

STM32F103 ARM Cortex-M3 (or similar)

  - 128KB Flash
  - 20KB RAM
  - 72MHz clock speed
  - Bootloader ID: 1EAF:0003 (STM32duino "Maple" bootloader)


## Physical Layout

  - 16x mechanical key switches in (4x4 grid)
  - 3x rotary encoders with push buttons
  - 16x per-key RGB LEDs (one under each switch)
  - 6x underglow RGB LEDs
  - 1x 3 bit color layer indicator


### Flash Memory Layout (128KB total)

```
0x08000000 - 0x08001FFF : Bootloader (8KB)
0x08002000 - 0x0801EFFF : Firmware (116KB)
0x0801F000 - 0x0801FFFF : EEPROM Emulation (4KB)
```


## Matrix

```
        Key Pad                    Encoder
                                 Push Buttons

[R0C0][R1C0][R2C0][R3C0]     [R2C4]      [R1C4]
[R0C1][R1C1][R2C1][R3C1]
[R0C2][R1C2][R2C2][R3C2]
[R0C3][R1C3][R2C3][R3C3]           [R3C4] 
```

Row Pins:    PB0, PB10, PB2, PB1
Column Pins: PA0, PA1, PA3, PA2, PA4


## Rotary Encoders

Main: PA7 (A), PC15 (B)
Left: PC14 (A), PA6 (B)
Right: PA5 (A), PC13 (B)


## LED Power Control

PB14 - Active HIGH controls power to all LEDs

Notes:

  * if PB14 is not configured as an output pin the LED system seems to be powered.  If however 
      PB14 configured as output pin, then the default output is typically LOW and LEDs will 
      be powered off.
  * After powering on the LEDs, the pixel chains need to be initialized with pixel colors
  * After powering on the LEDs, the level indicator will automatically show color according 
      to PB6,7,8 (see below)
  * After powering on the LEDs, a 10-30ms delay seems to help stabilize things before sending
      pixel data.


## Layer Indicator LEDs

PB6 = Green (active LOW)
PB7 = Blue (active LOW)
PB8 = Red (active LOW)

(requires PB14 HIGH)


## Underglow LEDs

PB12 = Power enable (active LOW)
PB9 = WS2812 data (6 LEDs)

LED Positions: (when viewed from above device):

```
[2] [1] [0]
[5] [4] [3]
```

(requires PB14 HIGH)


## Per-Key LEDs

Per-key LEDs appear to use a constant current driver chip (possibly reused from manufacturer's full-size 
keyboards).  PB13 seems to control the output blank pin and requires a falling edge to clear output blank
and drive LEDs

ie:

  1. PB14 HIGH (LED system power on)
  2. A 10 to 30ms delay here seems to help stabilize things before sending pixel data
  3. PB13 HIGH (output blank on)
  4. A delay here probably also helps, but untested.
  5. PB13 LOW (falling edge to un-blank)

Only after un-blanking outputs will WS2812 data on PB11 control the LEDs.

PB11 = WS2812 data (17 LEDs, last not used)
PB13 = output blank (active HIGH)

LED positions:

```
[ 0] [ 1] [ 2] [ 3]
[ 7] [ 6] [ 5] [ 4]
[ 8] [ 9] [10] [11]
[15] [14] [13] [12]      [16] = unused
```

(requires PB14 HIGH)
