#ifndef SOIL_MOISTURE_H
#define SOIL_MOISTURE_H

/*
  ==============================================================================
  Canonical Driver: Soil Moisture Sensor

  Purpose:
  - Reads raw soil moisture ADC value.
  - Converts raw ADC value to percentage.
  - Used by Smart Irrigation module.
  ==============================================================================
*/

#include <Arduino.h>

class SoilMoisture {
public:
    int pin;

    SoilMoisture(int gpioPin);
    int read();
    int readPercentage();
};

#endif