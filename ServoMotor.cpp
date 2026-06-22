#include <Arduino.h>
#include "ServoMotor.h"

/*
  ==============================================================================
  Servo Motor Driver Implementation

  Hardware:
  - Servo signal wire connected to GPIO 18.
  - Servo must be powered from external 5V supply.
  - ESP32 GND and servo power supply GND must be common.
  ==============================================================================
*/

ServoMotor::ServoMotor(int pin) {
    servoPin = pin;
    freq = 50;          // Standard servo frequency
    resolution = 16;    // 16-bit PWM resolution
}

void ServoMotor::begin() {
    ledcAttach(servoPin, freq, resolution);
}

int ServoMotor::angleToDuty(int angle) {
    angle = constrain(angle, 0, 180);

    // Convert angle to servo pulse width
    int pulseWidthUs = map(angle, 0, 180, 500, 2500);

    // 50Hz servo period = 20ms = 20000us
    int periodUs = 1000000 / freq;

    // Maximum duty for selected resolution
    int maxDuty = (1 << resolution) - 1;

    return (int)(((long)pulseWidthUs * maxDuty) / periodUs);
}

void ServoMotor::moveToAngle(int angle) {
    int duty = angleToDuty(angle);
    ledcWrite(servoPin, duty);
}
