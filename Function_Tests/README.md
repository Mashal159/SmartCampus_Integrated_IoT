# SmartCampus Function Tests

Each subfolder is a separate Arduino sketch for testing one subsystem alone on the ESP32.

Open one folder at a time in Arduino IDE and upload its `.ino` file.

Serial Monitor baud rate:

`115200`

## Folders

- `Smoke_Test`: MQ2 on GPIO 34, LED on GPIO 12, buzzer on GPIO 15.
- `Lighting_Test`: shared lighting LEDs on GPIO 2, LDR on GPIO 36, ultrasonics on GPIO 13/14 and 32/33.
- `Gate_Test`: servo on GPIO 18, ultrasonic on GPIO 5/19.
- `Garbage_Test`: ultrasonic on GPIO 23/22.
- `Parking_Test`: ultrasonics on GPIO 21/4 and GPIO 26/25.
- `Irrigation_Test`: moisture sensor on GPIO 35, pump relay on GPIO 27.

## Commands

Some tests accept single-letter commands from Serial Monitor:

- Lighting: `a` AUTO, `o` ON, `f` OFF.
- Gate: `o` open, `c` close.
- Irrigation: `a` AUTO, `o` ON, `f` OFF.

## Notes

For ultrasonic sensors, `-1.0` means no valid echo was received.

For HC-SR04 echo pins, step the 5V ECHO signal down to 3.3V before the ESP32 GPIO.
