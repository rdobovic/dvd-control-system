# DVD Control System

Dvd Control system is device used by fire brigade to remotly and locally perform actions inside fire station. This version of DVDCS supports remote control by sending simple SMS commands.

```NOTE THAT README IS STILL INCOMPLETE```

## Parts used

- Arduino MEGA (joy-it MEGA)
- SIM900 2G Modem
- SD card module
- RTC DS1302
- Power supply 5V 4A
- 220V sensor for arduino (Home made)
- Relay array, 5 relays 250VAC 10A, combined with transistors BC5478 transistors are used to power up relays since arduino pins can't deliver enough current (diodes are also added but there is no actual use of them)
- BUS, two lines of shorted pins used to connect all GNDs and all VCCs together
- Step-up converter used to step-up voltage to 9V so arduino can be powered using Vin pin. (Vin pin is used instead of 5V pin to prevent current flow to computer when connected using USB)

## Intern connections

| Module | Module pin | Arduino MEGA pin | Power BUS |
| :--- | :---: | :---: | :---: |
| **Relay array** | 1 | 23 | |
|  | 2 | 25 | |
|  | 3 | 27 | |
|  | 4 | 29 | |
|  | 5 | 31 | |
|  | + | | 5V |
|  | - | | GND |
| **SIM900** Modem | TX 7 | RX3 15 | |
|  | RX 8 | TX3 14 | |
|  | GND | GND | |
|  | 9 | 9 | |
|  | Power jack + | | 5V |
|  | Power jack - | | GND |
| **RTC** DS1302 | CLK | 5 | |
|  | DAT | 4 | |
|  | RST | 3 | |
|  | VCC | | 5V |
|  | GND | | GND |
| **Buzzer** | S | 7 | |
|  | - | | GND |
| **Button 1** Neposredna opasnost | OUT | 39 | |
|  | IN | | 5V |
|  | GND | | GND |
| **Button 2** Prekid opasnosti | OUT | 37 | |
|  | IN | | 5V |
|  | GND | | GND |
| **Button 3** Vatrogasna uzbuna | OUT | 35 | |
|  | IN | | 5V |
|  | GND | | GND |
| **Button 4** Stop siren | OUT | 33 | |
|  | IN | | 5V |
|  | GND | | GND |
| **Ready indicator LED** | + | 8 | |
|  | - | | GND | |
| **SD card reader** | 3V3 | 3.3V | |
|  | CS | 53 | |
|  | MOSI | 51 | |
|  | SCK | 52 | |
|  | MISO | 50 | |
|  | GND | | GND |
|  | 5V | | 5V |
| **LCD Display I2C** | SDA | SDA 20 | |
|  | SCL | SCL 21 | |
|  | VCC | | 5V |
|  | GND | | GND |
| **Keypad** | 1 | 22 |
|  | 2 | 24 |
|  | 3 | 26 |
|  | 4 | 28 |
|  | 5 | 30 |
|  | 6 | 32 |
|  | 7 | 34 |
|  | 8 | 36 |
| **Magnet switch** Big door open | 

Each button and magnet switch is connected in following schematic. Note that Some buttons have their GND and VCC pins soldered together.
```
  VCC              ___|___              OUT
   o---------------o     o------o--------o
                                |
                                |
  GND              ______       |
   o--------------|______|-------
                    100k
```
