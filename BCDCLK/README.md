# RPI BCD clock

Binary-Coded Decimal Clock for RaspberryPI

- - -

## Hardware
- 1 x Pi Model 3B V1.2
- 1 x MAX7219 Dot Matrix Module (8x8 LED Matrix)
- N x Breadlines
- 1 x GPIO Extension Board
- 1 x breadboard

## Board

### Connection of MAX7210 Dot Matrix Module and RaspberryPI

|Module|RPI|
|-|-|
|VCC|5V|
|GND|GND|
|DIN|GPIO10|
|CS|GPIO8|
|CLK|GPIO11|

## Setting

### To use SPI
```bash
$ sudo raspi-config
```
Select `5 Interfacing Options` > `P4 SPI` and enable it.

- - -

## Output - 8x8 Matrix

Year[3] Year[2] Year[1] Year[0] Mon[1] Mon[0] Day[1] Day[0]    
Year[3] Year[2] Year[1] Year[0] Mon[1] Mon[0] Day[1] Day[0]    
Year[3] Year[2] Year[1] Year[0] Mon[1] Mon[0] Day[1] Day[0]    
Year[3] Year[2] Year[1] Year[0] Mon[1] Mon[0] Day[1] Day[0]    
Hour[1] Hour[0]  Min[1]  Min[0] Sec[1] Sec[0]  NONE  Weekday    
Hour[1] Hour[0]  Min[1]  Min[0] Sec[1] Sec[0]  NONE  Weekday    
Hour[1] Hour[0]  Min[1]  Min[0] Sec[1] Sec[0]  NONE  Weekday    
Hour[1] Hour[0]  Min[1]  Min[0] Sec[1] Sec[0]  NONE  Weekday    

- Weekday : [SUN, MON, TUE, WED, THU, FRI, SAT]
