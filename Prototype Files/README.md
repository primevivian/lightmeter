# PROTOTYPE INFORMATION:

## Parts:
- Arduino Pro Micro [ATMega32u4]
- TEMT6000 Light Sensor (This sensor has terrible resolution, find a better one in the future)
- TP4056 Charging Module
- Mystery Feature Phone Battery
- 128x64 I2C OLED Display
- 3x 6mm Button Switches

## Pinout:
| Pro Micro Pin | Connected To | Notes |
|---|---|---|
| 15 | Measure button | Other leg to GND (internal pull-up) |
| 14 | + button | Other leg to GND (internal pull-up) |
| 16 | - button | Other leg to GND (internal pull-up) |
| A1 | TEMT6000 SIG | Breakout has the 10k resistor onboard |
| 2 (SDA) | OLED SDA | I2C address 0x3C |
| 3 (SCL) | OLED SCL | |
| VCC | TEMT6000 VCC, OLED VCC | Code assumes a 5V reference for A1 |
| GND | TEMT6000 GND, OLED GND, all buttons | |

