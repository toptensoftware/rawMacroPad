# MMD KM16 Keyboard Hardware Analysis - Complete Summary

## Device Overview
- **Name:** MMD KM16
- **MCU:** STM32F103 (ARM Cortex-M3)
  - 128KB Flash (8KB bootloader + 116KB firmware + 4KB EEPROM emulation)
  - 20KB RAM
  - 72MHz clock speed
  - Bootloader ID: 1EAF:0003 (STM32duino "Maple" bootloader)

## Physical Layout
- **16 mechanical key switches** (4Ã—4 grid)
- **3 rotary encoders** with push buttons
- **24 RGB LEDs:**
  - 16 per-key LEDs (one under each switch)
  - 6 underglow LEDs
  - 2 layer indicator LEDs (bar-shaped)

## GPIO Pinout

### Keyboard Matrix (4 rows Ã— 5 columns)
**Outputs (Rows - 4 pins):**
- **PB0** - Row 0
- **PB1** - Row 3
- **PB2** - Row 2
- **PB10** - Row 1

**Inputs (Columns - 5 pins):**
- **PA0** - Column 0
- **PA1** - Column 1
- **PA2** - Column 3
- **PA3** - Column 2
- **PA4** - Column 4

*Note: The 5th column (PA4) likely handles the 3 encoder push buttons, giving 4Ã—4=16 regular keys + 3 encoder buttons = 19 total keys in the matrix.*

### Rotary Encoders (6 pins - all inputs)
- **PA5** - Encoder signal (A or B)
- **PA6** - Encoder signal (A or B)
- **PA7** - Encoder signal (A or B)
- **PC13** - Encoder signal (A or B)
- **PC14** - Encoder signal (A or B)
- **PC15** - Encoder signal (A or B)

*Note: 3 encoders Ã— 2 quadrature signals (A/B) each = 6 input pins. Exact mapping to be determined through testing.*

### RGB LEDs - VERIFIED

**Layer Indicator LEDs (2 LEDs - Direct GPIO Control):**
- **PB6** - Green channel (active LOW)
- **PB7** - Blue channel (active LOW)
- **PB8** - Red channel (active LOW)
- Type: Direct GPIO control (not WS2812)
- Default state on power-up: ON (all channels LOW = white)

**Underglow LEDs (6 LEDs - WS2812 Chain 1):**
- **PB9** - WS2812 serial data output
- **PB12** - Power enable (active LOW)
- Count: 6 LEDs
- Protocol: WS2812, GRB byte order
- Physical order: BackRight, BackCenter, BackLeft, FrontLeft, FrontCenter, FrontRight

