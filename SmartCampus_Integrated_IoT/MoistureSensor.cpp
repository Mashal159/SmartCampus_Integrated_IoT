#include "MoistureSensor.h"

/*
  ==============================================================================
  Soil Moisture Sensor Driver Implementation

  Calibration:
  - Common temporary values are used until the sensor is calibrated in real soil.
  - Dry soil / air: around 4095
  - Wet soil / water: around 1200
  ==============================================================================
*/

#define SOIL_ADC_DRY 4095
#define SOIL_ADC_WET 1200

SoilMoisture::SoilMoisture(int gpioPin) {
    pin = gpioPin;
    pinMode(pin, INPUT);
}

int SoilMoisture::read() {
    return analogRead(pin);
}

int SoilMoisture::readPercentage() {
    int raw = read();
    int percentage = map(raw, SOIL_ADC_DRY, SOIL_ADC_WET, 0, 100);

    return constrain(percentage, 0, 100);
}
