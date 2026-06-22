# SmartCampus Integrated IoT

ESP32 FreeRTOS firmware for the embedded-systems part of a Smart Campus graduation project.

## Repository Layout

- `SmartCampus_Integrated_IoT/` - main ESP32 Arduino FreeRTOS firmware
- `Function_Tests/` - separated ESP32 sketches for testing each subsystem individually

## Main Features

- Smart gate control using servo motor and ultrasonic detection
- Garbage-bin level monitoring using ultrasonic distance sensing
- Parking spot monitoring using ultrasonic sensors
- Smart lighting using LDR, ultrasonic presence detection, and shared LED output
- Smoke alarm using MQ2 sensor, LED, buzzer, and server alarm state
- Smart irrigation using soil moisture sensing, relay control, and hysteresis
- FreeRTOS task separation for concurrent subsystem operation

## Setup

1. Open the `SmartCampus_Integrated_IoT/` folder.
2. Copy `config.example.h` to `config.h`.
3. Put your real Wi-Fi credentials in `config.h`.
4. Open `SmartCampus_Integrated_IoT/SmartCampus_Integrated_IoT.ino` in Arduino IDE.
5. Select the ESP32 board and upload.

`config.h` is intentionally ignored by Git because it contains private Wi-Fi credentials.

## Current GPIO Map

| Function | GPIO |
|---|---|
| Gate servo | 18 |
| Gate ultrasonic TRIG/ECHO | 5 / 19 |
| Garbage ultrasonic TRIG/ECHO | 23 / 22 |
| Parking spot 1 TRIG/ECHO | 21 / 4 |
| Parking spot 2 TRIG/ECHO | 26 / 25 |
| MQ2 smoke sensor | 34 |
| Smoke LED / buzzer | 12 / 15 |
| Moisture sensor / pump relay | 35 / 27 |
| Lighting ultrasonic 1 TRIG/ECHO | 13 / 14 |
| Lighting ultrasonic 2 TRIG/ECHO | 32 / 33 |
| Lighting LDR / LED output | 36 / 2 |
