#include "MQ2.h"

/*
  ==============================================================================
  MQ2 Gas Sensor Driver Implementation

  Hardware:
  - MQ2 AO connected to GPIO 34.
  - GPIO 34 is ADC1 and input-only.
  - MQ2 heater powered from 5V.
  ==============================================================================
*/

MQ2::MQ2(int gpioPin) {
    pin = gpioPin;
    pinMode(pin, INPUT);
}

int MQ2::read() {
    return analogRead(pin);
}

float MQ2::readVoltage() {
    int value = analogRead(pin);
    return (value / 4095.0) * 3.3;
}