**Per-Key LEDs (16 LEDs - WS2812 Chain 2 with Constant Current Driver):**
- **PB11** - WS2812 serial data output
- **PB14** - LED driver chip power enable (active HIGH - must be HIGH)
- **PB13** - LED driver output blank (active HIGH - must be LOW to enable LED outputs)
- Count: 16 LEDs (17th data position unused)
- Protocol: WS2812, GRB byte order
- Driver: Constant current LED driver chip (likely reused design from manufacturer's full-size keyboards)
- Physical layout (4×4 grid, left to right, top to bottom):
  ```
  [0]  [1]  [2]  [3]
  [7]  [6]  [5]  [4]
  [8]  [9]  [10] [11]
  [15] [14] [13] [12]
  ```
  Pattern: Row 0 left-to-right, Row 1 right-to-left, Row 2 left-to-right, Row 3 right-to-left (serpentine)

**Design Note:** The per-key LEDs use a constant current driver chip (with power enable and output blank pins) for better brightness control and consistency. This is likely a reused design from the manufacturer's full-size keyboards, where proper LED driving is more critical. The underglow uses simpler direct power switching since it's less visible.

**Important Notes:**
- Two separate WS2812 chains on different data pins (PB9 and PB11)
- Per-key LEDs require PB14 HIGH and PB13 HIGH->LOW transition to enable
- All WS2812 communication uses bit-banging at 72MHz

### USB
- **PA11** - USB D- (hardware fixed)
- **PA12** - USB D+ (hardware fixed)

## Firmware Architecture

### Matrix Scanning
- **Method:** Traditional row/column scanning
- **Scan function:** `FUN_0800db28` at address 0x0800DB28
- **Operation:** 
  - Drives one row LOW at a time (output)
  - Reads all 5 column states (inputs with pull-ups)
  - Repeats for all 4 rows
- **Row pin table:** Located at 0x0800ED48
- **Column pin table:** Located at 0x0800F0F8

### LED Control
- **Protocol:** WS2812 (bit-banging implementation)
- **Update function:** `FUN_08004ab0` at address 0x08004AB0
- **Bit-bang function:** `FUN_08003de8` at address 0x08003DE8
- **Timing:** Software delays generate precise WS2812 timing (0.4Î¼s/0.8Î¼s pulses)
- **24 LEDs total** sent as three 8-LED chunks

### Pin Configuration Function
- **Function:** `FUN_0800c41c` at address 0x0800C41C
- Configures GPIO pins as input/output
- Sets/clears pins via BSRR (Bit Set/Reset Register)
- Uses mode lookup table at 0x0800F1A9

## Memory Map

### Flash Layout (128KB total)
```
0x08000000 - 0x08001FFF : Bootloader (8KB)
0x08002000 - 0x0801EFFF : Firmware (116KB)
0x0801F000 - 0x0801FFFF : EEPROM Emulation (4KB)
```

### Key Data Structures
- **Matrix row pins:** 0x0800ED48 (4 entries)
  - 0x40010C00 (PB0)
  - 0x40010C0A (PB10)
  - 0x40010C02 (PB2)
  - 0x40010C01 (PB1)
- **Matrix column pins:** 0x0800F0F8 (5 entries)
  - 0x40010800 (PA0)
  - 0x40010801 (PA1)
  - 0x40010803 (PA3)
  - 0x40010802 (PA2)
  - 0x40010804 (PA4)
- **LED index mapping:** 0x0800EDEC - 0x0800EDFB
- **GPIO config modes:** 0x0800F1A9
- **RCC base pointer:** 0x080098E4 (points to 0x40021000)

### RAM
- Base: 0x20000000
- Size: 20KB (0x20000000 - 0x20004FFF)

## Clock Configuration
- **RCC_APB2ENR enabled peripherals:**
  - AFIO (Alternate Function I/O)
  - GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG clocks enabled
  - Bitmask: 0x1FD

## VIA Configuration
- **Vendor ID:** 0x5343
- **Product ID:** 0x0080
- **Matrix config:** 5 rows Ã— 4 columns (logical mapping)
- **Features:**
  - RGB underglow (6 LEDs)
  - RGB matrix (per-key backlighting - 16 LEDs)
  - Layer indicator
  - Rotary encoder support

## Verified QMK Port Configuration

### Pin Assignments for QMK config.h
```c
// Matrix - VERIFIED
#define MATRIX_ROWS 4
#define MATRIX_COLS 5
#define MATRIX_ROW_PINS { PB0, PB10, PB2, PB1 }
#define MATRIX_COL_PINS { PA0, PA1, PA2, PA3, PA4 }
#define DIODE_DIRECTION COL2ROW

// Encoders - VERIFIED
// Order: Main/Center, Left, Right
#define ENCODERS_PAD_A { PA7, PC14, PA5 }
#define ENCODERS_PAD_B { PC15, PA6, PC13 }

// Encoder buttons are in the matrix:
// Main encoder button:  Row 3, Col 4
// Left encoder button:  Row 2, Col 4  
// Right encoder button: Row 1, Col 4

// Layer Indicator LEDs - VERIFIED (Direct GPIO, not WS2812)
#define LAYER_INDICATOR_LED_GREEN PB6
#define LAYER_INDICATOR_LED_BLUE PB7
#define LAYER_INDICATOR_LED_RED PB8
// These are active LOW - digitalWrite LOW = LED ON

// Underglow LEDs - VERIFIED
#define RGBLIGHT_DI_PIN PB9
#define RGBLED_NUM 6
#define RGBLIGHT_POWER_PIN PB12  // Active LOW
#define WS2812_BYTE_ORDER WS2812_BYTE_ORDER_GRB

// Per-Key RGB Matrix - VERIFIED
#define RGB_MATRIX_LED_COUNT 16
#define RGB_DI_PIN PB11
#define RGB_MATRIX_DRIVER_POWER_PIN PB14    // Constant current driver chip power (active HIGH)
#define RGB_MATRIX_DRIVER_BLANK_PIN PB13    // Driver output blank (active HIGH - LOW enables outputs)
#define WS2812_BYTE_ORDER WS2812_BYTE_ORDER_GRB

// Per-key LED physical layout (serpentine pattern)
// LED indices in physical 4x4 grid:
//  0  1  2  3
//  7  6  5  4
//  8  9 10 11
// 15 14 13 12
```

### LED Initialization Sequence
Per-key LEDs use a constant current driver chip that requires specific initialization:
```c
// 1. Enable LED driver chip power
pinMode(PB14, OUTPUT);
digitalWrite(PB14, HIGH);

// 2. Set output blank pin HIGH (outputs disabled)
pinMode(PB13, OUTPUT);
digitalWrite(PB13, HIGH);

// 3. Un-blank outputs (enable LED outputs)
digitalWrite(PB13, LOW);

// 4. Now per-key LEDs can be controlled via PB11 data pin
// Note: PB13 is active HIGH blank, so LOW = outputs enabled
```

Underglow initialization:
```c
// Simple power switch - set LOW to enable
pinMode(PB12, OUTPUT);
digitalWrite(PB12, LOW);

// Now underglow LEDs can be controlled via PB9
```

**Design rationale:** Per-key LEDs use a constant current driver (common in full-size keyboards) for better brightness control. The underglow uses simple power switching since it's less critical for visibility.

### LAYOUT Macro
The physical layout is rotated 90° from the matrix, requiring this mapping:

```c
// Physical layout macro
// k00-k03: Top physical row (left to right)
// k10-k13: Second physical row
// k20-k23: Third physical row  
// k30-k33: Bottom physical row

#define LAYOUT( \
    k00, k01, k02, k03, \
    k10, k11, k12, k13, \
    k20, k21, k22, k23, \
    k30, k31, k32, k33  \
) { \
    { k00, k10, k30, k20, KC_NO }, \
    { k01, k11, k31, k21, ENC_R }, \
    { k02, k12, k32, k22, ENC_L }, \
    { k03, k13, k33, k23, ENC_M }  \
}
```

**Explanation:**
- Physical position k00 (top-left) maps to matrix R0C0
- Physical position k01 (second from left, top row) maps to matrix R1C0
- Physical position k02 (third from left, top row) maps to matrix R2C0
- Physical position k03 (top-right) maps to matrix R3C0
- Columns in physical order 0,1,2,3 map to matrix columns 0,1,3,2
- ENC_R, ENC_L, ENC_M are encoder button keycodes in column 4

### Matrix Scanning Notes - VERIFIED
- Diode direction: COL2ROW (columns are inputs, rows are outputs) ✅
- Rows driven LOW during scan (not HIGH) ✅
- Columns use internal pull-ups ✅
- Standard switch matrix with diodes for anti-ghosting ✅
- **Physical layout is rotated 90° clockwise from matrix coordinates**
- Column order in physical layout: 0, 1, 3, 2 (Col 3 before Col 2)

### LED Notes - FULLY VERIFIED
- **Three separate LED subsystems:**
  1. Layer indicators: Direct GPIO (PB6, PB7, PB8)
  2. Underglow: WS2812 on PB9 with PB12 power enable
  3. Per-key: WS2812 on PB11 with PB14 power + PB13 latch enable
- Layer indicators default to ON (white) on boot
- Underglow: 6 LEDs, PB12 must be LOW to power on
- Per-key: 16 LEDs, requires PB14 HIGH and PB13 HIGH->LOW edge
- Original firmware sends per-key LEDs in 3 chunks of 8 (total 24 bytes for 16 LEDs + 1 unused)
- Uses bit-banging (no hardware PWM/DMA) ✅
- Per-key LED order is serpentine pattern (alternating left-right, right-left)
- WS2812 timing verified at 72MHz (400ns/800ns pulses) ✅
- Byte order: GRB for both chains (except possibly LED 11)
- Consider using hardware PWM+DMA in QMK for better performance

### Encoder Notes - VERIFIED
- 6 GPIO pins dedicated to 3 rotary encoders ✅
- Each encoder uses 2 pins (A and B quadrature signals) ✅
- All configured as inputs with pull-ups ✅
- **Verified pin pairings:**
  - Main/Center: PA7 (A), PC15 (B), Button at R3C4
  - Left: PC14 (A), PA6 (B), Button at R2C4
  - Right: PA5 (A), PC13 (B), Button at R1C4
- Direction detection confirmed working (clockwise/counter-clockwise) ✅
- Encoder buttons are part of the matrix in column 4 ✅

## Remaining Unknowns

1. **17th LED data position:** Original firmware sends data for 17 LEDs but only 16 are physically present
   - Position 16 in chain appears unused
   - May be for future hardware revision or layer indicator integration

## WS2812 Timing (PB11 - Per-Key LEDs)
Calibrated timing for 72MHz STM32F103:
- **1 bit:** 55 NOPs high (~764ns), 33 NOPs low (~459ns)
- **0 bit:** 26 NOPs high (~361ns), 64 NOPs low (~890ns)

Note: QMK firmware should use hardware PWM/DMA drivers instead of bit-banging for more reliable timing.

## Reverse Engineering Methods Used
- Ghidra disassembly and decompilation
- GPIO register pattern recognition
- Function call graph analysis
- Memory structure examination
- Pin table discovery through cross-references
- WS2812 bit-timing pattern identification
- STM32 peripheral register analysis

## Important Functions Identified

| Function Address | Purpose |
|-----------------|---------|
| 0x08002238 | Reset handler (firmware entry point) |
| 0x08009628 | Main function |
| 0x0800DB28 | Matrix scanning routine |
| 0x08004D2C | GPIO pin read function |
| 0x080052CC | GPIO pin write function |
| 0x0800C41C | GPIO configuration function |
| 0x08003DE8 | WS2812 bit-bang function (sends one byte) |
| 0x08004AB0 | LED update function (sends 8 LEDs) |

## Testing Results - FULLY VERIFIED

### 1. Matrix Pinout - ✅ CONFIRMED
**Physical Layout (90° rotated from matrix coordinates):**
```
Physical Layout (4×4 grid):
┌────┬────┬────┬────┐
│R0C0│R1C0│R2C0│R3C0│  ← Top physical row
├────┼────┼────┼────┤
│R0C1│R1C1│R2C1│R3C1│  ← Second physical row
├────┼────┼────┼────┤
│R0C3│R1C3│R2C3│R3C3│  ← Third physical row (Col 3!)
├────┼────┼────┼────┤
│R0C2│R1C2│R2C2│R3C2│  ← Bottom physical row (Col 2!)
└────┴────┴────┴────┘
```

**Key Finding:** Matrix is rotated 90° clockwise from physical layout
- Physical rows = Matrix columns (in order: 0, 1, 3, 2)
- Physical columns = Matrix rows (in order: 0, 1, 2, 3)
- Column 3 comes before Column 2 in physical layout

**All 16 keys tested and responding correctly**

### 2. Encoder Pins - ✅ CONFIRMED
**Main/Center Encoder:**
- Pin A: PA7
- Pin B: PC15
- Button: Row 3, Col 4
- Direction detection: Verified (CW/CCW)

**Left Encoder:**
- Pin A: PC14
- Pin B: PA6
- Button: Row 2, Col 4
- Direction detection: Verified (CW/CCW)

**Right Encoder:**
- Pin A: PA5
- Pin B: PC13
- Button: Row 1, Col 4
- Direction detection: Verified (CW/CCW)

**All encoder rotations and button presses confirmed working**

### 3. LED System - ✅ FULLY MAPPED

**Layer Indicators (2 bar-shaped LEDs):**
- Control: Direct GPIO (active LOW)
- PB6 = Green, PB7 = Blue, PB8 = Red
- Default state: ON (white) at power-up
- Fully functional and mapped

**Underglow LEDs (6 WS2812 LEDs):**
- Data pin: PB9
- Power enable: PB12 (active LOW)
- Physical order verified: BackRight, BackCenter, BackLeft, FrontLeft, FrontCenter, FrontRight
- Byte order: GRB
- Fully functional and mapped

**Per-Key LEDs (16 WS2812 LEDs under switches):**
- Data pin: PB11
- Power enable: PB14 (must be HIGH)
- Latch enable: PB13 (requires HIGH->LOW falling edge)
- Physical layout verified (serpentine pattern):
  ```
  [0]  [1]  [2]  [3]
  [7]  [6]  [5]  [4]
  [8]  [9]  [10] [11]
  [15] [14] [13] [12]
  ```
- Byte order: GRB (LED 11 may differ)
- 17th data position sent by firmware but unused
- Fully functional and mapped

**Critical Discovery - Per-Key LED Driver Architecture:**
Per-key LEDs use a constant current driver chip (likely reused from manufacturer's full-size keyboards):
1. PB14 enables driver chip power (active HIGH)
2. PB13 controls output blank (active HIGH - must be LOW to enable outputs)
3. Initialization sequence: PB14 HIGH, PB13 HIGH, then PB13 LOW
4. Only after un-blanking outputs will WS2812 data on PB11 control the LEDs
5. This provides better brightness control and consistency than simple power switching

### 4. USB - ✅ CONFIRMED
- PA11/PA12 USB pins confirmed working
- CDC serial communication functional
- Maple DFU bootloader (1EAF:0003) confirmed

---

*Analysis completed using Ghidra on firmware extracted from MMD KM16 device (120KB firmware dump). All addresses are from the firmware binary loaded at base address 0x08002000.*

*Hardware verification performed using Arduino IDE with STM32duino on the actual device. All matrix, encoder, and USB functionality confirmed working. LED functionality pending power enable pin identification.*

*Reverse engineering performed to enable QMK firmware development for this keyboard.*

## Arduino Testing Summary

### Development Environment
- **IDE:** Arduino IDE with STM32duino (official STMicroelectronics core)
- **Board:** Generic STM32F103C series (STM32F103C8)
- **Bootloader:** Maple DFU Bootloader 2.0 (Device ID: 1EAF:0003)
- **Upload Method:** dfu-util via command line
- **USB Support:** CDC (generic 'Serial' supersede U(S)ART)
- **Clock Speed:** 72MHz (verified)

### Test Results
All hardware components have been verified working through Arduino test sketches:

1. **Serial Communication Test** ✅
   - USB CDC serial working correctly
   - 115200 baud rate
   - Bidirectional communication confirmed

2. **Matrix Scanner Test** ✅
   - All 16 keys responding correctly
   - Physical layout mapped to matrix coordinates
   - Debouncing working properly
   - 90° rotation from matrix to physical layout confirmed

3. **Encoder Button Test** ✅
   - All 3 encoder buttons detected
   - Positions confirmed: R3C4 (Main), R2C4 (Left), R1C4 (Right)

4. **Encoder Rotation Test** ✅
   - All 3 encoders responding
   - Direction detection (CW/CCW) confirmed
   - Pin pairs verified through testing

5. **Layer Indicator LED Test** ✅
   - Direct GPIO control verified
   - PB6 (Green), PB7 (Blue), PB8 (Red) all functional
   - Active LOW operation confirmed
   - Default state: ON (white) at power-up

6. **Underglow LED Test** ✅
   - WS2812 protocol on PB9 working
   - PB12 power enable (active LOW) verified
   - All 6 LEDs functional
   - Physical order mapped
   - GRB byte order confirmed

7. **Per-Key RGB LED Test** ✅
   - WS2812 protocol on PB11 working
   - Power enable sequence discovered:
     - PB14 must be HIGH
     - PB13 must transition HIGH->LOW (falling edge)
   - All 16 LEDs functional and mapped
   - Serpentine physical layout confirmed
   - GRB byte order (except possibly LED 11)

### Key Discoveries

**LED Subsystem Architecture:**
- Three independent LED systems (not one unified chain as initially thought)
- Layer indicators use direct GPIO, not WS2812
- Underglow and per-key use separate WS2812 chains on different pins
- Complex power enable scheme for per-key LEDs

**Per-Key LED Power Enable Sequence:**
The most challenging aspect was discovering the constant current LED driver architecture:
- Initial hypothesis: Single power enable pin like underglow
- Reality: Constant current driver chip with separate power and blank pins
- PB14 provides driver chip power (active HIGH)
- PB13 controls output blank function (active HIGH - LOW enables outputs)
- LEDs only respond to data after: PB14=HIGH, PB13=HIGH→LOW sequence
- Design reused from manufacturer's full-size keyboards for consistency

**Physical Layout Quirks:**
- Matrix rotated 90° clockwise from physical arrangement
- Column wiring order: 0, 1, 3, 2 (Col 3 before Col 2)
- Per-key LED serpentine pattern (alternating row directions)

### Arduino Test Sketches Created
- Serial output test
- Matrix scanner with debouncing
- Encoder button mapper
- Encoder rotation decoder with direction
- Comprehensive hardware test (all inputs)
- Layer indicator LED test
- Underglow LED test with color cycling
- Per-key LED power enable finder (interactive)
- Per-key LED mapping test (walking LED)
- Complete LED demonstration
- Clock speed verification

### Next Steps
1. Port complete configuration to QMK firmware
2. Implement LED initialization sequences in QMK
3. Create proper LAYOUT macro for 90° rotation and column reordering
4. Test and debug QMK build
5. Implement Raw HID for Cantabile integration
