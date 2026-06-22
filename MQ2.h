#ifndef MQ2_H
#define MQ2_H

/*
  ==============================================================================
  Canonical Driver: MQ2 Gas Sensor

  Purpose:
  - Reads raw ADC value from MQ2 analog output.
  - Used by Smoke Detector module.
  ==============================================================================
*/

#include <Arduino.h>

class MQ2 {
public:
    int pin;

    MQ2(int gpioPin);
    int read();
    float readVoltage();
};

#endif